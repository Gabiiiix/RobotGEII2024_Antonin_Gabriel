#ifndef asservissement_H
#define	asservissement_H
#define SEND_PID_DATA 0x0070

typedef struct _PidCorrector
{
    double Kp;
    double Ki;
    double Kd;
    double erreurProportionelleMax;
    double erreurIntegraleMax;
    double erreurDeriveeMax;
    double erreurIntegrale;
    double epsilon_1;
    double erreur;
    //For Debug only
    double corrP;
    double corrI;
    double corrD;
}PidCorrector;

extern double ConsigneLineaire, ConsigneAngulaire;
extern short FlagConsigneR;

extern volatile PidCorrector PidX;
extern volatile PidCorrector PidTheta;


void SetupPidAsservissement(volatile PidCorrector* PidCorr, double Kp, double Ki, double Kd, double proportionelleMax, double integralMax, double deriveeMax);
void SendPIDData(volatile PidCorrector* PidCorr, char c);

#endif /* asservissement_H */