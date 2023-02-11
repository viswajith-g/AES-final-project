#ifndef _LED_H
#define _LED_H


#define OFF_LED     0 //nothing   
#define RED_LED     1 //red
#define BLUE_LED    2 //GReen
#define YELLOW_LED  3 //Red and green
//#define BLUE_LED    4 //Blue
#define PURPLE_LED  5 //Purple 101 BLUE red
#define CYAN_LED    6 //110 BLUe green
#define WHITE_LED   7 //white 


//RGB on 2,4,8 respectively
void led_init(void);

void led_red(int color);

#endif
