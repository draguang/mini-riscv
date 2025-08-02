#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include "Vtop.h"
#include "verilated.h"
//#include <nvboard.h>
#include "verilated_vcd_c.h"
#include "svdpi.h"
#define N 50
void load(const char* filename,uint8_t*rom,size_t max)
{
  FILE *file = fopen(filename,"rb");
  if(!file)
  {
    printf("open failed!\n");
    fprintf(stderr, "Failed to open file: %s\n", filename);
    exit(1);
  }
  fseek(file, 0, SEEK_END);
  size_t file_size = ftell(file);
  fseek(file, 0, SEEK_SET);
  if (file_size > max) 
  {
    printf("file is flowing\n");
    fclose(file);
    exit(1);
  }
  size_t bytes_read = fread(rom, 1, file_size, file);
  if (bytes_read!=file_size)
  {
    printf("reading failed!\n");
    fclose(file);
    exit(1);
  }
}
static uint8_t*ram;
static uint8_t*rom;
extern "C" {
  int trap();
}
extern "C" int8_t pmem_read_ram(int raddr){
  switch(raddr-(raddr&~0x3u))
  {
    case 0:return ram[raddr&~0x3u];
    case 1:return ram[raddr&~0x3u+1];
    case 2:return ram[raddr&~0x3u+2];
    case 3:return ram[raddr&~0x3u+3];
  }
}
extern "C" int pmem_read_rom(int raddr){
  return rom[raddr & ~0x3u]|rom[(raddr & ~0x3u)+1]<<8|rom[(raddr & ~0x3u)+2]<<16|rom[(raddr & ~0x3u)+3]<<24;
}
extern "C" void pmem_write(int waddr, int wdata, int wmask){
  int waddr_a = waddr & ~0x3u;
  int32_t original_data = 
        (ram[waddr_a])           |
        (ram[waddr_a + 1] << 8)  |
        (ram[waddr_a + 2] << 16) |
        (ram[waddr_a + 3] << 24);
  int32_t new_data = original_data;
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
VerilatedVcdC *tfp = new VerilatedVcdC;
void single_cycle() {
  top->clk = 0; top->eval();tfp->dump(context->time());context->timeInc(1);
  top->clk = 1; top->eval();tfp->dump(context->time());context->timeInc(1);
}

void reset(int n) {
  top->rst = 1;
  while (n -- > 0) single_cycle();
  top->rst = 0;
}

int main(int argc, char** argv)
{
    ram = (uint8_t *)malloc(0x100000000*sizeof(int8_t));
    rom = (uint8_t *)malloc(0x100000000*sizeof(int8_t));
    if(!ram || !rom) 
    {
      fprintf(stderr, "Memory allocation failed!\n");
      exit(1);
    }
    load("/home/guanglong/ysyx-workbench/npc/csrc/sum.bin",rom,0x100000000);
    load("/home/guanglong/ysyx-workbench/npc/csrc/sum.bin",ram,0x100000000);
    // for(int i = 0;i<100;i++)
    // {
    //   printf("0x%x\n",ram[i]);
    // }
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
    
    while(!trap()) 
    {
        //nvboard_update();
        single_cycle();
        //nvboard_update();
        // printf("0x%x 0x%x\n",ram[331731],ram[331732]);
    }
    tfp->close();
}