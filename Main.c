// Main.c
// Runs on LM4F120/TM4C123
// You may use, edit, run or distribute this file 
// You are free to change the syntax/organization of this file

// Jonathan W. Valvano 2/20/17, valvano@mail.utexas.edu
// Modified by Sile Shu 10/4/17, ss5de@virginia.edu
// Modified by Mustafa Hotaki 7/29/18, mkh3cf@virginia.edu

#include <stdint.h>
#include "OS.h"
#include "tm4c123gh6pm.h"
#include "LCD.h"
#include <string.h> 
#include "UART.h"
#include "FIFO.h"
#include "joystick.h"
#include "PORTE.h"
#include "block.h"
#include "LED.h"
#include "prand.h"
#include "screens.h"
#include "buzzer.h"

// Constants
#define BGCOLOR     			LCD_BLACK
#define CROSSSIZE            	5
#define PERIOD               	4000000   // DAS 20Hz sampling period in system time units
#define PSEUDOPERIOD         	8000000
#define LIFETIME             	1000
#define RUNLENGTH            	600 // 30 seconds run length
#define CROSS_WIDTH						4

cube* cubeArray[5];
block BlockArray[HORIZONTALNUM][VERTICALNUM];

extern Sema4Type LCDFree;
extern int gameOver;
extern int16_t life;
extern int16_t score;
extern int16_t currentLife;
int scale = 1;
extern Sema4Type ActiveScreen;

extern int cheatScreenInit;
extern char cheatInput[];
int startBuzzer;

extern int currentIndexForCheatInput;
extern int joystickSpeedFlag;
extern int godModeFlag;
extern int livesMultiplierFlag;
extern int cubeLifeFlag;
extern int scoreMultiplierFlag;
int16_t highScore;

extern int gameStart;
extern int secretInput;
extern int restartInput;
int selectFlag;

uint16_t origin[2]; 	// The original ADC value of x,y if the joystick is not touched, used as reference
int16_t x = 63;  			// horizontal position of the crosshair, initially 63
int16_t y = 63;  			// vertical position of the crosshair, initially 63
int16_t prevx, prevy;	// Previous x and y values of the crosshair
uint8_t select;  			// joystick push
uint8_t area[2];
uint32_t PseudoCount;

unsigned long NumCreated;   		// Number of foreground threads created
unsigned long NumSamples;   		// Incremented every ADC sample, in Producer
unsigned long UpdateWork;   		// Incremented every update on position values
unsigned long Calculation;  		// Incremented every cube number calculation
unsigned long DisplayCount; 		// Incremented every time the Display thread prints on LCD 
unsigned long ConsumerCount;		// Incremented every time the Consumer thread prints on LCD
unsigned long Button1RespTime; 		// Latency for Task 2 = Time between button1 push and response on LCD 
unsigned long Button2RespTime; 		// Latency for Task 7 = Time between button2 push and response on LCD
unsigned long Button1PushTime; 		// Time stamp for when button 1 was pushed
unsigned long Button2PushTime; 		// Time stamp for when button 2 was pushed

// DEBUG UTILITY
//#define DEBUG

//---------------------User debugging-----------------------
unsigned long DataLost;     // data sent by Producer, but not received by Consumer
long MaxJitter;             // largest time jitter between interrupts in usec
#define JITTERSIZE 64
unsigned long const JitterSize=JITTERSIZE;
unsigned long JitterHistogram[JITTERSIZE]={0,};
unsigned long TotalWithI1;
unsigned short MaxWithI1;

void Device_Init(void){
	UART_Init();
	BSP_LCD_OutputInit();
	BSP_Joystick_Init();
}


// background thread executed at 20 Hz
//******** Producer *************** 
int UpdatePosition(uint16_t rawx, uint16_t rawy, jsDataType* data){
	if (rawx > origin[0]){
		x = x + scale*((rawx - origin[0]) >> 9);
	}
	else{
		x = x - scale*((origin[0] - rawx) >> 9);
	}
	if (rawy < origin[1]){
		y = y + scale*((origin[1] - rawy) >> 9);
	}
	else{
		y = y - scale*((rawy - origin[1]) >> 9);
	}
	if (x > 127){
		x = 127;}
	if (x < 0){
		x = 0;}
	if (y > 112 - CROSSSIZE){
		y = 112 - CROSSSIZE;}
	if (y < 0){
		y = 0;}
	if (joystickSpeedFlag || godModeFlag){
		data->x = x+1; data->y = y+1;
	}
	else{
		data->x = x; data->y = y;
	}
	return 1;
}

