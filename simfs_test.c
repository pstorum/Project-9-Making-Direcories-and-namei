#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ctest.h"
#include "image.h"
#include "block.h"
#include "mkfs.h"
#include "inode.h"
#include "free.h"
#include "pack.h"
#include "ls.h"

#define BLOCK_SIZE 4096
#define NUM_BLOCKS 1024
#define NUM_ALLOC_BLOCKS 7
#define BITS_PER_BYTE 8
#define INODE_BLOCK 1
#define FREE_BLOCK 2
#define DIRECTORY_BLOCK_NUM 7

#ifdef CTEST_ENABLE


void init_image_fd(void){
    char buffer[BLOCK_SIZE] = {0};
    memset(buffer, 0, sizeof(buffer));
    for(int i = 0; i < NUM_BLOCKS; i++){
        write(image_fd, buffer, sizeof(buffer));
    }
}
//testing image.c
void test_image_open(void){
    char *filename = "test_file";
    int truncate = 1;
    CTEST_ASSERT(image_open(filename, truncate) == 0, "Testing successful file open");
}

void test_image_close(void){
    CTEST_ASSERT(image_close() == 0, "Testing successful file close");
    CTEST_ASSERT(image_close() == -1, "Testing unsuccessful file close");
}

//testing block.c
void test_bread(void){
    char *filename = "test_file";
    int truncate = 1;
    CTEST_ASSERT(image_open(filename, truncate) == 0, "Testing successful file open");
    init_image_fd();
    char buffer[BLOCK_SIZE] = {0};
    unsigned char block[BLOCK_SIZE];
    bread(1, block);
    CTEST_ASSERT(memcmp(buffer, block, BLOCK_SIZE) == 0, "Test if block 1 is filled with all zero's");
    CTEST_ASSERT(image_close() == 0, "Testing successful file close");
}

void test_bwrite(void){
    //image setup
    char *filename = "test_file";
    int truncate = 1;
    CTEST_ASSERT(image_open(filename, truncate) == 0, "Testing successful file open");
    init_image_fd();

    char ones_block[BLOCK_SIZE] = {1};
    unsigned char block[BLOCK_SIZE] = {1};
    unsigned char get_block[BLOCK_SIZE];

    //test successful write.
    bwrite(1, block);
    bread(1, get_block);
    CTEST_ASSERT(memcmp(ones_block, get_block, BLOCK_SIZE) == 0, "Test if block 1 has been written with all 1's");    
    
    //test file close
    CTEST_ASSERT(image_close() == 0, "Testing successful file close");
}

void test_alloc(void){
    //image setup
    char *filename = "test_file";
    int truncate = 1;
    image_open(filename, truncate);
    init_image_fd();

    unsigned char block[BLOCK_SIZE];

    //everything is zero, we can assume first bit to be allocate will be zero
    alloc();
    bread(FREE_BLOCK, block);
    int byte_num = 0 / 8;
    int bit_num = 0 % 8;
    int get_value = (block[byte_num] >> bit_num) & 1;
    CTEST_ASSERT(get_value == 1, "Testing successful of 1st alloc");
    // test for second run of alloc()
    alloc();
    bread(FREE_BLOCK, block);
    byte_num = 1 / 8;
    bit_num = 1 % 8;
    get_value = (block[byte_num] >> bit_num) & 1;
    CTEST_ASSERT(get_value == 1, "Testing successful of 2nd alloc");

    //test of no availible space for allocation
    int alloc_return_value;
    for(int x = 0; x< BITS_PER_BYTE*BLOCK_SIZE; x++){
        alloc_return_value = alloc();
    }
    CTEST_ASSERT(alloc_return_value == -1, "Testing successful of alloc on full block");


    image_close();

}

