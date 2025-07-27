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

#include "sdb.h"

#define NR_WP 32



WP wp_pool[NR_WP] = {};
WP *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }
  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP *newwp()
{
  if(free_ == NULL)
    assert(0);
  else
  {
    WP *result = free_;
    free_ = free_->next;
    result->next = head;
    head = result;
    return result;
  }
}

void free_wp(WP *wp)
{
  wp->value = 0;
  memset(wp->expr,0,sizeof(wp->expr));
  if (wp == NULL)  return;
  wp->next = free_;
  free_ = wp;
}



char is_enable(WP *wp)
{
  #ifdef CONFIG_WATCHPOINT
    return 'y';
  #endif
  return 'n';
}

void print_wp()
{
  if(head == NULL)
    printf("No setted watchpoint!\n");
  else
  {
    bool success = true;
    WP *count = head;
    printf("%-8s %-8s %-8s %-8s\n","NO","value","enable","expr");
    while(count!=NULL)
    {
      printf("%-8d 0x%08x %-8c %-8s\n",count->NO,expr(count->expr,&success),is_enable(count),count->expr);
      count = count->next;
    }
  }
}

int delete_watchpoint(int no)
{
  int is_exist = 0;
  WP **temp = &head;
  while(*temp != NULL)
  {
    WP *entry = *temp;
    if(entry->NO == no)
    {
      *temp = entry->next;
      free_wp(entry);
      is_exist = 1;
      continue;
    }
    else
      temp = &(entry->next);
  }   
  return is_exist;
}