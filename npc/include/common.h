#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdbool.h>
#include <unistd.h>
#include <assert.h>
#include <regex.h>
#include <time.h>
#include "svdpi.h"


//config section
#define CONFIG_WATCHPOINT 1
#define CONFIG_TARGET_AM 1
#define CONFIG_ITRACE 1

//define section
#define ARRLEN(arr) (int)(sizeof(arr) / sizeof(arr[0]))
#define paddr_t uint32_t
#define word_t uint32_t


//struct section
typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  /* TODO: Add more members if necessary */
  char expr[2048];
  uint32_t value;
  
} WP;
enum{
    NPC_RUNNING,NPC_STOP,NPC_ABORT,NPC_END,NPC_QUIT
};
typedef struct{
    int state;
}NPCState;
extern NPCState npc_state;
typedef struct{
    word_t pc;
    word_t gpr[32];
}CPUstate;

//function section
void sdb_mainloop();
void cpu_exec(uint64_t i);
uint32_t expr(char *e, bool *success);
void init_wp_pool();
WP *newwp();
void free_wp(WP *wp);
void print_wp();
int delete_watchpoint(int no);
uint32_t isa_reg_str2val(const char *s, bool *success);
void isa_reg_display();
void wp_monitor();
extern "C" {
    uint8_t pmem_read_ram(uint32_t raddr);
}
extern WP *head;


#endif