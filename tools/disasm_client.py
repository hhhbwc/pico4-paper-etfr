#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Dump libeyetrackingclient.pxr.so symbols + disasm GetEyeTrackingData paths + struct layout
from elftools.elf.elffile import ELFFile
from capstone import *
from capstone.arm64 import *

path = r"C:\Users\wzy\.openclaw\workspace\paper-etfr\svclib\libeyetrackingclient.pxr.so"
with open(path,'rb') as f:
    elf = ELFFile(f)
    text = elf.get_section_by_name('.text')
    base = text['sh_addr']; data = text.data()
    md = Cs(CS_ARCH_ARM64, CS_MODE_ARM)

    addrs={}
    for secn in ('.dynsym','.symtab'):
        sec = elf.get_section_by_name(secn)
        if not sec: continue
        for s in sec.iter_symbols():
            if s['st_value'] and s.name: addrs[s['st_value']]=s.name

    print("=== TrackingClient symbols ===")
    for a,n in sorted(addrs.items()):
        if 'TrackingClient' in n or 'EyeTracking' in n or 'pxr_eyepose' in n or 'GetEye' in n or 'algorithm' in n.lower():
            print(f"  0x{a:08x} sz={0} {n}")

    def disasm(addr, n, label):
        print(f"\n===== {label} @0x{addr:x} =====")
        off=addr-base; code=data[off:off+n*4] if 0<=off<len(data) else b''
        for ins in md.disasm(code,addr):
            print(f"  0x{ins.address:08x}: {ins.mnemonic:8s} {ins.op_str}")

    # GetEyeTrackingData family
    for a,n in sorted(addrs.items()):
        if n.endswith('GetEyeTrackingData') and 'TrackingClient' in n:
            print(f"\n### {n} @ 0x{a:x}")
            disasm(a, 90, n)
print("done")
