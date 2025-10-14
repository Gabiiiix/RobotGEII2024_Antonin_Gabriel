#include <xc.h>
#include "UART_Protocol.h"
#include "UART.h"
#include "main.h"
#include "IO.h"
#include "CB_TX1.h"
#include "CB_TX2.h"
#include "asservissement.h"
#include "Utilities.h"
#include "Robot.h"
#include "timer.h"

#define LINEAIRE 0
#define ANGULAIRE 1

double ConsigneLineaire;
double ConsigneAngulaire;

short FlagConsigneR = 0;
short FlagPIDCoeff = 0;

double test = 0;


unsigned char UartCalculateChecksum(int msgFunction, int msgPayloadLength, unsigned char* msgPayload) {
    //Fonction prenant entree la trame et sa longueur pour calculer le checksum
    unsigned char checksum = 0xFE;
    checksum = checksum ^ (msgFunction >> 8);
    checksum = checksum ^ (msgFunction & 0x00FF);
    checksum = checksum ^ (msgPayloadLength >> 8);
    checksum = checksum ^ (msgPayloadLength & 0x00FF);

    for (int i = 0; i < msgPayloadLength; i++) {
        checksum = checksum ^ msgPayload[i];
    }

    return checksum;
}

void UartEncodeAndSendMessage(int msgFunction, int msgPayloadLength, unsigned char* msgPayload) {
    int msgLength = msgPayloadLength + 6;
    unsigned char msg[msgLength];

    msg[0] = 0b11111110;
    msg[1] = msgFunction >> 8;
    msg[2] = msgFunction & 0x00FF;
    msg[3] = msgPayloadLength >> 8;
    msg[4] = msgPayloadLength & 0x00FF;

    for (int i = 0; i < msgPayloadLength; i++) {
        msg[i + 5] = msgPayload[i];
    }
    msg[msgLength - 1] = UartCalculateChecksum(msgFunction, msgPayloadLength, msgPayload);

    SendMessageDirect(msg, msgLength);
}

int msgDecodedFunction = 0;
int msgDecodedPayloadLength = 0;
unsigned char msgDecodedPayload[128];
int msgDecodedPayloadIndex = 0;
unsigned char receivedChecksum = 0;
unsigned char calculatedChecksum = 0;


void UartDecodeMessage(unsigned char c) {
    static int rcvState = 0;

    switch (rcvState) {
        case Waiting:
            if (c == 0xFE) {
                rcvState = FunctionMSB;
            }
            break;

        case FunctionMSB:
            msgDecodedFunction = c << 8;
            rcvState = FunctionLSB;
            break;

        case FunctionLSB:
            msgDecodedFunction = msgDecodedFunction | c;
            rcvState = PayloadLengthMSB;
            break;

        case PayloadLengthMSB:
            msgDecodedPayloadLength = c << 8;
            rcvState = PayloadLengthLSB;
            break;

        case PayloadLengthLSB:
            msgDecodedPayloadLength = msgDecodedPayloadLength | c;
            rcvState = Payload;
            break;

        case Payload:
            msgDecodedPayload[msgDecodedPayloadIndex] = c;
            msgDecodedPayloadIndex++;

            if (msgDecodedPayloadIndex > msgDecodedPayloadLength - 1) {
                rcvState = CheckSum;
                msgDecodedPayloadIndex = 0;
            }
            break;

        case CheckSum:
            receivedChecksum = c;
            calculatedChecksum = UartCalculateChecksum(msgDecodedFunction, msgDecodedPayloadLength, msgDecodedPayload);

            if (calculatedChecksum == receivedChecksum) {
                UartProcessDecodedMessage(msgDecodedFunction, msgDecodedPayloadLength, msgDecodedPayload);
            }
            rcvState = Waiting;
            break;

        default:
            rcvState = Waiting;
            break;
    }
}

void UartProcessDecodedMessage(int function, int payloadLength, unsigned char* payload) {
    int Numled;
    switch (function) {
        case 0x0020:
            Numled = payload[0];

            if (Numled == 0) {
                LED_VERTE_2 = payload[1];
            } else if (Numled == 1) {
                LED_ROUGE_2 = payload[1];
            } else if (Numled == 2) {
                LED_ORANGE_2 = payload[1];
            } else if (Numled == 3) {
                LED_BLEUE_2 = payload[1];
            } else if (Numled == 4) {
                LED_BLANCHE_2 = payload[1];
            }
            break;
            
        case 0x0071:
            ConsigneLineaire = getDouble(payload,0);
            ConsigneAngulaire = getDouble(payload,8);
            break;
            
        case 0x0072:
            test = getDouble(payload,32);
            SetupPidAsservissement(&PidX, getDouble(payload,0),getDouble(payload,8),getDouble(payload,16),getDouble(payload,24),getDouble(payload,32),getDouble(payload,40));
            SetupPidAsservissement(&PidTheta, getDouble(payload,48),getDouble(payload,56),getDouble(payload,64),getDouble(payload,72),getDouble(payload,80),getDouble(payload,88));
            FlagPIDCoeff = 1;
            break;
            
        case 0x0100:
            robotState.consigneLineaire = getFloat(payload, 0);
            robotState.consigneTheta = getFloat(payload, 4);
            break;
           
//        case 0x0040:
//
//            break;
    }
}
//*************************************************************************/
//Fonctions correspondant aux messages
//*************************************************************************/