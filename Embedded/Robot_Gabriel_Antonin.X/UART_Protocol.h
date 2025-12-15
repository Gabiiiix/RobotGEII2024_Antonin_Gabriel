#ifndef UART_PROTOCOL_H
#define	UART_PROTOCOL_H

#define Waiting 0
#define FunctionMSB 1
#define FunctionLSB 2
#define PayloadLengthMSB 3
#define PayloadLengthLSB 4
#define Payload 5
#define CheckSum 6

#define Identification 1
#define CentreX 2
#define CentreY 3
#define Width 4
#define Height 5


typedef struct arUcoObject
{
    int Id;
    int cx;
    int cy;
    int width;
    int height;
}arUcoObject;

extern volatile arUcoObject Cible;

unsigned char UartCalculateChecksum(int msgFunction, int msgPayloadLength, unsigned char* msgPayload);
void UartEncodeAndSendMessage(int msgFunction, int msgPayloadLength, unsigned char* msgPayload);
void UartDecodeMessage(unsigned char c);
void UartProcessDecodedMessage(int function, int payloadLength, unsigned char* payload);
void UartDecodeMessageJevois(unsigned char c);

#endif	/* UART_PORTOCOL_H */