//testing free.c
void test_set_free(void){
    //image setup
    char *filename = "test_file";
    int truncate = 1;
    image_open(filename, truncate);
    init_image_fd();
    unsigned char block[BLOCK_SIZE];
    bread(1, block);

    //file empty, set any of them
    set_free(block, 6, 1);
    int byte_num = 6 / 8;
    int bit_num = 6 % 8;
    int get_value = (block[byte_num] >> bit_num) & 1;
    CTEST_ASSERT(get_value == 1, "Testing successful of set free");

    //if bit is already 1, should be able to overwrite without issue
    set_free(block, 6, 1);
    get_value = (block[byte_num] >> bit_num) & 1;
    CTEST_ASSERT(get_value == 1, "Testing successful of set free overwrite");

    //free the bit
    set_free(block, 6, 0);
    get_value = (block[byte_num] >> bit_num) & 1;
    CTEST_ASSERT(get_value == 0, "Testing successful of freeing a bit");

    image_close();
}

void test_find_free(void){
    //image setup
    char *filename = "test_file";
    int truncate = 1;
    image_open(filename, truncate);
    init_image_fd();
    unsigned char block[BLOCK_SIZE];
    bread(1, block);

    //find first byte, file is empty should return a 0.
    int byte1 = find_free(block);
    CTEST_ASSERT(byte1 == 0, "Test successful of finding first free");

    //find another byte, when the previous byte has been set as not free
    set_free(block, byte1, 1);
    int byte2 = find_free(block);
    CTEST_ASSERT(byte2 == 1, "Test successful of finding second free");

    image_close();
}

//testing inode.c
void test_ialloc(void){
    //image setup
    char *filename = "test_file";
    int truncate = 1;
    image_open(filename, truncate);
    init_image_fd();
    incore_reset();
    //inode map is empty, inode_num of first returned inode should be zero
    struct inode *test_node = ialloc();
    CTEST_ASSERT(test_node->inode_num == 0, "Testing that the first returned inode has a inode_num value of zero");

    //test init values are correct
    CTEST_ASSERT(test_node->size == 0, "Testing that the inode init value of size is correct");
    CTEST_ASSERT(test_node->owner_id == 0, "Testing that the inode init value of owner_id is correct");   
    CTEST_ASSERT(test_node->flags == 0, "Testing that the inode init value of flags is correct");   
    CTEST_ASSERT(test_node->permissions == 0, "Testing that the inode init value of permissions is correct");
    CTEST_ASSERT(test_node->ref_count == 1, "Testing that the inode init value of ref_count is correct");       
    
    //inode map has one inode, inode_num of second returned inode should be one
    struct inode *test_node_two = ialloc();
    CTEST_ASSERT(test_node_two->inode_num == 1, "Testing that the second returned inode has a inode_num value of one");
    image_close();
}

