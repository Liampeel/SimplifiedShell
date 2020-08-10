/***************************************************************************//**
  @file         shell.c
  @author       SC16LEP

*******************************************************************************/

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>



#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

/*
  Function Declarations for builtin shell commands:
 */
int cdCmd(char **args);
int helpCmd(char **args);
int exitCmd(char **args);
int infoCmd(char **args);
int pwdCmd(char **args);
int launchCmd(char **args);
int executeFunc(char **args);
void pipeFunction(char **args);
int lsCmd(char **args);
int redirectCmd(char **args, char* filePosition);


/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "info",
  "pwd",
  "ex",
  "exb",
  "ls",


};

int (*builtin_func[]) (char **) = {
  &cdCmd,
  &helpCmd,
  &exitCmd,
  &infoCmd,
  &pwdCmd,
  &launchCmd,
  &launchCmd,
  &lsCmd,




};

int numBuiltins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/

/**
   @brief Bultin command: change directory.
   @param args List of args.  args[0] is "cd".  args[1] is the directory.
   @return Always returns 1, to continue executing.
 */
int cdCmd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror(RED "error" RESET);
    }
  }
  return 1;
}

int helpCmd(char **args)
{
  int i;
  printf(RED "Liam Peel's Shell\n" RESET);
  printf(GRN "Type command names and arguments, and then hit enter.\n" RESET);
  printf(BLU "The following commands are built in:\n" RESET);

  for (i = 0; i < numBuiltins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int infoCmd(char **args)
{
  printf(MAG "COMP2211 Simplified Shell by SC16LEP\n" RESET);

  return 1;
}

 int pwdCmd(char **args)
{
char cwd[1024];
   if (getcwd(cwd, sizeof(cwd)) != NULL)
       fprintf(stdout, YEL "Current working dir: %s\n" RESET, cwd);
   else
       perror(RED "getcwd() error" RESET);
   return 1;
}


/**
   @brief Builtin command: exit.
   @param args List of args.  Not examined.
   @return Always returns 0, to terminate execution.
 */
int exitCmd(char **args)
{
  return 0;
}

int lsCmd(char **args)
{

  system("ls");

  return 1;
}



int redirectCmd(char **args, char* filePosition)
{
int pid;

pid = fork();
if (pid == 0)
{
  int fd = open(filePosition, O_CREAT | O_TRUNC | O_WRONLY, 0600);
  dup2(fd, STDOUT_FILENO);
  close(fd);
  execvp(args[0], args);
}
waitpid(pid,NULL,0);

return 1;
}


/**
  @brief Launch a program and wait for it to terminate.
  @param args Null terminated list of arguments (including program).
  @return Always returns 1, to continue execution.
 */
int launchCmd(char **args)
{
  pid_t pid;
  int status;
  bool background = false;
  int numOfArgs = 0;
  int i;

  if(strcmp(args[0],"exb"))
  {
    background = true;
  }


  for(i=1; args[i]!=NULL; i++)
  {
    numOfArgs += 1;
  }

  char **ignoreFArgs = malloc((numOfArgs+100)*sizeof(char));

  for(i=0; i<numOfArgs; i++)
  {
    ignoreFArgs[i] = malloc((256)*sizeof(char));
  }
  for(i=0; i<numOfArgs; i++)
  {
    ignoreFArgs[i] = args[i+1];
  }

  int executePositionForRedirect = 0;
  int filePosition = 0;
  bool redirect = false;
for(i = 0; i < numOfArgs; i++)
  if (strcmp(ignoreFArgs[i], ">")==0)
  {
    filePosition = i+1;
    executePositionForRedirect = i-1;
    redirect = true;

    break;
  }
  char **fileArgs = malloc((executePositionForRedirect+2)*sizeof(char));
  for(i=0; i<executePositionForRedirect+1; i++)
  {
    fileArgs[i] = malloc((256)*sizeof(char));
  }
  for(i=0; i<executePositionForRedirect+1; i++)
  {
    fileArgs[i] = ignoreFArgs[i];
  }

  if(redirect)
  {
    printf("running redirect\n" );
    redirectCmd(fileArgs, ignoreFArgs[filePosition]);;
  }

  if(!redirect)
  {
    pid = fork();
    if (pid == 0) {
      // Child process
      if (execvp(ignoreFArgs[0], ignoreFArgs) == -1) {
        perror(RED "error" RESET);
      }
      exit(EXIT_FAILURE);
    } else if (pid < 0) {
      // Error forking
      perror(RED "error" RESET);
    }
    else if(!background){
      // Parent process
      do {
        waitpid(pid, &status, WUNTRACED);
      } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    } else if(background){

    }

  }


  for (i = 0;  i < numOfArgs; i++)
    if (strcmp(ignoreFArgs[i],"|")==0)
    {
    pipeFunction(ignoreFArgs);
    }


  return 1;
}


/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
 */
int executeFunc(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < numBuiltins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }



  return launchCmd(args);
}

void pipeFunction(char **args)
{
int filedes[2];
int filedes2[2];

int num_cmds = 0;

char *command[256];

pid_t pid;

int err = -1;
int end = 0;


int i = 0;
int j = 0;
int k = 0;
int l = 0;


while (args[l] != NULL){
  if (strcmp(args[l],"|") == 0){
    num_cmds++;
  }
  l++;
}
num_cmds++;


while (args[j] != NULL && end != 1){
  k = 0;

  while (strcmp(args[j],"|") != 0){
    command[k] = args[j];
    j++;
    if (args[j] == NULL){

      end = 1;
      k++;
      break;
    }
    k++;
  }

  command[k] = NULL;
  j++;


  if (i % 2 != 0){
    pipe(filedes); // for odd i
  }else{
    pipe(filedes2); // for even i
  }

  pid=fork();

  if(pid==-1){
    if (i != num_cmds - 1){
      if (i % 2 != 0){
        close(filedes[1]); // for odd i
      }else{
        close(filedes2[1]); // for even i
      }
    }
    printf(RED "Child process could not be created\n" RESET);
    return;
  }
  if(pid==0){
    // If we are in the first command
    if (i == 0){
      dup2(filedes2[1], STDOUT_FILENO);
    }

    else if (i == num_cmds - 1){
      if (num_cmds % 2 != 0){ // for odd number of commands
        dup2(filedes[0],STDIN_FILENO);
      }else{ // for even number of commands
        dup2(filedes2[0],STDIN_FILENO);
      }

    }else{ // for odd i
      if (i % 2 != 0){
        dup2(filedes2[0],STDIN_FILENO);
        dup2(filedes[1],STDOUT_FILENO);
      }else{ // for even i
        dup2(filedes[0],STDIN_FILENO);
        dup2(filedes2[1],STDOUT_FILENO);
      }
    }

    if (execvp(command[0],command)==err){
      kill(getpid(),SIGTERM);
    }
  }

  // CLOSING DESCRIPTORS ON PARENT
  if (i == 0){
    close(filedes2[1]);
  }
  else if (i == num_cmds - 1){
    if (num_cmds % 2 != 0){
      close(filedes[0]);
    }else{
      close(filedes2[0]);
    }
  }else{
    if (i % 2 != 0){
      close(filedes2[0]);
      close(filedes[1]);
    }else{
      close(filedes[0]);
      close(filedes2[1]);
    }
  }

  waitpid(pid,NULL,0);

  i++;
}
}


#define rlBufsize 1024
/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
 */
char *readlineFunc(void)
{
  int bufsize = rlBufsize;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, RED "allocation error\n" RESET);
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += rlBufsize;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, RED "allocation error\n" RESET);
        exit(EXIT_FAILURE);
      }
    }
  }
}



#define tokBufsize 64
#define tokDelim " \t\r\n\a"
/**
   @brief Split a line into tokens (very naively).
   @param line The line.
   @return Null-terminated array of tokens.
 */
char **splitlineFunc(char *line)
{
  int bufsize = tokBufsize, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, RED "allocation error\n" RESET);
    exit(EXIT_FAILURE);
  }

  token = strtok(line, tokDelim);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += tokBufsize;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
		free(tokens_backup);
        fprintf(stderr, RED "allocation error\n" RED);
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, tokDelim);
  }
  tokens[position] = NULL;
  return tokens;
}

/**
   @brief Loop getting input and executing it.
 */
void loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = readlineFunc();
    args = splitlineFunc(line);
    status = executeFunc(args);

    free(line);
    free(args);
  } while (status);
}

/**
   @brief Main entry point.
   @param argc Argument count.
   @param argv Argument vector.
   @return status code
 */
int main(int argc, char **argv)
{

  // Run command loop.
  loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}
