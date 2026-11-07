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
#include <inttypes.h>

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  word_t old_val;
  char expr[64];
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

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

WP* new_wp() {
  if (free_ == NULL) {
    assert(0);
  }
  WP *ans = free_;
  free_ = free_->next;
  return ans;
}

void free_wp(WP *wp) {
  if (head->NO == wp->NO) {
    head = head->next;
    wp->next = free_;
    free_ = wp;
    return;
  }
  WP* p = head;
  while (p != NULL && p->next != wp) {
    p = p->next;
  }
  if (p != NULL) {
  //head = head->next;
    p->next = wp->next;
    wp->next = free_;
    free_ = wp;
  }
  else {
    printf("The watchpoint is not in use or does not exist\n");
  }
  return;
} 

WP* get_no_position (int no) {
  if (head->NO == no) {
    return head;
  }
  WP *wp_p = head;
  while (wp_p != NULL && wp_p->NO != no)
  {
    wp_p = wp_p->next;
  }
  if (wp_p == NULL) {
    printf ("The watchpoint is not in use or not exist\n");
    return NULL;
  }
  return wp_p;
}

void add_wp (WP *wp, char *args) {
  if (head == NULL) {
    head = wp;
    wp->NO = 0;
    wp->next = NULL;    
  }
  else {
  wp->NO = head->NO + 1;
  wp->next = head;
  head = wp;
  }
  strcpy(wp->expr, args);
  bool success = true;
  wp->old_val = expr(args, &success);
  return;
}

void wp_display() {
  WP* wp = head;
  while (wp != NULL) {
    printf ("NO: %d\t Expr:%s\t Val: 0x%" PRIxMAX "\n", wp->NO, wp->expr, (uintmax_t)wp->old_val);
    wp = wp->next;
  }
  return;
}

bool check_wp() {
  WP *wp = head;
  bool success = false;
  while (wp != NULL) {
    word_t new_val = expr(wp->expr, &success);
    if (new_val != wp->old_val) {
      printf("Watchpoint %d triggered: %s\n", wp->NO, wp->expr);
      printf("Old value: 0x%" PRIxMAX ", New value: 0x%" PRIxMAX "\n", (uintmax_t)wp->old_val, (uintmax_t)new_val);
      wp->old_val = new_val; // Update the old value
      return false;
    }
    wp = wp->next;
  }
  return true;
}