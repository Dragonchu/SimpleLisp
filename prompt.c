#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

#ifdef _WIN32
#include <string.h>
static char buffer[2048];
/* Fake readline function */
char *readline(char *prompt)
{
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char *cpy = malloc(strlen(buffer) + 1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy) - 1] = '\0';
  return cpy;
}
/* Fake add_history function */
void add_history(char *unused) {}

/* Otherwise include the editline headers */
#else
#include <editline/readline.h>
#endif
/* lval types */
enum
{
  LVAL_NUM,
  LVAL_ERR
};
/* errors */
enum
{
  LERR_DIV_ZERO,
  LERR_BAD_OP,
  LERR_BAD_NUM
};

typedef struct
{
  int type;
  long num;
  int err;
} lval;

lval lval_num(long x)
{
  lval v;
  v.num = x;
  v.type = LVAL_NUM;
  return v;
}

lval lval_err(int x)
{
  lval v;
  v.err = x;
  v.type = LVAL_ERR;
  return v;
}

void lval_print(lval v)
{
  switch (v.type)
  {
  case LVAL_NUM:
    printf("%li", v.num);
    break;
  case LVAL_ERR:
    if (v.err == LERR_DIV_ZERO)
      printf("Error: Division By Zero");
    if (v.err == LERR_BAD_NUM)
      printf("Error: Invalid Num");
    if (v.err == LERR_BAD_OP)
      printf("Error: Invalid Operator");
    break;
  default:
    break;
  }
}

void lval_println(lval v)
{
  lval_print(v);
  putchar('\n');
}

lval eval_op(lval x, char *op, lval y)
{
  if (x.type == LVAL_ERR)
    return x;
  if (y.type == LVAL_ERR)
    return y;
  if (strcmp(op, "+") == 0)
    return lval_num(x.num + y.num);
  if (strcmp(op, "-") == 0)
    return lval_num(x.num - y.num);
  if (strcmp(op, "*") == 0)
    return lval_num(x.num * y.num);
  /* 在mac silicon下用除0得到的结果的未定义的，但不会退出程序*/
  if (strcmp(op, "/") == 0)
    return y.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.num / y.num);
  return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t *t)
{
  if (strstr(t->tag, "number"))
  {
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
  }

  char *op = t->children[1]->contents;

  lval x = eval(t->children[2]);

  /* 下面的循环会使得操作符操作的对象是后面的所有元素，e.g. + 5 5 5,*/
  int i = 3;
  while (strstr(t->children[i]->tag, "expr"))
  {
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }
  return x;
}

int main(int argc, char **argv)
{
  /* Create Some Pasers */
  mpc_parser_t *Number = mpc_new("number");
  mpc_parser_t *Operator = mpc_new("operator");
  mpc_parser_t *Expr = mpc_new("expr");
  mpc_parser_t *Lispy = mpc_new("lispy");
  /* Language Define */
  mpca_lang(MPCA_LANG_DEFAULT,
            "                                            \
            number    :/-?[0-9]+/;                       \
            operator  :'+'|'-'|'*'|'/';                  \
            expr      :<number>|'('<operator><expr>+')';  \
            lispy     :/^/<operator><expr>+/$/;          \
            ",
            Number, Operator, Expr, Lispy);
  puts("Lispy version0.0.0.1\n");
  puts("Press Crtrl+C to Exit\n");
  while (1)
  {
    char *input = readline("Lispy>");
    add_history(input);
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r))
    {
      lval result = eval(r.output);
      lval_println(result);
      mpc_ast_delete(r.output);
    }
    else
    {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
    free(input);
  }
  mpc_cleanup(4, Number, Operator, Expr, Lispy);
  return 0;
}