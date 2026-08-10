#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Build + DISASSEMBLE combined foveation+gaze shellcode, verify encodings.
from capstone import *
from capstone.arm64 import *
import struct

def enc_movz(d, imm16, bit=64, shift=0):
    if bit==64:
        return 0xd2800000 | (d&31) | ((imm16&0xffff)<<5) | ((shift//16)<<21)
    else:
        return 0x52800000 | (d&31) | ((imm16&0xffff)<<5) | ((shift//16)<<21)
def enc_movk(d, imm16, bit=64, shift=0):
    if bit==64:
        return 0xf2800000 | (d&31) | ((imm16&0xffff)<<5) | ((shift//16)<<21)
    else:
        return 0x72800000 | (d&31) | ((imm16&0xffff)<<5) | ((shift//16)<<21)
def enc_strb_w(rt,rn,imm): return 0x39000000|(rt&31)|((rn&31)<<5)|((imm&0xfff)<<10)
def enc_str_w(rt,rn,imm): return 0xb9000000|(rt&31)|((rn&31)<<5)|(((imm//4)&0xfff)<<10)
def enc_ret(): return 0xd65f03c0
def fbits(f): return struct.unpack('<I',struct.pack('<f',f))[0]

def build(sf_addr):
    sc=[]
    sc.append(0xa9bf7bfd)               # stp x29,x30,[sp,#-16]!
    sc.append(0xd2800120)               # movz x0,#9
    lo=sf_addr&0xffff; mid=(sf_addr>>16)&0xffff; hi=(sf_addr>>32)&0xffff
    sc.append(enc_movz(19,lo,64,0))
    sc.append(enc_movk(19,mid,64,16))
    sc.append(enc_movk(19,hi,64,32))
    sc.append(0xd63f0260)               # blr x19
    sc.append(0xaa0303e9)               # mov x9,x3
    sc.append(0x5280002a)               # mov w10,#1
    sc.append(enc_strb_w(10,9,0x48))
    for f,off in [(0.5,0x50),(0.2,0x54),(0.0,0x58)]:
        b=fbits(f); hi16=(b>>16)&0xffff; lo16=b&0xffff
        sc.append(enc_movz(27,hi16,32,16))
        sc.append(enc_movk(27,lo16,32,0))
        sc.append(enc_str_w(27,9,off))
    sc.append(0x52800000)               # mov w0,#0
    sc.append(0xa8c17bfd)               # ldp x29,x30,[sp],#16
    sc.append(enc_ret())
    return sc

def main():
    sf=0x7cd26dba74  # Pxr_SetFoveationLevel runtime addr
    sc=build(sf)
    md=Cs(CS_ARCH_ARM64,CS_MODE_ARM)
    raw=b''.join(struct.pack('<I',w) for w in sc)
    print("=== shellcode (%d words) ==="%len(sc))
    for ins in md.disasm(raw,0x1000):
        print("  %08x  %s %s"%(ins.address,ins.mnemonic,ins.op_str))
    print("\n=== C array ===")
    print("{ "+", ".join("0x%08xU"%(w&0xffffffff) for w in sc)+" }")
    print("\nnwords=%d (even? %s)"%(len(sc), len(sc)%2==0))

if __name__=='__main__': main()
