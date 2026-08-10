// inject_foveation.c - patch TrackingClient::GetEyeTrackingData (openxr_runtime)
// to call Pxr_SetFoveationLevel(9) + fill gaze, return 0.
// usage:
//   inject_foveation apply <pid> <geteye_addr_hex> <setfovea_addr_hex>
//   inject_foveation restore <pid> <geteye_addr_hex>
// Saves original as N 8-byte words; restore copies back exactly.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

#define NOP 0xd503201fU
#define MAXW 32
static const char* ORIG="/data/local/tmp/gaze_orig_fov.txt";
static uint32_t SC[MAXW]; static int NW=0;

static int attach(pid_t pid){ if(ptrace(PTRACE_ATTACH,pid,0,0)<0){perror("attach");return-1;} waitpid(pid,0,0); return 0; }
static long peek(pid_t pid,uint64_t a){ errno=0; return ptrace(PTRACE_PEEKTEXT,pid,(void*)a,0); }
static int poke64(pid_t pid,uint64_t a,uint64_t w){ errno=0; if(ptrace(PTRACE_POKETEXT,pid,(void*)a,(void*)w)<0&&errno){perror("poke");return-1;} return 0; }

static void build(uint64_t sf){
    int c=0;
    SC[c++]=0xa9bf7bfdU; SC[c++]=0xd2800120U;
    uint32_t lo=(uint32_t)sf&0xffff, mid=((uint32_t)(sf>>16))&0xffff, hi=(uint32_t)(sf>>32)&0xffff;
    SC[c++]=0xd2800013U|(lo<<5); SC[c++]=0xf2800013U|(mid<<5); SC[c++]=0xf2a00013U|(hi<<5);
    SC[c++]=0xd63f0260U;
    SC[c++]=0xaa0303e9U; SC[c++]=0x5280002aU; SC[c++]=0x3901212aU;
    SC[c++]=0x52a7e01bU; SC[c++]=0x7280001bU; SC[c++]=0xb900513bU;
    SC[c++]=0x52a7c99bU; SC[c++]=0x729999bbU; SC[c++]=0xb900553bU;
    SC[c++]=0x52a0001bU; SC[c++]=0x7280001bU; SC[c++]=0xb900593bU;
    SC[c++]=0x52800000U; SC[c++]=0xa8c17bfdU; SC[c++]=0xd65f03c0U;
    if(c%2) SC[c++]=NOP;
    NW=c;
}

int main(int argc,char**argv){
    if(argc<3){fprintf(stderr,"usage: %s <apply|restore> <pid> <addr_hex> [setfovea_hex]\n",argv[0]);return 1;}
    char* op=argv[1]; pid_t pid=atoi(argv[2]); uint64_t g=strtoull(argv[3],0,0);
    if(attach(pid)<0)return 1;
    if(strcmp(op,"apply")==0){
        if(argc<5){fprintf(stderr,"need setfovea\n");ptrace(PTRACE_DETACH,pid,0,0);return 1;}
        build(strtoull(argv[4],0,0));
        FILE* fp=fopen(ORIG,"w");
        if(fp){for(int i=0;i<NW/2;i++){long v=peek(pid,g+i*8);fprintf(fp,"0x%016lx\n",(unsigned long)v);}fclose(fp);}
        for(int i=0;i<NW;i+=2){ uint64_t w=(uint64_t)(SC[i]&0xffffffffu)|((uint64_t)(SC[i+1]&0xffffffffu)<<32); poke64(pid,g+(i/2)*8,w); }
        printf("apply %d instr @0x%llx\n",NW,(unsigned long long)g);
        printf("verify word0=0x%016lx\n",(unsigned long)peek(pid,g));
    } else if(strcmp(op,"restore")==0){
        FILE* fp=fopen(ORIG,"r"); if(!fp){fprintf(stderr,"no orig\n");ptrace(PTRACE_DETACH,pid,0,0);return 1;}
        char line[32]; int i=0;
        while(fgets(line,sizeof line,fp)){ uint64_t v=strtoull(line,0,0); poke64(pid,g+i*8,v); i++; }
        fclose(fp); printf("restored %d 8-byte words\n",i);
        printf("verify word0=0x%016lx\n",(unsigned long)peek(pid,g));
    } else fprintf(stderr,"bad op\n");
    ptrace(PTRACE_DETACH,pid,0,0);
    return 0;
}
