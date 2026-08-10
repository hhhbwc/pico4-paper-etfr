#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Disassemble key StandardAlgorithm functions to reverse structures
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

    def disasm(addr, n=120, label=None):
        name = addrs.get(addr) or ''
        # crude demangle short
        short = name.split('E')[-1] if name else ''
        print(f"\n===== {label or name} @ 0x{addr:x} =====")
        off = addr-base
        code = data[off:off+n*4] if 0<=off<len(data) else b''
        for ins in md.disasm(code, addr):
            line = f"  0x{ins.address:08x}: {ins.mnemonic:8s} {ins.op_str}"
            print(line)
        print()

    disasm(0x462ec, 150, "StandardAlgorithm::generateFakeData")
