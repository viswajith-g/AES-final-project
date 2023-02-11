#include "block.h"
#include "LCD.h"
#include "os.h"
#include "prand.h"
#include "screens.h"
#include "buzzer.h"

int16_t score;
int16_t life;
extern int gameOver;
Sema4Type scoreFree;
Sema4Type lifeFree;
Sema4Type activeCubesFree;
int16_t activeCubes;
int16_t timeLeft;
int16_t scoreAdd = 1;
int16_t numberOfCubesToReactivate; // Number of cubes to regenerate

extern cube* cubeArray[5];
extern block BlockArray[HORIZONTALNUM][VERTICALNUM];
extern Sema4Type LCDFree;
extern int livesMultiplierFlag;
extern int cubeLifeFlag;
extern int scoreMultiplierFlag;
extern int godModeFlag;

int lifeMultiplier = 1;
int cubeLifeMultiplier = 1;
int scoreMultiplier = 1;

int colors[8] = {LCD_BLUE, LCD_GREEN, LCD_ORANGE, LCD_CYAN, LCD_MAGENTA, LCD_YELLOW, LCD_WHITE, LCD_GREY};

void BlockArrayInit(void){
	int16_t X=0;
	int16_t Y=0;
	int16_t i=0;
  OS_InitSemaphore(&scoreFree, 1);
	OS_InitSemaphore(&lifeFree, 1);
	OS_InitSemaphore(&activeCubesFree, 1);
	
	
	score = 0;
	if (livesMultiplierFlag || godModeFlag){
		//life = LIFE_START * lifeMultiplier;
	}
	else life = LIFE_START*lifeMultiplier;
	
	activeCubes = 0;
	for (X = 0; X < VERTICALNUM; X++){
		for (Y = 0; Y < HORIZONTALNUM; Y++){
			OS_InitSemaphore(&BlockArray[X][Y].BlockFree, 1);
		}
	}
	for( i=0; i<5; i++){
		cubeArray[i] = 0;
	}
}


//------------------Score/Life Functions------------------
// functions to manipulate and retrieve the global score and life variables
int16_t addScore(){
	OS_Wait(&scoreFree);
	score = score + scoreAdd ;
	OS_Signal(&scoreFree);
	return 1;
}

int16_t getCurrentScore(){
		if (scoreMultiplierFlag || godModeFlag){
    return score * scoreMultiplier;
		}
		else return score;
}

int16_t decrementLife(){
	OS_Wait(&lifeFree);
	life--;
	OS_Signal(&lifeFree);
	return 1;
}

int16_t getCurrentLife(){
	return life;
}

int16_t GameStatus(){
	return (life > 0) && (gameOver == 0);
}

int16_t getTimeLeft(){
	return timeLeft;
}

//------------------Cube Movement--------------------------


// Decrements the count of active cubes on the screen
// Triggers whenever a cube is hit
int16_t decrementActiveCubes(){
	OS_Wait(&activeCubesFree);
	activeCubes--;
	OS_Signal(&activeCubesFree);
	return 1;
}

// Increments the count of active cubes on the screen
// Triggers whenever a cube is created
int16_t incrementActiveCubes(){
	OS_Wait(&activeCubesFree);
	activeCubes++;
	OS_Signal(&activeCubesFree);
	return 1;
}

// Returns the count of active cubes on the screen
// Triggers when checking how many cubes are on the screen
int16_t getActiveCubes(){
	return activeCubes;
}

// Initialization function for cubse
void initializeCube(cube* CubeToInit){
	int16_t desiredX, desiredY; // Creates desired X and Y variables 
	desiredX = get_random_grid_dim(); // Sets random X and Y variables
	desiredY = get_random_grid_dim();
	// Accessing the semaphore like this may not work...?
	while (BlockArray[desiredX][desiredY].BlockFree.Value < 1){ // Keeps generating random number until it finds a spot that is free
		desiredX = get_random_grid_dim(); // Generates a new X and Y location
		desiredY = get_random_grid_dim();
	}
	CubeToInit->x_location = desiredX; // If it gets here, then it has found a free location!
	CubeToInit->y_location = desiredY; // Sets the cube initial location the desiredX and desiredY
  OS_bWait(&BlockArray[CubeToInit->x_location][CubeToInit->y_location].BlockFree); // Gets semaphore to mark that location as occupied
	//BlockArray[CubeToInit->x_location][CubeToInit->y_location].BlockFree.Value = 0; 

	incrementActiveCubes(); // Increments active cubes counter
	CubeToInit->id = getActiveCubes(); // ID will range from 1 to 5 (in theory)
	cubeArray[CubeToInit->id-1] = CubeToInit;
	CubeToInit->hit = 0; // Sets hit variable to "NOT HIT"
	//if (cubeLifeFlag || godModeFlag){
	CubeToInit->lifespan = LIFE_SPAN*cubeLifeMultiplier; // sets lifespan for 25 "ticks" (So block can move 25 times before dying)
	timeLeft = LIFE_SPAN*cubeLifeMultiplier;
	//}
	//else CubeToInit->lifespan = LIFE_SPAN;
	CubeToInit->color = colors[get_random_grid_dim()]; // Gets random color
	CubeToInit->direction = get_random_direction(); // Gets random direction (0-3)
	CubeToInit->error = 0;
}

