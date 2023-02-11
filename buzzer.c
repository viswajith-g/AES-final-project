#include <stdint.h>
#include "buzzer.h"
#include "tm4c123gh6pm.h"
#include "os.h"


/*
 Initialize a GPIO Pin for output 
 specifically- pin PF2 on launchpad;
 which is J4.40 on the booster pack
*/
void BSP_Buzzer_Init(void)
{
	SYSCTL_RCGCGPIO_R |= 0x00000020; // start the timer for port f
	while ((SYSCTL_PRGPIO_R & 0x20) != 0x20) {}; // wait until it's ready
		
	GPIO_PORTF_AMSEL_R &= ~0x04; //disable analog on PF2
	GPIO_PORTF_DIR_R |= 0x04; //make PF2 output
	GPIO_PORTF_AFSEL_R &= ~0x04; //disable alt function on PF2
	GPIO_PORTF_DEN_R |= 0x04; //enable digital I/O on PF2
}

/*
 Send a PWM signal of duty/100
 aka write 1 to PF2 duty% of the time
*/
void BSP_Buzzer_Output(unsigned int duty)
{
	int i;
	
	for (i = 0; i < 50; i++)
	{
		if (i < duty)
		{
			GPIO_PORTF_DATA_R |= 0x04;
		}
		else
		{
			GPIO_PORTF_DATA_R &= ~0x04;
		}
	}
}

void SoundBuzzer(unsigned int time, unsigned int duty)
{
	int i;
	for (i = 0; i < time; i++)
	{
		BSP_Buzzer_Output(duty);
	}
	BSP_Buzzer_Output(0);
}

void PlayStartSound(void)
{
	SoundBuzzer(100000, 35);
	SoundBuzzer(20000, 30);
	SoundBuzzer(20000, 43);
	SoundBuzzer(20000, 30);
	SoundBuzzer(20000, 35);
}

void PlayEndSound(void)
{
	SoundBuzzer(12000, 34);
	SoundBuzzer(12000, 30);
	SoundBuzzer(12000, 43);
	SoundBuzzer(15000, 46);
}

void PlayInvalidCheatSound(void)
{
	SoundBuzzer(5000, 15);
}

void PlayCheat1Sound(void)
{
	SoundBuzzer(5000, 30);
}

void PlayCheat2Sound(void)
{
	SoundBuzzer(5000, 34);
}

void PlayCheat3Sound(void)
{
	SoundBuzzer(10000, 43);
}

void PlayCheat4Sound(void)
{
	SoundBuzzer(10000, 46);
}

void PlayCheat5Sound(void)
{
	SoundBuzzer(10000, 35);
}

void PlayBlockKillSound(void){
	SoundBuzzer(500, 30);
	SoundBuzzer(500, 35);
}


void ChooseCheatSound(char* entered_cheat)
{
	return;
}

//void PlayBlockKillSound(void)
//{
//	SoundBuzzer(TIME_1MS*0.8, 10);
//}
