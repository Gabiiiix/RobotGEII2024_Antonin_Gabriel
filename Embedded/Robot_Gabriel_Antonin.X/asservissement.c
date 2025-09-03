#include "asservissement.h"
#include "Utilities.h"
#include "UART_Protocol.h"

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
