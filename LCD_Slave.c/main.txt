//-------------------------------------------------------------------------------
// MSP430 C Code for use with TI Code Composer Studio and MSP430FR2310
// C. Girardot, J.Hughes EELE465, Project 05
// April 5. 2024
//
//  This code was written to be the slave device as the project 5 LED Light bar slave
// Displays the ambient room temperature averaged over n amount of samples. 
//  Display should look like:
//
//  Enter n  :  4
//  T  =  294 K  21.0 C
//
//
// Variables:
//      i                   Indexing variable for loops
//      DataIn              Holds the data recieved through I2C
//--------------------------------------------------------------------------------

#include "msp430fr2310.h"
#include <msp430.h>

volatile short DataIn = '0';
int i;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	

    UCB0CTLW0 |= UCSWRST;

    UCB0CTLW0 |= UCMODE_3;
    UCB0CTLW0 &= ~UCMST;
    UCB0I2COA0 = 0x0010;
    UCB0I2COA0 |= UCOAEN;
    UCB0CTLW0 &= ~UCTR;
    UCB0CTLW1 &= ~UCASTP0;
    UCB0CTLW1 &= ~UCASTP1;

    P1SEL1 &= ~BIT3;
    P1SEL0 |= BIT3;

    P1SEL1 &= ~BIT2;
    P1SEL0 |= BIT2;

    PM5CTL0 &= ~LOCKLPM5;

    UCB0CTLW0 &= ~UCSWRST;

//---------LCD PORTS---------------
	P2DIR |=BIT6; //RS
	P2DIR |=BIT0; //E
    
    P1DIR |=BIT4; //DB4
	P1DIR |=BIT5; //DB5
	P1DIR |=BIT6; //DB6
	P1DIR |=BIT7; //DB7

    P2OUT &= ~BIT6;
	P2OUT &= ~BIT0;

	P1OUT &= ~BIT4;
	P1OUT &= ~BIT5;
	P1OUT &= ~BIT6;
	P1OUT &= ~BIT7;
//----------END LCD PORTS-----------

	UCB0IE |= UCRXIE0;

	lcd_init();
    __delay_cycles(100000);

    __enable_interrupt();

	int i =0;

	lcd_init();
    __delay_cycles(1000000);

	while(1){
    }

	return 0;
}

//------------------------------------------------------------------
//  Latch LCD Command
//------------------------------------------------------------------
void latch(void){
    P2OUT |= BIT0;      
    __delay_cycles(1);
    P2OUT &= ~BIT0;
    return;
}
//--------------------------END LCD Command--------------------------


//------------------------------------------------------------------
//  Command LCD Command
//------------------------------------------------------------------
void command(char i){
    P1OUT = i & 0xF0;
    P2OUT &= ~BIT6;
    latch();
    P1OUT = ((i << 4) & 0xF0);
    latch();
    return;
}
//------------------------------END LCD Command--------------------------

//--------------------------------------------------------------------------
//  Write to LCD command
//--------------------------------------------------------------------------
void write(char i){
    P1OUT = i & 0xF0;
    P2OUT |= BIT6;
    latch();
    P1OUT = ((i << 4) & 0xF0);
    P2OUT |= BIT6;
    latch();
    
    return;
}
//----------------------END LECD WRITE COMMAND----------------------

//------------------------------------------------------------------------
//  LCD INIT
//------------------------------------------------------------------------
void lcd_init(void){
    //set to 4 bit mode
    __delay_cycles(50000);
    P2OUT &= ~BIT6;     //clear RS
    P1OUT = 0x30;
    latch();
    __delay_cycles(4100);
    latch();
    __delay_cycles(100);
    latch();


    //set to 4 bit mode
    P1OUT = 0x20;
    latch();
    __delay_cycles(40);

    //4 bit with font
    command(0x28);
    __delay_cycles(40);

    //display on/no cursor
    command(0x0C);
    __delay_cycles(40);

    //clear
    command(0x01);
    __delay_cycles(1640);

    //entry mode
    command(0x06);
    __delay_cycles(1640);

    char L1[] = "Enter n : ";
    char L2[] = "T =    ";
    
    for(i = 0; i<(sizeof(L1) - 1); i++){
        write(L1[i]);
    }
    command(0xC0);
    for(i = 0; i<(sizeof(L2) - 1); i++){
        write(L2[i]);
    }
    write(223);
    write('K');
    write(' ');
    write(' ');
    write(' ');
    write(' ');
    write(' ');
    write(223);
    write('C');
}
//-----------------------------END LCD INIT---------------------------

//-------------------------------------------------------------------------------
// write number
//-------------------------------------------------------------------------------
void write_number(int temp){
    short digit1 = temp/100;
    short digit2 = (temp - (digit1 * 100)) / 10;
    short digit3 = temp - (digit1 * 100) - (digit2 * 10);
    if(digit1 != 0){
        write('0' + digit1);
    } 
    if(digit1 != 0 || digit2 != 0){
        write('0' + digit2);
    }
    write('0' + digit3);
}
//------------------------------end write number---------------------------------

//-------------------------------------------------------------------------------
// Write first Line
//-------------------------------------------------------------------------------
void write_first_line(short n) {
    char L1[] = "Enter n : ";
    command(0x02);
    for(i = 0; i<(sizeof(L1) - 1); i++){
        write(L1[i]);
    }
    write('0' + n);
}
//---------------------------------End Write First Line------------------------

//-------------------------------------------------------------------------------
// Write Temp/Bottom line to screen
//-------------------------------------------------------------------------------
void write_temp(short temp){
    temp = temp + 100;
    short whole_celsius = temp/10;
    short fraction_celsius = temp - (whole_celsius *10);
    int degrees_kelvin = whole_celsius + 273;

    command(0xC0);
    write('T');
    write(' ');
    write('=');
    write(' ');
    write_number(degrees_kelvin);
    write(223);
    write('K');
    write(' ');
    write_number(whole_celsius);
    write('.');
    write_number(fraction_celsius);
    write(223);
    write('C');
    write(' ');
}
//---------------------------END write temp-------------------------------------

//-------------------------------------------------------------------------------
// I2C B0 Interrupt
//-------------------------------------------------------------------------------
#pragma vector = EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void){
    DataIn = UCB0RXBUF;
    if(DataIn == '#'){
        command(0x01);
        lcd_init();
        __delay_cycles(30000);
    }else if(DataIn == 0){
        
    }else if(DataIn < 10){
        write_first_line(DataIn);
    } else if (DataIn != 0){
        write_temp(DataIn);
    }

}
//----------------------------End I2C B0 Interrupt--------------------------------

