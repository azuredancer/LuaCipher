/*
The implementation of DES algorithm(http://www.itl.nist.gov/fipspubs/fip46-2.htm).

==============================================================================================
copyright 2011 Eric Zhang. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this list of
       conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice, this list
       of conditions and the following disclaimer in the documentation and/or other materials
       provided with the distribution.

THIS SOFTWARE IS PROVIDED BY ERIC ZHANG ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ERIC ZHANG OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of Eric Zhang. 
==============================================================================================
*/

#include <stdlib.h>
#include <string.h>

#include "des.h"

static int byte_map[8] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
static int byte_set0_map[8] = {0x7F,0xBF,0xDF,0xEF,0xF7,0xFB,0xFD,0xFE};
static int byte_set1_map[8] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

/* Permuted choice 1 table */
static int pc1_map[56] = {
    57,49,41,33,25,17,9,
    1, 58,50,42,34,26,18,
    10,2, 59,51,43,35,27,
    19,11,3, 60,52,44,36,
    63,55,47,39,31,23,15,
    7, 62,54,46,38,30,22,
    14,6, 61,53,45,37,29,
    21,13,5, 28,20,12,4
};

/* Permuted choice 2 table */
static int pc2_map[48] = {
    14,17,11,24,1, 5,
    3, 28,15,6, 21,10,
    23,19,12,4, 26,8,
    16,7, 27,20,13,2,
    41,52,31,37,47,55,
    30,40,51,45,33,48,
    44,49,39,56,34,53,
    46,42,50,36,29,32
};

/* Shift table */
static int key_shift_map[16] = {1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};

/* Initial permutation table */
static int ip_map[64] = {
    58,50,42,34,26,18,10,2,
    60,52,44,36,28,20,12,4,
    62,54,46,38,30,22,14,6,
    64,56,48,40,32,24,16,8,
    57,49,41,33,25,17,9, 1,
    59,51,43,35,27,19,11,3,
    61,53,45,37,29,21,13,5,
    63,55,47,39,31,23,15,7
};

/* Inverse of the initial permutation table */
static int _ip_map[64] = {
    40,8,48,16,56,24,64,32,
    39,7,47,15,55,23,63,31,
    38,6,46,14,54,22,62,30,
    37,5,45,13,53,21,61,29,
    36,4,44,12,52,20,60,28,
    35,3,43,11,51,19,59,27,
    34,2,42,10,50,18,58,26,
    33,1,41,9, 49,17,57,25
};

/* E bit selection table */
static int e_map[48] = {
    32,1, 2, 3, 4, 5,
    4, 5, 6, 7, 8, 9,
    8, 9, 10,11,12,13,
    12,13,14,15,16,17,
    16,17,18,19,20,21,
    20,21,22,23,24,25,
    24,25,26,27,28,29,
    28,29,30,31,32,1
};

/* P permutation table */
static int p_map[32] = {
    16,7,20,21,
    29,12,28,17,
    1, 15,23,26,
    5, 18,31,10,
    2, 8, 24,14,
    32,27,3, 9,
    19,13,30,6,
    22,11,4, 25
};

/* S permute choice table */
static int s_map[8][4][16] = {
    {
        {14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7},
        {0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8},
        {4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0},
        {15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13}
    },
    {
        {15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10},
        {3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5},
        {0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15},
        {13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9}
    },
    {
        {10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8},
        {13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1},
        {13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7},
        {1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12}
    },
    {
        {7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15},
        {13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9},
        {10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4},
        {3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14}
    },
    {
        {2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9},
        {14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6},
        {4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14},
        {11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3}
    },
    {
        {12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11},
        {10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8},
        {9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6},
        {4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13}
    },
    {
        {4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1},
        {13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6},
        {1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2},
        {6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12}
    },
    {
        {13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7},
        {1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2},
        {7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8},
        {2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11}
    }
};

static int get_bit(byte b, int pos) {
    return (b & byte_map[pos]) == 0x00 ? 0 : 1;
}

