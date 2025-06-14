#include <xc.h>
#include "timer.h"
#include "IO.h"
#include "PWM.h"
#include "ADC.h"
#include "main.h"
#include "CB_TX1.h"
#include "CB_TX2.h"
#include "CB_RX2.h"
#include "UART_Protocol.h"
#include "QEI.h"
#include "UART.h"
#include "asservissement.h"

#define LINEAIRE 0
#define ANGULAIRE 1

unsigned long timestamp=0;
unsigned long timestop=0;
unsigned long time = 0;
unsigned long timeoscillo=0;
unsigned int tock = 0;

//Initialisation d?un timer 16 bits

void InitTimer1(void) {
    //Timer1 pour horodater les mesures (1ms)
    T1CONbits.TON = 0; // Disable Timer
    T1CONbits.TCKPS = 0b10; //Prescaler
    //11 = 1:256 prescale value
    //10 = 1:64 prescale value
    //01 = 1:8  prescale value
    //00 = 1:1 prescale value
    T1CONbits.TCS = 0; //clock source = internal clock
    //PR1 = 0x1D4C;
    //PR1 = 0x4e2;  //6kHz
    PR1 = 0x249f; //100Hz

    IFS0bits.T1IF = 0; // Clear Timer Interrupt Flag
    IEC0bits.T1IE = 1; // Enable Timer interrupt
    T1CONbits.TON = 1; // Enable Timer
    SetFreqTimer1(100);
}
//Interruption du timer 1

void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0;
    timeoscillo++;
    PWMUpdateSpeed();
    ADC1StartConversionSequence();
    DetectionCapteur();
    QEIUpdateData();
    tock++;
    if(tock == 25){
        SendPositionData();
        tock = 0;
    }
    

        
//    SendMessage((unsigned char*) "Bonjour", 7);
}
//Initialisation d?un timer 32 bits

void InitTimer23(void) {
    T3CONbits.TON = 0; // Stop any 16-bit Timer3 operation
    T2CONbits.TON = 0; // Stop any 16/32-bit Timer3 operation
    T2CONbits.T32 = 1; // Enable 32-bit Timer mode
    T2CONbits.TCS = 0; // Select internal instruction cycle clock
    T2CONbits.TCKPS = 0b00; // Select 1:1 Prescaler
    TMR3 = 0x00; // Clear 32-bit Timer (msw)
    TMR2 = 0x00; // Clear 32-bit Timer (lsw)
    PR3 = 0x0393; // Load 32-bit period value (msw)
    PR2 = 0x8700; // Load 32-bit period value (lsw)
    IPC2bits.T3IP = 0x01; // Set Timer3 Interrupt Priority Level
    IFS0bits.T3IF = 0; // Clear Timer3 Interrupt Flag
    IEC0bits.T3IE = 1; // Enable Timer3 interrupt
    T2CONbits.TON = 1; // Start 32-bit Timer
}


//Interruption du timer 32 bits sur 2-3     
//unsigned char toggle = 0;

//void __attribute__((interrupt, no_auto_psv)) _T23Interrupt(void) {
//
//    IFS0bits.T3IF = 0; // Clear Timer3 Interrupt Flag
//    LED_ORANGE_2 = !LED_ORANGE_2;
////    if (toggle == 0) {
////        PWMSetSpeed(10, MOTEUR_DROIT);
////        PWMSetSpeed(10, MOTEUR_GAUCHE);
////        toggle = 1;
////    } else {
////        PWMSetSpeed(-10, MOTEUR_DROIT);
////        PWMSetSpeed(-10, MOTEUR_GAUCHE);
////        toggle = 0;
////    }
//SendMessage((unsigned char*) "Bonjour", 7);
//
//}

void SetFreqTimer1(float freq) {
    T1CONbits.TCKPS = 0b00; //00 = 1:1 prescaler value
    if (FCY / freq > 65535) {
        T1CONbits.TCKPS = 0b01; //01 = 1:8 prescaler value
        if (FCY / freq / 8 > 65535) {
            T1CONbits.TCKPS = 0b10; //10 = 1:64 prescaler value
            if (FCY / freq / 64 > 65535) {
                T1CONbits.TCKPS = 0b11; //11 = 1:256 prescaler value
                PR1 = (int) (FCY / freq / 256);
            } else
                PR1 = (int) (FCY / freq / 64);
        } else
            PR1 = (int) (FCY / freq / 8);
    } else
        PR1 = (int) (FCY / freq);
}

void InitTimer4(void) {
    //Timer1 pour horodater les mesures (1ms)
    T4CONbits.TON = 0; // Disable Timer
    T4CONbits.TCKPS = 0b10; //Prescaler
    T4CONbits.TCS = 0; //clock source = internal clock
    PR4 = 0x249f; //100Hz
    IFS1bits.T4IF = 0; // Clear Timer Interrupt Flag
    IEC1bits.T4IE = 1; // Enable Timer interrupt
    T4CONbits.TON = 1; // Enable Time
}

void __attribute__((interrupt, no_auto_psv)) _T4Interrupt(void) {
    IFS1bits.T4IF = 0;
    timestamp = timestamp + 1;
    timestop = timestop + 1;
    time = time + 1;
    OperatingSystemLoop();
    if(CB_RX2_IsDataAvailable()){
        CB_RX2_Get();
    }
}

void SetFreqTimer4(float freq) {
    T4CONbits.TCKPS = 0b00; //00 = 1:1 prescaler value
    if (FCY / freq > 65535) {
        T4CONbits.TCKPS = 0b01; //01 = 1:8 prescaler value
        if (FCY / freq / 8 > 65535) {
            T4CONbits.TCKPS = 0b10; //10 = 1:64 prescaler value
            if (FCY / freq / 64 > 65535) {
                T4CONbits.TCKPS = 0b11; //11 = 1:256 prescaler value
                PR4 = (int) (FCY / freq / 256);
            } else
                PR4 = (int) (FCY / freq / 64);
        } else
            PR4 = (int) (FCY / freq / 8);
    } else
        PR4 = (int) (FCY / freq);
}
