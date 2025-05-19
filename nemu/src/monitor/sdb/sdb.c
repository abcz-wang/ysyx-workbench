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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <memory/paddr.h>
#include "sdb.h"


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
//输入c，运行程序
static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

//输入q,state设置为QUIT,退出.
static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT; 
  return -1;
}
//键入help
static int cmd_help(char *args);
static int  cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Lets the program pause after executing N instructions using single step execution,When N is not given, the default is 1", cmd_si},
  { "info", "Print register status or Print watchpoint information", cmd_info},
  {"x", "Finds the value of the expression EXPR, uses the result as the starting memory address, and outputs consecutive N 4 bytes in hexadecimal.", cmd_x}
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
static int  cmd_si(char *args){
  char *arg = strtok(args, " ");
  if (arg == NULL){
    cpu_exec(1);
    printf("Run 1 step\n");

  }
  else{
    char *end;
    long val = strtol(arg, &end, 0);
      if (*end != '\0') {
  printf("Invalid number: %s\n", arg);
  return 0;
  }
    else{
      cpu_exec((uint64_t)val);
      printf("Run %lu steps\n", val);
    }

  }
  return 0;
}
static int cmd_info(char *args){
  char *arg = strtok(args, " ");
  if (arg == NULL) {
    printf("Usage: info r|w\n");
    return 0;
  }
  
  if (strcmp(arg, "r") == 0) {
    isa_reg_display();
  }
  else if (strcmp(arg, "w") == 0) {
    // 打印监测点信息
  }
  else {
    printf("Unknown info command: %s\n", arg);
  }

  return 0;
}
static int cmd_x(char *args){
  char *arg1 = strtok(args, " ");
  char *arg2 = strtok(NULL, " ");
  if(arg1==NULL||arg2== NULL){
    printf("Usage: x N EXPR\n");
    return 0;
  }
  char *extra = strtok(NULL, " ");
if (extra != NULL) {
  printf("Too many arguments. Usage: x N EXPR\n");
  return 0;
}

  char*end1 ,*end2;
  int n=strtol(arg1,&end1,0);
  paddr_t addr = strtol(arg2,&end2,0);

  if (*end1 != '\0' || *end2 != '\0') {
    printf("Usage: x N EXPR (both must be numbers)\n");
    return 0;
  }

  else{
  // 正确方式：每次读取4字节，连续读取N次
  for (int i = 0; i < n; i++) {
    word_t data = paddr_read(addr + i * 4, 4);
    printf("0x%08x: 0x%08x\n", addr + i * 4, data);
  }

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
