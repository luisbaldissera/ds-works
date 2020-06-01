#include "cat.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

// Prints out a file
void cat(char const *fname) {
    char buffer[512];
    int size = 512;
    int fd = open(fname, O_RDONLY);
    while (size == 512) {
        size = read(fd, buffer, 512);
        write(1, buffer, size);
    }
    close(fd);
}
