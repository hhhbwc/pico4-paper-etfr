#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Build + DISASSEMBLE the gaze shellcode to verify encodings before deploying.
from capstone import *
from capstone.arm64 import *
import struct, sys

def enc_movz_w(d, imm16, shift=0):
    # MOVZ Wd, #imm16, LSL #shift  32-bit
    return 0x52800000 | (d & 31) | ((imm16 & 0xffff) << 5) | ((shift//16) << 21)
def enc_movk_w(d, imm16, shift=0):
    # MOVK Wd, #imm16, LSL #shift  32-bit
    return 0x72800000 | (d & 31) | ((imm16 & 0xffff) << 5) | ((shift//16) << 21)
def enc_mov_x(d, n):
    # MOV Xd, Xn
    return 0xaa0003e0 | (d & 31) | ((n & 31) << 16)
def enc_strb_w(rt, rn, imm):
    # STRB Wt, [Xn, #imm]  unsigned immediate, 0-4095
    return 0x39000000 | (rt & 31) | ((rn & 31) << 5) | ((imm & 0xfff) << 10)
def enc_str_w(rt, rn, imm):
    # STR Wt, [Xn, #imm]  0-16380 step4
    return 0xb9000000 | (rt & 31) | ((rn & 31) << 5) | (((imm//4) & 0xfff) << 10)
def enc_stp_x(pre, rt1, rt2, rn, imm, post=False):
    # 64-bit STP Xt1,Xt2,[Xn,#imm]!  pre-index base = 0xa9800000
    # post-index [Xn],#imm base = 0xa8800000
    base = 0xa9800000 if pre else 0xa8800000
    imm7 = (imm >> 3) & 0x7f
    return base | (rt2 & 31) | ((rt1 & 31) << 10) | (((imm7) & 0x7f) << 15) | ((rn & 31) << 5)
def enc_ldp_x(pre, rt1, rt2, rn, imm):
    # 64-bit LDP Xt1,Xt2,[Xn],#imm  post-index base = 0xa8c00000
    base = 0xa9c00000 if pre else 0xa8c00000
    imm7 = (imm >> 3) & 0x7f
    return base | (rt2 & 31) | ((rt1 & 31) << 10) | ((imm7 & 0x7f) << 15) | ((rn & 31) << 5)
def enc_ret():
    return 0xd65f03c0
def enc_movz_w0_imm(imm16):  # mov w0,#imm (upper) then movk low
    pass

def float_to_bits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]

def build(gx, gy, gz):
    code = []
    # stp x29, x30, [sp, #-16]!   (pre-index)
    code.append(enc_stp_x(True, 29, 30, 31, -16))
    # mov x9, x3   (out ptr)
    code.append(enc_mov_x(9, 3))
    # strb w10(=1), [x9, #0x48]  => validity
    code.append(0x5280002a)  # mov w10,#1
    code.append(enc_strb_w(10, 9, 0x48))
    # store 3 floats
    vals = [ (float_to_bits(gx),0x50), (float_to_bits(gy),0x54), (float_to_bits(gz),0x58) ]
    reg = 27
    for bits, off in vals:
        hi = (bits >> 16) & 0xffff; lo = bits & 0xffff
        code.append(enc_movz_w(reg, hi, 16))
        code.append(enc_movk_w(reg, lo, 0))
        code.append(enc_str_w(reg, 9, off))
    # mov w0,#0
    code.append(0x52800000)
    # ldp x29, x30, [sp], #16  (post-index)
    code.append(enc_ldp_x(False, 29, 30, 31, 16))
    code.append(enc_ret())
    return code

def main():
    gx,gy,gz = 0.2, 0.0, 0.0
    code = build(float(gx),float(gy),float(gz))
    print("=== shellcode (%d words) ===" % len(code))
    md = Cs(CS_ARCH_ARM64, CS_MODE_ARM)
    raw = b''.join(struct.pack('<I',w) for w in code)
    for ins in md.disasm(raw, 0x1000):
        print("  %08x  %s %s" % (ins.address, ins.mnemonic, ins.op_str))
    # also verify movz/movk decode with a known float (0.5 = 0x3f000000)
    print("\n=== verify float 0.5 (0x3f000000) movz/movk ===")
    c2 = build(0.5,0,0)
    raw2 = b''.join(struct.pack('<I',w) for w in c2)
    for ins in md.disasm(raw2,0x2000):
        print("  %08x  %s %s" % (ins.address, ins.mnemonic, ins.op_str))
    # output C array
    print("\n=== C array ===")
    print("{", ", ".join("0x%08xU"%(w&0xffffffff) for w in code), "}")

if __name__=='__main__':
    main()