// This task implements the motions of the cubes
// Note: Each cube runs their own version of this thread
	// TASKS
// 1.allocate an idle cube for the object (DONE)
// 2.initialize color/shape and the first direction (DONE)
// 3.move the cube while it is not hit or expired (TODO)
void CubeThread (void){
	cube CurrentCube; // Creates a new cube object
	int16_t current_x;
	int16_t current_y;
	int count = 0;
	int sleepCt = 0;
	initializeCube(&CurrentCube);
	
	while(GameStatus()){ // Implement until the game is over		
		current_x = CurrentCube.x_location;
		current_y = CurrentCube.y_location;
		
		
		if(CurrentCube.hit == 1){
			//free part in cube array
			PlayBlockKillSound();
			addScore();
			break;
		} 
		else if (CurrentCube.lifespan <= 0){
			decrementLife();
			break;
		}
		
		count = 0;
		
		
		while( (current_x <= 0 && CurrentCube.direction == 1) ||
					 (current_y <= 0 && CurrentCube.direction == 0) ||
					 (current_x >= 5 && CurrentCube.direction == 2) ||
					 (current_y >= 5 && CurrentCube.direction == 3) )
		{
					 CurrentCube.direction = get_random_direction();
					 count++;
					 if(count> 100){
						 break;
					 }
		}
		
		//update direction
		switch (CurrentCube.direction)
		{
		case 0: //up
				CurrentCube.y_location--;
				break;
		
		case 1: //left
				CurrentCube.x_location--;
				break;
		
		case 2: //right
				CurrentCube.x_location++;
				break;
		
		case 3: //down
				CurrentCube.y_location++;
		}

		//error check for cube location
		if (CurrentCube.y_location < 0 || CurrentCube.y_location>5 || CurrentCube.x_location<0 || CurrentCube.x_location>5){
			CurrentCube.error++;
		}

		OS_bSignal(&BlockArray[current_x][current_y].BlockFree);
		OS_bWait(&BlockArray[CurrentCube.x_location][CurrentCube.y_location].BlockFree);
		OS_bWait(&LCDFree);
		BSP_LCD_FillRect(current_x*BLOCK_WIDTH, current_y*BLOCK_HEIGHT, BLOCK_WIDTH,BLOCK_HEIGHT, LCD_BLACK);
		BSP_LCD_FillRect(CurrentCube.x_location*BLOCK_WIDTH, CurrentCube.y_location*BLOCK_HEIGHT, BLOCK_WIDTH,BLOCK_HEIGHT, CurrentCube.color);
		OS_bSignal(&LCDFree);
		CurrentCube.lifespan--;
		if (CurrentCube.id == getActiveCubes()){
			timeLeft--;			//this works but stops count when we kill the cube with highest ID which is weird. activeCubes should change each time
											// a cube is killed. 
		}
		//OS_Sleep(250);
		sleepCt = 0;
		while(sleepCt <25){ //moving every 250 ms
			sleepCt++;
			if(CurrentCube.hit == 1){
					//free part in cube array
					break;
			} 
			if (!GameStatus()){
				CurrentCube.hit = 1;
				break;
			}
			OS_Sleep(10);
		}
	}
	cubeArray[CurrentCube.id-1] = 0;
	decrementActiveCubes();
	OS_bSignal(&BlockArray[CurrentCube.x_location][CurrentCube.y_location].BlockFree);
	OS_bWait(&LCDFree);
	BSP_LCD_FillRect(current_x*BLOCK_WIDTH, current_y*BLOCK_HEIGHT, BLOCK_WIDTH,BLOCK_HEIGHT, LCD_BLACK);
	OS_bSignal(&LCDFree);
	if(getActiveCubes() <=0 && getCurrentLife() > 0){ //put gameover Game over screen here
		CubeResurrection();
	}
	else if (getActiveCubes() <= 0 && getCurrentLife() <= 0 && !gameOver){
		gameOver = 1;
		OS_AddThread(&gameOverScreen, 128, 3);
	}
	OS_Kill(); // Cube should disappear, kill the thread
}

void CubeResurrection(void){ 
	// Regenerates 1-5 cubes when lives are left but no cubes are left
	int16_t cubeNumber; // Used in for loop to add the appropriate number of threads
	numberOfCubesToReactivate = get_random_cube_num(); // Generate a random number from 1-5
	for (cubeNumber = 1; cubeNumber <= numberOfCubesToReactivate; cubeNumber++){ // Loops from 1 and potentially up to 5
		OS_AddThread(&CubeThread, 8, 3); // MAYBE FIDDLE WITH THESE PARAMETER
	}
}
