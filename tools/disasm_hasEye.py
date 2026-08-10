#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Disasm TrackingService::hasEyeCamera to design a clean "return true" hook
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
    # hasEyeCamera 0x2ecdc sz 308
    disasm(0x2ecdc, 77, "TrackingService::hasEyeCamera")
    # also hasEyeCamera in the service interface / Bp (client proxy) 0x57e20
    disasm(0x57e20, 40, "BpEyeTrackingService::hasEyeCamera")
print("done")
