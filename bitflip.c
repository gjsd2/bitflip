#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

//Set up a global interrupt boolean so that we can quit at the next complete file
bool signal_interrupt = false;
void sig_handler(int signo) {
  if (signo == SIGINT)
    fprintf(stderr, "Exiting...\n");
    signal_interrupt = true;
}

int main(int argc, char *argv[]) {
  bool rename = false;
  int opt;
  if (signal(SIGINT, sig_handler) == SIG_ERR) {
    fprintf(stderr, "Error setting up SIGINT handler.\n");
    exit(EXIT_FAILURE);
  }

  while ((opt = getopt(argc, argv, "rh")) != -1) {
    switch (opt) {
    case 'r': rename = true; break;
    case 'h':
      fprintf(stderr, "Usage: %s [-r] [file...]\n", argv[0]);
      exit(EXIT_FAILURE);
      break;
    }
  }
  // optind should now contain a list of args, if < argc.

  while(true) {
    printf("Sleeping ...\n");
    printf("%i\n",sleep(10));
    if (signal_interrupt) {
      printf("Stopping now!\n");
      break;
    }
  }
  return EXIT_SUCCESS;
}
