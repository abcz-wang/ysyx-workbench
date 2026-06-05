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
static int cmd_help(char *args);
static int  cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);
static int cmd_p(char *args);
static int cmd_w(char *args);
static int cmd_d(char *args);
static int cmd_expr_test(char *args);


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
  {"x", "Finds the value of the expression EXPR, uses the result as the starting memory address, and outputs consecutive N 4 bytes in hexadecimal.", cmd_x},
  {"p","Find the value of the expression EXPR",cmd_p},
  {"w","Suspend program execution when the value of expression EXPR changes.",cmd_w},
  {"d","	Deletes the watchpoint with ID N.",cmd_d},
  { "expr_test", "Test expression evaluation from a file", cmd_expr_test },


};

#define NR_CMD ARRLEN(cmd_table)//cmd_table的长度
//键入help
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
//打印寄存器和监视点
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
    list_watchpoints();
  }
  else {
    printf("Unknown info command: %s\n", arg);
  }

  return 0;
}
//扫描内存
static int cmd_x(char *args) {
  // 参数分解
  char *arg1 = strtok(args, " ");     // N
  char *arg2 = strtok(NULL, " ");     // EXPR
  char *extra = strtok(NULL, " ");    // 检查是否有多余参数

  if (arg1 == NULL || arg2 == NULL) {
    printf("Usage: x N EXPR\n");
    return 0;
  }
  if (extra != NULL) {
    printf("Too many arguments. Usage: x N EXPR\n");
    return 0;
  }

  // 解析第一个参数 N
  char *end1;
  int n = strtol(arg1, &end1, 0);
  if (*end1 != '\0' || n <= 0) {
    printf("Error: N must be a positive number.\n");
    printf("Usage: x N EXPR\n");
    return 0;
  }

  // 第二个参数：表达式求值
  bool success = false;
  word_t addr = expr(arg2, &success);
  if (!success) {
    printf("Error: Invalid expression: %s\n", arg2);
    printf("Usage: x N EXPR\n");
    return 0;
  }

  // 正常打印内存
  for (int i = 0; i < n; i++) {
    word_t data = paddr_read(addr + i * 4, 4);
    printf("0x%08x: 0x%08x\n", addr + i * 4, data);
  }

  return 0;
}

static int cmd_p(char *args) {
  if (args == NULL) {
    printf("Usage: p EXPR\n");
    return 0;
  }

  bool success = false;
  word_t result = expr(args, &success);

  if (success) {
    printf("Result = %u (0x%x)\n", result, result);  // 打印十进制和十六进制
  } else {
    printf("Error: Invalid expression.\n");
  }

  return 0;
}

static int cmd_w(char *args) {
  if (args == NULL) {
    printf("Usage: w EXPR\n");
    return 0;
  }

  word_t val = 0;
  bool success = true;

  // 解析表达式
  val = expr(args, &success);
  if (!success) {
    printf("Invalid expression: %s\n", args);
    return 0;
  }

  // 创建一个新的监视点
  WP *wp = new_wp();
  if (wp == NULL) {
    printf("No free watchpoints available!\n");
    return 0;
  }

  // 保存表达式和当前值
  strncpy(wp->expr, args, sizeof(wp->expr) - 1);
  wp->expr[sizeof(wp->expr) - 1] = '\0';
  wp->last_val = val;

  // 打印成功信息
  printf("Set watchpoint %d: %s == " FMT_WORD "\n", wp->NO, wp->expr, wp->last_val);
  return 0;
}

static int cmd_d(char *args) {
  if (args == NULL || *args == '\0') {
      printf("Error: No watchpoint number specified.\n");
      return 0;
  }

  // 跳过前导空格
  while (isspace((unsigned char)*args)) {
      args++;
  }

  char *endptr;
  long num = strtol(args, &endptr, 10);

  // 检查是否成功转换数字
  if (endptr == args) {
      printf("Error: '%s' is not a valid number.\n", args);
      return 0;
  }

  // 检查剩余字符是否全为空格
  while (*endptr != '\0') {
      if (!isspace((unsigned char)*endptr)) {
          printf("Error: Invalid characters in number '%s'.\n", args);
          return 0;
      }
      endptr++;
  }

  // 检查数值范围
  if (num <= 0 ) {
      printf("Error: Number %ld is out of valid range .\n", num);
      return 0;
  }

  int no = (int)num;

  // 尝试删除监视点
  if (delete_wp(no)) {
      printf("Watchpoint %d deleted.\n", no);
  } else {
      printf("Error: Watchpoint %d not found.\n", no);
  }

  return 0;
}

static int cmd_expr_test(char *args) {
  if (args == NULL) {
    printf("Usage: expr_test <file>\n");
    return 0;
  }

  FILE *fp = fopen(args, "r");
  if (fp == NULL) {
    printf("Cannot open file: %s\n", args);
    return 0;
  }

  char line[65536];
  int total = 0, pass = 0;

  while (fgets(line, sizeof(line), fp) != NULL) {
    // 去掉末尾换行
    line[strcspn(line, "\n")] = '\0';

    // 格式: 结果 表达式
    char *expr_str = NULL;
    unsigned expected = strtoul(line, &expr_str, 10);

    // 跳过结果和表达式之间的空格
    while (*expr_str == ' ') expr_str++;

    bool success = false;
    word_t result = expr(expr_str, &success);

    if (success && result == expected) {
      pass++;
    } else {
      printf("FAIL: %s\n  expected = %u, got = %u\n",
             expr_str, expected, result);
    }
    total++;
  }

  fclose(fp);
  printf("Passed %d/%d\n", pass, total);
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