//testing mkfs.c
void test_mkfs(void){
    //image setup
    char *filename = "test_file";
    int truncate = 1;
    image_open(filename, truncate);
    incore_reset();
    mkfs();

    //check block 2 for correct allocation
    unsigned char free_block[BLOCK_SIZE];
    bread(FREE_BLOCK, free_block);
    int first_seven_allocated = 0;
    for(int i = 0; i <NUM_ALLOC_BLOCKS; i++){
        int byte_num = i / 8;
        int bit_num = i % 8;
        int get_value = (free_block[byte_num] >> bit_num) & 1;
        if(get_value != 1){
            first_seven_allocated = 1;
            break;
        }
    }
    CTEST_ASSERT(first_seven_allocated == 0, "Testing Block 2 has first 7 allocated");

    //test if the first slot of the inode block is used
    unsigned char inode_block_map[BLOCK_SIZE];
    bread(INODE_BLOCK, inode_block_map);
    int byte_num = 0 / 8;
    int bit_num = 0 % 8;
    int get_value = (inode_block_map[byte_num] >> bit_num) & 1;
    CTEST_ASSERT(get_value == 1, "Testing successful setting of inode map");

    //test if the values of the first inode are correct.
    struct inode *free_inode = find_incore_free();
    int directory_inode_num = 0;
    
    read_inode(free_inode, directory_inode_num);
    CTEST_ASSERT(free_inode->size == 64, "Testing successful reading of directory size");
    CTEST_ASSERT(free_inode->flags == 2, "Testing successful reading of directory flags");
    CTEST_ASSERT(free_inode->block_ptr[0] == 7, "Testing successful reading of directory block_ptr[0]");
    
    //Testing successful writing of directory data
    unsigned char directory_data_block[BLOCK_SIZE];
    bread(7, directory_data_block);
    unsigned char expected_directory_data_block[BLOCK_SIZE];
    //block 8 should be empty
    bread(8, expected_directory_data_block);
    

    write_u16(expected_directory_data_block+0, 0);
    strcpy((char*)expected_directory_data_block+2, ".");
    write_u16(expected_directory_data_block+32, 0);
    strcpy((char*)expected_directory_data_block+34, "..");
    
    CTEST_ASSERT(memcmp(directory_data_block, expected_directory_data_block, BLOCK_SIZE) == 0, "Testing successful writing of directory data");

    //Test directory_open
    struct directory *dir = directory_open(0);
    CTEST_ASSERT(dir->inode->size == 64, "Testing successful opening of directory by directory inode size");
    CTEST_ASSERT(dir->inode->flags == 2, "Testing successful opening of directory by directory inode flags");
    CTEST_ASSERT(dir->offset == 0, "Testing successful opening of directory by checking init offset");

    //Test directory_get
    struct directory_entry ent;

    int success_direct_get = directory_get(dir, &ent);
    CTEST_ASSERT(success_direct_get == 0, "Testing successful direct_get checking return value");
    CTEST_ASSERT(dir->offset == 32, "Testing successful direct_get by checking offset increment");
    CTEST_ASSERT(strcmp(ent.name, ".") == 0, "Testing successful direct_get by checking entry name");
    CTEST_ASSERT(ent.inode_num == 0, "Testing successful direct_get by checking entry inode_num");

    success_direct_get = directory_get(dir, &ent);
    success_direct_get = directory_get(dir, &ent);
    CTEST_ASSERT(success_direct_get == -1, "Testing offset out of bounds");

    //test directory_close
    //unsure how to test
    // iput() is already tested, and free removes dir from memory
    directory_close(dir);

    image_close();
}

void test_find_incore(void){
    //find 1st free, set values, find_incore based on set value
    struct inode *test_node = find_incore_free();
    CTEST_ASSERT(test_node->ref_count == 0, "Testing find free");
    test_node->inode_num = 5;
    test_node->size = 2;
    test_node->link_count = 3;

    struct inode *test_find_node = find_incore(5);
    CTEST_ASSERT(test_find_node->size == 2, "Testing the found node for correct size value");
    CTEST_ASSERT(test_find_node->link_count == 3, "Testing the found node for correct link_count value");
    
    //find 2nd free, set values, find_incore based on set value
    struct inode *test_node_2 = find_incore_free();
    CTEST_ASSERT(test_node_2->ref_count == 0, "Testing find free");
    test_node_2->inode_num = 6;
    test_node_2->size = 7;
    test_node_2->link_count = 8;

    struct inode *test_find_node_2 = find_incore(6);
    CTEST_ASSERT(test_find_node_2->size == 7, "Testing the found node for correct size value");
    CTEST_ASSERT(test_find_node_2->link_count == 8, "Testing the found node for correct link_count value");
    
    //find inode that doesn't exist
    struct inode *test_find_node_3 = find_incore(3);
    CTEST_ASSERT(test_find_node_3 == NULL, "Testing, found no inode");

    //test when incore is full.
    struct inode *full = find_incore_free();
    for(int i = 0; i<=MAX_SYS_OPEN_FILES; i++){
        full = find_incore_free();
        if(full == NULL){
            break;
        }
        full->ref_count += 1;
    }
    CTEST_ASSERT(full == NULL, "Testing, found no free incore inode");
    incore_reset();

}
void test_read_write_inode(void){
    //image setup
    char *filename = "test_file";
    int truncate = 1;
    image_open(filename, truncate);
    init_image_fd();
    incore_reset();
    //inode setup for writing
    struct inode *test_node = find_incore_free();
    test_node->size = 1;
    test_node->owner_id = 2;
    test_node->permissions = 3;
    test_node->flags = 4;
    test_node->link_count = 5;
    test_node->block_ptr[0] = 7;
    test_node->block_ptr[1] = 8;
    test_node->inode_num = 120;

    //write the inode
    write_inode(test_node);

    //get the inode we just wrote to image
    struct inode *test_read_node = find_incore_free();
    int inode_num = 120;
    
    read_inode(test_read_node, inode_num);
    CTEST_ASSERT(test_read_node->size == 1, "Testing successful reading and previous writing of size");
    CTEST_ASSERT(test_read_node->owner_id == 2, "Testing successful reading and previous writing of owner_id");
    CTEST_ASSERT(test_read_node->permissions == 3, "Testing successful reading and previous writing of permissions");
    CTEST_ASSERT(test_read_node->flags == 4, "Testing successful reading and previous writing of flags");
    CTEST_ASSERT(test_read_node->link_count == 5, "Testing successful reading and previous writing of link_count");
    CTEST_ASSERT(test_read_node->block_ptr[0] == 7, "Testing successful reading and previous writing of block_ptr 0");
    CTEST_ASSERT(test_read_node->block_ptr[1] == 8, "Testing successful reading and previous writing of block_ptr 1");

    image_close();
}

