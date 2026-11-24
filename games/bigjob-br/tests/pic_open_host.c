#define _CRT_SECURE_NO_WARNINGS
#include "cpu.h"
void pic_open_auto(CPU *);
static FILE *file;
static long base;
static unsigned length, position, next_selector, failed;
static uint8_t memory[65536 * 64];
static uint32_t selectors[65536];
uint16_t g_wsel;
void recomp_sel_write(uint16_t s, uint16_t o, uint16_t v) { (void)s; (void)o; (void)v; abort(); }

static unsigned arg(CPU *c, unsigned offset) { return mem_read16(c,c->ss,c->sp+4+offset); }
static void result(CPU *c, uint32_t value) { c->ax=(uint16_t)value; c->dx=(uint16_t)(value>>16); c->sp+=4; }
static int read_bytes(void *data, unsigned n)
{
    if (position > length || n > length-position || fread(data,1,n,file)!=n) {
        failed=1;
        return 0;
    }
    position+=n;
    return 1;
}
static uint16_t word(void)
{
    uint8_t b[2];
    return read_bytes(b,2) ? (uint16_t)((b[0]<<8)|b[1]) : 0;
}
static void allocate(CPU *c)
{
    if (next_selector >= 64) abort();
    unsigned s=next_selector++;
    memset(memory+s*65536u,0,65536);
    result(c,s<<16);
}
void seg030_0113(CPU *c) { allocate(c); }
void seg030_0163(CPU *c) { allocate(c); }
void seg030_025B(CPU *c) { result(c,0); }
void seg039_1BEC(CPU *c) { result(c,0); }
void seg039_10B0(CPU *c) { result(c,0); }
void seg039_1EA1(CPU *c)
{
    unsigned off=arg(c,0), seg=arg(c,2);
    mem_write32(c,(uint16_t)seg,(uint16_t)off, (2u<<16)|0x100);
    result(c,(seg<<16)|off);
}
void seg039_10EE(CPU *c)
{
    unsigned off=arg(c,4), seg=arg(c,6), n=arg(c,8);
    for (unsigned i=0;i<n;i++) mem_write16(c,(uint16_t)seg,(uint16_t)(off+2*i),word());
    result(c,0);
}
void seg039_116D(CPU *c)
{
    unsigned off=arg(c,4), seg=arg(c,6), n=arg(c,8);
    for (unsigned i=0;i<n;i++) {
        uint32_t high=word(), low=word();
        mem_write32(c,(uint16_t)seg,(uint16_t)(off+4*i),(high<<16)|low);
    }
    result(c,0);
}
void seg039_126D(CPU *c) { seg039_10EE(c); }
/* This fixture provides just the reader vtable used by this PIC constructor. */
void dispatch_far(CPU *c,uint16_t seg,uint16_t off)
{
    if(seg!=2) abort();
    if(off==0) {result(c,0); return;}
    if(off==4) {result(c,failed); return;}
    if(off==8) {
        unsigned p=arg(c,4), s=arg(c,6), n=arg(c,8);
        uint8_t tmp[32];
        if(n>sizeof tmp) abort();
        if(!read_bytes(tmp,n)) {result(c,0); return;}
        for(unsigned i=0;i<n;i++) mem_write8(c,(uint16_t)s,(uint16_t)(p+i),tmp[i]);
        result(c,n);
        return;
    }
    if(off==12) {
        unsigned p=arg(c,4)|(arg(c,6)<<16);
        if(arg(c,8)!=0 || p>length || fseek(file,base+(long)p,SEEK_SET)) failed=1;
        position=p;
        result(c,0);
        return;
    }
    abort();
}

int main(int argc,char **argv)
{
    if(argc!=4) return 2;
    file=fopen(argv[1],"rb");
    base=strtol(argv[2],NULL,10);
    length=strtoul(argv[3],NULL,10);
    if(!file || fseek(file,base,SEEK_SET)) return 2;
    next_selector=50;
    CPU c={0};
    c.mem=memory; c.mem_size=sizeof memory; c.sel_base=selectors;
    for(unsigned i=1;i<64;i++) selectors[i]=65536*i;
    c.cs=34; c.ss=1; c.ds=49; c.sp=0xff00;
    memcpy(memory+selectors[34]+0x634,"MMFW Pictures",14);
    for(unsigned i=0;i<16;i+=4) mem_write32(&c,2,(uint16_t)(0x100+i),(2u<<16)|i);
    mem_write32(&c,1,c.sp+12,(3u<<16)|0x200);
    pic_open_auto(&c);
    fclose(file);
    if(c.sp!=0xff04) return 3;
    if(!c.ax || failed) return 4;
    unsigned n=mem_read16(&c,3,0x206);
    uint32_t offsets=mem_read32(&c,3,0x208);
    printf("%u\n",n);
    for(unsigned i=0;i<=n;i++)
        printf("%u\n",mem_read32(&c,(uint16_t)(offsets>>16),(uint16_t)(offsets+4*i)));
    return 0;
}
