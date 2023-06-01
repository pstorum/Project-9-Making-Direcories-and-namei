#ifndef BLOCK_H
#define BLOCK_H

#include "image.h"
#include "free.h"

#ifndef BLOCK_SIZE
#define BLOCK_SIZE 4096
#endif
#define FREE_MAP_BLOCK 2
#define SET_FREE_ALLOCATED 1

unsigned char *bread(int block_num, unsigned char *block);
void bwrite(int block_num, unsigned char *block);
int alloc(void);

#endif