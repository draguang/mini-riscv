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
#include <isa.h>
#include <stdbool.h>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <memory/paddr.h>
bool check_parentheses(int p, int q);
int is_operator(int type);
enum {
  TK_NOTYPE = 256, TK_EQ,
  TK_NUM,DEREF,TK_NEQ,TK_AND,
  TK_HEX,TK_REG,TK_LM
  /* TODO: Add more token types */

};
int get_priority(int type) {
  switch(type) {
    case DEREF: return 5;
    case '+': case '-': return 3;
    case '*': case '/': return 4;
    case TK_NEQ: case TK_EQ: return 2;
    case TK_AND: return 1;
    default: return 0;
  }
}

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"\\-", '-'},
  {"\\*", '*'},
  {"\\/", '/'},
  {"\\)", ')'},
  {"\\(", '('},
  {"0[xX][0-9a-fA-F]+",TK_HEX},
  {"[0-9]+",TK_NUM},
  {"!=",TK_NEQ},
  {"&&",TK_AND},
  {"\\$[a-zA-Z0-9]+",TK_REG},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[2048];
} Token;

static Token tokens[2048] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if(nr_token == 2047)
        memset(tokens,0,sizeof(tokens));
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        //Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            //i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        switch (rules[i].token_type) {
          case TK_NUM:
            tokens[nr_token].type = TK_NUM;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;
          case '+':
          case '-':
          case '/':
          case '(':
          case ')':
            tokens[nr_token].type = rules[i].token_type;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;
          case TK_EQ:
            tokens[nr_token].type = TK_EQ;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;
          case TK_NEQ:
            tokens[nr_token].type = TK_NEQ;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;
          case TK_AND:
            tokens[nr_token].type = TK_AND;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;
          case '*':
            if (nr_token == 0 ||(nr_token>0&&( tokens[nr_token - 1].type == '+' || 
            tokens[nr_token - 1].type == '-' || 
            tokens[nr_token - 1].type == '*' || 
            tokens[nr_token - 1].type == '/' ||
            tokens[nr_token - 1].type == '='))) 
            {
              tokens[nr_token].type = DEREF;
              break;
            }
            else
            {
              tokens[nr_token].type = rules[i].token_type;
              strncpy(tokens[nr_token].str, substr_start, substr_len);
              tokens[nr_token].str[substr_len] = '\0';
              nr_token++;
              break;
            }
          case TK_HEX:
            tokens[nr_token].type = TK_HEX;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;
          case TK_REG:
            tokens[nr_token].type = TK_REG;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;
          case TK_NOTYPE:
            break;
          default: break;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

uint32_t eval(int p,int q);
uint32_t expr(char *e, bool *success) {
  *success = true;
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  return eval(0,nr_token-1);
}
uint32_t eval(int p,int q)
{
  bool success = false;
  uint32_t result = 0;
  if (p > q) {
    printf("Invalid,p>q了\n");
    return -1;
  }
  else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    switch(tokens[p].type)
    {
      case TK_NUM:
        sscanf(tokens[p].str,"%u",&result);
        return result;
      case TK_HEX:
        sscanf(tokens[p].str,"%x",&result);
        return result;
      case TK_REG:
        result = isa_reg_str2val(tokens[p].str,&success);
        if(!success)
        {
          printf("Invalid register\n");
          return 0;
        }
        if(result<0)
          return (uint32_t)result; 
        return result;
      default:
        printf("Invalid token type\n");
        return 0;
    }
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1);
  }
  else {
    /* We should do more things here. */
    int op = -1;
    int kuohao = 0;

    if(tokens[p].type == DEREF) 
    {
      if(p+1 > q) 
      {
          printf("Invalid dereference expression\n");
          return 0;
      }
      uint32_t addr;
      if(tokens[p+1].type == '(') {
          if(!check_parentheses(p+1, q)) {
              printf("Mismatched parentheses in dereference\n");
              return 0;
          }
          addr = eval(p+2, q-1);  
      } 
      else 
      {
          addr = eval(p+1, q);     
      }
      return paddr_read(addr, 4);  
    }
    for(int i = p;i<q+1;i++)
    {
      if(tokens[i].type == '(') kuohao++;
      else if(tokens[i].type == ')') kuohao--;
      else if(kuohao == 0&&is_operator(tokens[i].type))
      {
        if(op == -1 || get_priority(tokens[i].type) < get_priority(tokens[op].type)||
        get_priority(tokens[i].type) == get_priority(tokens[op].type)) 
          op = i;
      }
    }
    int32_t val1 = eval(p, op - 1);
    int32_t val2 = eval(op + 1, q);
    //printf("计算: %d %c %d\n", val1, tokens[op].type, val2);
    switch (tokens[op].type) 
    {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      case TK_AND: return val1 && val2;
      case TK_NEQ: return val1 != val2;
      case TK_EQ: return val1 == val2;
      default: assert(0);
    }
  }
}
int is_operator(int type) 
{
    return type == '+' || type == '-' || type == '*' || type == '/'||
    type == TK_AND || type == TK_NEQ||type == TK_EQ||type == DEREF;
}
bool check_parentheses(int p,int q)
{
  int kuohao = 0;
  for(int i = p;i<q+1;i++)
  {
    if(tokens[i].type == '(') kuohao++;
    else if(tokens[i].type == ')') kuohao--;
    if(kuohao<0) return false;
  }
  if(kuohao!=0) return false;
  kuohao = 0;
  for(int i = p;i<q+1;i++)
  {
    if(tokens[i].type == '(') kuohao++;
    else if(tokens[i].type == ')') kuohao--;
    if(kuohao == 0&&i!=q) return false;
  }
  return true;
}
