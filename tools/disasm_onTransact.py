#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Find hasEyeCamera's binder transaction code from BnEyeTrackingService::onTransact
from elftools.elf.elffile import ELFFile
from capstone import *
from capstone.arm64 import *

path = r"C:\Users\wzy\.openclaw\workspace\paper-etfr\svclib\libpxreyetrackingservice.so"
with open(path,'rb') as f:
    elf = ELFFile(f)
    text = elf.get_section_by_name('.text')
    base = text['sh_addr']; data = text.data()
    md = Cs(CS_ARCH_ARM64, CS_MODE_ARM)
    addrs={}
    for secn in ('.dynsym','.symtab'):
        sec=elf.get_section_by_name(secn)
        if not sec: continue
        for s in sec.iter_symbols():
            if s['st_value'] and s.name: addrs[s['st_value']]=s.name
    def disasm(addr,n,label):
        print(f"===== {label} @0x{addr:x} =====")
        off=addr-base; code=data[off:off+n*4] if 0<=off<len(data) else b''
        for ins in md.disasm(code,addr):
            print(f"  0x{ins.address:08x}: {ins.mnemonic:8s} {ins.op_str}")
    # BnEyeTrackingService::onTransact 0x58418 sz3516 - scan for code comparisons near calls to hasEyeCamera(0x2ecdc)
    # We'll disassemble big chunk and look for the hasEyeCamera handler: typically cmp w2,#N ; ...; bl hasEyeCamera
    disasm(0x58418, 300, "BnEyeTrackingService::onTransact")
print("done")
