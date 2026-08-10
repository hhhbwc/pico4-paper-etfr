#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Full disasm of TobiiAlgorithm::onFrameAvailable + subscribe/start to see if external frames can activate it
from elftools.elf.elffile import ELFFile
from capstone import *
from capstone.arm64 import *

path = r"C:\Users\wzy\.openclaw\workspace\paper-etfr\svclib\libpxreyetrackingservice.so"
with open(path,'rb') as f:
    elf = ELFFile(f)
    text = elf.get_section_by_name('.text')
    base = text['sh_addr']; data = text.data()
    md = Cs(CS_ARCH_ARM64, CS_MODE_ARM)
    def disasm(addr,n,label):
        print(f"===== {label} @0x{addr:x} =====")
        off=addr-base; code=data[off:off+n*4] if 0<=off<len(data) else b''
        for ins in md.disasm(code,addr):
            print(f"  0x{ins.address:08x}: {ins.mnemonic:8s} {ins.op_str}")
    # TobiiAlgorithm::onFrameAvailable 0x53750 (sz 124)
    disasm(0x53750, 31, "TobiiAlgorithm::onFrameAvailable (full)")
    # find what reads [x0+0x2f70] - the start flag. search entire .text for ldurb/ldrb [x,#0x2f70]
    print("\n=== who reads/writes [x+0x2f70] (Tobii start flag) ===")
    import struct
    for i in range(0,len(data)-4,4):
        insn=struct.unpack('<I',data[i:i+4])[0]
        # ldrb wX,[xY,#imm] : 0x39400000 | Rt | Rn<<5 | imm<<10 ; ldrb 0x39400000, imm 0-4095
        if (insn & 0xFFC00000)==0x39400000:
            imm=(insn>>10)&0xFFF
            if imm==0x2f70:
                print(f"  ldrb [..#0x2f70] at .text+0x{i:x}")
        # strb 0x39000000
        if (insn & 0xFFC00000)==0x39000000:
            imm=(insn>>10)&0xFFF
            if imm==0x2f70:
                print(f"  strb [..#0x2f70] at .text+0x{i:x}")
        # ldrh 0x79400000
        if (insn & 0xFFC00000)==0x79400000:
            imm=(insn>>10)&0xFFF
            if imm==0x2f70:
                print(f"  ldrh [..#0x2f70] at .text+0x{i:x}")
print("done")
