#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

typedef void (*sighandler_t)(int);

#define LIMIT 20
#define PERIOD 5

/* global variables for communication between handlers and main */
int sigalarm_count = 0;
int sigint_count = 0;

// SIGALARM SIGHANDLER
sighandler_t sigalarm_handler(int signum, sighandler_t handler) {
  sigset_t unblock, blockAll, oldMask;

  // i) block all signals
  sigfillset(&blockAll); // all signal bits set */
  sigemptyset(&unblock); // no signal bits set
  sigprocmask(SIG_BLOCK, &blockAll, &oldMask); /* block everything */


  // ii) increment sigalarm_count
  sigalarm_count = sigalarm_count + 1;

  // iii) call to reset alarm
  alarm(PERIOD);

  /// iv) restore the original set of blocked signals
  sigprocmask(SIG_SETMASK, &oldMask, NULL);
  
} // END SIGALARM SIGHANDLER

// SIGINT SIGHANDLER
sighandler_t sigint_handler(int signum, sighandler_t handler){
  sigset_t unblock, blockAll, oldMask;

  // i) block all signals
  sigfillset(&blockAll); // all signal bits set */
  sigemptyset(&unblock); // no signal bits set
  sigprocmask(SIG_BLOCK, &blockAll, &oldMask); /* block everything */

  // ii) increment sigint_count
  sigint_count = sigint_count + 1;

  /// iii) restore the original set of blocked signals
  sigprocmask(SIG_SETMASK, &oldMask, NULL);
  
} // END SIGINT SIGHANDLER

int main() {
  int f1 = fork();
  // Parent
  if (f1 > 0) {
    // Setup a signal handler for SIGALRM and SIGINT
    struct sigaction sa1;
    sa1.sa_handler = &sigalarm_handler;
    sa1.sa_flags = SA_RESTART;
    sigaction(SIGALRM, &sa1, NULL);

    struct sigaction sa2;
    sa2.sa_handler = &sigint_handler;
    sa2.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa2, NULL);

    // setting alarm
    alarm(PERIOD);
    sigset_t mask;
    sigemptyset(&mask);

    while ((sigint_count <= LIMIT) && (sigalarm_count <= LIMIT)) {   
      printf(".\n");
      sigsuspend(&mask);
    };
    printf("%i %i\n", sigalarm_count, sigint_count);
  } // End Parent

   //Child
  if (f1 == 0) {
    pid_t parentID = getppid();
    getchar();
    for (int i = 0; i < LIMIT; i++) {
      kill(parentID, SIGINT);
    }
    printf("Child: finished sending %i %i times\n", SIGINT, LIMIT);
  } // End Child

 return 0;
}
