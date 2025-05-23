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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";
int buf_index=0;
#define MAX_DEPTH 10
void gen_num() {
  unsigned num = rand() % 50 + 1;
  int len = snprintf(buf + buf_index, sizeof(buf) - buf_index, "%u", num);
  buf_index += len;
  buf[buf_index] = '\0';  // 添加终结符，防止后续使用未终止字符串
}



void gen(char c) {
  buf[buf_index++] = c;
  buf[buf_index] = '\0';
}

void gen_rand_op() {
  char ops[] = {'+', '-', '*', '/'};
  char op = ops[rand() % 4];
  gen(op);
}
static void gen_rand_expr(int depth) {
  if(depth>=MAX_DEPTH){
    gen_num();
    return;
  }

  if (buf_index >= sizeof(buf) - 10) {
    gen_num(); // 强制生成数字
    return;
  }
// 调整概率：数字40%，括号20%，运算符40%
int choice = rand() % 5;
if (choice < 2) { // 0-1: 数字
  gen_num();
} else if (choice < 3) { // 2: 括号
  gen('(');
  gen_rand_expr(depth + 1);
  gen(')');
} else { // 3-4: 运算符
  gen_rand_expr(depth + 1);
  gen_rand_op();
  gen_rand_expr(depth + 1);
}
}


int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    buf[0] = '\0';          // 清空表达式缓存
    buf_index = 0;          // 重置指针
    gen_rand_expr(0);

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
