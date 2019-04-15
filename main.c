#include "msp.h"
#include <stdio.h>
#include "math.h"

#define FREQ_1_5_MHz 1500000
#define FREQ_3_MHz 3000000
#define FREQ_6_MHz 6000000
#define FREQ_12_MHz 12000000
#define FREQ_24_MHz 24000000
#define FREQ_48_MHz 48000000

//declared here to avoid implicit function calls
void LCD_init();
void LCD_wr();
void delay_ms();
void set_DCO();
void LCD_wr_NIBBLE();
void Nibble();
void command();
void wirte();

int freq = FREQ_48_MHz;

//Delay functions
void delay_us(int us){
    int j;
    for (j = 0; j < us; j++)
        __delay_cycles(3); //delays for about 1us
}

void delay_ms(int ms, int freq){
    int i, j;
    for (j = 0; j < ms; j++)
        for (i = freq/10000; i > 0; i--);
}


void set_DCO(int freq){
    CS->KEY = CS_KEY_VAL;   //unlock CS register
    CS->CTL0 = 0;
    if (freq == FREQ_1_5_MHz) {
        CS->CTL0 = CS_CTL0_DCORSEL_0;
        CS->CTL1 = CS_CTL1_SELA_2 | CS_CTL1_SELS_3 | CS_CTL1_SELM_3;

    } else if (freq == FREQ_3_MHz) {
        CS->CTL0 = CS_CTL0_DCORSEL_1;
        CS->CTL1 = CS_CTL1_SELA_2 | CS_CTL1_SELS_3 | CS_CTL1_SELM_3;

    } else if (freq == FREQ_6_MHz){
        CS->CTL0 = CS_CTL0_DCORSEL_2;
        CS->CTL1 = CS_CTL1_SELA_2 | CS_CTL1_SELS_3 | CS_CTL1_SELM_3;

    } else if (freq == FREQ_12_MHz){
        CS->CTL0 = CS_CTL0_DCORSEL_3;
        CS->CTL1 = CS_CTL1_SELA_2 | CS_CTL1_SELS_3 | CS_CTL1_SELM_3;

    } else if (freq == FREQ_24_MHz){
        CS->CTL0 = CS_CTL0_DCORSEL_4;
        CS->CTL1 = CS_CTL1_SELA_2 | CS_CTL1_SELS_3 | CS_CTL1_SELM_3;

    } else if (freq == FREQ_48_MHz){ // all statements needed for 48MHz to prevent bricking
        while ((PCM->CTL1 & PCM_CTL1_PMR_BUSY));
             PCM->CTL0 = PCM_CTL0_KEY_VAL | PCM_CTL0_AMR_1;
        while ((PCM->CTL1 & PCM_CTL1_PMR_BUSY));

        FLCTL->BANK0_RDCTL = (FLCTL->BANK0_RDCTL &
         ~(FLCTL_BANK0_RDCTL_WAIT_MASK)) | FLCTL_BANK0_RDCTL_WAIT_1;
        FLCTL->BANK1_RDCTL = (FLCTL->BANK0_RDCTL &
         ~(FLCTL_BANK1_RDCTL_WAIT_MASK)) | FLCTL_BANK1_RDCTL_WAIT_1;

        CS->CTL0 = CS_CTL0_DCORSEL_5;
        CS->CTL1 = CS->CTL1 & ~(CS_CTL1_SELM_MASK | CS_CTL1_DIVM_MASK) | CS_CTL1_SELM_3;

    } else {
        CS->CTL0 |= CS_CTL0_DCORSEL_4;  //default frequency
        CS->CTL1 = CS_CTL1_SELA_2 | CS_CTL1_SELS_3 | CS_CTL1_SELM_3;
    }
    CS->KEY = 0;           //lock CS register
}

//LCD functions
void LCD_init(void) {
    P2->OUT = 0x00;            // set all pins on port to 0 or outputs
    delay_ms(100, freq);
    P2->OUT = 0x30;
    delay_ms(5, freq);
    Nibble();
    delay_ms(1, freq);
    Nibble();
    delay_ms(1, freq);
    Nibble();
    delay_ms(100, freq);
    P2->OUT=0x20;
    Nibble();
    command(0x28);       //set_function
    command(0x10);       //set cursor
    command(0x0F);       //display ON
    command(0x06);       //clear display
}

void Nibble(void){
    P3->OUT |= 0x40;                        //P3.6 (EN) HIGH
    delay_ms(5, freq);                      //delay
    P3->OUT &= ~0x40;                       //P3.6 (EN) LOW
}

void command(int data){
    P2->OUT = (data & 0xF0);                //send 4 bits
    P5->OUT &= (~0x01);                     //5.0 (RS) LOW
    P5->OUT &= ~(0x04);                     //5.2 (R/W) LOW
    Nibble();
    P2->OUT = ((data & 0x0F)<<4);           // send next 4
    Nibble();
}

