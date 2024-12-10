#ifndef CB_TX2_H
#define	CB_TX2_H

void SendMessage2(unsigned char* message, int length);
void CB_TX2_Add(unsigned char value);
unsigned char CB_TX2_Get(void);
unsigned char CB_TX2_IsTranmitting(void);
void SendOne2();
int CB_TX2_GetDataSize(void);
int CB_TX2_GetRemainingSize(void);

#endif