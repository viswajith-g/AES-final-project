/*
* C file for pseudo random number generation
* Zeta group, RTOS Fall 2021
*/

#include "prand.h"

unsigned int lfsr32;
unsigned int lfsr31;

// shift values in lfsrs
int shift_lfsr(unsigned int *lfsr, unsigned int poly_mask)
{
	int feedback;
	
	feedback = *lfsr & 1;
	*lfsr >>= 1;
	if (feedback == 1){
		*lfsr ^= poly_mask;
	}
	return *lfsr;
}

// set seed values
void init_lfsrs(void)
{
	lfsr32 = 0xABCDE   ;
	lfsr31 = 0x23456789  ;
}

int get_random(void)
{
	// shift 32 bit lsfr twice, then xor with 31 bit lfsr
	// uses bottom 16 bits
	shift_lfsr(&lfsr32, POLY_MASK_32);
	return (shift_lfsr(&lfsr32, POLY_MASK_32) ^ shift_lfsr(&lfsr31, POLY_MASK_31)) & 0xffff;
}

unsigned int get_random_direction(void)
{
	unsigned int rand_num;
	unsigned int rand_dir;
	rand_num = (unsigned int) get_random();
	rand_dir = rand_num % 4;
	return rand_dir;
}

unsigned int get_random_bounce(int x)
{
	unsigned int rand_num;
	unsigned int rand_dir;
	rand_num = (unsigned int) get_random();
	rand_dir = ((rand_num % 3)+x+1)%4;
	return rand_dir;
}

unsigned int get_random_cube_num(void)
{
	unsigned int rand_num;
	unsigned int rand_block_num;
	rand_num = (unsigned int) get_random();
	rand_block_num = rand_num % 5;
	rand_block_num++;
	return rand_block_num;
}

unsigned int get_random_grid_dim(void)
{
	unsigned int rand_num;
	unsigned int rand_dim_num;
	rand_num = (unsigned int) get_random();
	rand_dim_num = rand_num % 6;
	return rand_dim_num;
}
