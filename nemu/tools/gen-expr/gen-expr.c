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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
// this should be enough
static size_t origin_length = 65536;
char *buf = NULL;
char *code_buf = NULL;
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";
static void larger(size_t length);
static uint32_t choose(uint32_t num)
{
  return rand()%num;
}

static void gen_num()
{
  uint32_t num = rand()%9+1;
  char num_str[2];
  snprintf(num_str, sizeof(num_str), "%d", num);
  larger(strlen(num_str));
  strcat(buf, num_str);
}

static void gen(char c)
{
  char des[2] = {c,'\0'};
  larger(1);
  strcat(buf,des);
}

static void gen_rand_op()
{
  if (buf[0] != '\0' && strchr("+-*/", buf[strlen(buf)-1]))
    return;
  char op[4] = {'+','-','*','/'};
  char des[2] = {op[choose(4)], '\0'};
  if(strlen(buf)+1<origin_length)
    strcat(buf,des);
}

static void gen_rand_expr(int depth) {
  if(depth>10)
  {
    gen_num();
    return;
  }
  switch (choose(3)) {
    case 0: 
    if(buf[0] == '\0' || !isdigit(buf[strlen(buf)-1])) 
    {
        gen_num();
    }
    break;
    case 1: 
    if(buf[0] == '\0' || 
       strchr("+-*/(", buf[strlen(buf)-1])) {
        gen('(');
        gen_rand_expr(depth+1);
        gen(')');
    }
    break;
    default:
    gen_rand_expr(depth+1);
    if(buf[0] != '\0' && !strchr("+-*/(", buf[strlen(buf)-1])) 
    {
        gen_rand_op();
        gen_rand_expr(depth+1);
    }
    break;
  }
}

static int is_division_by_zero() 
{
  char *p = buf;
  while (*p) 
  {
    if (*p == '/' && *(p + 1) == '0') 
    {
      return 1;
    }
    p++;
  }
  return 0;
}

static void larger(size_t length)
{
  if(strlen(buf)+length+1>origin_length)
  {
    origin_length*=2;
    char*new = realloc(buf,origin_length);
    assert(new != NULL);
    buf = new;
  }
}

int main(int argc, char *argv[]) {
  buf = malloc(origin_length);
  code_buf = malloc(origin_length+128);
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i++) {
    int zero = 0;
    while (!zero) {
      memset(buf, '\0', origin_length);
      gen_rand_expr(0);
      if (is_division_by_zero()) continue;
      
      sprintf(code_buf, code_format, buf);
      FILE *fp = fopen("./code_gen.c", "w");
      assert(fp != NULL);
      fputs(code_buf, fp);
      fclose(fp);

      int ret = system("gcc code_gen.c -o expr -Wall -Werror 2>/dev/null");
      if (ret != 0)
      {
        unlink("code_gen.c");
        unlink("expr");
        continue;
      }   
      
      fp = popen("./expr 2>&1", "r");  
      if (fp == NULL) continue;
      
      char output[128];
      int result;
      if (fgets(output, sizeof(output), fp) == NULL) {
        pclose(fp);
        continue; 
      }
      pclose(fp);
      
      if (strstr(output, "Floating point exception") != NULL) {
        continue;  
      }
      
      if (sscanf(output, "%d", &result) != 1) {
        continue;  
      }
      
      zero = 1;
      printf("%u %s\n", result, buf);
    }
    
  }
  free(buf);
  free(code_buf);
  return 0;
}