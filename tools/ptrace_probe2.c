// ptrace_probe2.c - minimal attach/detach test, no regs
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    if (argc < 2) return 1;
    pid_t pid = atoi(argv[1]);
    fprintf(stderr, "attach %d...\n", pid);
    if (ptrace(PTRACE_ATTACH, pid, 0, 0) < 0) { perror("attach"); return 1; }
    int st;
    waitpid(pid, &st, 0);
    fprintf(stderr, "attached, wait done\n");
    // just read a word via peektext to confirm memory access
    long v = ptrace(PTRACE_PEEKTEXT, pid, (void*)(argc>2?strtoul(argv[2],0,0):0x400000), 0);
    fprintf(stderr, "peektext @0x%lx => 0x%lx (errno=%d %s)\n", (unsigned long)(argc>2?strtoul(argv[2],0,0):0x400000), (unsigned long)v, errno, strerror(errno));
    ptrace(PTRACE_DETACH, pid, 0, 0);
    fprintf(stderr, "detached ok\n");
    return 0;
}
