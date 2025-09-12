#include "asservissement.h"
#include "Utilities.h"
#include "UART_Protocol.h"
#include "QEI.h"

volatile PidCorrector PidX;
volatile PidCorrector PidTheta;

void SetupPidAsservissement(volatile PidCorrector* PidCorr, double Kp, double Ki, double Kd, double proportionelleMax, double integralMax, double deriveeMax)
{
    PidCorr->Kp = Kp;
    PidCorr->erreurProportionelleMax = proportionelleMax; //On limite la correction due au Kp
    PidCorr->Ki = Ki;
    PidCorr->erreurIntegraleMax = integralMax; //On limite la correction due au Ki
    PidCorr->Kd = Kd;
    PidCorr->erreurDeriveeMax = deriveeMax;
}

void SendPIDData(volatile PidCorrector* PidData, char c){
    unsigned char PIDPayload[49];
    getBytesFromDouble(PIDPayload, 0, (double)PidData->Kp );
    getBytesFromDouble(PIDPayload, 8,(double)PidData->erreurProportionelleMax );
    getBytesFromDouble(PIDPayload, 16, (double)PidData->Ki );
    getBytesFromDouble(PIDPayload, 24, (double)PidData->erreurIntegraleMax );
    getBytesFromDouble(PIDPayload, 32, (double)PidData->Kd );
    getBytesFromDouble(PIDPayload, 40, (double)PidData->erreurDeriveeMax );
    PIDPayload[48] = c;

    UartEncodeAndSendMessage(SEND_PID_DATA, 49, PIDPayload);
}
double Correcteur(volatile PidCorrector* PidCorr, double erreur){
    PidCorr->erreur = erreur;
    double erreurProportionnelle = LimitToInterval(PidCorr->epsilon_1,-PidCorr->erreurProportionelleMax/PidCorr->Kp, PidCorr->erreurProportionelleMax/PidCorr->Kp);
    PidCorr->corrP = PidCorr->Kd * erreurProportionnelle;
    
    PidCorr->erreurIntegrale += PidCorr->erreur;
    PidCorr->erreurIntegrale = LimitToInterval(PidCorr->erreurIntegrale + PidCorr->epsilon_1/FREQ_ECH_QEI, -PidCorr->erreurIntegraleMax / PidCorr->Ki,PidCorr->erreurIntegraleMax / PidCorr->Ki);
    PidCorr->corrI = PidCorr->Ki*PidCorr->erreurIntegrale;
    
    double erreurDerivee = (erreur - PidCorr->epsilon_1)*FREQ_ECH_QEI;
    double deriveeBornee = LimitToInterval(erreurDerivee, -PidCorr->erreurDeriveeMax/PidCorr->Kd, PidCorr->erreurDeriveeMax/PidCorr->Kd);
    PidCorr->epsilon_1 = erreur;
    PidCorr->corrD = deriveeBornee * PidCorr->Kd;
    
    return PidCorr->corrP+PidCorr->corrI+PidCorr->corrD;
    }
