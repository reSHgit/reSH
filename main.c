/***************************************************************************//**

  @file         main.c

  @author       iH8Ra1n

  @date         Monday, October 9th, 2017

  @brief        reSH (REdone SHell)

*******************************************************************************/

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

/*
  Function Declarations for builtin shell commands:
 */
int reSH_cd(char **args);
int reSH_help(char **args);
int reSH_exit(char **args);
int reSH_cRaShThEsHeLl(char **args);
int reSH_eNaBlEdEbUgMoDe(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
int debugModeEnabled;
void sigstop(int p){
    printf("");
}
char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "cRaShThEsHeLl",
  "eNaBlEdEbUgMoDe"
};

int (*builtin_func[]) (char **) = {
  &reSH_cd,
  &reSH_help,
  &reSH_exit,
  &reSH_cRaShThEsHeLl,
  &reSH_eNaBlEdEbUgMoDe
};

int reSH_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int reSH_eNaBlEdEbUgMoDe(char **args) 
{
    debugModeEnabled = 1;
    return 1;
}

/*
  Builtin function implementations.
*/

/**
   @brief Bultin command: change directory.
   @param args List of args.  args[0] is "cd".  args[1] is the directory.
   @return Always returns 1, to continue executing.
 */
int reSH_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "reSH: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("reSH");
    }
  }
  return 1;
}
int reSH_cRaShThEsHeLl(char **args)
{
    char buffer[10];
    strcpy(buffer, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
}

/**
   @brief Builtin command: print help.
   @param args List of args.  Not examined.
   @return Always returns 1, to continue executing.
 */
int reSH_help(char **args)
{
  int i;
  printf("iH8Ra1n's reSH\n");
  printf("It is used like many other shells. Simply type a program name and arguments, and it will open. \n");
  printf("The following are built in:\n");

  for (i = 0; i < reSH_num_builtins() - 2; i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

/**
   @brief Builtin command: exit.
   @param args List of args.  Not examined.
   @return Always returns 0, to terminate execution.
 */
int reSH_exit(char **args)
{
  return 0;
}

/**
  @brief Launch a program and wait for it to terminate.
  @param args Null terminated list of arguments (including program).
  @return Always returns 1, to continue execution.
 */
int reSH_launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      printf("reSH: error: command not found\n");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    printf("reSH: error: command not found\n");
    perror("reSH");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
 */
int reSH_execute(char **args)
{
  int i;

    if (debugModeEnabled == 1) {
        system("printf 'sHeLl VeRsIoN: b1\nuNaMe -A oUtPuT: '; uname -a; printf 'tImE: '; date");
    }
    
  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < reSH_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return reSH_launch(args);
}

#define reSH_RL_BUFSIZE 1024
char *reSH_read_line(void)
{
  int bufsize = reSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "reSH: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    c = getchar();
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;
    if (position >= bufsize) {
      bufsize += reSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "reSH: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

#define reSH_TOK_BUFSIZE 64
#define reSH_TOK_DELIM " \t\r\n\a"
char **reSH_split_line(char *line)
{
  int bufsize = reSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "reSH: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, reSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += reSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "reSH: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, reSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}
void reSH_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("reSH $ ");
    line = reSH_read_line();
    args = reSH_split_line(line);
    status = reSH_execute(args);

    free(line);
    free(args);
  } while (status);
}
void intHandler(int dummy) {
    int jidnahfdifhdhdh = 0;
}
int main(int argc, char **argv)
{
  // Load config files, if any.

  // Run command loop.
    
  signal(SIGINT, intHandler);
  signal(SIGTSTP, intHandler);
  reSH_loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}
