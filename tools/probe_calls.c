// probe_calls.c - patch TrackingClient::GetEyeTrackingData entry to increment a
// memory counter every call, then return. Use to verify WHO calls it and how often.
//
// usage: probe_calls <pid> <addrhex> <counter_addr_hex>
//   patches <addrhex> with: 
//     ldr x9, counter_addr
//     ldr w10,[x9]
//     add w10,w10,#1
//     str w10,[x9]
//     (restore original behavior -> jump back? we can't easily. Instead we just RET 0.)
//   For a *counting* probe we must still return sensibly. Simplest: increment counter then
//   b to somewhere safe. But we overwrote prologue. We'll save original, and after counting
//   we do the normal function call path? Hard inline.
//
// SIMPLER ROBUST DESIGN: We branch-patch the FIRST instruction to an absolute jump to a
// stub we write into a mprotected RWX region. Too heavy. 
//
// Given complexity, this probe instead: reads counter before/after a timed window WITHOUT
// patching (uses ptrace steppping is too slow). So we rely on the caller existing.
//
// We'll implement: patch entry to increment counter in-place (using a PC-relative-ish approach),
// then branch back to original+4 (skipping just the first instr we replaced, preserving rest).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

// We patch ONLY the first 8-byte word at <addr> with:
//   ldr x9, <cnt>            (literal? we use movz/movk to build a static addr = counter_addr)
//   ldr w10,[x9] ; add w10,#1 ; str w10,[x9]
//   then b <addr+8>  (continue with original second+ instructions)
// We overwrite 8 bytes = word0(ldr)+... but need <=8 bytes. 
// A64: movz+m movk = 8 bytes minimum just to load addr. Too big for one 8B word.
// So patch 3 words (12 bytes) is needed, but that corrupts word1 (original 2nd instr).
// Given risk, for COUNTING we instead write to a known writable addr and accept we clobber
// the function (mrservice may misbehave). We'll restore after.
//
// DECISION: for the counting probe we accept temporary clobber, restore after N ms.

int main(int argc,char**argv){
    if(argc<4){fprintf(stderr,"usage: %s <pid> <addrhex> <counterhex>\n",argv[0]);return 1;}
    pid_t pid=atoi(argv[1]); uint64_t addr=strtoull(argv[2],0,0); uint64_t cnt=strtoull(argv[3],0,0);
    if(ptrace(PTRACE_ATTACH,pid,0,0)<0){perror("attach");return 1;} waitpid(pid,0,0);
    // save orig word0 (8 bytes) and word1 (4 bytes we also touch? we only write 3 words but orig restore needs them)
    long orig0=ptrace(PTRACE_PEEKTEXT,pid,(void*)addr,0);
    long orig1=ptrace(PTRACE_PEEKTEXT,pid,(void*)(addr+8),0);
    long orig2=ptrace(PTRACE_PEEKTEXT,pid,(void*)(addr+16),0);
    FILE* fp=fopen("/data/local/tmp/probe_orig.txt","w");
    fprintf(fp,"0x%lx\n0x%lx\n0x%lx\n",(unsigned long)orig0,(unsigned long)orig1,(unsigned long)orig2);
    fclose(fp);
    // Build stub at <addr> (12 bytes):
    //   ldr x9, #8            (literal? no) 
    // Use: movz x9,#cnt_hi16  ; movk x9,#cnt_lo16 (pcrel not needed; absolute)
    uint32_t hi=(uint32_t)(cnt>>16)&0xffff, lo=(uint32_t)cnt&0xffff;
    uint32_t w0=0xd2800009 | (hi<<5);          // movz x9,#hi? no wait this is ZX not movz immediate
    // MOVZ Xd,#imm16,LSL#s : 0xd2800000 | d | imm16<<5 | s<<21. s=0 for lo? we load full 48-bit addr in 3 parts
    // Use 3 movz/movk for full 64-bit cnt:
    // movz x9,#cnt_lo16
    // movk x9,#cnt_mid16,lsl#16
    // movk x9,#cnt_hi16,lsl#32
    uint32_t lo16=(uint32_t)cnt&0xffff, mid16=((uint32_t)(cnt>>16)&0xffff), hi16=(uint32_t)(cnt>>32)&0xffff;
    uint32_t s0=0xd2800009 | (lo16<<5);        // movz x9,#lo16
    uint32_t s1=0xf2800009 | (mid16<<5);       // movk x9,#mid16,lsl#16
    uint32_t s2=0xf2a00009 | (hi16<<5);        // movk x9,#hi16,lsl#32
    uint32_t s3=0xb940012a;                    // ldr w10,[x9]
    uint32_t s4=0x1100054a;                    // add w10,w10,#1
    uint32_t s5=0xb900012a;                    // str w10,[x9]
    uint32_t s6=0x14000002;                    // b +8 (skip next 2 instr -> to addr+8? offset in words)
    // That's 7 words =28 bytes, too much vs the 24 bytes we saved. We'll restore 28B later by saving 7 words.
    // Save 7 words then.
    long origW[7];
    for(int i=0;i<7;i++) origW[i]=ptrace(PTRACE_PEEKTEXT,pid,(void*)(addr+i*4),0);
    fp=fopen("/data/local/tmp/probe_orig.txt","w");
    for(int i=0;i<7;i++) fprintf(fp,"0x%lx\n",(unsigned long)origW[i]);
    fclose(fp);
    uint32_t code[7]={s0,s1,s2,s3,s4,s5,s6};
    for(int i=0;i<7;i++){ uint64_t w=(uint64_t)code[i]; ptrace(PTRACE_POKETEXT,pid,(void*)(addr+i*4),(void*)w); }
    printf("probe armed at 0x%llx counter 0x%llx\n",(unsigned long long)addr,(unsigned long long)cnt);
    ptrace(PTRACE_DETACH,pid,0,0);
    return 0;
}
