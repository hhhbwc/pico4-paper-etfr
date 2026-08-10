// patch_func.c - generic single-function patch with apply/restore.
// Patch payload: "mov w8,#1; strb w8,[x1]; ret" (force hasEyeCamera=true, x1=out ptr)
//   w8=1: 0x52800008
//   strb w8,[x1]: 0x39000028
//   ret: 0xd65f03c0
// usage: patch_func apply <pid> <addr_hex>
//        patch_func restore <pid> <addr_hex>
// Saves 3 words orig.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

static const uint32_t PATCH[3] = {0x52800028, 0x39000028, 0xd65f03c0}; // mov w8,#1; strb w8,[x1]; ret
static const char* ORIG="/data/local/tmp/patch_orig.txt";
static int atom_ok=0;

static int attach(pid_t pid){ if(ptrace(PTRACE_ATTACH,pid,0,0)<0){perror("attach");return-1;} waitpid(pid,0,0); return 0; }
static long peek(pid_t pid,uint64_t a){ errno=0; return ptrace(PTRACE_PEEKTEXT,pid,(void*)a,0); }
static int poke64(pid_t pid,uint64_t a,uint64_t w){ errno=0; if(ptrace(PTRACE_POKETEXT,pid,(void*)a,(void*)w)<0&&errno){perror("poke");return-1;} return 0; }

int main(int argc,char**argv){
    if(argc<3){fprintf(stderr,"usage: %s <apply|restore> <pid> <addr_hex>\n",argv[0]);return 1;}
    char* op=argv[1]; pid_t pid=atoi(argv[2]); uint64_t a=strtoull(argv[3],0,0);
    if(attach(pid)<0)return 1;
    if(strcmp(op,"apply")==0){
        // save 3 8-byte words (covers 6 instructions; we patch 3 words = 12 bytes but write in 8B pairs)
        // We need PATCH to fit. 3 instr = 12 bytes. Write as 8B word0 (instr0|instr1<<32) + partial word1 (instr2 + pad NOP).
        // Save 2 8-byte words (16 bytes) to restore cleanly.
        FILE* fp=fopen(ORIG,"w");
        if(fp){ for(int i=0;i<2;i++){ long v=peek(pid,a+i*8); fprintf(fp,"0x%016lx\n",(unsigned long)v);} fclose(fp); }
        uint64_t w0=(uint64_t)PATCH[0]|((uint64_t)PATCH[1]<<32);
        uint64_t w1=(uint64_t)PATCH[2]|((uint64_t)0xd503201f<<32); // ret + nop
        poke64(pid,a,w0); poke64(pid,a+8,w1);
        printf("PATCHED hasEyeCamera=true @0x%llx\n",(unsigned long long)a);
        printf("verify 0x%016lx\n",(unsigned long)peek(pid,a));
    } else if(strcmp(op,"restore")==0){
        FILE* fp=fopen(ORIG,"r"); if(!fp){fprintf(stderr,"no orig\n");ptrace(PTRACE_DETACH,pid,0,0);return 1;}
        char line[32]; int i=0;
        while(fgets(line,sizeof line,fp)){ uint64_t v=strtoull(line,0,0); poke64(pid,a+i*8,v); i++; }
        fclose(fp); printf("RESTORED %d words\n",i);
        printf("verify 0x%016lx\n",(unsigned long)peek(pid,a));
    } else fprintf(stderr,"bad op\n");
    ptrace(PTRACE_DETACH,pid,0,0);
    return 0;
}
