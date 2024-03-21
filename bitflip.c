#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

typedef enum {
  NO_MODE,
  STEALTH_MODE,
  SEEK_MODE,
  RENAME_MODE,
  TRANSFER_MODE
} crypt_mode_t;

//Set up a global interrupt boolean so that we can quit at the next complete file
bool signal_interrupt = false;
void sig_handler(int signo) {
  if (signo == SIGINT)
    fprintf(stderr, "Exiting...\n");
    signal_interrupt = true;
}

//Debug printer for char arrays
void hexprint_array(char a[], int len) {
  for ( int i = 0; i < len; i++) { printf("%02hhX", a[i]); }
  printf("\n");
}

//Where the 'encryption' happens.
void encrypt_buffer(char buffer[], int buflen) {
  for ( int i = 0 ; i < buflen ; i++) {
    buffer[i] = ~buffer[i];
  }
}

//Execute an encryption action on a single file, varying by mode.
int encrypt_file(crypt_mode_t mode, char *filename) {
  fprintf(stderr, "Working on: %s\n", filename);
//  newfilename = filename
  FILE* fp_r;
  FILE* fp_w;
  int num_r;
  int num_w;
  char buf[2097152];
  switch (mode) {
    case SEEK_MODE:
      fprintf(stderr, "Seek mode detected ...\n");
      fp_w = fopen(filename, "rb+");
      if (fp_w == NULL) return EXIT_FAILURE;
      while (!feof(fp_w)) {
        num_r = fread(buf, sizeof(buf), 1, fp_w);
        hexprint_array(buf, sizeof(buf)/sizeof(buf[0]));
/*        encrypt_buffer(buf, sizeof(buf)/sizeof(buf[0]));
        fseek(fp_w, -num_r, SEEK_CUR);
        num_w = fwrite(buf, sizeof(buf), 1, fp_w);
        if (num_r != num_w ) {
          fprintf(stderr, "Mismatched read and write!\n");
        }*/
      }
    break;
    case STEALTH_MODE:
      fprintf(stderr, "Stealth mode detected ...\n");
      fp_r = fopen(filename, "rb");
/*      if (fp_r == NULL) return EXIT_FAILURE;
      fp_w = fopen(filename, "wb");
      if (fp_w == NULL) return EXIT_FAILURE;
      while (!feof(fp_r)) {
        fread(buf, sizeof(buf), 1, fp_r);
        encrypt_buffer(buf, sizeof(buf));
        fwrite(buf, sizeof(buf), 1, fp_w);
      }*/
    break;
    case RENAME_MODE:
      fprintf(stderr, "Rename mode detected ...\n");
    break;
    case TRANSFER_MODE:
      fprintf(stderr, "Transfer mode detected ...\n");
    break;
  }

  return EXIT_SUCCESS;
}

//Traverse a specified directory, preventing interruptions except between files.
int encrypt_dir(crypt_mode_t mode, char dir[], int length) {
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

//Handle exiting with help nicely after every check
void help_exit(char *name) {
  fprintf(stderr, "Usage: %s [-rstk] [directory]\n", name);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  crypt_mode_t mode = NO_MODE;
  int opt;
  if (signal(SIGINT, sig_handler) == SIG_ERR) {
    fprintf(stderr, "Error setting up SIGINT handler.\n");
    exit(EXIT_FAILURE);
  }

  while ((opt = getopt(argc, argv, "skrth")) != -1) {
    switch (opt) {
    case 's': mode = STEALTH_MODE; break;
    case 'k': mode = SEEK_MODE; break;
    case 'r': mode = RENAME_MODE; break;
    case 't': mode = TRANSFER_MODE; break;
    case 'h': help_exit(argv[0]);
    }
  }
  if ( mode == NO_MODE) help_exit(argv[0]);
  if ( optind == argc) help_exit(argv[0]);

//  char buf[] = { 8,16 };
//  printf("Initial: "); hexprint_array(buf, sizeof(buf)/sizeof(buf[0]));
//  encrypt_buffer(buf, sizeof(buf)/sizeof(buf[0]));
//  printf("Final: "); hexprint_array(buf, sizeof(buf)/sizeof(buf[0]));

  return encrypt_file(mode, argv[optind]);
//  return encrypt_dir(mode, argv[optind]);
}