void write(int data){
    P2->OUT = (data & 0xF0);
    P5->OUT |= 0x01;                        //5.0 (RS) HIGH
    P5->OUT &= ~(0x04);                     //5.2 (R/W) LOW
    Nibble();
    P2->OUT = ((data & 0x0F)<<4);
    Nibble();
}

void LCD_wr_NIBBLE(int data) {
    P2->OUT = data;
}

void clear_LCD(){
    command(0x01);
}

void home_LCD(){
    command(0x02);
}
/*
void set_row_low(){
    P2->OUT = ~0x40;  //P2.3
    P5->OUT = ~0xC2;  //P5.7, 5.6, 5.1
    //P5->OUT = ~0x40;  //P5.6
    //P5->OUT = ~0x40;  //P5.1
}
*/

void scan(){
    //delay_ms(50,freq);
    P2->OUT &= ~0x08;   //Set row 1 low
    P5->OUT |= 0xC2;   //set rows 2-4 high
    if (P3->IN & 0x01){
        __delay_cycles(1);
        //write('4');
    } else {
        clear_LCD();
        write('1');
        delay_ms(1000,freq);
    }
    if (P3->IN & 0x20){
        __delay_cycles(1);
        //write('4');
    } else {
        clear_LCD();
        write('2');
        delay_ms(1000,freq);

    }
    if (P3->IN & 0x80){
        __delay_cycles(1);
        //write('4');
    } else {
        clear_LCD();
        write('3');
        delay_ms(1000,freq);

    }
    //delay_ms(50,freq);
    P2->OUT |= 0x08;    //Set row 1 high P2.3
    P5->OUT |= 0xC0;    //set rows 3,4 high P5.7, P5.6
    P5->OUT &= ~0x02;   //set row 2 low  &~ P5.1
    if (P3->IN & 0x01){
        __delay_cycles(1);
        //write('4');
    } else {
        clear_LCD();
        write('4');
        delay_ms(1000,freq);

    }
    if (P3->IN & 0x20){
        __delay_cycles(1);
        //write('4');
    } else {
        clear_LCD();
        write('5');
        delay_ms(1000,freq);

    }
    if (P3->IN & 0x80){
        __delay_cycles(1);
        //write('4');
    } else {
        clear_LCD();
        write('6');
        delay_ms(1000,freq);

       //write('6');
    }
    P2->OUT |= 0x08;    //Set row 1 high
    P5->OUT |= 0x82;    //set rows 2,4 high
    P5->OUT &= ~0x40;   //set row 3 low
    if (P3->IN & 0x01){
        __delay_cycles(1);
        //write('4');
    } else {
        clear_LCD();
        write('7');
        delay_ms(1000,freq);

    }
    if (P3->IN & 0x20){
        __delay_cycles(1);
        //write('4');
    } else {
        clear_LCD();
        write('8');
        delay_ms(1000,freq);

    }
    if (P3->IN & 0x80){
        __delay_cycles(1);
        //write('4');
    } else {
        clear_LCD();
        write('9');
        delay_ms(1000,freq);

    }
    P2->OUT |= 0x08;    //Set row 1 high
    P5->OUT |= 0x42;    //set rows 2,3 high
    P5->OUT &= ~0x80;   //set row 4 low
    if (P3->IN & 0x01){
        __delay_cycles(1);
        //write('4');
    } else {
        clear_LCD();
        write('*');
        delay_ms(1000,freq);

    }
    if (P3->IN & 0x20){
        __delay_cycles(1);
        //write('4');
    } else {
        clear_LCD();
        write('0');
        delay_ms(1000,freq);

    }
    if (P3->IN & 0x80){
        __delay_cycles(1);
        //write('4');
    } else {
        clear_LCD();
        write('#');
        delay_ms(1000,freq);

    }

}

