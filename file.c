#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  // Error on insufficient args
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <file_target>\n", argv[0]);
    return 1;
  }

  const char *file_target = argv[1];

  // Open file in binary mode to safely handle all byte values
  FILE *fp = fopen(file_target, "rb");
  if (!fp) {
    perror("Error: Unable to open file for reading");
    return 1;
  }

  // Determine byte count of file
  if (fseek(fp, 0, SEEK_END) != 0) {
    perror("Error: Unable to seek to end of file");
    fclose(fp);
    return 1;
  }

  long file_size = ftell(fp);

  if (file_size <= 0) {
    fprintf(stderr, "Error: File is empty or unreadable.\n");
    fclose(fp);
    return 1;
  }
  rewind(fp);

  // Allocate buffer and load file of arbitrary size into memory
  unsigned char *file_readin = malloc(file_size);

  if (!file_readin) {
    perror("Error: Memory allocation failed");
    fclose(fp);
    return 1;
  }

  size_t bytes_read = fread(file_readin, 1, file_size, fp);
  fclose(fp);

  if (bytes_read != (size_t)file_size) {
    fprintf(stderr, "Error: Unable to read complete file into memory.\n");
    free(file_readin);
    return 1;
  }

  // Seed PRNG (combining time and process ID for rapid runs) and pick offset
  srand((unsigned int)(time(NULL) ^ getpid()));
  long rand_num = rand() % file_size;

  // Reopen file in binary write mode (truncates existing file)
  fp = fopen(file_target, "wb");
  if (!fp) {
    perror("Error: Unable to open file for writing");
    free(file_readin);
    return 1;
  }

  // Write first half (bytes before rand_num)
  if (rand_num > 0) {
    fwrite(file_readin, 1, rand_num, fp);
  }

  // Write second half (bytes after rand_num, skipping 1 byte)
  long second_half_size = file_size - (rand_num + 1);
  if (second_half_size > 0) {
    fwrite(file_readin + rand_num + 1, 1, second_half_size, fp);
  }

  fclose(fp);
  free(file_readin);

  return 0;
}
