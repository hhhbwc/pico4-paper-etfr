#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Disasm CameraManager image-input entry points + frame struct to design image injection
from elftools.elf.elffile import ELFFile
from capstone import *
from capstone.arm64 import *

path = r"C:\Users\wzy\.openclaw\workspace\paper-etfr\svclib\libpxreyetrackingservice.so"
with open(path,'rb') as f:
    elf = ELFFile(f)
    text = elf.get_section_by_name('.text')
    base = text['sh_addr']; data = text.data()
    md = Cs(CS_ARCH_ARM64, CS_MODE_ARM)

    def disasm(addr, n, label):
        print(f"\n===== {label} @0x{addr:x} =====")
        off=addr-base; code=data[off:off+n*4] if 0<=off<len(data) else b''
        for ins in md.disasm(code,addr):
            print(f"  0x{ins.address:08x}: {ins.mnemonic:8s} {ins.op_str}")

    # CameraManager::addImageListener (image entry) 0x3492c sz264
    disasm(0x3492c, 66, "CameraManager::addImageListener")
    # CameraManager::openCamera 0x34b18 sz1624
    disasm(0x34b18, 80, "CameraManager::openCamera (top)")
    # TrackingService::onFrameAvailable 0x2efc8 sz200
    disasm(0x2efc8, 50, "TrackingService::onFrameAvailable")
print("done")
