#ifndef UART_H

#define UART_H

void InitUART1(void);
void InitUART2(void);
void SendMessageDirect(unsigned char* message, int length);

#endif /* UART_H */