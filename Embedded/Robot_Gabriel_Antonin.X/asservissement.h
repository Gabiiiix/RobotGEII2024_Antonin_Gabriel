#ifndef asservissement_H
#define	asservissement_H
#define SEND_PID_DATA 0x0070
#define SEND_PID_ERROR_DATA_1 0x0075
#define SEND_PID_ERROR_DATA_2 0x0076



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
extern short FlagPIDCoeff;


enum stateGhost
{
    IDLE,
    ROTATION,
    DEPLACEMENTLINEAIRE
};
short stateGhost;

extern volatile PidCorrector PidX;
extern volatile PidCorrector PidTheta;


void SetupPidAsservissement(volatile PidCorrector* PidCorr, double Kp, double Ki, double Kd, double proportionelleMax, double integralMax, double deriveeMax);
void SendPIDData(volatile PidCorrector* PidCorr, char c);
double Correcteur(volatile PidCorrector* PidCorr, double erreur);
void Ghost();
void SendPIDUpdateData();

#endif /* asservissement_H */