#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <wait.h>

typedef void (*sighandler_t)(int);

// global variables
volatile sig_atomic_t sig1arrived = 0;
volatile sig_atomic_t sig2arrived = 0;
int userSigCount = 0;
int pidB, pidC, pidD;
int status, status2, status3;

// SIGUSR1 SIGHANDLER (Used for traveling from C -> B -> A -> D)
void sigusr1_handler() {
  sig1arrived = 1;
  fflush(stdout);
  }

// SIGUSR2 SIGHANDLER (used for traveling from D -> A -> B -> C)
void sigusr2_handler() {
  sig2arrived = 1;
  //printf("2\n");
  fflush(stdout);
  } // END SIGUSR2 SIGHANDLER

void greet(char myName) {
  printf("Process %c, PID %d greets you.\n",myName,getpid());
  fflush(stdout);
}

void goaway(char myName) {
  printf("Process %c exits normally\n", myName);
  fflush(stdout);
  exit(0);
}

int main() {
  sigset_t mask;
  sigfillset(&mask);
  sigdelset(&mask, SIGUSR1);
  sigdelset(&mask, SIGUSR2);

  // setup SIGUSR1
  struct sigaction sa1;
  sa1.sa_handler = sigusr1_handler;
  sa1.sa_flags = SA_RESTART;
  sigaction(SIGUSR1, &sa1, NULL);

  // setup SIGUSR2
  struct sigaction sa2;
  sa2.sa_handler = sigusr2_handler;
  sa2.sa_flags = SA_RESTART;
  sigaction(SIGUSR2, &sa2, NULL);

  greet('A');
  pidB = fork();
  if (pidB == 0) { //only b will run
    greet('B');
    pidC = fork();
    if (pidC == 0) { //only c will run
      greet('C');
      kill(getppid(), SIGUSR1); // send signal saying C greeted
      // MUST WAIT FOR D TO GREET BEFORE EXITING
      sigsuspend(&mask);
      sig2arrived = 0;
      //sigsuspend(&mask);
      kill(getppid(), SIGUSR1);
      goaway('C');
    }
    else {
      // B
      //sigsuspend(&mask);
      while(userSigCount < 3){
        sigsuspend(&mask);
        if (sig1arrived) {
          sig1arrived = 0;
          kill(getppid(),SIGUSR1); // Send to A
          userSigCount++;
        }
        else if (sig2arrived) {
          kill(pidC, SIGUSR2); // Send to C
          sig2arrived = 0;
          userSigCount++;
        }
      }
      
      wait(&status3);
      printf("Process %c here: Process %c exited with status 0x%x\n", 'B', 'C', WEXITSTATUS(status3)); //Mourns
      goaway('B');
    }
  }

  pidD = fork();
  if (pidD == 0) { // D

    // MUST WAIT FOR C TO CALL GREET
    //while (!sig1arrived){
    sigsuspend(&mask);
    //}
    sig1arrived = 0;
    greet('D');
    kill(getppid(), SIGUSR2); // send signal to A saying D greeted
    // MUST WAIT FOR C TO EXIT BEFORE EXITING
    sigsuspend(&mask);
    // while (!sig1arrived){
    //   sigsuspend(&mask);
    // }
    sig1arrived = 0;
    goaway('D');
    
  } 
  // Rest of A
  while(userSigCount < 3) {
      sigsuspend(&mask);
      if (sig1arrived) {
        sig1arrived = 0;
        kill(pidD,SIGUSR1); // Send to D
        userSigCount++;
      }
      else if (sig2arrived) {
        kill(pidB, SIGUSR2); // Send to B
        sig2arrived = 0;
        userSigCount++;
      }
  }
  wait(&status);
  printf("Process %c here: Process %c exited with status 0x%x\n", 'A', 'D', WEXITSTATUS(status)); //Mourns
  wait(&status2);
  printf("Process %c here: Process %c exited with status 0x%x\n", 'A', 'B', WEXITSTATUS(status2)); //Mourns
  goaway('A');
  return 0;
}