#include "../include/shell_common.h"

char *fix_filename(char *filename) {
  char first2[] = {filename[0], filename[1], '\0'};
  if (strcmp(first2, "./") == 0 || strcmp(first2, "..") == 0 ||
      filename[0] == '/') {
    return filename;
  } else {
    char *path = (char *)malloc(100);
    strcpy(path, "./");
    strcat(path, filename);
    return path;
  }
}