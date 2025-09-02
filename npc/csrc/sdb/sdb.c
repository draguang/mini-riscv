#include "common.h"

WP *head = NULL;

void wp_monitor();
extern uint8_t pmem_read_ram(int raddr);
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(npc) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_q(char *args) {
  npc_state.state = NPC_QUIT;
  return -1;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
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
    word_t ret = pmem_read_ram(address+3)<<24|
                 pmem_read_ram(address+2)<<16|
                 pmem_read_ram(address+1)<<8|
                 pmem_read_ram(address);
    printf("0x%x: 0x%08x\n", address, ret);  
    address += direct;
  }
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

    WP *wp = newwp();
    strncpy(wp->expr,args,sizeof(wp->expr)-1);
    wp->expr[sizeof(wp->expr)-1] = '\0';
    wp->value = result;
    printf("setting success!\n");
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
  { "x","scanning the ram",cmd_x},
  { "p","calculate the expressions",cmd_p},
  { "info","Print the infomation",cmd_info},
  { "w","set the watchpoint",cmd_w},
  { "d","delete the watchpoint by number",cmd_d},
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

void sdb_mainloop() {
  for (char *str; (str = rl_gets()) != NULL; ) {
    
    npc_state.state = NPC_RUNNING;
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