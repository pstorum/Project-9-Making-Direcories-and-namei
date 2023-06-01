#include "inode.h"
#include <stdio.h>
#include <string.h>

static struct inode incore[MAX_SYS_OPEN_FILES] = {0};

struct inode *ialloc(void){

    unsigned char inode_map[BLOCK_SIZE];
    int inode_num;
    
    //read in the block
    bread(INODE_MAP_BLOCK, inode_map);
    
    //find free, if none are free return NULL
    inode_num = find_free(inode_map);
    if (inode_num == -1) {
        return NULL;
    }

    //get a free inode from iget
    struct inode *incore_inode = iget(inode_num);
    if(incore_inode == NULL){
        return NULL;
    }

    //init inode
    incore_inode->size = 0;
    incore_inode->owner_id = 0;
    incore_inode->flags = 0;
    incore_inode->permissions = 0;

    for(int i = 0; i<INODE_PTR_COUNT; i++){
        incore_inode->block_ptr[i] = 0;
    }
    incore_inode->inode_num = inode_num;
    //set as allocated and write
    set_free(inode_map, inode_num, SET_ALLOCATED);    
    bwrite(INODE_MAP_BLOCK, inode_map);

    return incore_inode;
}

struct inode *find_incore_free(void){
    //loop through inodes, if incore.ref_count == zero then the inode is not in use
    //we return the unused inode
    for(int i = 0; i<MAX_SYS_OPEN_FILES; i++){
        if(incore[i].ref_count == 0){
            return incore+i;
        }
    }
    return NULL;
}
struct inode *find_incore(unsigned int inode_num){
    //loop through inodes, if incore.inode_num == the passed in indode_num
    //we found the right inode and can return it
    for(int i = 0; i<MAX_SYS_OPEN_FILES; i++){
        if(incore[i].inode_num == inode_num){
            return incore+i;
        }
    }
    return NULL;
}

void read_inode(struct inode *in, int inode_num){
    //find offset in bytes
    int block_num = inode_num / INODES_PER_BLOCK + INODE_FIRST_BLOCK;
    int block_offset = inode_num % INODES_PER_BLOCK;
    int block_offset_bytes = block_offset * INODE_SIZE;
    
    //get block from disk
    unsigned char block[BLOCK_SIZE];
    bread(block_num, block);

    //get data from the block
    //set data in the inode passed in
    in->size = read_u32(block+block_offset_bytes+0);
    in->owner_id = read_u16(block+block_offset_bytes+4);
    in->permissions = read_u8(block+block_offset_bytes+6);
    in->flags = read_u8(block+block_offset_bytes+7);
    in->link_count = read_u8(block+block_offset_bytes+8);

    for(int x = 0; x < INODE_PTR_COUNT; x++){
        in->block_ptr[x] = read_u16(block+block_offset_bytes+(9+(x*BLOCK_PTR_SIZE)));
    }

}
void write_inode(struct inode *in){
    //get inode num and find the offset in bytes
    int inode_num = in->inode_num;
    int block_num = inode_num / INODES_PER_BLOCK + INODE_FIRST_BLOCK;
    int block_offset = inode_num % INODES_PER_BLOCK;
    int block_offset_bytes = block_offset * INODE_SIZE;

    //get block from disk
    unsigned char block[BLOCK_SIZE];
    bread(block_num, block);

    //update block values
    write_u32(block+block_offset_bytes+0, in->size);
    write_u16(block+block_offset_bytes+4, in->owner_id);
    write_u8(block+block_offset_bytes+6, in->permissions);
    write_u8(block+block_offset_bytes+7, in->flags);
    write_u8(block+block_offset_bytes+8, in->link_count);

    for(int x = 0; x < INODE_PTR_COUNT; x++){
        write_u16(block+block_offset_bytes+(9+(x*BLOCK_PTR_SIZE)), in->block_ptr[x]);
    }

    //write block back to disk
    bwrite(block_num, block);
}

struct inode *iget(int inode_num){
    struct inode *find_node = find_incore(inode_num);
    if (find_node != NULL){
        find_node->ref_count++;
        return find_node;
    }

    struct inode *free_inode = find_incore_free();
    if (free_inode == NULL){
        return NULL;
    }
    read_inode(free_inode, inode_num);
    free_inode->ref_count = 1;
    free_inode->inode_num = inode_num;
    return free_inode;
}

void iput(struct inode *in){
    if(in->ref_count == 0){
        return;
    }

    in->ref_count--;

    if(in->ref_count == 0){
        write_inode(in);
    }
}

//function for reseting incore to init for testing purposes
void incore_reset(void){
    memset(&incore, 0, sizeof(incore));
}