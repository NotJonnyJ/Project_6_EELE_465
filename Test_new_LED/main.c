//-------------------------------------------------------------------------------
// MSP430 C Code for use with TI Code Composer Studio and MSP430FR2355
// C. Girardot, J.Hughes EELE465, Project 05
// April 5. 2024
//
// Read in keypad input, and display averaged temperature on LCD. The temperature will be averaged
//  using a moving average window that changes depending on the key pressed.
//
// Read in keypad input, display patterns on presses of A, B, C, D
// A: 10101010
// B: Binary Counter
// C: Walking zero
// D: Another Pattern
//
// Variables:
//      Slave_Address1      Holds slave addr. for LED Slave
//      Slave_Address2      Holds slave addr. for LCD Slave
//      switch_In           Used to decode keypress into char_In.
//      char_In:            Switch input decoded to the character on the switch
//      i :                 loop index
//      ADC_Value           Used to pull vaules from the LM19 Temp sensor using ADC
//      I2C_Message         Stores the value about to leave the master over the I2C bus.
//      n                   the averaging window size.
//      timer               Flag to determine if the timer interrupt has triggered.
//      voltage             The voltage value from the ADC.
//      temperature         The temp found using voltage
//      tempAverage         The averaged temperature over n samples
//      tempArray           Array used to store the found temperature samples 
//--------------------------------------------------------------------------------
#include "msp430fr2355.h"
#include "pmm.h"
#include <msp430.h> 
#include <stdint.h>
#include <math.h>

//--------------------------------------------------------------------------------
// Variable declaration
//--------------------------------------------------------------------------------
#define Slave_Address1 0x020; //LED
//#define Slave_Address3 0b11010001;

int switch_In;
int i;
char char_In;
short I2C_Message[1];
volatile int n = 0;



//Function declerations
void I2C_INIT();
void keypadInit();


//-----------------------------------START MAIN------------------------
int main(void) {

    volatile uint32_t i;

    WDTCTL = WDTPW | WDTHOLD;

    I2C_INIT();
    __delay_cycles(5000);
    keypadInit();

    PM5CTL0 &= ~LOCKLPM5;
    P3IE |= 0x0F;
    P3IFG &= 0x00;

    UCB1IE |= UCTXIE0;
    UCB1IE |= UCRXIE0;

    __enable_interrupt();

    while (1) {

        
        P3DIR &= 0x00;
        P3DIR |= 0xF0;
        P3OUT |= 0xF0;
        
        __delay_cycles(250);
    }
}
//-----------------------------------END MAIN-------------------------------

//-------------------------------------------------------------------------------
// I2C INIT
//-------------------------------------------------------------------------------
void I2C_INIT(){
    UCB1CTLW0 |= UCSWRST;    
    UCB1CTLW0 |= UCSSEL_3;   
    UCB1BRW = 10;           
    UCB1CTLW0 |= UCMODE_3;   
    UCB1CTLW0 |= UCMST;          
    UCB1CTLW0 |= UCTR;       
    UCB1CTLW1 |= UCASTP_2;   

    P4SEL1 &= ~BIT7;
    P4SEL0 |= BIT7;
    P4SEL1 &= ~BIT6;
    P4SEL0 |= BIT6;
    UCB1CTLW0 &= ~UCSWRST;
}
//---------------------------------END_I2C_INIT----------------------------------

//-------------------------------------------------------------------------------
// Keypad Initialization
//-------------------------------------------------------------------------------
void keypadInit(){
    P3DIR &= 0x00;
    P3OUT &= 0x00;
    P3REN &= 0x00;
    P3IES &= 0xF0;
}
//-----------------------------End_keypad_init-----------------------------

//-------------------------------------------------------------------------------
// Get Row
//-------------------------------------------------------------------------------
int get_Row() {
    int i;
    int result;
    P3DIR &= 0x00;
    P3OUT &= 0x00;
    P3REN &= 0x00;
    P3DIR |= 0x0F;
    P3REN |= 0xF0;
    P3OUT |= 0x0F;
    for(i = 50; i < 0; i--){}
    result = (P3IN & 0xF0);
    return result;
}
//---------------------------------End Get Row-----------------------------------

