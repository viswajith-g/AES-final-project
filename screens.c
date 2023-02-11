#include "screens.h"
#include "os.h"
#include "LCD.h"
#include "block.h"
#include <string.h>
#include "os.h"
#include "buzzer.h"

#define BGCOLOR     			LCD_BLACK
#define NUMBEROFINPUTS		5
int gameStart = 0; // Integer to tell when game is started (0 is not started, 1 is started) 
int gameOver = 0; // Integer to tell when game is over (0 is not over, 1 is over)
int secretInput = 0; // Integer to tell when the secret input is pushed (1 is pushed, 0 is not pushed)
int startScreenInit = 0;
int cheatScreenInit = 0;
int endScreenInit = 0;
int restartInput = 0;
Sema4Type ActiveScreen;
extern Sema4Type LCDFree;
extern int score;
int livesMultiplierFlag;
int cubeLifeFlag;
int joystickSpeedFlag;
int scoreMultiplierFlag;
int godModeFlag;
extern int selectFlag;
extern uint32_t ThreadNum;
extern int16_t highScore;
int16_t finalScore;

extern int scale;
extern int startBuzzer;

char cheatInput[NUMBEROFINPUTS];
int currentIndexForCheatInput = 0;

void clearCheats(void){
	livesMultiplierFlag = 0;
	cubeLifeFlag = 0;
	joystickSpeedFlag = 0;
	scoreMultiplierFlag = 0;
	godModeFlag = 0;
}

void startScreen(void){
		clearCheats();
    while(!gameStart && !gameOver){
				OS_bWait(&ActiveScreen);
        OS_bWait(&LCDFree);
			
				if (!startScreenInit){ // If start screen has not been run before
					BSP_LCD_FillScreen(BGCOLOR); // Clears screen 
					BSP_LCD_DrawRectOutline(LCD_GREEN); // Draws green outline

					// Writes text to screen
					BSP_LCD_DrawString(6, 1, "Team Zeta", LCD_WHITE);  
					BSP_LCD_DrawString(5, 2, "Presents...", LCD_WHITE); 
					if (!startBuzzer){
						PlayStartSound();
						startBuzzer = 1;
					}
					BSP_LCD_DrawString(5, 5, "Game-Z (TM)", LCD_WHITE);  
					BSP_LCD_DrawString(3, 9, "Press any button", LCD_WHITE);  
					BSP_LCD_DrawString(6, 10, "to start!", LCD_WHITE);

					// Draws pseudo crosshair and block, with s p e e d lines as well!
					BSP_LCD_DrawCrosshair(50, 72, LCD_GREEN);
					BSP_LCD_FillRect(80, 65, 15, 15, LCD_RED);
					BSP_LCD_DrawFastHLine(40, 69, 6, LCD_YELLOW);
					BSP_LCD_DrawFastHLine(35, 72, 10, LCD_YELLOW);
					BSP_LCD_DrawFastHLine(40, 75, 7, LCD_YELLOW);

					// sets startScreenInit to 1 to prevent this from running again in this viewing session
					startScreenInit = 1;
					selectFlag = 1;
				}
				
				// secretInput triggered when a certain button is hit
        if (secretInput){	
            OS_AddThread(cheatScreen, 128, 0); // Adds thread to go to cheatScreen
            secretInput = 0; // Resets secretInput, to be used on exiting cheatScreen so that the cheatScreen can be entered over and over again
					  startScreenInit = 0; // Sets startScreenInit to 0 to allow users to return to this screen later
						OS_bSignal(&ActiveScreen);	
						OS_bSignal(&LCDFree);
						OS_Suspend(); // Triggers this screen to switch threads and give up the semaphores to the cheat screen
        }
				else {
					OS_bSignal(&LCDFree);
					OS_bSignal(&ActiveScreen);
				}
    }
		if (gameStart){
			OS_bWait(&LCDFree);
			BSP_LCD_FillScreen(BGCOLOR); // Clears screen 
			OS_bSignal(&LCDFree);
		}
		startScreenInit = 0;
		// If you're here, then you've probably started the game, so we'll initialize all the functions required
		OS_Kill();
}


