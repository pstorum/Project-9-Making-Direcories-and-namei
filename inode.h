#ifndef INODE_H
#define INODE_H

#include "free.h"
#include "block.h"
#include "image.h"
#include "pack.h"

#define INODE_MAP_BLOCK 1
#define NUM_INODES 1024
#define SET_ALLOCATED 1
#define INODE_SIZE 64
#define INODE_FIRST_BLOCK 3

#define MAX_SYS_OPEN_FILES 64

#define INODES_PER_BLOCK (BLOCK_SIZE/INODE_SIZE)

#define INODE_PTR_COUNT 16

#define BLOCK_PTR_SIZE 2

struct inode {
    unsigned int size;
    unsigned short owner_id;
    unsigned char permissions;
    unsigned char flags;
    unsigned char link_count;
    unsigned short block_ptr[INODE_PTR_COUNT];

    unsigned int ref_count;  // in-core only
    unsigned int inode_num;
};

struct inode *ialloc(void);
struct inode *find_incore_free(void);
struct inode *find_incore(unsigned int inode_num);
void read_inode(struct inode *in, int inode_num);
void write_inode(struct inode *in);
struct inode *iget(int inode_num);
void iput(struct inode *in);
void incore_reset(void);

#endif