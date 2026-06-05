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



static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;//head用于组织使用中的监视点结构, free_用于组织空闲的监视点结构
static int wp_counter = 0; 

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    //wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp(){
  if(free_==NULL){
    printf("No free watchpoint available!\n");
    assert(0);
  }
  WP *wp=free_;
  free_=free_->next;
  wp->NO = ++wp_counter;
  wp->next=head;
  head=wp;
  wp->expr[0]='\0';
  wp->last_val=0;
  return wp;
}

void free_wp(WP *wp) {
  // 从 head 链表中移除
  if (head == wp) {
    head = wp->next;
  } else {
    WP *prev = head;
    while (prev && prev->next != wp) {
      prev = prev->next;
    }
    if (prev) {
      prev->next = wp->next;
    }
  }

  // 放回 free 链表
  wp->next = free_;
  free_ = wp;
}
// 删除编号为 no 的监测点，成功返回 true，失败返回 false
bool delete_wp(int no) {
  WP *prev = NULL;
  WP *cur = head;

  while (cur != NULL) {
    if (cur->NO == no) {
      if (prev == NULL) {
        head = cur->next;
      } else {
        prev->next = cur->next;
      }

      free_wp(cur);
      return true;
    }
    prev = cur;
    cur = cur->next;
  }

  return false; // 没有找到
}
//列出检查点
void list_watchpoints() {
  if (head == NULL) {
    printf("No watchpoints.\n");
    return;
  }

  printf("%-4s %-20s %-12s\n", "NO", "Expression", "Last Value");
  printf("---- -------------------- ------------\n");

  WP *wp = head;
  while (wp != NULL) {
    printf("%-4d %-20s 0x%-10x\n", wp->NO, wp->expr, wp->last_val);
    wp = wp->next;
  }
}
//检查监视点是否触发
bool check_watchpoints() {
  WP *wp = head;
  bool changed = false;

  while (wp != NULL) {
    bool success = true;
    word_t val = expr(wp->expr, &success);  // 重新计算表达式

    if (!success) {
      printf("Failed to evaluate expression: %s\n", wp->expr);
      wp = wp->next;
      continue;
    }

    if (val != wp->last_val) {
      printf("\nWatchpoint %d triggered:\n", wp->NO);
      printf("  Expression: %s\n", wp->expr);
      printf("  Old value : 0x%x\n", wp->last_val);
      printf("  New value : 0x%x\n", val);
      wp->last_val = val;
      changed = true;
    }

    wp = wp->next;
  }

  return changed;
}
