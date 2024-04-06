//-------------------------------------------------------------------------------
// MSP430 C Code for use with TI Code Composer Studio and MSP430FR2310
// C. Girardot, J.Hughes EELE465, Project 05
// April 5. 2024
//
//  This code was written to be the slave device as the project 5 LED Light bar slave
// Displays an LED bar pattern depending on the I2C letter code recieved
//  
//
// Read in keypad input, display patterns on presses of A, B, C, D
// A: 10101010
// B: Binary Counter
// C: Walking zero
// D: Another Pattern
//
// Variables:
//      i                   Indexing variable for loops
//      j
//      pattern_B_Counter   Keep track of which step pattern B is currently on
//      pattern_C_Counter   Keep track of which step pattern C is currently on
//      pattern             Holds the letter pulled through I2C
//      patternD_State      Keep track of which step pattern D is currently on
//      DataIn              Holds the data recieved through I2C
//      newDataFlag         Flag to determine if new data has been recieved.
//--------------------------------------------------------------------------------



#include "msp430fr2310.h"
#include <msp430.h> 

int i;
int j;
int pattern_B_Counter = 0;
int pattern_C_Counter = 1;
char pattern = '0';
int patternD_State = 0;
volatile char DataIn = '0';
int newDataFlag = 0;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer

    UCB0CTLW0 |= UCSWRST; 
    //Setup I2C recieve mode as slave with SA of 20
    UCB0CTLW0 |= UCMODE_3;
    UCB0CTLW0 &= ~UCMST;
    UCB0I2COA0 = 0x0020;
    UCB0I2COA0 |= UCOAEN;
    UCB0CTLW0 &= ~UCTR;
    UCB0CTLW1 &= ~UCASTP0;
    UCB0CTLW1 &= ~UCASTP1;

    //Set P1.3 as I2C Clock, set P1.2 as I2C Data
    
    P1SEL1 &= ~BIT3;
    P1SEL0 |= BIT3;

    P1SEL1 &= ~BIT2;
    P1SEL0 |= BIT2;

    //Pinouts for LED BAR
    P1OUT &= 0x00;
    P1DIR |= 0xFF;
    P1DIR &= ~(BIT2 | BIT3);

    P2OUT &= 0x00;
    P2DIR |= (BIT0 |BIT6);

    PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings
    UCB0CTLW0 &= ~UCSWRST;
    UCB0IE |= UCRXIE0; // ENABLE I2C Rx0
    __enable_interrupt(); // enable maskable IRQs

	while(1){
        if(newDataFlag = 1){
            newDataFlag = 0;
            switch (DataIn) {
                case 'A':
                    pattern = 'A';
                    pattern_A();
                    break;
                case 'B':
                    pattern = 'B';
                    pattern_B();
                    break;
                case 'C':
                    pattern = 'C';
                    pattern_C();
                    break;
                case 'D':
                    pattern = 'D';
                    pattern_D();
                    break;
                case 0x2A:
                    clear_LED();
                    break;
                case 'X':
                    blink_LED();
                    break;
                case 'L':
                    wrong_Pattern();
                    break;
                default:
                    break;
	        }

        }
        
    }
	return 0;
}

//-------------------------------------------------------------------------------
// LED Pattern A
//-------------------------------------------------------------------------------
void pattern_A(){
    //Pattern A: Static 10101010
    P1OUT &= BIT2 | BIT3;
    P1OUT |= 0xA2;
    P2OUT &= 0x00;
    P2OUT |= BIT6;
}
//-----------------------------End LED Pattern A----------------------------------

//-------------------------------------------------------------------------------
// LED Pattern B
//-------------------------------------------------------------------------------
void pattern_B(){
    //Pattern B: Binary counter
    P1OUT &= BIT2 | BIT3;

    if(pattern == 'B'){
        //Increment, display counter on P1 (LED bar)
        if(pattern_B_Counter <  255){
            pattern_B_Counter++;
        } else {
            pattern_B_Counter = 0;
        }
        P1OUT &= BIT2 | BIT3;
        P2OUT &= ~(BIT0 | BIT6);

        P1OUT |= pattern_B_Counter & BIT0;
        P1OUT |= pattern_B_Counter & BIT1;
        P1OUT |= pattern_B_Counter & BIT4;
        P1OUT |= pattern_B_Counter & BIT5;
        P1OUT |= pattern_B_Counter & BIT6;
        P1OUT |= pattern_B_Counter & BIT7;

        if((pattern_B_Counter & BIT2) != 0){
            P2OUT |= BIT0;
            //could maybe remove else statements. Give it a test
        } else {
            P2OUT &= ~BIT0;
        }
        if((pattern_B_Counter & BIT3) != 0){
            P2OUT |= BIT6;
        } else {
            P2OUT &= ~BIT6;
        }
        delay();
        delay();
        delay();
        delay();
    }

}
//-----------------------------End LED Pattern B----------------------------------

