#ifndef PWM_H
#define PWM_H

#define MOTEUR_DROIT 1
#define MOTEUR_GAUCHE 0

void InitPWM(void);
void PWMUpdateSpeed();
void PWMSetSpeedConsigne(float vitesseEnPourcents, char moteur);
#endif /* PWM_H */