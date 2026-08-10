#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Disassemble Pxr_GetEyeTrackingData stub + trace where it goes
from elftools.elf.elffile import ELFFile
from capstone import *
from capstone.arm64 import *

path = r"C:\Users\wzy\.openclaw\workspace\paper-etfr\svclib\libopenxr_api.so"
with open(path, 'rb') as f:
    elf = ELFFile(f)
    text = elf.get_section_by_name('.text')
    base = text['sh_addr']
    data = text.data()
    md = Cs(CS_ARCH_ARM64, CS_MODE_ARM)
    md.detail = True

    # find symbol addresses
    addrs = {}
    dynsym = elf.get_section_by_name('.dynsym')
    for s in dynsym.iter_symbols():
        if s['st_value'] and s.name:
            addrs[s['st_value']] = s.name
    symtab = elf.get_section_by_name('.symtab')
    if symtab:
        for s in symtab.iter_symbols():
            if s['st_value'] and s.name:
                addrs[s['st_value']] = s.name

    def disasm_at(addr, n=40, label=None):
        off = addr - base
        print(f"\n===== {label or addrs.get(addr, hex(addr))} @ 0x{addr:x} =====")
        code = data[off:off+n*4] if 0 <= off < len(data) else b''
        for ins in md.disasm(code, addr):
            print(f"  0x{ins.address:08x}: {ins.mnemonic:10s} {ins.op_str}")
            # if branch, print target resolution
            if ins.mnemonic.startswith('b') and not ins.mnemonic.startswith('bl'):
                try:
                    tgt = int(ins.op_str, 16)
                    nm = addrs.get(tgt)
                    print(f"                -> target 0x{tgt:x} {('('+nm+')') if nm else ''}")
                except: pass

    # Pxr_GetEyeTrackingData stub
    disasm_at(0x21988, 12, "Pxr_GetEyeTrackingData")
    disasm_at(0x2310c, 12, "Pxr_GetEyeTrackingData1")
    # the real impl pointers
    disasm_at(0x23080, 8, "Pxr_WantEyeTrackingService")
    disasm_at(0x2309c, 8, "Pxr_GetEyeTrackingSupported")
print("done")