static void set_bit(byte *b, int pos, int value) {
    *b = (value == 0) ? *b & byte_set0_map[pos] : *b | byte_set1_map[pos];
}

static void mapping(const byte *in, byte *out, int *mtable, int size) {
    int i, j, bit, map_value, b, pos;
    
    for(i=0; i<size; i++) {
        for(j=0; j<8; j++) {
            map_value = mtable[i * 8 + j] - 1;
            b = map_value / 8;
            pos = map_value % 8;
            bit = get_bit(in[b], pos);
            set_bit(&out[i], j, bit);
        }
    }
}

/* Permuted choice 1 */
static void mapping_pc1(const byte *key_in, byte *key_out) {
    mapping(key_in, key_out, pc1_map, 7);
}

/* Permuted choice 2 */
static void mapping_pc2(const byte *key_in, byte *key_out) {
    mapping(key_in, key_out, pc2_map, 6);
}

/* Shifts */
static void shift_key(byte *key, int round) {
    int shift = key_shift_map[round];
    int bit1, bit2, bit3, bit4, i;

    if(shift == 1) {
        bit1 = get_bit(key[0], 0);
        bit2 = get_bit(key[3], 4);
        
        for(i=0; i<7; i++) {
            key[i] = key[i] << 1;
            if(i < 6) {
                set_bit(&key[i], 7, get_bit(key[i + 1], 0));
            }
        }

        set_bit(&key[3], 3, bit1);
        set_bit(&key[6], 7, bit2);
    } else if(shift == 2) {
        bit1 = get_bit(key[0], 0);
        bit2 = get_bit(key[0], 1);
        bit3 = get_bit(key[3], 4);
        bit4 = get_bit(key[3], 5);
        
        for(i=0; i<7; i++) {
            key[i] = key[i] << 2;
            if(i < 6) {
                set_bit(&key[i], 6, get_bit(key[i + 1], 0));
                set_bit(&key[i], 7, get_bit(key[i + 1], 1));
            }
        }

        set_bit(&key[3], 2, bit1);
        set_bit(&key[3], 3, bit2);
        set_bit(&key[6], 6, bit3);
        set_bit(&key[6], 7, bit4);
    }
}

/* Generate key schedule */
static void gen_key_schedule(const byte *key, byte schedule[][6]) {
    byte choice1_key[7];
    int i;
    
    mapping_pc1(key, choice1_key);
    for(i=0; i<16; i++) {
        shift_key(choice1_key, i);
        mapping_pc2(choice1_key, schedule[i]);
    }
}

/* Initial permutation */
static void mapping_ip(const byte *data_in, byte *data_out) {
    mapping(data_in, data_out, ip_map, 8);
}

/* Inverse of initial permutation */
static void mapping_ip_inverse(const byte *data_in, byte *data_out) {
    mapping(data_in, data_out, _ip_map, 8);
}

/* E bit selection */
static void mapping_e(const byte *data_in, byte *data_out) {
    mapping(data_in, data_out, e_map, 6);
}

/* P permutation */
static void mapping_p(const byte *data_in, byte *data_out) {
    mapping(data_in, data_out, p_map, 4);
}

/* S permute choice */
static void mapping_s(const byte *data_in, byte *data_out) {
    int i;
    byte row1, row2, col1, col2, col3, col4, row, col;
    byte out[8];

    for(i=0; i<8; i++) {
        row1 = get_bit(data_in[(i * 6) / 8], (i * 6) % 8);
        row2 = get_bit(data_in[(i * 6 + 5) / 8], (i * 6 + 5) % 8);
        col1 = get_bit(data_in[(i * 6 + 1) / 8], (i * 6 + 1) % 8);
        col2 = get_bit(data_in[(i * 6 + 2) / 8], (i * 6 + 2) % 8);
        col3 = get_bit(data_in[(i * 6 + 3) / 8], (i * 6 + 3) % 8);
        col4 = get_bit(data_in[(i * 6 + 4) / 8], (i * 6 + 4) % 8);

        row = (row1 << 1) | row2;
        col = (col1 << 3) | (col2 << 2) | (col3 << 1) | col4;

        out[i] = s_map[i][row][col];
    }

    for(i=0; i<4; i++) {
        data_out[i] = (out[i * 2] << 4) | out[i * 2 + 1];
    }
}

