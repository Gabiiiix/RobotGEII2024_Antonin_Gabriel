#include "asservissement.h"
#include "Utilities.h"
#include "UART_Protocol.h"
#include "QEI.h"
#include "ToolBox.h"

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
    FlagConsigneR=1;
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

void SendPIDUpdateData(){
    unsigned char PIDPayload[80];
    getBytesFromDouble(PIDPayload, 0, PidX.erreur);
    getBytesFromDouble(PIDPayload,8,PidTheta.erreur);
    getBytesFromDouble(PIDPayload,16, ConsigneLineaire);
    getBytesFromDouble(PIDPayload,24, ConsigneAngulaire);
    getBytesFromDouble(PIDPayload, 32, PidX.corrP);
    getBytesFromDouble(PIDPayload,40,PidTheta.corrP);
    getBytesFromDouble(PIDPayload, 48, PidX.corrI);
    getBytesFromDouble(PIDPayload,56,PidTheta.corrI);
    getBytesFromDouble(PIDPayload, 64, PidX.corrD);
    getBytesFromDouble(PIDPayload,72,PidTheta.corrD);
    
    UartEncodeAndSendMessage(SEND_PID_ERROR_DATA,80,PIDPayload);
}

double Correcteur(volatile PidCorrector* PidCorr, double erreur){
    PidCorr->erreur = erreur;
    double erreurProportionnelle = LimitToInterval(erreur,-PidCorr->erreurProportionelleMax/PidCorr->Kp, PidCorr->erreurProportionelleMax/PidCorr->Kp);
//    double erreurProportionnelle = LimitToInterval(erreur,-50.0, 50.0);
    PidCorr->corrP = PidCorr->Kp * erreurProportionnelle;
    
    PidCorr->erreurIntegrale += PidCorr->erreur/FREQ_ECH_QEI;
    PidCorr->erreurIntegrale = LimitToInterval(PidCorr->erreurIntegrale + PidCorr->erreur/FREQ_ECH_QEI, -PidCorr->erreurIntegraleMax / PidCorr->Ki,PidCorr->erreurIntegraleMax / PidCorr->Ki);
    PidCorr->corrI = PidCorr->Ki*PidCorr->erreurIntegrale;
    
    double erreurDerivee = (erreur - PidCorr->epsilon_1)*FREQ_ECH_QEI;
    double deriveeBornee = LimitToInterval(erreurDerivee, -PidCorr->erreurDeriveeMax/PidCorr->Kd, PidCorr->erreurDeriveeMax/PidCorr->Kd);
    PidCorr->epsilon_1 = erreur;
    PidCorr->corrD = deriveeBornee * PidCorr->Kd;
    
    return PidCorr->corrP+PidCorr->corrI+PidCorr->corrD;
    }
