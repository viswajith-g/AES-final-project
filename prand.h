/*
* Header file for pseudo random number generation
* Zeta group, RTOS Fall 2021
*/
#ifndef _PRAND_H_
#define _PRAND_H_


#define POLY_MASK_32 0xB4BCD35C
#define POLY_MASK_31 0x7A5BC2E3

int shift_lfsr(unsigned int *lfsr, unsigned int poly_mask);

void init_lfsrs(void);

int get_random(void);

// function to get a random number [0,3]
unsigned int get_random_direction(void);

// function to get a random number [1, 5]
unsigned int get_random_cube_num(void);

// function to get a random number [0, 5]
unsigned int get_random_grid_dim(void);

unsigned int get_random_bounce(int x);
#endif