//-------------------------------------------------------------------------------
// Get Column
//-------------------------------------------------------------------------------
int get_Col(){
    int result;
    P3DIR &= 0x00;
    P3OUT &= 0x00;
    P3REN &= 0x00;
    P3DIR |= 0xF0;
    P3REN |= 0x0F;
    P3OUT |= 0xF0;
    for(i = 0; i < 50; i++){}
    result = (P3IN & 0x0F);
    return result;
}
//------------------------------End Get Column-----------------------------------

//-------------------------------------------------------------------------------
// Get Character
//-------------------------------------------------------------------------------
char get_Char(int keycode){
    switch(keycode){
            case(0x11):
                    return '1';
                    break;
            case(0x12):
                    return '2';
                    break;
            case(0x14):
                    return '3';
                    break;
            case(0x18):
                    return 'A';
                    break;
            case(0x21):
                    return '4';
                    break;
            case(0x22):
                    return '5';
                    break;
            case(0x24):
                    return '6';
                    break;
            case(0x28):
                    return 'B';
                    break;
            case(0x41):
                    return '7';
                    break;
            case(0x42):
                    return '8';
                    break;
            case(0x44):
                    return '9';
                    break;
            case(0x48):
                    return 'C';
                    break;
            case(0x81):
                    return '*';
                    break;
            case(0x82):
                    return '0';
                    break;
            case(0x84):
                    return '#';
                    break;
            case(0x88):
                    return 'D';
                    break;
            default:
                return;
                break;              
    }
}
//------------------------------End Get Character---------------------------------

//-------------------------------------------------------------------------------
// Delay
//-------------------------------------------------------------------------------
void delay(){
    int i;
    for (i = 0; i<30000; i++){};
}
//--------------------------------End Delay--------------------------------------

//----------------------------------------------------------------
//  I2C Send LED
//----------------------------------------------------------------
void I2CSendLED(){
        UCB1TBCNT = 2;  
        UCB1CTLW1 |= UCASTP_2;   
        UCB1I2CSA = Slave_Address1; 
        UCB1CTLW0 |= UCTR; 
        UCB1CTLW0 |= UCTXSTT;
        __delay_cycles(30000);
}
//----------------------------End I2C Send LED--------------------------------

//-------------------------------------------------------------------------------
// Port 3 (Keypad) Interrupt
//-------------------------------------------------------------------------------
#pragma vector = PORT3_VECTOR
__interrupt void ISR_Port3_LSN(void){
    __disable_interrupt();
    switch_In = get_Row() + get_Col();
    char_In = get_Char(switch_In);
    P3IFG &= 0x00;
    __enable_interrupt();
    if(char_In != 216){
            if(char_In == 'A'){
                I2C_Message[0] = 'A';
                I2CSendLED();
            }
            if(char_In == 'B'){
                I2C_Message[0] = 'B';
                I2CSendLED();
            }
            if(char_In == 'C'){
                I2C_Message[0] = 'C';
                I2CSendLED();
                delay();
            }
            if(char_In == 'D'){
                I2C_Message[0] = 'D';
                I2CSendLED();
                delay();
            }
            if(char_In == '*'){
                I2C_Message[0] = '*';
                I2CSendLED();
            }
            if(char_In == '#'){
                I2C_Message[0] = '#';
                I2CSendLCD();
                for(i = 0; i < 3; i++){
                    __delay_cycles(30000);
                }
                n = 0;
            }
        
    }
}
//----------------------------End Port 3 Interrupt--------------------------------

//-------------------------------------------------------------------------------
// I2C EUSCIB0
//-------------------------------------------------------------------------------
#pragma vector = EUSCI_B1_VECTOR
__interrupt void EUSCI_B1_I2C_ISR(void) {
    __disable_interrupt();

    UCB1TXBUF = I2C_Message[0];

    __enable_interrupt();
}
//----------------------------End USCIB0 I2C Interrupt--------------------------------