void main(void) {
    //puts("what\n");
    WDTCTL = WDTPW | WDTHOLD;  // Stop watchdog timer

    //GPIO PIN CONFIGURATIONS
    //RS
    P5->SEL1 &= ~0x01;         // configure P5.0 as simple I/O
    P5->SEL0 &= ~0x01;
    P5->DIR |= 0x01;           // P5.0 set as output pin
    //RW
    P5->SEL1 &= ~0x04;         // configure P5.2 as simple I/O
    P5->SEL0 &= ~0x04;
    P5->DIR |= 0x04;           // P5.2 set as output pin
    //E
    P3->SEL1 &= ~0x40;         // configure P3.6 as simple I/O
    P3->SEL0 &= ~0x40;
    P3->DIR |= 0x40;           // P3.6 set as output pin
    //DB4
    P2->SEL1 &= ~0x10;         // configure P2.4 as simple I/O
    P2->SEL0 &= ~0x10;
    P2->DIR |= 0x10;           // P2.4 set as output pin
    //DB5
    P2->SEL1 &= ~0x20;         // configure P2.5 as simple I/O
    P2->SEL0 &= ~0x20;
    P2->DIR |= 0x20;           // P2.5 set as output pin
    //DB6
    P2->SEL1 &= ~0x40;         // configure P2.6 as simple I/O
    P2->SEL0 &= ~0x40;
    P2->DIR |= 0x40;           // P2.6 set as output pin
    //DB7
    P2->SEL1 &= ~0x80;         // configure P2.7 as simple I/O
    P2->SEL0 &= ~0x80;
    P2->DIR |= 0x80;           // P2.7 set as output pin

    //GPIO PIN CONFIG for Keypad
    //
    P3->SEL1 &= ~0x20;         // configure P3.5 as simple I/O
    P3->SEL0 &= ~0x20;
    P3->DIR &= ~0x20;          // P3.5 set as input
    P3->REN |= 0x20;           // P3.5 pull resistor enabled
    P3->OUT |= 0x20;           // Pull up/down is selected by P1->OUT

    P3->SEL1 &= ~0x01;         // configure P3.0 as simple I/O
    P3->SEL0 &= ~0x01;
    P3->DIR &= ~0x01;          // P3.0 set as input
    P3->REN |= 0x01;           // P3.0 pull resistor enabled
    P3->OUT |= 0x01;           // Pull up/down is selected by P1->OUT

    P3->SEL1 &= ~0x80;         // configure P3.7 as simple I/O
    P3->SEL0 &= ~0x80;
    P3->DIR &= ~0x80;          // P3.7 set as input
    P3->REN |= 0x80;           // P3.7 pull resistor enabled
    P3->OUT |= 0x80;           // Pull up/down is selected by P1->OUT

    P2->SEL1 &= ~0x08;         // configure P2.3 as simple I/O (row)
    P2->SEL0 &= ~0x08;
    P2->DIR |= 0x08;           // P2.3 set as output pin

    P5->SEL1 &= ~0x80;         // configure P5.7 as simple I/O (row)
    P5->SEL0 &= ~0x80;
    P5->DIR |= 0x80;           // P5.7 set as output pin

    P5->SEL1 &= ~0x40;         // configure P5.6 as simple I/O (row)
    P5->SEL0 &= ~0x40;
    P5->DIR |= 0x40;           // P5.6 set as output pin

    P5->SEL1 &= ~0x02;         // configure P5.0 as simple I/O (row)
    P5->SEL0 &= ~0x02;
    P5->DIR |= 0x02;           // P5.0 set as output pin

    //testing switch
    P1->SEL1 &= ~BIT1;         // configure P1.1 as simple I/O
    P1->SEL0 &= ~BIT1;
    P1->DIR  &= ~BIT1;         // P1.1 set as input
    P1->REN |= BIT1;           // P1.1 pull resistor enabled
    P1->OUT |= BIT1;           // Pull up/down is selected by P1->OUT


    P2->SEL1 &= ~BIT0;         // configure P2.0 as simple I/O
    P2->SEL0 &= ~BIT0;
    P2->DIR |= BIT0;           // P2.0 set as output pin


    set_DCO(freq);             // defined as 48MHz
    LCD_init();
    clear_LCD();

    /*
    write('n');
    delay_ms(500, freq);
    write('i');
    delay_ms(500, freq);
    write('g');
    delay_ms(500, freq);
    write('g');
    delay_ms(500, freq);
    write('a');
    delay_ms(500, freq);

    //home_LCD();
    */

    //P2->OUT &= ~0x40;   //Set row 1 low
    //P5->OUT |= ~0xC2;   //set rows 2-4 high
    /*
    while (1){
        if (P1->IN & 0x01){
            if
            write('3');
            //printf("1\n");
            delay_ms(1000,freq);
        }
        //delay_ms(250,freq);
        //        __delay_cycles(1);      //keeps the program running
    }
    */

    while (1) {
        scan();
//        delay_ms(500,freq);

        /*
        if (P3->IN & 0x01) {    // active high
//            P2->OUT |= BIT0;   // turn on P2.0 red LED when SW is pressed
//            delay_ms(1000, freq);
//            write('2');
//
            P2->OUT &= ~BIT0;  // turn off P2.0 red LED when SW is not pressed
            //write('2');
            delay_ms(1000, freq);
            //write('3');

        }

        else {
//            P2->OUT &= ~BIT0;  // turn off P2.0 red LED when SW is not pressed
//            //write('2');
//            delay_ms(1000, freq);
//            //write('3');
//
            P2->OUT |= BIT0;   // turn on P2.0 red LED when SW is pressed
            delay_ms(1000, freq);
            write('2');
            //write('2');
             *
             */



    }



}
