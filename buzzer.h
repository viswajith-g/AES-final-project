#ifndef _BUZZER_H
#define _BUZZER_H
/*
 Initialize a GPIO Pin for output 
 specifically- pin PF2 on launchpad;
 which is J4.40 on the booster pack
 Input: none
 Output: none
*/
void BSP_Buzzer_Init(void);

/*
 Send a PWM signal of duty cycle/100
 Input: duty cycle percentage
 Output: none, but should result in a buzzer sound
*/
void BSP_Buzzer_Output(unsigned int duty);

/*
 Sound the buzzer for time long at duty duty cycle
*/
void SoundBuzzer(unsigned int time, unsigned int duty);

/*
 Play the "Game Start" sound sequence
*/
void PlayStartSound(void);

/* 
 Play the "Game Over" sound sequence
*/
void PlayEndSound(void);

/*
 Play the "Invalid Cheat" sound
*/
void PlayInvalidCheatSound(void);

/*
 Play the "Cheat 1 applied" sound
*/
void PlayCheat1Sound(void);

/*
 Play the "Cheat 2 applied" sound
*/
void PlayCheat2Sound(void);

/* 
 Play the "Cheat 3 applied" sound
*/
void PlayCheat3Sound(void);

/*
 Play the "Cheat 4 applied" sound
*/
void PlayCheat4Sound(void);

/*
 Play the "Cheat 5 applied" sound
*/
void PlayCheat5Sound(void);

/*
 Play a specific cheat sound given what cheat was chosen
*/
void ChooseCheatSound(char* entered_cheat);

/*
 Play a "crosshair killed block" sound
*/
void PlayBlockKillSound(void);

#endif
