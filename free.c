#include "free.h"
#include <stdio.h>

int find_low_clear_bit(unsigned char x)
{
    for (int i = 0; i < 8; i++)
        if (!(x & (1 << i)))
            return i;

    return -1;
}

unsigned char* set_free(unsigned char *block, int num, int set)
{   
    //set the bytes and bits
    int byte_num = num / 8;
    int bit_num = num % 8;

    //depending if we want to allocate or deallocate we bitclear or bitset
    if (set == 1) {
        block[byte_num] |= (1 << bit_num);
    } else {
        block[byte_num] &= ~(1 << bit_num);
    }

    return block;
}

int find_free(unsigned char *block)
{   
    //loop through all blocks
    for (int byte_num = 0; byte_num < BLOCK_SIZE; byte_num++) {
        unsigned char byte = block[byte_num];
        //check if byte is full and if not, find lowest bit available
        if (byte != 0xFF) {
            int bit_num = find_low_clear_bit(byte);
            return byte_num * 8 + bit_num;
        }
    }

    return -1;
}