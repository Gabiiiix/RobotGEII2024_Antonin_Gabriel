#include "asservissement.h"
#include "Utilities.h"
#include "UART_Protocol.h"
#include "QEI.h"
#include "ToolBox.h"
#include "Robot.h"

volatile PidCorrector PidX;
volatile PidCorrector PidTheta;

float thetaArret;
float thetaRestant;
float incrementTheta;

float distanceRestant;
float distanceArret;
float incrementDistance;

float consigneLineaire_1 = 0;
float consigneTheta_1 = 0;

void SetupPidAsservissement(volatile PidCorrector* PidCorr, double Kp, double Ki, double Kd, double proportionelleMax, double integralMax, double deriveeMax)
{
    PidCorr->Kp = Kp;
    PidCorr->erreurProportionelleMax = proportionelleMax; //On limite la correction due au Kp
    PidCorr->Ki = Ki;
    PidCorr->erreurIntegraleMax = integralMax; //On limite la correction due au Ki
    PidCorr->Kd = Kd;
    PidCorr->erreurDeriveeMax = deriveeMax;
    FlagConsigneR=1;
    
    robotState.accelerationAngulaireGhost = 0.02;
    robotState.accelerationLineaireGhost = 0.05;
    
    robotState.vitesseAngulaireMax = 0.1;
    robotState.vitesseLineaireMax = 1;
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
    unsigned char PIDPayload[40];
    getBytesFromDouble(PIDPayload, 0, PidX.erreur);
    getBytesFromDouble(PIDPayload,8,PidTheta.erreur);
    getBytesFromDouble(PIDPayload,16, ConsigneLineaire);
    getBytesFromDouble(PIDPayload,24, ConsigneAngulaire);
    getBytesFromDouble(PIDPayload, 32, PidX.corrP);
    
    UartEncodeAndSendMessage(SEND_PID_ERROR_DATA_1,40,PIDPayload);
    
    getBytesFromDouble(PIDPayload,0,PidTheta.corrP);
    getBytesFromDouble(PIDPayload, 8, PidX.corrI);
    getBytesFromDouble(PIDPayload,16,PidTheta.corrI);
    getBytesFromDouble(PIDPayload, 24, PidX.corrD);
    getBytesFromDouble(PIDPayload,32,PidTheta.corrD);
    
    UartEncodeAndSendMessage(SEND_PID_ERROR_DATA_2,40,PIDPayload);
    
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

void Ghost(){
    switch(stateGhost){
        case (stateGhost.IDLE) :
            if((consigneLineaire_1 != robotState.consigneLineaire || consigneTheta_1 != robotState.consigneTheta) && robotState.vitesseLineaireFromOdometry == 0){
                stateGhost = stateGhost.ROTATION;
                consigneLineaire_1 = robotState.consigneLineaire;
                consigneTheta_1 = robotState.consigneTheta;
            }
            break;
            
        
            
        case(stateGhost.ROTATION):
            thetaRestant = ModuloByAngle(robotState.angleGhost,robotState.consigneTheta) - robotState.angleGhost;
            thetaArret = robotState.vitesseAngulaireGhost / 2.0 * robotState.accelerationAngulaireGhost;
            incrementTheta = robotState.vitesseAngulaireGhost * (1.0/FREQ_ECH_QEI);
            
            if (robotState.vitesseAngulaireGhost < 0){
                thetaArret = -thetaArret;
            }
            
            if(((thetaArret >= 0 && thetaRestant >=0) || (thetaArret <= 0 && thetaRestant <= 0)) && Abs(thetaRestant) >= Abs(thetaArret)){
                if(thetaRestant > 0){
                    robotState.vitesseAngulaireGhost = Min(robotState.vitesseAngulaireGhost + robotState.accelerationAngulaireGhost/FREQ_ECH_QEI, robotState.vitesseAngulaireMax);
                }
                else if(thetaRestant > 0){
                    robotState.vitesseAngulaireGhost = Max(robotState.vitesseAngulaireGhost - robotState.accelerationAngulaireGhost/FREQ_ECH_QEI, -robotState.vitesseAngulaireMax);
                }
            }
            else{
                if(robotState.vitesseAngulaireGhost > 0){
                    robotState.vitesseAngulaireGhost = Max(robotState.vitesseAngulaireGhost - robotState.accelerationAngulaireGhost/FREQ_ECH_QEI, 0);
                }
                else if (robotState.vitesseAngulaireGhost < 0){
                    robotState.vitesseAngulaireGhost = Min(robotState.vitesseAngulaireGhost + robotState.accelerationAngulaireGhost/FREQ_ECH_QEI, 0);
                }
                
                if(Abs(thetaRestant) < Abs(incrementTheta)){
                    incrementTheta = thetaRestant;
                }
            }
            
            robotState.angleGhost  = robotState.angleGhost + incrementTheta;
            
            
            if (robotState.vitesseAngulaireGhost == 0 && Abs(thetaRestant) < 0.01){
                robotState.angleGhost = robotState.consigneTheta;
                stateGhost = stateGhost.DEPLACEMENTLINEAIRE;
            }
            break;
            
        
        case(stateGhost.DEPLACEMENTLINEAIRE):
            distanceRestant = PythagorTheorem(robotState.xPosGhost,robotState.yPosGhost) - robotState.consigneLineaire;
            distanceArret = robotState.vitesseLineaireGhost / 2.0 * robotState.accelerationLineaireGhost;
            incrementDistance = robotState.vitesseLineaireGhost * (1.0/FREQ_ECH_QEI);
             
            if((distanceRestant >= 0 && distanceArret >= 0) && (distanceRestant >= distanceArret)){
                
            }
            break;
        
            
        }

            
            
                    
             
}
