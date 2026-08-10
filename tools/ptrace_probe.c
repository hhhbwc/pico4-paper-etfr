/*
 * ptrace_probe.c - SAFE read-only ptrace attach to a target pid.
 * Attaches, reads registers once, detaches. NO writes, NO modification.
 * Usage: ptrace_probe <pid>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <unistd.h>
#include <elf.h>
#include <sys/uio.h>
#ifndef NT_PRSTATUS
#define NT_PRSTATUS 1
#endif

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr, "usage: %s <pid>\n", argv[0]); return 1; }
    pid_t pid = atoi(argv[1]);

    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) < 0) {
        fprintf(stderr, "attach failed pid=%d: %s\n", pid, strerror(errno));
        return 1;
    }
    waitpid(pid, NULL, 0);
    printf("attached to %d\n", pid);

    struct user_pt_regs regs;
    memset(&regs, 0, sizeof(regs));
    struct iovec io;
    io.iov_base = &regs;
    io.iov_len = sizeof(regs);
    if (ptrace(PTRACE_GETREGSET, pid, (void*)NT_PRSTATUS, &io) < 0) {
        perror("getregset");
        // fallback to GETREGS
        if (ptrace(PTRACE_GETREGS, pid, NULL, &regs) < 0) perror("getregs");
    }
    printf("pc=0x%llx sp=0x%llx\n", (unsigned long long)regs.pc, (unsigned long long)regs.sp);

    ptrace(PTRACE_DETACH, pid, NULL, NULL);
    printf("detached (no modification done)\n");
    return 0;
}