void test_iget_iput(void){
    incore_reset();
    //tests for initial and repeated iget() on same inode_num
    struct inode *test_node = iget(120);
    CTEST_ASSERT(test_node->ref_count == 1, "Testing sucssesful returning of an inode from iget");
    test_node = iget(120);
    CTEST_ASSERT(test_node->ref_count == 2, "Testing sucssesful returning of same inode from iget");

    //test for iput() on test_node
    iput(test_node);
    CTEST_ASSERT(test_node->ref_count == 1, "Testing sucssesful returning of an inode from iput");

    //test for iput on inode block that hasn't been initialized
    struct inode *test_node_two = find_incore_free();
    read_inode(test_node_two, 6);
    iput(test_node_two);
    CTEST_ASSERT(test_node_two->inode_num == 0, "Testing iput when given an inode_num that doesn't exist");
    
}

void test_ls(void){
    //image setup
    incore_reset();
    char *filename = "test_file";
    int truncate = 1;
    image_open(filename, truncate);
    
    mkfs();
    ls();

    image_close();
}

void new_dir_test(void){
    incore_reset();
    char *filename = "test_file";
    int truncate = 1;
    image_open(filename, truncate);
    
    mkfs();

    int bad_dir = directory_make("foo");
    CTEST_ASSERT(bad_dir == -1, "Testing for bad directory name");

    directory_make("/foo");


    struct directory *dir;
    struct directory_entry ent;

    dir = directory_open(0);

    directory_get(dir, &ent);
    directory_get(dir, &ent);
    directory_get(dir, &ent);
    directory_close(dir);


    CTEST_ASSERT(ent.inode_num == 1, "Testing correct new directory inode num");
    CTEST_ASSERT(strcmp(ent.name,"foo") == 0, "Testing correct new directory name");

    ls();

    image_close(); 
}


int main(void){
    CTEST_VERBOSE(1);

    test_image_open();
    test_image_close();
    test_bread();
    test_bwrite();
    test_set_free();
    test_find_free();

    test_ialloc();
    test_alloc();
    test_mkfs();
    
    test_find_incore();
    test_read_write_inode();
    test_iget_iput();

    new_dir_test();
    //test_ls();
        
    CTEST_RESULTS();

    CTEST_EXIT();

}
#else

int main(void)
{
    char *filename = "test_file";
    int truncate = 1;
    image_open(filename, truncate);
    image_close();
}

#endif