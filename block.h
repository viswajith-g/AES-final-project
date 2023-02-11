#ifndef _BLOCK_H
#define _BLOCK_H
#include "os.h"

#define VERTICALNUM 			6
#define HORIZONTALNUM 			6

#define LIFE_START              5     
#define LIFE_SPAN               25        
#define BLOCK_HEIGHT            18
#define BLOCK_WIDTH             21
//#define lifeMultiplier   				2
//#define cubeLifeMultiplier			2
#define joystickSpeedMultiplier	2
//#define scoreMultiplier					2


extern int cubeLifeMultiplier;
extern int lifeMultiplier;
extern int scoreMultiplier;
extern int16_t life;
extern Sema4Type lifeFree;

typedef struct {
 Sema4Type BlockFree; // > 0 is free, = 0 is busy
//	uint32_t x;
//	uint32_t y;
} block;

typedef struct{
 int16_t 		lifespan; // In ticks, or how many times the cube can move
 int16_t		id; // ID will be from 1 to 5
 int16_t 		color; // Color will be stored in hex
 int16_t 		x_location; // 0-6, splitting screen into a 6x6 box
 int16_t 		y_location; // 0-6
 int16_t 		hit; // 0 if not hit, 1 if hit
 int16_t 		direction; // 0-3, randomly generated
 int16_t        error;
} cube;



void BlockArrayInit(void);

int16_t addScore(void);

int16_t getCurrentScore(void);

int16_t decrementLife(void);

int16_t getCurrentLife(void);

int16_t GameStatus(void);

int16_t decrementActiveCubes(void);

int16_t incrementActiveCubes(void);

int16_t getActiveCubes(void);

int16_t getTimeLeft(void);

void initializeCube(cube* CubeToInit);

void CubeThread (void);

void CubeResurrection(void);


#endif

