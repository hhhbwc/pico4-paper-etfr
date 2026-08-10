#!/usr/bin/env python3
from capstone import *
from capstone.arm64 import *
import struct
md=Cs(CS_ARCH_ARM64,CS_MODE_ARM)
words=[0x52800008,0x39000028,0xd65f03c0,0xd503201f]
raw=b''.join(struct.pack('<I',w) for w in words)
for ins in md.disasm(raw,0x10):
    print("0x{:08x}  {} {}".format(ins.address,ins.mnemonic,ins.op_str))
