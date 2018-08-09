
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

char *lsh_read_line();
char **lsh_split_line(char *line);
int lsh_launch(char **args);
int lsh_execute(char **args);

void lsh_loop();

int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[])(char **) = {
    &lsh_cd,
    &lsh_help,
    &lsh_exit
};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int main(int argc, char **argv) {
  // Load config files, if any

  // Run command loop
  lsh_loop();

  // Perform any shutdown/cleanup
  return EXIT_SUCCESS;
}

int lsh_cd(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}
int lsh_help(char **args) {
  int i;
  printf("Stephen Brennan's LSH\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < lsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}
int lsh_exit(char **args) {
  return 0;
}

void lsh_loop() {
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = lsh_read_line();
    args = lsh_split_line(line);
    status = lsh_execute(args);

    free(line);
    free(args);
  } while (status);
}

char *lsh_read_line() {
  int c;
  int bufsize = LSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  if (!buffer) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    c = getchar();

    if (c == '\n' || c == EOF) {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;
    if (position >= bufsize) {
      bufsize += LSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

char **lsh_split_line(char *line) {
  char **args;
  char *tok, *brkt;
  int i;
  int bufsize = LSH_TOK_BUFSIZE;

  args = malloc(bufsize * sizeof(char *));
  if (!args) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }
  for (i = 0, tok = strtok_r(line, LSH_TOK_DELIM, &brkt); tok;
       i++, tok = strtok_r(NULL, LSH_TOK_DELIM, &brkt)) {
    args[i] = tok;

    if (i >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      args = realloc(args, bufsize * sizeof(char *));
      if (!args) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
  args[i] = NULL;
  return args;
}

int lsh_execute(char **args) {
  int i;
  if (args[0] == NULL) {
    return 1;
  }
  for (i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }
  return lsh_launch(args);
}

int lsh_launch(char **args) {
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
  } else if (pid < 0) {
    perror("lsh");
  } else {
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  return 1;
}


