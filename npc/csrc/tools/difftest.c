// #include "common.h"
// #include <difftest-def.h>
// #include "svdpi.h"
// extern "C" {
//   int trap();
// }
// extern "C" {
//     int get_pc();
// }
// extern "C" {
//     int get_reg(int num);
// }
// extern CPUstate npc_cpu;
// void npc_init()
// {
//     difftest_init(1);
// }
// void npc_step()
// {
//     npc_cpu.pc = get_pc();
//     for(int i = 0;i<32;i++)
//     {
//         npc_cpu.gpr[i] = get_reg(i);
//     }
//     difftest_regcpy(&npc_cpu,DIFFTEST_TO_REF);
//     difftest_exec(1);
// }