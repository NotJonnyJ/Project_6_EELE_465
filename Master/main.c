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
#define Slave_Address2 0x010; //LCD
#define Slave_Address3 0x0D1; //RTC
#define Slave_Address4 0b1001001; //LM29 Temp Sensor
int switch_In;
int i;
char char_In;
volatile unsigned int ADC_Value = 0;
short I2C_Message[1];
volatile int n = 0;
volatile int timer = 0;
volatile int timerPos = 0;
volatile float voltage = 0;
volatile float temperature = 0;
volatile float tempAverage = 0;
volatile float tempArray[9];
volatile float LM92_array[9];
char Data_in;

//Function declerations
void I2C_INIT();
void keypadInit();
void ADCInit();
void timerInit();

//-----------------------------------START MAIN------------------------
int main(void) {
    for(i = 0; i <= 8; i++){
        tempArray[i] = 0;
    }
    __delay_cycles(5000);
    volatile uint32_t i;

    WDTCTL = WDTPW | WDTHOLD;
    I2C_INIT();
    __delay_cycles(5000);

    keypadInit();
    __delay_cycles(5000);

    ADCInit();
    __delay_cycles(5000);

    timerInit();
    __delay_cycles(5000);

    PM5CTL0 &= ~LOCKLPM5;
    __delay_cycles(1000);
    UCB1CTLW0 &= ~UCSWRST;
    __delay_cycles(1000);

    P3IE |= 0x0F;
    P3IFG &= 0x00;
    UCB1IE |= UCTXIE0;
    UCB1IE |= UCRXIE0;
    TB0CCTL0 &= ~CCIFG;
    TB0CCTL0 |= CCIE;

    __enable_interrupt();

    while (1) {
        if(timer == 1){
            timer = 0;
            if(timerPos == 1){ // 0.5 Seconds has passed -- Pull temp data from LM19 and LM92
                halfSecond();
                //Setup data collection from LM92 on Peltier Device
                //Average Data over moving average window
            }else if(timerPos == 2){  // 0.5 Seconds has passed -- Pull RTC Data
                fullSecond();
                //Send RTC time data to LCD in seconds
                //Also update temp average
            }
        }else{
            P3DIR &= 0x00;
            P3DIR |= 0xF0;
            P3OUT |= 0xF0;
            __delay_cycles(5000);
        }
    }
}
//-----------------------------------END MAIN-------------------------------




//---------------------------------------------------------------------------
// Main Timing Blocks
//---------------------------------------------------------------------------
void halfSecond(){
    ADCCTL0 |= ADCENC | ADCSC; //Enable the Start conversion
    __delay_cycles(10000);
    insertTemp(temperature, tempArray); 
    __delay_cycles(5000);
    tempAverage = averager(tempArray);
    __delay_cycles(5000);

}

void fullSecond(){
    ADCCTL0 |= ADCENC | ADCSC; //Enable the Start conversion
    __delay_cycles(10000);
    insertTemp(temperature, tempArray); 
    __delay_cycles(5000);
    tempAverage = averager(tempArray);
    __delay_cycles(5000);

    I2C_Message[0] = 0; // Asks RTC for seconds register on RTC
    I2CSendRTC();       // Because UCB1TBCNT is set to 2 we should pull 2 bytes from the RTC

    //Collect data from RTC
    //Process data to determine time
}
//-----------------------------End Timing Blocks-----------------------------






//--------------------------------------------------------------------------------
// Averager - averages temp array over n
//--------------------------------------------------------------------------------
float averager(float tempArray[]){
    float total = 0;
    if(n!= 0){
        for(i = 0; i < n; i++){
        total = total + tempArray[i];
        }
        return total/n;
    } else{
        return 0;
    }
}
//-----------------------------END Averager Func---------------------------------


//--------------------------------------------------------------------------------
// Insert Temperature into tempArray
//--------------------------------------------------------------------------------
void insertTemp(float temperature, float tempArray[]){
    for(i = 8; i > 0; i--){
        tempArray[i] = tempArray[i-1];
    }
    tempArray[0] = temperature;
}
//-----------------------------END Insert Temp---------------------------------

