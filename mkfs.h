#ifndef MKFS_H
#define MKFS_H

#include "image.h"
#include "block.h"
#include "inode.h"

#define NUM_BLOCKS 1024
#define NUM_ALLOC_BLOCKS 7
#define DIRECTORY_FLAG 2
#define INIT_DIRECTORY_SIZE 64
#define DIRECTORY_ENTRY_SIZE 32
#define FILE_NAME_OFFSET 2
#define ROOT_INODE_NUM 0

struct directory {
    struct inode *inode;
    unsigned int offset;
};

struct directory_entry {
    unsigned int inode_num;
    char name[16];
};

void mkfs(void);
struct directory *directory_open(int inode_num);
int directory_get(struct directory *dir, struct directory_entry *ent);
void directory_close(struct directory *d);

//removed "char *path" for now since the instructions return root directory for now
//leaving it cause build warning for unused param
struct inode *namei();
int directory_make(char *path);

#endif