// This function handles everything on the cheat screen!
void cheatScreen(void){
		int arrayIterator; // Initializes array iterator variable, used to iterate through the cheatInput array
		cheatInput[NUMBEROFINPUTS-1] = '\0'; // Ends the cheatInput array with a null character so that strncmp can be used to check for which cheats are activated
		OS_bWait(&ActiveScreen); // Gets semaphore for the active screen
			while(!gameStart){ // Only allowed to run while the game is NOT running (i.e. at the start or end screen)
        OS_bWait(&LCDFree); // Gets LCDFree semaphore
			  if (!cheatScreenInit){ // If it is the first time initializing the cheatScreen (for this input session), runs the following
					BSP_LCD_FillScreen(BGCOLOR); // Clears the background
					BSP_LCD_DrawRectOutline(LCD_CYAN); // Adds a lovely cyan outline
					BSP_LCD_DrawString(5, 2, "Enter Cheat:", LCD_WHITE); // Text to tell the player to enter cheats

					// Draws horizontal underlines for the NUMBEROFINPUTS characters
					for (arrayIterator = 0; arrayIterator < NUMBEROFINPUTS-1; arrayIterator++){
						BSP_LCD_DrawFastHLine(35+arrayIterator*15, 55, 10, LCD_WHITE);
					}

					if (livesMultiplierFlag){
						BSP_LCD_DrawHeart(20,10,LCD_YELLOW); // Draws yellow heart on top left of screen to show cheat is enabled
					}
					if (cubeLifeFlag){
						BSP_LCD_DrawHourglass(40,10,LCD_YELLOW); // Draws yellow heart on top left of screen to show cheat is enabled
					}
					if (joystickSpeedFlag){
						BSP_LCD_DrawLightning(60,10,LCD_YELLOW); // Draws yellow heart on top left of screen to show cheat is enabled
					}
					if (scoreMultiplierFlag){
						BSP_LCD_Draw2x(13,1,LCD_YELLOW); // Draws yellow heart on top left of screen to show cheat is enabled
					}
					if (godModeFlag){
						BSP_LCD_DrawStar(105,10,LCD_YELLOW); // Draws yellow heart on top left of screen to show cheat is enabled
					}

						
					// Sets global varaible to 1 so button inputs know to input cheats into the array vs running their actions
					cheatScreenInit = 1;
				}

				// Draws every character within the array onto the screen
				// Draw any enabled cheats

					if (livesMultiplierFlag){
						BSP_LCD_DrawHeart(20,10,LCD_YELLOW); // Draws yellow heart on top left of screen to show cheat is enabled
					}
					else{
						BSP_LCD_DrawHeart(20,10,LCD_BLACK); // Draws yellow heart on top left of screen to show cheat is disabled
					}
					if (cubeLifeFlag){
						BSP_LCD_DrawHourglass(40,10,LCD_YELLOW); // Draws yellow heart on top left of screen to show cheat is enabled
					}
					else{
						BSP_LCD_DrawHourglass(40,10,LCD_BLACK); // Draws yellow heart on top left of screen to show cheat is disabled						
					}
					if (joystickSpeedFlag){
						BSP_LCD_DrawLightning(60,10,LCD_YELLOW); // Draws yellow heart on top left of screen to show cheat is enabled
					}
					else {
						BSP_LCD_DrawLightning(60,10,LCD_BLACK); // Draws yellow heart on top left of screen to show cheat is disabled
					}
					if (scoreMultiplierFlag){
						BSP_LCD_Draw2x(13,1,LCD_YELLOW); // Draws yellow heart on top left of screen to show cheat is enabled
					}
					else {
						BSP_LCD_Draw2x(13,1,LCD_BLACK); // Draws yellow heart on top left of screen to show cheat is disabled
					}
					if (godModeFlag){
						BSP_LCD_DrawStar(105,10,LCD_YELLOW); // Draws yellow heart on top left of screen to show cheat is enabled
					}
					else {
						BSP_LCD_DrawStar(105,10,LCD_BLACK); // Draws yellow heart on top left of screen to show cheat is disabled
					}
				// Displays current input
				for (arrayIterator = 0; arrayIterator < NUMBEROFINPUTS-1; arrayIterator++){
					BSP_LCD_DrawChar(35+arrayIterator*15, 38, cheatInput[arrayIterator], LCD_WHITE, BGCOLOR, 2); 
				}

//				BSP_LCD_Message(1, 1, 3, "Threads", ThreadNum); 
				
				// This is mainly used to clean up the screen after a cheat has been put in, does not impact the entering of the first cheat
				if (currentIndexForCheatInput == 1){
					for (arrayIterator = 1; arrayIterator < NUMBEROFINPUTS-1; arrayIterator++) { // Clears out array (but keeps the char that was already put in the array)
						cheatInput[arrayIterator] = ' ';
					}	
					BSP_LCD_DrawRectOutline(LCD_CYAN); // Resets outline to cyan color
					BSP_LCD_DrawString(2, 7, "Cheat Activated!", BGCOLOR); // Clears any text on the screen
					BSP_LCD_DrawString(4, 7, "Invalid Cheat!", BGCOLOR); // Clears any text on the screen
					BSP_LCD_DrawString(2, 7, "Cheat Deactivated!", BGCOLOR); // Clears any text on the screen
					BSP_LCD_DrawString(2, 7, "Time of ur life!", BGCOLOR);// If valid, then display affirmation message
				}
				
				// Once array is full, check to see if cheat is valid
				if (currentIndexForCheatInput == 4){
					if (strncmp(cheatInput, "AAAA", 4) == 0){ // Compares against known cheats
						PlayCheat1Sound();
						// TODO: ACTIVATE CHEAT HERE (By setting global var I hope)
						if (!livesMultiplierFlag)
						{
							lifeMultiplier = 2;
							BSP_LCD_DrawString(3, 7, "Cheat Activated!", LCD_GREEN);		// If valid, then display affirmation message
							BSP_LCD_DrawRectOutline(LCD_GREEN); // Outlines screen green
							livesMultiplierFlag = 1;				//More lives cheat
							selectFlag = 1;
						}
						else
						{
							lifeMultiplier = 1;
							BSP_LCD_DrawString(2, 7, "Cheat Deactivated!", LCD_YELLOW);		// If valid, then display affirmation message
							BSP_LCD_DrawRectOutline(LCD_YELLOW); // Outlines screen green
							livesMultiplierFlag = 0;
						}
						OS_Wait(&lifeFree);
						life = life*lifeMultiplier;
						OS_Signal(&lifeFree);
					}
					else if (strncmp(cheatInput, "AAAB", 4) == 0){ // Compares against known cheats
						PlayCheat2Sound();
						if(!cubeLifeFlag)
						{
							BSP_LCD_DrawString(3, 7, "Cheat Activated!", LCD_GREEN);			// If valid, then display affirmation message
							BSP_LCD_DrawRectOutline(LCD_GREEN); // Outlines screen green
							cubeLifeFlag = 1;				//cubes die slower		
							selectFlag = 1;
							// TODO: ACTIVATE CHEAT HERE (By setting global var I hope)
							cubeLifeMultiplier = 2;
						}
						else
						{
							BSP_LCD_DrawString(2, 7, "Cheat Deactivated!", LCD_YELLOW);		// If valid, then display affirmation message
							BSP_LCD_DrawRectOutline(LCD_YELLOW);
							cubeLifeMultiplier = 1;
							cubeLifeFlag = 0;
						}
					}
					else if (strncmp(cheatInput, "AABB", 4) == 0){ // Compares against known cheats
						PlayCheat3Sound();
						if(!joystickSpeedFlag){
							BSP_LCD_DrawString(2, 7, "Cheat Activated!", LCD_GREEN);		 // If valid, then display affirmation message
							BSP_LCD_DrawRectOutline(LCD_GREEN); // Outlines screen green
							joystickSpeedFlag = 1;			//joystick should move faster
							scale = 2;
						}
						else{
							BSP_LCD_DrawString(2, 7, "Cheat Deactivated!", LCD_YELLOW);		// If valid, then display affirmation message
							BSP_LCD_DrawRectOutline(LCD_YELLOW); // Outlines screen green
							joystickSpeedFlag = 0;				//More lives cheat
							scale = 1;
						}						
					}
					else if (strncmp(cheatInput, "BAAB", 4) == 0){ // Compares against known cheats
						PlayCheat4Sound();
						if (!scoreMultiplierFlag)
						{
							BSP_LCD_DrawString(3, 7, "Cheat Activated!", LCD_GREEN);			// If valid, then display affirmation message
							BSP_LCD_DrawRectOutline(LCD_GREEN); // Outlines screen green
							scoreMultiplierFlag = 1;			//score = 2 times actual score.
							selectFlag = 1;
							// TODO: ACTIVATE CHEAT HERE (By setting global var I hope)
							scoreMultiplier = 2;
						}
						else
						{
							BSP_LCD_DrawString(2, 7, "Cheat Deactivated!", LCD_YELLOW);		// If valid, then display affirmation message
							BSP_LCD_DrawRectOutline(LCD_YELLOW);
							scoreMultiplier = 1;
							scoreMultiplierFlag = 0;
						}
					}
					else if (strncmp(cheatInput, "ABBA", 4) == 0){ // Compares against known cheats
						PlayCheat1Sound();
						BSP_LCD_DrawString(3, 7, "Time of ur life!", LCD_GREEN);// If valid, then display affirmation message
						BSP_LCD_DrawRectOutline(LCD_GREEN); // Outlines screen green
						//get memed. 
					}
					else if (strncmp(cheatInput, "BBAB", 4) == 0){ // Compares against known cheats
						PlayCheat5Sound();
						if (!godModeFlag)
						{
							BSP_LCD_DrawString(3, 7, "Cheat Activated!", LCD_GREEN);	 // If valid, then display affirmation message
							BSP_LCD_DrawRectOutline(LCD_GREEN); // Outlines screen green
							godModeFlag = 1;
							selectFlag = 1;
							// TODO: ACTIVATE CHEAT HERE (By setting global var I hope)
							lifeMultiplier = 3;
							cubeLifeMultiplier = 3;
							scoreMultiplier = 10;
							scale = 2;
							
						  OS_Wait(&lifeFree);
							life = life*lifeMultiplier;
							OS_Signal(&lifeFree);
							
							//BSP_LCD_DrawStar(10,10,LCD_YELLOW); // Draws yellow heart on top left of screen to show cheat is enabled
							//extraLives = 1;
						}
						else
						{
							BSP_LCD_DrawString(2, 7, "Cheat Deactivated!", LCD_YELLOW);		// If valid, then display affirmation message
							BSP_LCD_DrawRectOutline(LCD_YELLOW);
							godModeFlag = 0;
							lifeMultiplier = 1;
							cubeLifeMultiplier = 1;
							scoreMultiplier = 1;
							scale = 1;
						}
					}
					else {
						PlayInvalidCheatSound();
						BSP_LCD_DrawString(2, 7, "Invalid Cheat!", LCD_RED); // If invalid, then display negatory message
						BSP_LCD_DrawRectOutline(LCD_RED); // Outlines screen red
					}
					selectFlag = 1;
					currentIndexForCheatInput = 0; // Resets cheat input index
				}
				
        if (secretInput){
						for (arrayIterator = 0; arrayIterator < NUMBEROFINPUTS-1; arrayIterator++) { // Clears out array
							cheatInput[arrayIterator] = ' ';
						}
						currentIndexForCheatInput = 0; // Resets cheat input index
            secretInput = 0; // Sets secretInput var to 0 to allow players to reenter this screen
					  cheatScreenInit = 0; // Sets cheatScreenInit to 0 to indicate that player is not in cheat menu anymore
						BSP_LCD_DrawString(5, 3, "Enter Cheat:", BGCOLOR); // Clears "Enter Cheat:" from screen
						OS_bSignal(&LCDFree); // Frees semaphores
					  OS_bSignal(&ActiveScreen);
            OS_Kill(); // Kills the thread
        }
				OS_bSignal(&LCDFree); // Frees semaphore for a second
    }
		OS_bSignal(&ActiveScreen); // Frees semaphore if game were to suddenly begin from the cheat screen somehow
		OS_Kill();
}