//-------------------------------------------------------------------------------
// LED Pattern C
//-------------------------------------------------------------------------------
    void pattern_C(){
        //Pattern C: Walking 0
        P1OUT &= BIT2 | BIT3;
        //Reset counter, turn on all bits besides bit 0

        P1OUT |= (~pattern_C_Counter & BIT0);
        P1OUT |= (~pattern_C_Counter & BIT1);
        P1OUT |= (~pattern_C_Counter & BIT4);
        P1OUT |= (~pattern_C_Counter & BIT5);
        P1OUT |= (~pattern_C_Counter & BIT6);
        P1OUT |= (~pattern_C_Counter & BIT7);

        if((pattern_C_Counter & BIT2) != 0){
            P2OUT &= ~BIT0;
        } else {
            P2OUT |= BIT0;
        }
        if((pattern_C_Counter & BIT3) != 0){
            P2OUT &= ~BIT6;
        } else {
            P2OUT |= BIT6;
        }
        delay();
        delay();

        //Rotate pattern left. If bit 8 is OFF, wrap to bit 0
        if(pattern == 'C'){
            if(pattern_C_Counter < 128){
                pattern_C_Counter *= 2;
            } else {
                pattern_C_Counter = 1;
            }

            P1OUT &= BIT2 | BIT3;
            P2OUT &= ~(BIT0 | BIT6);
            P1OUT |= (~pattern_C_Counter & BIT0);
            P1OUT |= (~pattern_C_Counter & BIT1);
            P1OUT |= (~pattern_C_Counter & BIT4);
            P1OUT |= (~pattern_C_Counter & BIT5);
            P1OUT |= (~pattern_C_Counter & BIT6);
            P1OUT |= (~pattern_C_Counter & BIT7);

            if((pattern_C_Counter & BIT2) != 0){
                P2OUT &= ~BIT0;
            } else {
                P2OUT |= BIT0;
            }
            if((pattern_C_Counter & BIT3) != 0){
                P2OUT &= ~BIT6;
            } else {
                P2OUT |= BIT6;
            }
            delay();
            delay();
        }

    }
//-----------------------------End LED Pattern C----------------------------------

//-------------------------------------------------------------------------------
// LED Pattern D
//-------------------------------------------------------------------------------
    void pattern_D(){
        //Pattern C: Walking 0
        P1OUT &= BIT2 | BIT3;
        P2OUT &= ~(BIT2 | BIT3);
        //Step 1
        P1OUT |= BIT4;
        P2OUT |= BIT6;
        delay();
        delay();
        delay();
        P1OUT &= ~BIT4;
        P2OUT &= ~BIT6;
        
        //Step 2
        P1OUT |= BIT5;
        P2OUT |= BIT0;
        delay();
        delay();
        delay();
        P1OUT &= ~BIT5;
        P2OUT &= ~BIT0;

        //Step 3
        P1OUT |= BIT6;
        P1OUT |= BIT1;
        delay();
        delay();
        delay();
        P1OUT &= ~BIT6;
        P1OUT &= ~BIT1;
       
        //Step 4
        P1OUT |= BIT7;
        P1OUT |= BIT0;
        delay();
        delay();
        delay();
        P1OUT &= ~BIT7;
        P1OUT &= ~BIT0;

        //Step 3
        P1OUT |= BIT6;
        P1OUT |= BIT1;
        delay();
        delay();
        delay();
        P1OUT &= ~BIT6;
        P1OUT &= ~BIT1;

        //Step 2
        P1OUT |= BIT5;
        P2OUT |= BIT0;
        delay();
        delay();
        delay();
        P1OUT &= ~BIT5;
        P2OUT &= ~BIT0;
        
    }
//-----------------------------End LED Pattern C----------------------------------

//--------------------------------------------------------------------------------
// Unlock Ccndition LED pattern
//--------------------------------------------------------------------------------
    void unlock_Pattern(){
        blink_LED();
        clear_LED();
}
//-------------------------------End Blink LED------------------------------------

//--------------------------------------------------------------------------------
// Wrong Password LED pattern
//--------------------------------------------------------------------------------
void wrong_Pattern(){
    blink_LED();
    blink_LED();
    clear_LED();
}
//-------------------------------End Blink LED------------------------------------

//--------------------------------------------------------------------------------
// Blink LED
//--------------------------------------------------------------------------------
    void blink_LED(){
        //Turn all LEDs OFF, delay, all ON
        P1OUT &= BIT2 | BIT3;
        P2OUT &= ~(BIT0 | BIT6);
         __delay_cycles(15000);
        P1OUT |= 0x03;
        P1OUT |= 0xF0;
        P2OUT |= (BIT6 | BIT0);
        __delay_cycles(15000);
    }
//-------------------------------End Blink LED------------------------------------

//--------------------------------------------------------------------------------
// Clear LED
//--------------------------------------------------------------------------------
    void clear_LED(){
        //Turn all LEDs OFF, delay, all ON
        P1OUT &= BIT2 | BIT3;
        P2OUT &= ~(BIT0 | BIT6);
        delay();
    }
//-------------------------------End Blink LED------------------------------------

//-------------------------------------------------------------------------------
// Delay
//-------------------------------------------------------------------------------
    void delay(){
        for (i = 0; i<30000; i++){};
    }
//--------------------------------End Delay--------------------------------------
#pragma vector = EUSCI_B0_VECTOR // Triggers when RX buffer is ready for data,
                                 // after start and ack
__interrupt void EUSCI_B0_I2C_ISR(void) { 
    DataIn = UCB0RXBUF; // Store data in variable
    newDataFlag = 1;
}