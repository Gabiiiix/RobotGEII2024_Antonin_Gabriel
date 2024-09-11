#ifndef PWM_H
#define PWM_H

#define MOTEUR_DROIT 1
#define MOTEUR_GAUCHE 0

void InitPWM(void);
void PWMSetSpeed(float vitesseEnPourcents, int moteur);
#endif /* PWM_H */