void gameOverScreen(void){
		// Triggers this screen to run when game is over		
		// Display game over and final score
		// Ask user to hit specific button to restart
		// Secret cheat input? Can add screen to task

	while(gameOver){
				OS_bWait(&ActiveScreen);
        OS_bWait(&LCDFree);

				if (!endScreenInit){ // If end screen has not been run before
					
					BSP_LCD_FillScreen(BGCOLOR); // Clears screen 
					BSP_LCD_DrawRectOutline(LCD_RED); // Draws red outline

					// Writes text to screen
					BSP_LCD_DrawString(6, 2, "GAME OVER", LCD_WHITE);
					finalScore = getCurrentScore();
					BSP_LCD_Message(0, 4, 5, "Score:", finalScore);
					if (finalScore > highScore){
							highScore = finalScore;
							BSP_LCD_DrawString(5, 7, "High Score!", LCD_WHITE); 
					}
					
					
					BSP_LCD_DrawString(4, 9, "Please Press B", LCD_WHITE);  
					BSP_LCD_DrawString(6, 10, "to Restart", LCD_WHITE);
					// sets endScreenInit to 1 to prevent this from running again in this viewing session
					endScreenInit = 1;
					PlayEndSound();
					
				}
				
				// secretInput triggered when a certain button is hit
        if (restartInput){					
            OS_AddThread(startScreen, 128, 5); // Adds thread to go to startScreen
            restartInput = 0; // Resets secretInput, to be used on exiting cheatScreen so that the cheatScreen can be entered over and over again
					  endScreenInit = 0; // Sets startScreenInit to 0 to allow users to return to this screen later
						gameOver = 0;
						gameStart = 0;
						startBuzzer = 0;
					  lifeMultiplier = 1;
					  scoreMultiplier = 1;
					  cubeLifeMultiplier = 1;
						selectFlag = 1;
						BlockArrayInit();
						clearCheats();
						OS_bSignal(&LCDFree);
						OS_bSignal(&ActiveScreen);
						OS_Kill(); // Triggers this screen to switch threads and give up the semaphores to the cheat screen
        }
				else {
					OS_bSignal(&LCDFree);
					OS_bSignal(&ActiveScreen);
				}

    }
		clearCheats();
		OS_Kill();
	
}






