/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/
#include <memory/paddr.h>
#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <stdlib.h>
#include <stdio.h>
WP *head = NULL;
static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_si(char *args)
{
  uint64_t num = 0;
  if(args == NULL)
    cpu_exec(1);
  else
  {
    sscanf(args,"%lu",&num);
    cpu_exec(num);
  }
  return 0;
}

static int cmd_info(char *args) 
{
  char *arg = strtok(NULL, " ");
  if (arg == NULL) {
    return 0;
  }
  else if(arg[0] == 'r') {
    isa_reg_display();
  }
  else if (arg[0] == 'w') {
    print_wp();
  }
  return 0;
}

static int cmd_x(char *args)
{
  char *n = strtok(NULL, " ");
  if (n == NULL) 
  {
    printf("Argument required.\n");
    return 0;
  }
  int32_t num = 1;
  char *addr = strtok(NULL, " ");
  bool success = true;
  paddr_t address;
  if(addr == NULL)
  {
    addr = n;
    address = expr(addr,&success);
  }
  else
  {
    sscanf(n,"%d",&num);
    address = expr(addr,&success);
  }
  int direct = num > 0 ? 4 : -4;  
    num = num > 0 ? num : -num;
    for ( ; num > 0; --num) 
    {
      word_t ret = paddr_read(address, 4); 
      printf("0x%x: 0x%08x\n", address, ret);  
      address += direct;
    }
  return 0;
}

static int cmd_d(char *args)
{

  int no;
  sscanf(args,"%d",&no);
  if(delete_watchpoint(no))
    printf("The watchpoint has been deleted\n");
  else
    printf("Invalid NO\n"); 
  return 0;
}

static int cmd_p(char *args)
{
  bool success = true;
  uint32_t result = expr(args,&success);
  if(!success)
    printf("Invalid expressions.\n");
  else
    printf("%u\n",result);
  return 0;
}

static int cmd_w(char *args)
{
  if(args == NULL)
  {
    printf("Invalid input");
    return 0;
  }
  bool success = true;
  uint32_t result = expr(args, &success);
  if(!success)
  {
    printf("Invalid expressions.\n");
    return 1;
  }

    WP *new = newwp();
    strncpy(new->expr,args,sizeof(new->expr)-1);
    new->expr[sizeof(new->expr)-1] = '\0';
    new->value = result;
    printf("setting success!\n");
    return 0;
}

static int cmd_ext(char *args)
{
  size_t origin_length = 0;
  char *line = NULL;
  ssize_t read;
  FILE *input = fopen("/home/guanglong/ysyx-workbench/nemu/tools/gen-expr/input","r");
  if(input == NULL)
  {
    printf("opening failed\n");
    return -1;
  }
  int all = 0;
  int pass = 0;
  uint32_t answer;
  bool success = true;
  uint32_t value;
  while((read = getline(&line,&origin_length,input))!=-1)
  {
    char *str = malloc(read+1);
    all++;
    sscanf(line, "%u %[^\n]", &answer, str);
    value = expr(str,&success);
    if(value == answer)
      pass++;
    else
      printf("NO.%d answer:%u your:%u\n",all,answer,value);
    free(str);
  }
  fclose(input);
  printf("pass:%d all:%d\n",pass,all);
  free(line);
  return 0;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si","Execute single programming",cmd_si},
  { "info","Print the infomation",cmd_info},
  { "x","scanning the ram",cmd_x},
  { "p","calculate the expressions",cmd_p},
  { "w","set the watchpoint",cmd_w},
  { "d","delete the watchpoint by number",cmd_d},
  { "ext","test the score of expr",cmd_ext}
  /* TODO: Add more commands */
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
