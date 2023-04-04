#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <wait.h>

//void dieWithError(char *);

void greet(char myName) {
  printf("Process %c, PID %d greets you.\n",myName,getpid());
  fflush(stdout);
}

void goaway(char myName) {
  printf("Process %c exits normally\n", myName);
  exit(0);
}

int main() {
  int f1 = fork();
  int f2 = fork();

  if (f1 > 0 && f2 > 0) {
    int status;
    int status2;
    greet('A');
    waitpid(f2, &status, 0);
    printf("Process %c here: Process %c exited with status 0x%x\n", 'A', 'D', WEXITSTATUS(status)); //Mourns
    waitpid(f1, &status2, 0);
    printf("Process %c here: Process %c exited with status 0x%x\n", 'A', 'B', WEXITSTATUS(status2)); //Mourns
    goaway('A');
  }
  if (f1 == 0 && f2 > 0) {
    int status3;
    greet('B');
    waitpid(f2, &status3, 0);
    printf("Process %c here: Process %c exited with status 0x%x\n", 'B', 'C', WEXITSTATUS(status3)); //Mourns
    goaway('B');
  }
  if (f1 > 0 && f2 == 0) {
    greet('C');
    goaway('C');
  }
  if (f1 == 0 && f2 == 0) {
    greet('D');
    goaway('D');
  }

  return 0;
}
