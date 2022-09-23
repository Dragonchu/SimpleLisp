#include <stdio.h>
#include <stdlib.h>

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
int main(int argc, char **argv)
{
  puts("Lispy version0.0.0.1\n");
  puts("Press Crtrl+C to Exit\n");
  while (1)
  {
    char *input = readline("Lispy>");
    add_history(input);
    printf("now you are a %s\n", input);
    free(input);
  }
  return 0;
}