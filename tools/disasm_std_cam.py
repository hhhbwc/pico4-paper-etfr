#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Deep-dive StandardAlgorithm image->gaze pipeline to see if it CONSUMES images.
# Look at openCameraList/openCamera, notifyFrameAvailable, watchCameraLoop, onAlgoResultAvailable
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

    # StandardAlgorithm::openCamera 0x45a40 (1116) + openCameraList 0x45e88
    disasm(0x45a40, 80, "StandardAlgorithm::openCamera (no-cam-list)")
    disasm(0x45e88, 60, "StandardAlgorithm::openCameraList")
print("done")
