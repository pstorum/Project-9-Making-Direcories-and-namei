#include <stdio.h>
#include <unistd.h>
#include "block.h"

unsigned char *bread(int block_num, unsigned char *block){
    //set the offset
    if (lseek(image_fd, block_num * BLOCK_SIZE, SEEK_SET) == -1) {
        return block;
    }

    //read data into block and return
    read(image_fd, block, BLOCK_SIZE);
    return block;
}

void bwrite(int block_num, unsigned char *block){
    //set the offset
    if (lseek(image_fd, block_num * BLOCK_SIZE, SEEK_SET) == -1) {
        return;
    }
    //write to image
    write(image_fd, block, BLOCK_SIZE);
}

int alloc(void){

    unsigned char free_map[BLOCK_SIZE];
    int block_num;

    //read in the block
    bread(FREE_MAP_BLOCK, free_map);

    //find free, if none are free return -1
    block_num = find_free(free_map);
    if (block_num == -1) {
        return -1;
    }

    //set as allocated and write
    set_free(free_map, block_num, SET_FREE_ALLOCATED);
    bwrite(FREE_MAP_BLOCK, free_map);

    return block_num;
}