//--------------------------------------------------------------------------------
// Convert temperature to send over I2C
//--------------------------------------------------------------------------------
short convertTempSend(float tempAverage){
    int a = tempAverage * 10;
    short tempChar = (short) a;
    return tempChar;
}
//-----------------------------END Convert temp---------------------------------

//-------------------------------------------------------------------------------
//Timer Init
//-------------------------------------------------------------------------------
void timerInit(){
    // 0.5 Second timer for Temp collection
    TB0CTL |= TBCLR;
    TB0CTL |= TBSSEL__ACLK;
    TB0CTL |= MC__UP;
    TB0CCR0 = 16284;
}
//-----------------------------END_Timer_INIT---------------------------------

//-------------------------------------------------------------------------------
// ADC INIT
//-------------------------------------------------------------------------------
void ADCInit(){
    P1SEL1 |= BIT2;
    P1SEL0 |= BIT2; 
    ADCCTL0 &= ~ADCSHT;
    ADCCTL0 |= ADCSHT_2; 
    ADCCTL0 |= ADCON;
    ADCCTL1 |= ADCSSEL_2;
    ADCCTL1 |= ADCSHP;
    ADCCTL2 &= ~ADCRES;
    ADCCTL2 |= ADCRES_2;
    ADCMCTL0 |= ADCINCH_2;
    ADCIE |= ADCIE0;
}
//--------------------------END_ADC_INIT----------------------------------

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
        UCB1CTLW0 |= UCTR;  
        UCB1TBCNT = 1;  
        UCB1I2CSA = Slave_Address1; 
        UCB1CTLW0 |= UCTR; 
        UCB1CTLW0 |= UCTXSTT;
}
//----------------------------End I2C Send LED--------------------------------

//----------------------------------------------------------------
//  I2C Send LCD
//----------------------------------------------------------------
void I2CSendLCD(){
        UCB1CTLW0 |= UCTR;  
        UCB1TBCNT = 1;  
        UCB1I2CSA = Slave_Address2; 
        UCB1CTLW0 |= UCTR; 
        UCB1CTLW0 |= UCTXSTT; 
}
//----------------END_LCD_SEND----------------------------

//----------------------------------------------------------------
//  I2C Send RTC
//----------------------------------------------------------------
void I2CSendRTC(){
        UCB1CTLW0 |= UCTR;   
        UCB1TBCNT = 2; // We want to pull from 2 registers 00h and 01h so we want 2 bytes of data  
        UCB1I2CSA = Slave_Address3; 
        UCB1CTLW0 |= UCTR; 
        UCB1CTLW0 |= UCTXSTT;
        
        while((UCB0IFG & UCSTPIFG) == 0){}
        UCB0IFG &= ~UCSTPIFG;

        UCB1CTLW0 &= ~UCTR; 
        UCB0CTLW0 |= UCTXSTT;

        while((UCB0IFG & UCSTPIFG) == 0){}
        UCB0IFG &= ~UCSTPIFG;
}
//----------------------------End I2C Send LED--------------------------------

//----------------------------------------------------------------
//  I2C Send LM92
//----------------------------------------------------------------
void I2CSendLM92(){
       UCB1CTLW0 |= UCTR;   
        UCB1TBCNT = 1; // We want to pull from 2 registers 00h and 01h so we want 2 bytes of data  
        UCB1I2CSA = Slave_Address4; 
        UCB1CTLW0 |= UCTR; 
        UCB1CTLW0 |= UCTXSTT;
        
        while((UCB0IFG & UCSTPIFG) == 0){}
        UCB0IFG &= ~UCSTPIFG;

        UCB1CTLW0 &= ~UCTR; 
        UCB0CTLW0 |= UCTXSTT;

        while((UCB0IFG & UCSTPIFG) == 0){}
        UCB0IFG &= ~UCSTPIFG;
}
//----------------------------End I2C Send LED--------------------------------


