#ifndef TIMER_H
#define TIMER_H

extern unsigned long timestamp;
extern unsigned long timestop;
extern unsigned long triggerCapteur;
extern unsigned long time;

void InitTimer23(void);
void InitTimer1(void);
void InitTimer4(void);
void SetFreqTimer1(float freq);
void SetFreqTimer4(float freq);
#endif /* TIMER_H */