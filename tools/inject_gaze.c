// inject_gaze.c - patch TrackingClient::GetEyeTrackingData in target process
// to fill a fixed gaze (0.2,0,0) and return success. Used only for live ETFR-linkage verification.
//
// usage:
//   inject_gaze <pid> apply   <addr_hex>            patch entry with gaze shellcode (0.2,0,0)
//   inject_gaze <pid> restore <addr_hex>            restore original first 16 words
//
// The patch (16 words) fills out[0x48]=1(valid), out[0x50/0x54/0x58]=gaze xyz, returns 0.
// Safe: uses its own 16-byte frame for lr/fp. Original instructions saved to /data/local/tmp/gaze_orig.txt
// so we can restore exactly.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

static const uint32_t SHELLCODE[16] = {
    0xa9bf77feU, // stp x30,x29,[sp,#-16]!
    0xaa0303e9U, // mov x9,x3
    0x5280002aU, // mov w10,#1
    0x3901212aU, // strb w10,[x9,#0x48]
    0x52a7c99bU, // mov w27,#0x3e4c0000
    0x729999bbU, // movk w27,#0xcccd   (0.2f=0x3E4CCCCD)
    0xb900513bU, // str w27,[x9,#0x50]
    0x52a0001bU, // movz w27,#0,lsl#16
    0x7280001bU, // movk w27,#0
    0xb900553bU, // str w27,[x9,#0x54]
    0x52a0001bU, // movz w27,#0,lsl#16
    0x7280001bU, // movk w27,#0
    0xb900593bU, // str w27,[x9,#0x58]
    0x52800000U, // mov w0,#0
    0xa8c177feU, // ldp x30,x29,[sp],#16
    0xd65f03c0U  // ret
};

static int attach(pid_t pid){ if(ptrace(PTRACE_ATTACH,pid,0,0)<0){perror("attach");return-1;} waitpid(pid,0,0); return 0; }
static long peek(pid_t pid, uint64_t a){ errno=0; return ptrace(PTRACE_PEEKTEXT,pid,(void*)a,0); }
static int poke(pid_t pid, uint64_t a, uint64_t w){ errno=0; if(ptrace(PTRACE_POKETEXT,pid,(void*)a,(void*)w)<0&&errno){ perror("poke"); return -1; } return 0; }

int main(int argc,char** argv){
    if(argc<4){ fprintf(stderr,"usage: %s <pid> <apply|restore> <addrhex>\n",argv[0]); return 1; }
    pid_t pid=atoi(argv[1]); char* op=argv[2]; uint64_t addr=strtoull(argv[3],0,0);
    if(attach(pid)<0) return 1;

    // PTRACE_POKETEXT writes 8-byte words on arm64; write pairs of shellcode words at 8B-aligned offsets.
    // addr must be 8-byte aligned (0x...0584 is).
    if(strcmp(op,"apply")==0){
        // save original 8-byte words (8 words = 16 instructions)
        FILE* fp=fopen("/data/local/tmp/gaze_orig.txt","w");
        if(fp){ for(int i=0;i<8;i++){ long v=peek(pid,addr+i*8); fprintf(fp,"0x%016lx\n",(unsigned long)v); } fclose(fp);
               printf("saved %d orig 8-byte words\n",8); }
        // patch pairs
        int ok=0;
        for(int i=0;i<8;i++){ uint64_t w=(uint64_t)SHELLCODE[i*2] | ((uint64_t)SHELLCODE[i*2+1]<<32);
            if(poke(pid,addr+i*8,w)<0){ fprintf(stderr,"patch fail 8B-word %d\n",i); break; } ok++; }
        printf("PATCHED %d/8 8-byte words @0x%llx\n",ok,(unsigned long long)addr);
        // verify
        long v=peek(pid,addr); printf("verify first 8B=0x%016lx (expect bf77fea9... no: a9bf77fe|aa0303e9 -> 0xaa0303e9a9bf77fe)\n",(unsigned long)v);
    } else if(strcmp(op,"restore")==0){
        FILE* fp=fopen("/data/local/tmp/gaze_orig.txt","r");
        if(!fp){ fprintf(stderr,"no orig file\n"); ptrace(PTRACE_DETACH,pid,0,0); return 1; }
        char line[32]; int i=0;
        while(fgets(line,sizeof line,fp)&&i<8){ uint64_t v=strtoull(line,0,0); poke(pid,addr+i*8,v); i++; }
        fclose(fp); printf("RESTORED %d 8-byte words\n",i);
        long v=peek(pid,addr); printf("verify first 8B=0x%016lx\n",(unsigned long)v);
    } else { fprintf(stderr,"bad op\n"); }

    ptrace(PTRACE_DETACH,pid,0,0);
    return 0;
}
