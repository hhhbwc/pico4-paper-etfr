// ptrace_rw.c - read/write remote process memory via ptrace (PEEK/POKETEXT)
// usage:
//   ptrace_rw <pid> read <addr_hex>        -> read 8 bytes
//   ptrace_rw <pid> readn <addr_hex> <bytes> -> read N bytes
//   ptrace_rw <pid> write <addr_hex> <hexbytes>  -> write bytes (hex string)
// Requires root (system processes). Attach, operate, detach.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdint.h>

// Android bionic may not expose process_vm_*; use pure ptrace PEEKTEXT/POKETEXT (8-byte words)

static int hexval(char c){ if(c>='0'&&c<='9')return c-'0'; if(c>='a'&&c<='f')return c-'a'+10; if(c>='A'&&c<='F')return c-'A'+10; return -1; }

int main(int argc, char** argv){
    if(argc<4){ fprintf(stderr,"usage: %s <pid> read|readn|write <addr_hex> [bytes|hexbytes]\n",argv[0]); return 1; }
    pid_t pid=atoi(argv[1]);
    char* op=argv[2];
    uint64_t addr=strtoull(argv[3],0,0);

    // attach
    if(ptrace(PTRACE_ATTACH,pid,NULL,NULL)<0){ perror("attach"); return 1; }
    waitpid(pid,NULL,0);

    int rc=0;
    if(strcmp(op,"read")==0){
        long v=ptrace(PTRACE_PEEKTEXT,pid,(void*)addr,NULL);
        if(v==-1&&errno) perror("peektext");
        else printf("read @0x%llx = 0x%016lx\n",(unsigned long long)addr,(unsigned long)v);
    } else if(strcmp(op,"readn")==0){
        size_t n=atoi(argv[4]); fprintf(stderr,"readn %zu bytes @0x%llx (word-by-word via ptrace)\n",n,(unsigned long long)addr);
        size_t words=(n+7)/8; unsigned char buf[4096]; memset(buf,0,sizeof buf);
        size_t got=0;
        for(size_t i=0;i<words;i++){
            long v=ptrace(PTRACE_PEEKTEXT,pid,(void*)(addr+i*8),NULL);
            if(v==-1&&errno){ perror("peektext"); break; }
            memcpy(buf+i*8,&v,8); got+=8;
        }
        if(got>n)got=n;
        printf("read %zu bytes @0x%llx:\n",got,(unsigned long long)addr);
        for(size_t i=0;i<got;i++){ printf("%02x%s",buf[i],(i%16==15)?"\n":" ");} printf("\n");
    } else if(strcmp(op,"write")==0){
        // parse hex
        char* h=argv[4]; size_t n=strlen(h)/2; unsigned char* b=malloc(n?n:1);
        for(size_t i=0;i<n;i++){ b[i]=(unsigned char)((hexval(h[i*2])<<4)|hexval(h[i*2+1])); }
        // POKETEXT word-by-word (must be 8-byte aligned address, works for any)
        size_t written=0;
        for(size_t i=0;i+8<=n;i+=8){ uint64_t w; memcpy(&w,b+i,8);
            errno=0;
            if(ptrace(PTRACE_POKETEXT,pid,(void*)(addr+i),(void*)w)<0 && errno){ perror("poketext"); break; }
            written+=8;
        }
        printf("write done: %zu/%zu bytes @0x%llx (unaligned tail %zu ignored)\n",written,n,(unsigned long long)addr,n%8);
        free(b);
    } else { fprintf(stderr,"bad op\n"); rc=1; }

    ptrace(PTRACE_DETACH,pid,NULL,NULL);
    return rc;
}
