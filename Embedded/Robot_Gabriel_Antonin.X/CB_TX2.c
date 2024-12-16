#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include "CB_TX2.h"

#define CBTX2_BUFFER_SIZE 128

int cbTx2Head;
int cbTx2Tail;

unsigned char cbTx2Buffer[CBTX2_BUFFER_SIZE];
unsigned char isTransmitting2 = 0;

void SendMessage2(unsigned char* message, int length) {
    unsigned char i = 0;
    if (CB_TX2_GetRemainingSize() > length) {
        //On peut écrire le message
        for (i = 0; i < length; i++)
            CB_TX2_Add(message[i]);
        if (!isTransmitting2)
            SendOne2();
    }
}

void CB_TX2_Add(unsigned char value) {
    cbTx2Buffer[cbTx2Head] = value;
    cbTx2Head++;
    if(cbTx2Head == CBTX2_BUFFER_SIZE){
        cbTx2Head=0;
    }
    
}

unsigned char CB_TX2_Get(void) {
    unsigned char value = cbTx2Buffer[cbTx2Tail];
    cbTx2Tail++;
        if(cbTx2Tail == CBTX2_BUFFER_SIZE){
            cbTx2Tail=0;
        }
    return value;
}

void __attribute__((interrupt, no_auto_psv)) _U2TXInterrupt(void) {
    IFS1bits.U2TXIF = 0; // clear TX interrupt flag
    if (cbTx2Tail != cbTx2Head) {
        SendOne2();
    } else
        isTransmitting2 = 0;
}

void SendOne2() {
    isTransmitting2 = 1;
    unsigned char value = CB_TX2_Get();
    U2TXREG = value; // Transmit one character
}

unsigned char CB_TX2_IsTranmitting(void) {
    return isTransmitting2;
}

int CB_TX2_GetDataSize(void) {
    //return size of data stored in circular buffer
    int dataSize = 0;
    if(cbTx2Head < cbTx2Tail){
         dataSize = ((cbTx2Head+CBTX2_BUFFER_SIZE) - cbTx2Head);
    }
    else{
         dataSize = (cbTx2Head - cbTx2Tail);
    }
    return dataSize;
}

int CB_TX2_GetRemainingSize(void) {
    //return size remaining stored in circular buffer
    int remainingSize = CBTX2_BUFFER_SIZE - CB_TX2_GetDataSize();

    return remainingSize;
}