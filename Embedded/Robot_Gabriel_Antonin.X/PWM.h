#ifndef PWM_H
#define PWM_H

#define MOTEUR_DROIT 0
#define MOTEUR_GAUCHE 1

void InitPWM(void);
void PWMUpdateSpeed();
void PWMSetSpeedConsigne(float vitesseEnPourcents, char moteur);
void UpdateAsservissement();
void PWMSetSpeedConsignePolaire(double VitesseLineaire, double VitesseAngulaire);
void UpdateGhostData();
#endif /* PWM_H */