// ptrace_dlopen.c - inject a shared object into a target process by:
//  1) saving PC/regs
//  2) calling dlopen(path) via the target's own libdl (through a trampoline)
//  3) restoring
// Usage: ptrace_dlopen <pid> <path_to_so>
// NOTE: full remote-call trampoline is complex; this does attach + verify.
// For the real injection we'll use a simpler approach: write a small loader
// that the process executes via PTRACE_POKETEXT into a rwx-stub, OR use
// /proc/<pid>/mem + a thread hijack. Given complexity, we first implement
// ATTACH + read proof of libdl, then a minimal cc/gcc-based remote call.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <unistd.h>
#include <elf.h>
#include <link.h>

int main(int argc,char**argv){
    if(argc<3){fprintf(stderr,"usage: %s <pid> <so_path>\n",argv[0]);return 1;}
    pid_t pid=atoi(argv[1]); const char* path=argv[2];
    printf("attach %d...\n",pid);
    if(ptrace(PTRACE_ATTACH,pid,0,0)<0){perror("attach");return 1;}
    waitpid(pid,0,0);
    printf("attached. reading regs...\n");
    struct iovec io; unsigned long regs[18]; memset(regs,0,sizeof regs);
    io.iov_base=regs; io.iov_len=sizeof regs;
    if(ptrace(PTRACE_GETREGSET,pid,NT_PRSTATUS,&io)<0) perror("getregset");
    else printf("pc=0x%lx sp=0x%lx x0=0x%lx\n",regs[16],regs[13],regs[0]);
    printf("dlopen injection via PTRACE trampoline not yet implemented here.\n");
    printf("(phase1: attach/read verified)\n");
    ptrace(PTRACE_DETACH,pid,0,0);
    return 0;
}