//---------------------------------------------------------------------------------------------------
//                              Interrupt Service Routines
//---------------------------------------------------------------------------------------------------

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
            if(char_In == '1'){
                n = 1;
                I2C_Message[0] = (char)(n);
                I2CSendLCD();
                for(i = 0; i < 2; i++){
                    __delay_cycles(30000);
                }
            }
            if(char_In == '2'){
                n = 2;
                I2C_Message[0] = (char)(n);
                I2CSendLCD();
                for(i = 0; i < 2; i++){
                    __delay_cycles(30000);
                }
            }
            if(char_In == '3'){
                n = 3;
                I2C_Message[0] = (char)(n);
                I2CSendLCD();
                for(i = 0; i < 2; i++){
                    __delay_cycles(30000);
                }
            }
            if(char_In == '4'){
                n = 4;
                I2C_Message[0] = (char)(n);
                I2CSendLCD();
                for(i = 0; i < 2; i++){
                    __delay_cycles(30000);
                }
            }
            if(char_In == '5'){
                n = 5;
                I2C_Message[0] = (char)(n);
                I2CSendLCD();
                for(i = 0; i < 2; i++){
                    __delay_cycles(30000);
                }
            }
            if(char_In == '6'){
                n = 6;
                I2C_Message[0] = (char)(n);
                I2CSendLCD();
                for(i = 0; i < 2; i++){
                    __delay_cycles(30000);
                }
            }
            if(char_In == '7'){
                n = 7;
                I2C_Message[0] = (char)(n);
                I2CSendLCD();
                for(i = 0; i < 2; i++){
                    __delay_cycles(30000);
                }
            }
            if(char_In == '8'){
                n = 8;
                I2C_Message[0] = (char)(n);
                I2CSendLCD();
                for(i = 0; i < 2; i++){
                    __delay_cycles(30000);
                }
            }
            if(char_In == '9'){
                n = 9;
                I2C_Message[0] = (char)(n);
                I2CSendLCD();
                for(i = 0; i < 2; i++){
                    __delay_cycles(30000);
                }
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
    switch(UCB0IV){
        case 0x16:
            Data_in = UCB0RXBUF;
            break;
        case 0x18:
            if((n > 0) || (I2C_Message[0] == 'A') || (I2C_Message[0] == 'B') || (I2C_Message[0] == 'C') || (I2C_Message[0] == 'D')){
            UCB1TXBUF = I2C_Message[0];
            }
            __delay_cycles(3000);
            break;
        default:
            break;
    }
    __enable_interrupt();
}
//----------------------------End USCIB0 I2C Interrupt--------------------------------

//-------------------------------------------------------------------------------
// ADC Interrupt
//-------------------------------------------------------------------------------
#pragma vector = ADC_VECTOR
__interrupt void ADC_ISR(void) {
    ADC_Value = ADCMEM0;
    __disable_interrupt();
    voltage = (ADC_Value*3.3)/(4095);
    __delay_cycles(5000);
    temperature = -1481.96 + sqrt(2196000 + (1.8639 - voltage)/(0.00000388));
    temperature = temperature;
    //Set scaler to correct the temperature 
    temperature = temperature - 2;
    __delay_cycles(5000);
    __enable_interrupt();
}
//-------------------------END_ADC_ISR----------------------------------

//-------------------------------------------------------------------------------
// Timer Interrupts
//-------------------------------------------------------------------------------
#pragma vector = TIMER0_B0_VECTOR
// 0.5 Second timer for Temp collection
__interrupt void ISR_TB0_Overflow(void) {
        timer = 1;
        timerPos++;
        TB0CCTL0 &= ~CCIFG;
        if((n != 0)){
            tempAverage = tempAverage - 10;
            I2C_Message[0] = convertTempSend(tempAverage);
            I2CSendLCD();
            for(i = 0; i < 5; i++){
            __delay_cycles(30000);
            }
        }
}
//-------------------------END_ADC_ISR----------------------------------

//---------------------------------End ISR's--------------------------------------