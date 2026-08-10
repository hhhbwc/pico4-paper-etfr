#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Find callers of generateFakeData + who sets the 0x4928 flag + structure of getFake
from elftools.elf.elffile import ELFFile
from capstone import *
from capstone.arm64 import *

path = r"C:\Users\wzy\.openclaw\workspace\paper-etfr\svclib\libpxreyetrackingservice.so"
with open(path,'rb') as f:
    elf = ELFFile(f)
    text = elf.get_section_by_name('.text')
    base = text['sh_addr']; data = text.data()
    md = Cs(CS_ARCH_ARM64, CS_MODE_ARM); md.detail=True

    addrs={}
    for secn in ('.dynsym','.symtab'):
        sec = elf.get_section_by_name(secn)
        if not sec: continue
        for s in sec.iter_symbols():
            if s['st_value'] and s.name: addrs[s['st_value']]=s.name

    print("=== callers of generateFakeData (0x462ec) ===")
    # scan whole .text for bl 0x462ec  (or bl offset)
    import struct
    target_off = 0x462ec - base
    for i in range(0, len(data)-4, 4):
        insn = struct.unpack('<I', data[i:i+4])[0]
        # BL immediate: cond=0x14, imm26
        if (insn & 0xFC000000) == 0x94000000:
            imm = insn & 0x03FFFFFF
            if imm & 0x2000000: imm -= 0x4000000  # sign extend
            dest = base + i + (imm<<2)
            if dest == 0x462ec:
                print(f"  caller at .text+0x{i:x} (0x{base+i:x})  bl generateFakeData")

    print("\n=== who references offset 0x4928 (fake flag) ===")
    # look for add x?, x?, #0x4928 near ldrb/strb
    for i in range(0, len(data)-4, 4):
        insn = struct.unpack('<I', data[i:i+4])[0]
        # add xD, xN, #imm: 0x91000000 base
        if (insn & 0xFF000000) == 0x91000000:
            imm12 = (insn >> 10) & 0xFFF
            if imm12 == 0x4928:
                print(f"  .text+0x{i:x}: add to #0x4928")
        # ldrb/strb with this offset base handled via add above; also check strb w, [x,#imm]
        if (insn & 0xFFC00000) == 0x39000000:  # strb imm
            imm12 = (insn >> 12) & 0xFFF
            if imm12 == 0x4928:
                print(f"  .text+0x{i:x}: strb offset 0x4928")

    print("\n=== getData / onFoveatedGazeAvailable disasm (structure consumers) ===")
    def disasm(addr, n=60, label=None):
        print(f"\n-- {label or addrs.get(addr,'?')} @0x{addr:x} --")
        off=addr-base; code=data[off:off+n*4] if 0<=off<len(data) else b''
        for ins in md.disasm(code,addr):
            print(f"  0x{ins.address:08x}: {ins.mnemonic:8s} {ins.op_str}")
    disasm(0x4af84, 90, "StandardAlgorithm::onFoveatedGazeAvailable")
print("done")
