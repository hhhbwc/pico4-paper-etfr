// patch_ret1.c - patch a void foo(T* out) function to always set *out=1 then return.
// Builds payload: mov w8,#1 ; strb w8,[<Rn>] ; ret   OR   mov w8,#1 ; str w8,[<Rn>] ; ret
// usage: patch_ret1 <pid> <addr_hex> <rn> <strb|str>   rn=X0..X30 (0..30), strb=1-byte, str=4-byte
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

static const char* ORIG="/data/local/tmp/patch_ret1_orig.txt";
static int attach(pid_t pid){ if(ptrace(PTRACE_ATTACH,pid,0,0)<0){perror("attach");return-1;} waitpid(pid,0,0); return 0; }
static long peek(pid_t pid,uint64_t a){ errno=0; return ptrace(PTRACE_PEEKTEXT,pid,(void*)a,0); }
static int poke64(pid_t pid,uint64_t a,uint64_t w){ errno=0; if(ptrace(PTRACE_POKETEXT,pid,(void*)a,(void*)w)<0&&errno){perror("poke");return-1;} return 0; }

int main(int argc,char**argv){
    if(argc<4){fprintf(stderr,"usage: %s <apply|restore> <pid> <addr_hex> [rn(0-30)] [strb|str]\n",argv[0]);return 1;}
    char* op=argv[1]; pid_t pid=atoi(argv[2]); uint64_t a=strtoull(argv[3],0,0);
    if(attach(pid)<0)return 1;
    if(strcmp(op,"restore")==0){
        FILE* fp=fopen(ORIG,"r"); if(!fp){fprintf(stderr,"no orig\n");ptrace(PTRACE_DETACH,pid,0,0);return 1;}
        char line[32]; int i=0;
        while(fgets(line,sizeof line,fp)){ uint64_t v=strtoull(line,0,0); poke64(pid,a+i*8,v); i++; }
        fclose(fp); printf("RESTORED %d words\nverify 0x%016lx\n",i,(unsigned long)peek(pid,a));
        ptrace(PTRACE_DETACH,pid,0,0); return 0;
    }
    if(argc<6){fprintf(stderr,"need rn + strb|str\n");ptrace(PTRACE_DETACH,pid,0,0);return 1;}
    int rn=atoi(argv[4]); int use_str = (strcmp(argv[5],"str")==0);
    // payload: mov w8,#1 (52800028); strb w8,[xn] (39000000|8|rn<<5) or str w8,[xn] (b9000000|8|rn<<5); ret
    uint32_t w0 = 0x52800028;                      // mov w8,#1
    uint32_t w1 = (use_str?0xb9000000:0x39000000) | (8) | ((rn&31)<<5);  // strb/str w8,[xrn]
    uint32_t w2 = 0xd65f03c0;                      // ret
    uint32_t w3 = 0xd503201f;                      // nop
    FILE* fp=fopen(ORIG,"w");
    if(fp){for(int i=0;i<2;i++){long v=peek(pid,a+i*8);fprintf(fp,"0x%016lx\n",(unsigned long)v);}fclose(fp);}
    uint64_t v0=(uint64_t)w0|((uint64_t)w1<<32);
    uint64_t v1=(uint64_t)w2|((uint64_t)w3<<32);
    poke64(pid,a,v0); poke64(pid,a+8,v1);
    printf("PATCHED *[x%d]=1 @0x%llx (%s)\n",rn,(unsigned long long)a,use_str?"str":"strb");
    printf("verify 0x%016lx\n",(unsigned long)peek(pid,a));
    ptrace(PTRACE_DETACH,pid,0,0);
    return 0;
}
