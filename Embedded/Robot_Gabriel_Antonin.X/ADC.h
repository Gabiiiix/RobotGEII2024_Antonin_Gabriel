#ifndef ADC_H
#define	ADC_H

void InitADC1(void);
void ADC1StartConversionSequence(void);
unsigned char ADCIsConversionFinished(void);
unsigned int * ADCGetResult(void);
void ADCClearConversionFinishedFlag(void);

#endif	/* ADC_H */