void Producer(void){
	uint16_t rawX,rawY; // raw adc value
	uint8_t select;
	jsDataType data;
	unsigned static long LastTime;  // time at previous ADC sample
	unsigned long thisTime;         // time at current ADC sample
	long jitter;                    // time between measured and expected, in us
	if (GameStatus()){
		BSP_Joystick_Input(&rawX,&rawY,&select);
		thisTime = OS_Time();       // current time, 12.5 ns
		UpdateWork += UpdatePosition(rawX,rawY,&data); // calculation work
		NumSamples++;               // number of samples
		if(JsFifo_Put(data) == 0){ // send to consumer
			DataLost++;
		}
		
	//calculate jitter
		if(UpdateWork > 1){    // ignore timing of first interrupt
			unsigned long diff = OS_TimeDifference(LastTime,thisTime);
			if(diff > PERIOD){
				jitter = (diff-PERIOD+4)/8;  // in 0.1 usec
			}
			else{
				jitter = (PERIOD-diff+4)/8;  // in 0.1 usec
			}
			if(jitter > MaxJitter){
				MaxJitter = jitter; // in usec
			}       // jitter should be 0
			if(jitter >= JitterSize){
				jitter = JITTERSIZE-1;
			}
			JitterHistogram[jitter]++; 
		}
		LastTime = thisTime;
	}
	
	if (select == 0 && !gameStart){
			if (selectFlag){
					selectFlag = 0;
					if(!gameStart){ // If game hasn't started, then this will trigger the cheat screen
					secretInput = 1;
					}
				}
		}
}

// background thread executes with SW1 button
// one foreground task created with button push
// foreground treads run for 2 sec and die
// ***********ButtonWork*************
/*
void ButtonWork(void){
	uint32_t StartTime,CurrentTime,ElapsedTime;
	StartTime = OS_MsTime();
	ElapsedTime = 0;
	OS_bWait(&LCDFree);
	Button1RespTime = OS_MsTime() - Button1PushTime; // LCD Response here
	BSP_LCD_FillScreen(BGCOLOR);
	//Button1FuncTime = OS_MsTime() - Button1PushTime;
	//Button1PushTime = 0;
	while (ElapsedTime < LIFETIME){
		CurrentTime = OS_MsTime();
		ElapsedTime = CurrentTime - StartTime;
		BSP_LCD_Message(0,5,0,"Life Time:",LIFETIME);
		BSP_LCD_Message(1,0,0,"Horizontal Area:",area[0]);
		BSP_LCD_Message(1,1,0,"Vertical Area:",area[1]);
		BSP_LCD_Message(1,2,0,"Elapsed Time:",ElapsedTime);
		OS_Sleep(50);

	}
	BSP_LCD_FillScreen(BGCOLOR);
	OS_bSignal(&LCDFree);
  OS_Kill();  // done, OS does not return from a Kill
} 
*/

//************SW1Push*************
// Called when SW1 Button pushed
// Adds another foreground task
// background threads execute once and return

/*
void SW1Push(void){
	// THIS BOTTOM BIT WILL CHANGE
  if(!gameStart && !gameOver){ // If game hasn't started and is not over, then this will trigger the cheat screen
		secretInput = 1;
  }
	else if (gameOver){
		restartInput = 1;
	}
  else if (OS_MsTime() > 20 ){ // debounce
    if(OS_AddThread(&ButtonWork,128,4)){
			OS_ClearMsTime();
      NumCreated++; 
    }
    OS_ClearMsTime();  // at least 20ms between touches
		Button1PushTime = OS_MsTime(); // Time stamp

  }
}
*/

//--------------end of Task 2-----------------------------

//------------------Task 3--------------------------------