/* Cipher Function f */
static void f(const byte *r_in, byte *r_out, const byte *k) {
    byte medi1[6], medi2[4];
    int i;
    
    mapping_e(r_in, medi1);

    for(i=0; i<6; i++) {
        medi1[i] ^= k[i];
    }

    mapping_s(medi1, medi2);
    mapping_p(medi2, r_out);
}

/* Encrypt data block with 64bit */
static void enc_block(const byte *in, byte *out, byte schedule[][6]) {
    byte medi1[8] = "\x00\x00\x00\x00\x00\x00\x00\x00", medi2[4] = "\x00\x00\x00\x00";
    byte l0[4], r0[4], l1[4], r1[4];
    int i, j;
    
    mapping_ip(in, medi1);

    memcpy(l0, &medi1[0], 4);
    memcpy(r0, &medi1[4], 4);

    for(i=0; i<16; i++) {
        memcpy(l1, r0, 4);

        f(r0, medi2, schedule[i]);
        
        for(j=0; j<4; j++) {
            r1[j] = l0[j] ^ medi2[j];
        }
        
        memcpy(l0, l1, 4);
        memcpy(r0, r1, 4);
    }

    memcpy(&medi1[4], l0, 4);
    memcpy(&medi1[0], r0, 4);

    mapping_ip_inverse(medi1, out);
}

/* Decrypt data block with 64bit */
static void dec_block(const byte *in, byte *out, byte schedule[][6]) {
    byte medi1[8] = "\x00\x00\x00\x00\x00\x00\x00\x00", medi2[4] = "\x00\x00\x00\x00";
    byte l0[4], r0[4], l1[4], r1[4];
    int i, j;
    
    mapping_ip(in, medi1);

    memcpy(l0, &medi1[0], 4);
    memcpy(r0, &medi1[4], 4);

    for(i=15; i>=0; i--) {
        memcpy(l1, r0, 4);

        f(r0, medi2, schedule[i]);
        
        for(j=0; j<4; j++) {
            r1[j] = l0[j] ^ medi2[j];
        }
        
        memcpy(l0, l1, 4);
        memcpy(r0, r1, 4);
    }
   
    memcpy(&medi1[4], l0, 4);
    memcpy(&medi1[0], r0, 4);

    mapping_ip_inverse(medi1, out);
}

/* Encrypt(ECB mode) */
void des_ecb_encrypt(const byte *in, byte *out, int inl, const byte *key) {
    int block_nums = inl / 8;
    int tail_nums = inl % 8;
    int i;
    byte schedule[16][6];
    byte in_block[8];
    byte out_block[8];

    gen_key_schedule(key, schedule);

    for(i=0; i<block_nums; i++) {
        memcpy(in_block, &in[i * 8], 8);
        enc_block(in_block, out_block, schedule);
        memcpy(&out[i * 8], out_block, 8);
    }

    if(tail_nums > 0) {
        memcpy(in_block, "\x00\x00\x00\x00\x00\x00\x00\x00", 8);
        memcpy(in_block, &in[i * 8], tail_nums);
        enc_block(in_block, out_block, schedule);
        memcpy(&out[i * 8], out_block, 8);
    }
}

/* Decrypt(ECB mode) */
void des_ecb_decrypt(const byte *in, byte *out, int inl, const byte *key) {
    int block_nums = inl / 8;
    int i;
    byte schedule[16][6];
    byte in_block[8];
    byte out_block[8];

    gen_key_schedule(key, schedule);

    for(i=0; i<block_nums; i++) {
        memcpy(in_block, &in[i * 8], 8);
        dec_block(in_block, out_block, schedule);
        memcpy(&out[i * 8], out_block, 8);
    }
}
