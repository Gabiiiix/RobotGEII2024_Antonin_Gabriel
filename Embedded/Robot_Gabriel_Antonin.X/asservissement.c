#include "asservissement.h"
#include "Utilities.h"
#include "UART_Protocol.h"


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
    getBytesFromDouble(PIDPayload, 0, PidData->Kp );
    getBytesFromFloat(PIDPayload, 8,PidData->erreurProportionelleMax );
    getBytesFromFloat(PIDPayload, 16, PidData->Ki );
    getBytesFromFloat(PIDPayload, 24, PidData->erreurIntegraleMax );
    getBytesFromFloat(PIDPayload, 32, PidData->Kd );
    getBytesFromFloat(PIDPayload, 40, PidData->erreurDeriveeMax );
    PIDPayload[49] = c;
    
    UartEncodeAndSendMessage(SEND_PID_DATA, 49, PIDPayload);
}