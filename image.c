#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "image.h"

int image_fd;

int image_open(char *filename, int truncate){
    
    //set flags
    int flags = O_RDWR|O_CREAT;

    //check if we need to truncate
    if(truncate){
        flags |= O_TRUNC;
    }

    //open file with specific flags
    image_fd = open(filename, flags, 0600);
    if (image_fd == -1){
        return -1;
    }
    return 0;
}

int image_close(void){
    //check for successful close
    if (close(image_fd == -1)){
        return -1;
    }
    return 0;
}