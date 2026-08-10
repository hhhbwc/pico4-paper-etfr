#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Find the SHORTEST path to feed a frame into the algorithm.
# Look at CameraImageListener::onFrameAvailable + StandardAlgorithm::onFrameAvailable
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
    disasm(0x36c1c, 90, "CameraManager::CameraImageListener::onFrameAvailable (pvr_frame_item_t*)")
    disasm(0x4b7ac, 20, "StandardAlgorithm::onFrameAvailable (noop?)")
    disasm(0x53750, 20, "TobiiAlgorithm::onFrameAvailable")
    disasm(0x37408, 20, "CameraImageListener::onError")
    # AlgorithmBase::onFrameAvailable 0x3d71c
    disasm(0x3d71c, 30, "AlgorithmBase::onFrameAvailable")
print("done")
