#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include "Vtop.h"
#include "verilated.h"
//#include <nvboard.h>
#include "verilated_vcd_c.h"
#include "svdpi.h"

static int8_t*ram;
static int8_t*rom;
extern "C" {
  int trap();
}
extern "C" int8_t pmem_read_ram(int raddr){
  return ram[raddr];
}
extern "C" int pmem_read_rom(int raddr){
  return rom[raddr & ~0x3u]|rom[(raddr & ~0x3u)+1]|rom[(raddr & ~0x3u)+2]|rom[(raddr & ~0x3u)+3];
}
extern "C" void pmem_write(int waddr, int wdata, char wmask){
  uint32_t waddr_a = waddr & ~0x3u;
  uint32_t original_data = 
        (ram[waddr_a + 0] << 0)  |
        (ram[waddr_a + 1] << 8)  |
        (ram[waddr_a + 2] << 16) |
        (ram[waddr_a + 3] << 24);
  uint32_t new_data = original_data;
    if (wmask & 0x1) new_data = (new_data & ~0x000000FFu) | (wdata & 0x000000FFu);
    if (wmask & 0x2) new_data = (new_data & ~0x0000FF00u) | (wdata & 0x0000FF00u); 
    if (wmask & 0x4) new_data = (new_data & ~0x00FF0000u) | (wdata & 0x00FF0000u); 
    if (wmask & 0x8) new_data = (new_data & ~0xFF000000u) | (wdata & 0xFF000000u);
  ram[waddr_a + 0] = (new_data >> 0)  & 0xFF;
  ram[waddr_a + 1] = (new_data >> 8)  & 0xFF;
  ram[waddr_a + 2] = (new_data >> 16) & 0xFF;
  ram[waddr_a + 3] = (new_data >> 24) & 0xFF;
}
//void nvboard_bind_all_pins(Vtop* top);
VerilatedContext *context = new VerilatedContext;
Vtop* top = new Vtop{context};

void single_cycle() {
  top->clk = 0; top->eval();
  top->clk = 1; top->eval();
}

void reset(int n) {
  top->rst = 1;
  while (n -- > 0) single_cycle();
  top->rst = 0;
}

int main(int argc, char** argv)
{
    svSetScope(svGetScopeFromName("Vtop"));
    ram = (int8_t *)malloc(0x100000000*sizeof(int8_t));
    rom = (int8_t *)malloc(0x100000000*sizeof(int8_t));
    if(!ram || !rom) 
    {
      fprintf(stderr, "Memory allocation failed!\n");
      exit(1);
    }
    context->commandArgs(argc,argv);
    VerilatedVcdC *tfp = new VerilatedVcdC;
    context->traceEverOn(true);
    top->trace(tfp,0);
    tfp->open("wave.vcd");
    Verilated::commandArgs(argc, argv);
    //nvboard_bind_all_pins(top);
    //nvboard_init();
    reset(10);  
    while(!context->gotFinish()) 
    {
        //nvboard_update();
        single_cycle();
        //nvboard_update();
        tfp->dump(context->time());
        context->timeInc(1);
        if(trap())
          break;
    }
    tfp->close();
}