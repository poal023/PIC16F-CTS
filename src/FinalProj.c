/*
* File:  newmain.c
* Author: lej990
*
* Created on October 9, 2020, 10:51 AM
*/

// PIC16F1829 Configuration Bit Settings
// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include "i2c.h"
#include "i2c_LCD.h"

// CONFIG1
#pragma config FOSC = INTOSC  // Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF    // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF   // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON    // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF     // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF    // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = OFF   // Brown-out Reset Enable (Brown-out Reset disabled)
#pragma config CLKOUTEN = OFF  // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = OFF    // Internal/External Switchover (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF   // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled)

// CONFIG2
#pragma config WRT = OFF    // Flash Memory Self-Write Protection (Write protection off)
#pragma config PLLEN = OFF   // PLL Enable (4x PLL disabled)
#pragma config STVREN = ON   // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO    // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LVP = ON    // Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)




/*Serial Configuration*/
#define BAUD 9600  //Bits per second transfer rate
#define FOSC 4000000L  //Frequency Oscillator
#define DIVIDER ((int)(FOSC/(16UL * BAUD) -1)) //Should be 25 for 9600/4MhZ
#define NINE_BITS 0
#define SPEED 0x4    //T Speed
#define RX_PIN TRISC5  //Recieve Pin
#define TX_PIN TRISC4  //Transmit Pin
#define I2C_SLAVE 0x27 //1E Channel

/*Xtal Macro*/
#define _XTAL_FREQ 4000000.0  /*for 4mhz*/

//Function Prototypes
void timer_config(void);
void clockAndpin_config(void);
void Display (int, unsigned char *, int *);
void I2C_LCD_Command(unsigned char,unsigned char);
void I2C_LCD_SWrite(unsigned char,unsigned char *, char);
void I2C_LCD_Init(unsigned char);
void I2C_LCD_Pos(unsigned char,unsigned char);
unsigned char I2C_LCD_Busy(unsigned char);

  

int main(int argc, char** argv) {

  unsigned int Tcount, Threshold; 
  int Touch[4], mode; //counter variable is going to be used later on
  int pass[4] = {0,0,0,0}; //array where the letters are going to be stored.
  unsigned char Sout[16];
  unsigned char *Sptr;
  int counter = 0; //to keep a counter on the elements entered
  int falseFlag = 0;
  Sptr = Sout; //buffer for the LCD to print out characters
  int truePass[4] = {0,3,2,1}; //Spells out "UAST"
  
  
  i2c_Init();				// Start I2C as Master 100KH
  I2C_LCD_Init(I2C_SLAVE); //pass I2C_SLAVE to the init function to create an instance
  
/*TouchPad Setup*/ 
  CPSCON0 	=  0x8C;  //Set up Touch sensing module control reg 0
  CPSCON1   =  0x03;  //Channel Select
  Touch[0] = 3;
  Touch[1] = 9;
  Touch[2] = 5;
  Touch[3] = 4;
/*Clock and Pin Configuration*/
  clockAndpin_config();   //Configures clock and pins, enables timers
  
/*USART CONFIG*/
  
  
/*Threshold Value*/
  Threshold 	=  0x1667; //using the threshold for all 3 channels, 
   RA5 = 1;
//Main Loop:
  I2C_LCD_Pos(I2C_SLAVE, 0x00);
  sprintf(Sout, "Enter Password:");
  I2C_LCD_SWrite(I2C_SLAVE, Sout, 15);
  I2C_LCD_Pos(I2C_SLAVE, 0x40);
  while(1){
   
    mode = 4;
    for(int j = 0; j < 4; j++){
      CPSCON1 = Touch[j];
      timer_config();
      while(!TMR0IF) continue;
      Tcount = (TMR1H << 8) + TMR1L;
      TMR0IF = 0; 
      if(Tcount < Threshold) mode = j;
    }
    if(counter == 0){
        pass[counter] = mode;
    }
    switch(mode){
        case 0:
            pass[counter] = 0;
            break;
       case 1:
            pass[counter] = 1;
            break;
       case 2:
            pass[counter] = 2;
            break;
        case 3:
            pass[counter] = 3;
            break;     
    }
    Display(mode, Sout, &counter); 
    
    //Display characters on the LCD.
    if(counter == 4){
        for(int i = 0; i < counter; i++){
        if(pass[i] != truePass[i]){
            RC6 = 1;
            __delay_ms(300);
            RC6 = 0;
            __delay_ms(300);
            I2C_LCD_Command(I2C_SLAVE, 0x01);
            I2C_LCD_Pos(I2C_SLAVE, 0x40);
            sprintf(Sout, "Wrong Password!");
            I2C_LCD_SWrite(I2C_SLAVE, Sout, 14);
            __delay_ms(1000);
            counter = 0;
            falseFlag = 1;
            break;
        }
        
      }
        if(falseFlag){
            I2C_LCD_Command(I2C_SLAVE, 0x01);
            I2C_LCD_Pos(I2C_SLAVE, 0x40);
            break;
        }
        I2C_LCD_Command(I2C_SLAVE, 0x01);
        I2C_LCD_Pos(I2C_SLAVE, 0x40);
        sprintf(Sout, "Correct Password!");
        I2C_LCD_SWrite(I2C_SLAVE, Sout, 16);
        __delay_ms(1000);
        RA5 = 1;
         __delay_ms(300);
        RA5 = 0;
        __delay_ms(300);
        RA5 = 1; 
        counter = 0;
        I2C_LCD_Pos(I2C_SLAVE, 0x40);
        I2C_LCD_Command(I2C_SLAVE, 0x01);
    }
 }
  
return (EXIT_SUCCESS);
}

      
void Display (int delay, unsigned char Sout[16], int *count){ 
  switch(delay){
    case 0:
      sprintf(Sout, "U");
      I2C_LCD_SWrite(I2C_SLAVE, Sout, 1);
      (*count)++;
      break;
    case 1:
      sprintf(Sout, "T");
      I2C_LCD_SWrite(I2C_SLAVE, Sout, 1);
      (*count)++;
     break;
    case 2:
      sprintf(Sout, "S");
      I2C_LCD_SWrite(I2C_SLAVE, Sout, 1);
     (*count)++;
      break;
    case 3:
      sprintf(Sout, "A");
      I2C_LCD_SWrite(I2C_SLAVE, Sout, 1);
      (*count)++;
     break;
    case 4:
      break;
      
   }
  
}


void clockAndpin_config(){
  OSCCON   =	0X6A; 	//set up 4MHz for Fosc
  INTCON   =  0;   // purpose of disabling the interrupts.
  OPTION_REG =  0XC5; 	// set up timer 0, and then timer 1
  T1CON  	=	0XC1;  //TMR 1 Enable
  T1GCON  	= 	0X81;  //81 is without toggle mode A1 is toggle mode
  TRISA  	= 	0x54;
  LATA      =   0x12;
  TRISC  	=	0XBF;
  LATC      =   0x40;
  PORTA  	= 	0;
  ANSELA 	=  0X10;
}

void timer_config(){
  TMR1ON 	=  0;
  TMR0  	=  0;
  TMR1H    =  0;
  TMR1L  	=  0;
  TMR1ON   =  1;
  TMR0IF 	=  0;   	//Clear the interrupt flag for Timer 1
  TMR0  	=  0; 
}

