#include "msp430fr2355.h"


#define Slave_Address3 0b1001000; //LM92
//#define Slave_Address3 0x0068; //RTC
short I2C_Message[1];
int i;
int Data_in1;
int Data_in2;
int finalData;
float temp;
short oldTemp;
int dataCount = 0;

//-----------------------------------START MAIN------------------------
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;

    UCB1CTLW0 |= UCSWRST;    
    UCB1CTLW0 |= UCSSEL__SMCLK;   
    UCB1BRW = 10;          

    UCB1CTLW0 |= UCMODE_3;   
    UCB1CTLW0 |= UCMST;          
    UCB1CTLW0 &= ~UCTR;
    UCB1I2CSA = Slave_Address3;

    UCB1CTLW1 |= UCASTP_2;   
    UCB1TBCNT = 1; // We want to pull from 2 registers 00h and 01h so we want 2 bytes of data  

    P4SEL1 &= ~BIT7;
    P4SEL0 |= BIT7;

    P4SEL1 &= ~BIT6;
    P4SEL0 |= BIT6;

    PM5CTL0 &= ~LOCKLPM5;
    UCB1CTLW0 &= ~UCSWRST;

    UCB1IE |= UCTXIE0;
    UCB1IE |= UCRXIE0;
    __enable_interrupt();

    I2C_Message[0] = 0x00; // Asks RTC for seconds register on RTC
     

    while (1) {
        
    UCB1TBCNT = 1;
    UCB1CTLW0 |= UCTR;
    UCB1CTLW0 |= UCTXSTT;
     while ((UCB1IFG & UCSTPIFG) == 0){}
        UCB1IFG &= ~UCSTPIFG;
    __delay_cycles(20000);

    UCB1TBCNT = 2;
    UCB1CTLW0 &= ~UCTR; 
    UCB1CTLW0 |= UCTXSTT;

    while((UCB1IFG & UCSTPIFG) == 0){}
        UCB1IFG &= ~UCSTPIFG;
    }

}
//-----------------------------------END MAIN-------------------------------


#pragma vector = EUSCI_B1_VECTOR
__interrupt void EUSCI_B1_I2C_ISR(void) {
    //UCB1TXBUF = 0x01;

    switch(UCB1IV){
         case 0x16:
            //Temp sensor conversion
                if(UCB1RXBUF && (dataCount == 0)){
                    dataCount++;
                    Data_in1 = UCB1RXBUF;
                    

                }else if(UCB1RXBUF && (dataCount == 1)){
                    dataCount = 0;
                    Data_in2 = UCB1RXBUF;
                   
                }
                Data_in1 = Data_in1 << 8;
                finalData = Data_in1 + Data_in2;
                finalData = finalData >> 3;
                
                if(finalData > 200){
                    temp = finalData*0.0625;
                }
                break;

         case 0x18:
                UCB1TXBUF = I2C_Message[0];
                break;

        default:
                break;
        }
    __delay_cycles(30000);
    

}
