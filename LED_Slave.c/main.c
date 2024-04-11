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
#include "math.h"

int i;
int j;
int pattern_B_Counter = 0;
int pattern_C_Counter = 1;
char pattern = '0';
int patternD_State = 0;
volatile char DataIn = '0';
int newDataFlag = 0;
int timer = 0;

void timerInit();

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

    timerInit();
    delay(1);

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
    TB0CCTL0 &= ~CCIFG;
    TB0CCTL0 |= CCIE;
    __enable_interrupt(); // enable maskable IRQs

	while(1){
        if(newDataFlag = 1){
            newDataFlag = 0;
            switch (DataIn) {
                case 'A':
                    if(pattern != 'A'){
                        timer = 0;
                        pattern = 'A';
                        clear_LED();
                    }
                     heat();
                    
                    break;
                case 'B':
                    if(pattern != 'B'){
                        timer = 0;
                        pattern = 'B';
                        clear_LED();
                        
                    }
                    cool();
                    
                    break;
                case 'C':
                    pattern = 'C';
                    break;
                case 'D':
                    pattern = 'D';
                    flashing();
                    break;
                case 0x2A:
                    clear_LED();
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
void heat(){
    //Scroll Up
    //scroll to right
    switch (timer) {
        case 1: 
            P1OUT |= BIT7;
            __delay_cycles(30000);
            break;
        case 2: 
            P1OUT |= BIT7;
            P1OUT |= BIT6;
            __delay_cycles(30000);
            break;
        case 3: 
            P1OUT |= 0b11100000;
            __delay_cycles(30000);
            break;
        case 4: 
            P1OUT |= 0b11110000;
            __delay_cycles(30000);
            break;
        case 5: 
            P1OUT |= 0b11110000;
            P2OUT |= BIT6;
            __delay_cycles(30000);
            break;
        case 6: 
            P1OUT |= 0b11110000;
            P2OUT |= BIT6;
            P2OUT |= BIT0;
            __delay_cycles(2000);
            break;
        case 7: 
            P1OUT |= 0b11110010;
            P2OUT |= BIT6;
            P2OUT |= BIT0;
            __delay_cycles(30000);
            break;
        case 8: 
            P1OUT |= 0b11110011;
            P2OUT |= BIT6;
            P2OUT |= BIT0;
            __delay_cycles(30000);
            break;
    
    }

    if(timer > 8){
        clear_LED();
        timer = 0;
    }
    

}
//-----------------------------End LED Pattern A----------------------------------

//-------------------------------------------------------------------------------
// LED Pattern B
//-------------------------------------------------------------------------------
void cool(){
    //Scroll down
    //scroll to left
    switch (timer) {
        case 1: 
            P1OUT |= BIT0;
            __delay_cycles(2000);
            break;
        case 2: 
            P1OUT |= 0b00000011;
            __delay_cycles(2000);
            break;
        case 3: 
            P1OUT |= 0b00000011;
            P2OUT |= BIT0;
            __delay_cycles(2000);
            break;
        case 4: 
            P1OUT |= 0b00000011;
            P2OUT |= 0b01000001;
            __delay_cycles(2000);
            break;
        case 5: 
             P1OUT |= 0b00010011;
            P2OUT |= 0b01000001;
            __delay_cycles(2000);
            break;
        case 6: 
            P1OUT |= 0b00110011;
            P2OUT |= 0b01000001;
            __delay_cycles(2000);
            break;
        case 7: 
            P1OUT |= 0b01110011;
            P2OUT |= 0b01000001;
            __delay_cycles(2000);
            break;
        case 8: 
            P1OUT |= 0b11110011;
            P2OUT |= 0b01000001;
            __delay_cycles(2000);
            break;
    
    }
        
    if(timer > 8){
        clear_LED();
        timer = 0;
    }

}
//-----------------------------End LED Pattern B----------------------------------


//-------------------------------------------------------------------------------
// LED Pattern D
//-------------------------------------------------------------------------------
void clear_LED(){
     //Turn all LEDs OFF, delay, all ON
        P1OUT &= BIT2 | BIT3;
        P2OUT &= ~(BIT0 | BIT6);
        delay();
        
}
//-----------------------------End LED Pattern C----------------------------------

//--------------------------------------------------------------------------------
// Blink LED
//--------------------------------------------------------------------------------
void flashing(){
    P1OUT &= BIT2 | BIT3;
    P2OUT &= ~(BIT0 | BIT6);
    if((timer - (timer/2) * 2) == 0){
        //One set of flash
        P1OUT |= BIT7;
        P1OUT |= BIT5;
        P2OUT |= BIT6;
        P1OUT |= BIT1;
       //delay();

    }else{
        //Second flash set
        P1OUT |= BIT6;
        P1OUT |= BIT4;
        P2OUT |= BIT0;
        P1OUT |= BIT0;
       //delay();
    }

}
//-------------------------------End Blink LED------------------------------------

void timerInit(){  
    // 0.5 Second timer for Temp collection
    TB0CTL |= TBCLR;
    TB0CTL |= TBSSEL__ACLK;
    TB0CTL |= MC__UP;
    TB0CCR0 = 10921;
}

//-------------------------------------------------------------------------------
// Delay
//-------------------------------------------------------------------------------
void delay(int n){
        for (i = 0; i<n; i++){
            __delay_cycles(30000);
        };
}
//--------------------------------End Delay--------------------------------------


#pragma vector = EUSCI_B0_VECTOR // Triggers when RX buffer is ready for data,
                                 // after start and ack
__interrupt void EUSCI_B0_I2C_ISR(void) { 
    DataIn = UCB0RXBUF; // Store data in variable
    newDataFlag = 1;
}


//-------------------------------------------------------------------------------
// Timer Interrupts
//-------------------------------------------------------------------------------
#pragma vector = TIMER0_B0_VECTOR
// 0.5 Second timer for Temp collection
__interrupt void ISR_TB0_Overflow(void) {      
        timer++;
        TB0CCTL0 &= ~CCIFG;      
}
//-------------------------END_ADC_ISR----------------------------------