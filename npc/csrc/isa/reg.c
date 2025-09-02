#include "common.h"

#include "svdpi.h"

extern "C" {
    int get_reg(int num);
}
extern "C" {
    int get_pc();
}
const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

uint32_t isa_reg_str2val(const char *s, bool *success) {
    if (s[0] != '$') 
    { 
        *success = false;
        return 0;
    }
    const char *reg_name = s + 1;
    svSetScope(svGetScopeFromName("TOP.top.gpr"));
    for (int i = 0; i < 32; i++) {
        if (strcmp(reg_name, regs[i]) == 0) {
            *success = true;
            return get_reg(i);
        }
    }
    svSetScope(svGetScopeFromName("TOP.top"));
    if(strcmp(reg_name, "pc") == 0)
    {
      *success = true;
      return get_pc();
    }
    return 0;  
}

void isa_reg_display() {
  int i = 0;
  printf("%-10s0x%08x\n","pc",get_pc());
  for(i = 0;i<32;i++)
  {
    svSetScope(svGetScopeFromName("TOP.top.gpr"));
    printf("%-10s0x%08x\n",regs[i],get_reg(i));
  }
}