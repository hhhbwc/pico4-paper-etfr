#!/usr/bin/env python3
# Simple test: does the ELF in svclib have a PT_INTERP/notes we can mine for toolchain?
import subprocess, os, sys
from elftools.elf.elffile import ELFFile

# check for wsl2
print("=== check WSL2 ===")
try:
    r = subprocess.run(['wsl','--status'], capture_output=True, text=True, timeout=10)
    print("WSL:", r.stdout.strip()[:200] if r.stdout else r.stderr.strip()[:200])
except Exception as e:
    print("no wsl command:", e)

# check for git bash / msys which may bundle gcc? no.
# check docker
try:
    r = subprocess.run(['docker','--version'], capture_output=True, text=True, timeout=5)
    print("docker:", r.stdout.strip() or r.stderr.strip())
except Exception as e:
    print("no docker")

print("done")