//******** Consumer *************** 
// foreground thread, accepts data from producer
// Display crosshair and its positions
// inputs:  none
// outputs: none
void Consumer(void){
	int i = 0;
	int xbox;
	int ybox;
	int xbox_upper, xbox_lower, ybox_lower, ybox_upper;
	while(1){
		if (GameStatus()){
			jsDataType data;
			JsFifo_Get(&data);
			OS_bWait(&LCDFree);
			if (gameStart){
			BSP_LCD_DrawCrosshair(prevx, prevy, LCD_BLACK); // Draw a black crosshair
			BSP_LCD_DrawCrosshair(data.x, data.y, LCD_RED); // Draw a red crosshair
			}
			ConsumerCount++;
			OS_bSignal(&LCDFree);
			prevx = data.x; 
			prevy = data.y;		
			
			//check for box hit //could see issues with this
			xbox = prevx / BLOCK_WIDTH;
			ybox = prevy / BLOCK_HEIGHT;
			xbox_lower = (prevx-CROSS_WIDTH)/BLOCK_WIDTH;
			xbox_upper = (prevx+CROSS_WIDTH)/BLOCK_WIDTH;
			ybox_lower = (prevy-CROSS_WIDTH)/BLOCK_HEIGHT;
			ybox_upper = (prevy+CROSS_WIDTH)/BLOCK_HEIGHT;
			i = 0;
			while( i< 5){
				if(cubeArray[i] != 0 ){
					if((cubeArray[i]->x_location == xbox && cubeArray[i]->y_location<=ybox_upper && cubeArray[i]->y_location>=ybox_lower)
							|| (cubeArray[i]->y_location == ybox && cubeArray[i]->x_location <= xbox_upper && cubeArray[i]->x_location >= xbox_lower) )
					{
						cubeArray[i]->hit = 1;
					}
				}
				i++;
			}
	  } 
		else{
//			OS_Kill();
						OS_Suspend();
		}
	// GAME OVER SCREEN IMPLEMENTATION

		/*
		if (getCurrentLife() == 1){
			decrementLife();
			gameOver = 1;
			OS_AddThread(&gameOverScreen, 128, 0); 
			OS_Kill();
		}
		*/
		//OS_Suspend();
	}

//  OS_Kill();  // done
}


//************ Display *************** 
// foreground thread, do some pseudo works to test if you can add multiple periodic threads
// inputs:  none
// outputs: none
void Display(void){
	while(1){
		if(GameStatus()){
			OS_bWait(&LCDFree);
			if (gameStart){
			BSP_LCD_Message(1, 4, 0, "Score:", getCurrentScore());
			BSP_LCD_Message(1, 5, 0, "Life: ", getCurrentLife());
			BSP_LCD_Message(1, 5, 12, "HS: ", highScore);
			}

			// Displays active powerups
			if (livesMultiplierFlag){
				BSP_LCD_DrawHeart(97,110, LCD_YELLOW);
			}
			if(cubeLifeFlag){
				BSP_LCD_DrawHourglass(117,110, LCD_YELLOW);
			}	
			if(joystickSpeedFlag){
				BSP_LCD_DrawLightning(107,110, LCD_YELLOW);
			}
			if(scoreMultiplierFlag){
				BSP_LCD_Draw2x(12,11, LCD_YELLOW);
			}
			if(godModeFlag){
				BSP_LCD_DrawStar(87,110, LCD_YELLOW);
			}
			DisplayCount++;
		
			OS_bSignal(&LCDFree);
		}
		else{
//			OS_Kill();
			OS_Suspend();
		}
	}
  //OS_Kill();  // done
}


void restartGame(void){
 	while(getActiveCubes() > 0){
		OS_Suspend();
	}
	clearCheats();			//to clear out icons because the cheats are not applied, and yet the icons are present.
	life = 5;
	score = 0;
	gameOver = 0;
	CubeResurrection();
}
void startGame(void){
  NumCreated = 0;
	
	NumCreated += OS_AddThread(&Consumer, 128, 1); 
	NumCreated += OS_AddThread(&Display, 128, 3);
	CubeResurrection();	
}

void resetGame(void){
	uint32_t StartTime,CurrentTime,ElapsedTime;
	gameOver = 1;
	life = 0;
	OS_Sleep(100); // wait
	StartTime = OS_MsTime();
	OS_bWait(&LCDFree);
	Button2RespTime = StartTime - Button2PushTime; // Response on LCD here
	BSP_LCD_FillScreen(BGCOLOR);
	while (ElapsedTime < 500){
		CurrentTime = OS_MsTime();
		ElapsedTime = CurrentTime - StartTime;
		BSP_LCD_DrawString(5,6,"Restarting",LCD_WHITE);
	}
	BSP_LCD_FillScreen(BGCOLOR);
	OS_bSignal(&LCDFree);
	// restart
	DataLost = 0;        // lost data between producer and consumer
  NumSamples = 0;
  UpdateWork = 0;
	MaxJitter = 0;       // in 1us units
	//x = 63; y = 63;
	restartGame();
  OS_Kill();  // done, OS does not return from a Kill
}


