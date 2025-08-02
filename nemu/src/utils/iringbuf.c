#include <common.h>


void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
typedef struct {
    uint64_t pc;
    uint32_t inst;
}ringbuf;
ringbuf iringbuf[20] = {0};
int current = 0;
void trace_buf(uint64_t pc,uint32_t inst)
{
    iringbuf[current].inst = inst;
    iringbuf[current].pc = pc;
    current = (current+1)%20;
}
void ringtrace()
{
    int now = current;
    char str[128];
    char *p;
    int i = (current == 0)?19:(current-1);
    do{
        if(iringbuf[i].pc == 0) continue;
        p = str;
        p+=snprintf(str,sizeof(str),"%s pc:0x%08lx : %08x\t",((i+1)%20==now)?"-->":"   ",iringbuf[i].pc,iringbuf[i].inst);
        disassemble(p,str+sizeof(str)-p,iringbuf[i].pc,(uint8_t*)&iringbuf[i].inst,4);
        printf("%s",str);
        i = (i+1)%20;
    }while(i!=current);
}
