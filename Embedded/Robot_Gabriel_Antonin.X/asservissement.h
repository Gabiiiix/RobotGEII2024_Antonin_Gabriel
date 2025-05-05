#ifndef asservissement_H
#define	asservissement_H

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

#endif asservissement_H