//------------------Task 7--------------------------------
// background thread executes with button2
// one foreground task created with button push
// ***********ButtonWork2*************
void Restart(void){
	resetGame();
}

// Called when SW1 Button pushed
// Adds another foreground task
// background threads execute once and return
void B1Push(void){
  if(!gameStart && !cheatScreenInit){ // If game hasn't started, then start the game here
	  gameStart = 1;
		BSP_LCD_FillScreen(BGCOLOR);
		startGame();
  }
	else if(cheatScreenInit){ // If on cheat input screen, add assigned character to array
		cheatInput[currentIndexForCheatInput] = 'A';
		currentIndexForCheatInput++;
	}
/*	else if (gameOver){
		restartInput = 1;
	}
*/
}

//************SW2Push*************
// Called when Button2 pushed
// Adds another foreground task
// background threads execute once and return
void B2Push(void){
  if(!gameStart && !cheatScreenInit){ // If game hasn't started, then start the game here
	  gameStart = 1;
		BSP_LCD_FillScreen(BGCOLOR);
		startGame();
  }
	else if(cheatScreenInit){ // If on cheat input screen, add assigned character to array
		cheatInput[currentIndexForCheatInput] = 'B';
		currentIndexForCheatInput++;
	}
	else if (gameOver){
		restartInput = 1;
	}
	
  else if (OS_MsTime() > 20 && gameStart && !gameOver){ // debounce
    if(OS_AddThread(&Restart,128,4)){
			OS_ClearMsTime();
      NumCreated++; 
    }
    OS_ClearMsTime();  // at least 20ms between touches
		Button2PushTime = OS_MsTime(); // Time stamp
  }
}
/*
void B3Push(void){
  if(!gameStart && !cheatScreenInit){ // If game hasn't started, then start the game here
	  gameStart = 1;
  }
	else if(cheatScreenInit){ // If on cheat input screen, add assigned character to array
		cheatInput[currentIndexForCheatInput] = 'X';
		currentIndexForCheatInput++;
	}
}

void B4Push(void){
  if(!gameStart && !cheatScreenInit){ // If game hasn't started, then start the game here
	  gameStart = 1;
  }
	else if(cheatScreenInit){ // If on cheat input screen, add assigned character to array
		cheatInput[currentIndexForCheatInput] = 'Y';
		currentIndexForCheatInput++;
	}
}
*/

// Fill the screen with the background color
// Grab initial joystick position to bu used as a reference
void CrossHair_Init(void){
	BSP_LCD_FillScreen(BGCOLOR);
	BSP_Joystick_Input(&origin[0],&origin[1],&select);
}

//******************* Main Function**********
int main(void){ //default main
  OS_Init();           // initialize, disable interrupts
  Device_Init();
	BSP_Buzzer_Init();
  CrossHair_Init();
  BlockArrayInit();
  DataLost = 0;        // lost data between producer and consumer
  NumSamples = 0;
	startBuzzer = 0;
  MaxJitter = 0;       // in 1us units
  PseudoCount = 0;
	selectFlag = 1;
	highScore = 0;
	
	// both semaphores init to 1 because initially both values are not used
	
	init_lfsrs();

//********initialize communication channels
  JsFifo_Init();

//*******attach background tasks***********
  OS_AddB1Task(&B1Push, 4);
  OS_AddB2Task(&B2Push, 4);
//	OS_AddB3Task(&B3Push, 4);
//	OS_AddB4Task(&B4Push, 4);
  OS_AddPeriodicThread(&Producer, PERIOD, 3); // 2 kHz real time sampling of PD3
	
	// Initializes ActiveScreen semaphore used in screens.c
	OS_InitSemaphore(&ActiveScreen, 1);
	
  NumCreated = 0 ;
// create initial foreground threads
  
  NumCreated += OS_AddThread(&startScreen, 128, 5); 
  OS_Launch(TIME_2MS); // doesn't return, interrupts enabled in here
	return 0;            // this never executes
}
