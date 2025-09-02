#include "common.h"
#include "Vtop.h"
#include "verilated.h"
//#include <nvboard.h>
#include "verilated_vcd_c.h"
#include "svdpi.h"
#define N 50
const char* image_path = NPC_IMAGE_PATH;
void init_wp_pool();
void wp_monitor();
void load(const char* filename,uint8_t*rom,size_t max);
// void npc_step();
// void init_npc();


extern uint8_t*ram;
extern uint8_t*rom;
extern "C" {
  int trap();
}
extern "C" {
    int get_pc();
}

//void nvboard_bind_all_pins(Vtop* top);
VerilatedContext *context = new VerilatedContext;
Vtop* top = new Vtop{context};
VerilatedVcdC *tfp = new VerilatedVcdC;
void cpu_exec(uint64_t i) {
  
  while(i--)
  {
    //npc_step();
    top->clk = 0; top->eval();tfp->dump(context->time());context->timeInc(1);
    top->clk = 1; top->eval();tfp->dump(context->time());context->timeInc(1);
    svSetScope(svGetScopeFromName("TOP.top"));
    if(trap())
    {
      printf("HIT GOOD TRAP at PC = 0x%x\n",get_pc());
      npc_state.state = NPC_END;
      return;
    }
    wp_monitor();
    if(npc_state.state == NPC_STOP) return;
  }
}

void reset(int n) {
  top->rst = 1;
  while (n -- > 0) cpu_exec(1);
  top->rst = 0;
}

int main(int argc, char** argv)
{
    //init_npc();
    printf("NPC_IMAGE_PATH = %s\n", NPC_IMAGE_PATH);
    ram = (uint8_t *)malloc(0x100000000*sizeof(int8_t));
    rom = (uint8_t *)malloc(0x100000000*sizeof(int8_t));
    if(!ram || !rom) 
    {
      fprintf(stderr, "Memory allocation failed!\n");
      exit(1);
    }
#ifndef CONFIG_TARGET_AM
    load("/home/guanglong/ysyx-workbench/npc/csrc/bin/mem.bin",rom,0x100000000);
    load("/home/guanglong/ysyx-workbench/npc/csrc/bin/mem.bin",ram,0x100000000);
#else
    load(image_path,rom,0x100000000);
    load(image_path,ram,0x100000000);
#endif
    context->commandArgs(argc,argv);
    context->traceEverOn(true);
    top->trace(tfp,0);
    tfp->open("wave.vcd");
    Verilated::commandArgs(argc, argv);
    //nvboard_bind_all_pins(top);
    //nvboard_init();
    reset(10);
    svSetScope(svGetScopeFromName("TOP.top"));
    int round = N;
    init_wp_pool();
    while(1) 
    {
        npc_state.state = NPC_RUNNING;
        //nvboard_update();
        sdb_mainloop();
        //cpu_exec(1);
        //nvboard_update();
        if(npc_state.state == NPC_QUIT) break;
        
    }
    free(rom);
    free(ram);
    tfp->close();
}