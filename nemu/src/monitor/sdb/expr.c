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
#include <memory/paddr.h>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define MAX_TOKEN_LEN 31
#define TOKENS_SIZE 32
enum {
  TK_NOTYPE = 256, 
  TK_EQ,
  TK_NUM,     // shuzi
  TK_HEX,     //十六进制
  TK_REG,      //寄存器
  TK_DEREF,     //指针解引用
  TK_NEG,        //负号
  TK_NOEQ,        // !=
  TK_AND          // &&
  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces，这里+为正则表达
  {"\\+", '+'},         // plus，\+匹配一个+字符 \\转义‘\’
  {"-", '-'},
  {"\\*",'*'},
  {"/",'/'},
  {"\\(",'('},
  {"\\)",')'},
  {"==", TK_EQ},        // equal
  {"!=",TK_NOEQ},
  {"&&",TK_AND},
  {"0[xX][0-9A-Fa-f]+",TK_HEX},
  {"[0-9]+",TK_NUM},
  {"\\$[a-zA-Z0-9]+",TK_REG}

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
  char str[MAX_TOKEN_LEN+1];
} Token;

static Token tokens[TOKENS_SIZE] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;
          case TK_HEX:
          case TK_REG:
          case TK_NUM:
          if(nr_token < TOKENS_SIZE){
            if(substr_len>MAX_TOKEN_LEN){
              printf("Token too long at position %d\n", position);
              return false;
            }
            tokens[nr_token].type = rules[i].token_type;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;
          }
          else{
            printf("Tokens too much !\n");
            return false;
          }
          default: 
          if(nr_token < TOKENS_SIZE){
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
          }
          else{
            printf("Tokens too much !\n");
            return false;
          }
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
    // 二次遍历，处理 * 和 - 的一元操作
    for (int i = 0; i < nr_token; i++) {
      if (tokens[i].type == '*') {
        if (i == 0 || (tokens[i - 1].type != TK_NUM && tokens[i - 1].type != TK_HEX &&
                       tokens[i - 1].type != ')' && tokens[i - 1].type != TK_REG)) {
          tokens[i].type = TK_DEREF;
        }
      } else if (tokens[i].type == '-') {
        if (i == 0 || (tokens[i - 1].type != TK_NUM && tokens[i - 1].type != TK_HEX &&
                       tokens[i - 1].type != ')' && tokens[i - 1].type != TK_REG)) {
          tokens[i].type = TK_NEG;
        }
      }
    }

  return true;
}
bool check_parentheses(int l, int r) {
  if (tokens[l].type != '(' || tokens[r].type != ')') return false;
  int balance = 0;
  for (int i = l; i <= r; i++) {
    if (tokens[i].type == '(') balance++;
    else if (tokens[i].type == ')') balance--;
    if (balance == 0 && i < r) return false;
  }
  return balance == 0;
}

int get_priority(int type) {
  switch (type) {
    case TK_AND: 
    return 0;
    case TK_EQ:
    case TK_NOEQ:
    return 1;
    case '+':
    case '-': 
    return 2;
    case '*':
    case '/': 
    return 3;
    case TK_NEG:
    case TK_DEREF: 
    return 4;
    default: return 100;
  }
}

int find_main_op(int l, int r) {
  int min = 100, main_op = -1, paren = 0;
  for (int i = l; i <= r; i++) {
    if (tokens[i].type == '(') paren++;
    else if (tokens[i].type == ')') paren--;
    else if (paren == 0) {
      int prio = get_priority(tokens[i].type);
      if (prio <= min && prio >= 0) {
        min = prio;
        main_op = i;
      }
    }
  }
  return main_op;
}



int eval(int l,int r){
  if(l>r){ printf("Error: Invalid expression\n");
    return 0;}
  if (l==r){   
    if (tokens[l].type == TK_NUM || tokens[l].type == TK_HEX) {
      char *endptr;
      long num = strtol(tokens[l].str, &endptr, 0);
      return (int)num;
    } 
    else if (tokens[l].type == TK_REG) {
      bool success = false;
      word_t val = isa_reg_str2val(tokens[l].str, &success);
      if (!success) {
        printf("Error: Invalid register %s\n", tokens[l].str);
        return 0;
      }
      return val;
    }
   }

    
   if (check_parentheses(l,r) == true){
    return eval(l+1,r-1);
   }

    int op = find_main_op(l,r);
    if (op == -1) {
        printf("Error: No main operator found\n");
        return 0;
    }
    
  // ---- 处理一元操作符 ----
  if (tokens[op].type == TK_NEG) {
    return -eval(op + 1, r);
  } else if (tokens[op].type == TK_DEREF) {
    word_t addr= eval(op+1,r);
    return paddr_read(addr,4);
  }

  // ---- 二元操作 ----
  int val1 = eval(l, op - 1);
  int val2 = eval(op + 1, r);

  switch (tokens[op].type) {
    case '+': return val1 + val2;
    case '-': return val1 - val2;
    case '*': return val1 * val2;
    case '/': return val1 / val2;
    case TK_EQ: return (val1 == val2);
    case TK_NOEQ: return (val1 != val2);
    case TK_AND: return (val1 && val2);
    default:
      printf("Error: Unknown binary operator.\n");
      return 0;
  }
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  *success = true;
  return eval(0,nr_token-1);
}
