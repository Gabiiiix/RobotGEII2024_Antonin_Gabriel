#include "asservissement.h"
#include "Utilities.h"
#include "UART_Protocol.h"
#include "QEI.h"
#include "ToolBox.h"
#include "Robot.h"
#include <math.h>

volatile PidCorrector PidX;
volatile PidCorrector PidTheta;
volatile PidCorrector PidXGhost;
volatile PidCorrector PidThetaGhost;

float thetaArret;
float thetaRestant;
float incrementTheta;

float distanceRestant;
float distanceArret;
float incrementDistance;

float consigneLineaireX_1 = 0;
float consigneLineaireY_1 = 0;
float consigneTheta_1 = 0;

int i = 0;

enum stateGhost stateGhost = IDLE;

Waypoint listeWaypoints[] = {
    {0,1},
    {0.5,1},
    {0.5,2},
    {-0.5,2},
    {-0.5,1},
    {0,1},
    {0,1.5}
};

void SetupPidAsservissement(volatile PidCorrector* PidCorr, double Kp, double Ki, double Kd, double proportionelleMax, double integralMax, double deriveeMax)
{
    PidCorr->Kp = Kp;
    PidCorr->erreurProportionelleMax = proportionelleMax; //On limite la correction due au Kp
    PidCorr->Ki = Ki;
    PidCorr->erreurIntegraleMax = integralMax; //On limite la correction due au Ki
    PidCorr->Kd = Kd;
    PidCorr->erreurDeriveeMax = deriveeMax;
    FlagConsigneR=1;
    
    robotState.accelerationAngulaireGhost = 1;
    robotState.accelerationLineaireGhost = 1;
    
    robotState.vitesseAngulaireMax = 5;
    robotState.vitesseLineaireMax = 5;
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
        case (IDLE) :
            
            if( listeWaypoints[i].x != robotState.consigneLineaireX || listeWaypoints[i].y != robotState.consigneLineaireY){
                robotState.consigneLineaireX = listeWaypoints[i].x;
                robotState.consigneLineaireY = listeWaypoints[i].y;
                robotState.consigneTheta = atan2(robotState.consigneLineaireY - robotState.yPosGhost , robotState.consigneLineaireX - robotState.xPosGhost);
            }
            
            if ((fabs(NormalizeAngle(consigneTheta_1 - robotState.consigneTheta)) || robotState.consigneLineaireX != consigneLineaireX_1 || robotState.consigneLineaireY != consigneLineaireY_1) > 0.01 && fabs(robotState.vitesseLineaireFromOdometry) < 0.1) {
                stateGhost = ROTATION;
                consigneTheta_1 = robotState.consigneTheta;
                consigneLineaireX_1 = robotState.consigneLineaireX;
                consigneLineaireY_1 = robotState.consigneLineaireY;
                if( i < 6){
                i++;
                }
            }
            break;
            
        
            
        case(ROTATION):
            thetaRestant = ModuloByAngle(robotState.angleGhost,robotState.consigneTheta);
            thetaArret = (robotState.vitesseAngulaireGhost * robotState.vitesseAngulaireGhost) / 2.0 * robotState.accelerationAngulaireGhost;
            incrementTheta = robotState.vitesseAngulaireGhost * (1.0/FREQ_ECH_QEI);
            
            if (robotState.vitesseAngulaireGhost < 0){
                thetaArret = -thetaArret;
            }
            
            if(((thetaArret >= 0 && thetaRestant >=0) || (thetaArret <= 0 && thetaRestant <= 0)) && fabs(thetaRestant) >= fabs(thetaArret)){
                if(thetaRestant > 0){
                    robotState.vitesseAngulaireGhost = Min(robotState.vitesseAngulaireGhost + robotState.accelerationAngulaireGhost/FREQ_ECH_QEI, robotState.vitesseAngulaireMax);
                }
                else if(thetaRestant < 0){
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
                
                if(fabs(thetaRestant) < fabs(incrementTheta)){
                    incrementTheta = thetaRestant;
                }
            }
            
            robotState.angleGhost  = robotState.angleGhost + incrementTheta;
            
            
            if (robotState.vitesseAngulaireGhost == 0 && fabs(thetaRestant) < 0.01){
                robotState.angleGhost = robotState.consigneTheta;
                stateGhost = DEPLACEMENTLINEAIRE;
            }
            break;
            
        
        case(DEPLACEMENTLINEAIRE):
        {
            float dirX = cosf(robotState.angleRadianFromOdometry);
            float dirY = sinf(robotState.angleRadianFromOdometry);
        

            float dx = robotState.consigneLineaireX - robotState.xPosGhost;
            float dy = robotState.consigneLineaireY - robotState.yPosGhost;


            float distanceRestant = dx * dirX + dy * dirY;  

            float incrementDistance = robotState.vitesseLineaireGhost * (1.0f / FREQ_ECH_QEI);


            float accel = robotState.accelerationLineaireGhost / FREQ_ECH_QEI;
            
        
            if (distanceRestant > 0.001f)
            {
     
                    robotState.vitesseLineaireGhost += accel;
                if (robotState.vitesseLineaireGhost > robotState.vitesseLineaireMax)
                    robotState.vitesseLineaireGhost = robotState.vitesseLineaireMax;
            }
            else if (distanceRestant < 0.001f)
            {
                // Aller vers l'arrière
                robotState.vitesseLineaireGhost -= accel;
                if (robotState.vitesseLineaireGhost < -robotState.vitesseLineaireMax)
                    robotState.vitesseLineaireGhost = -robotState.vitesseLineaireMax;
            }
            else
            {
                robotState.vitesseLineaireGhost = 0;
            }

            incrementDistance = robotState.vitesseLineaireGhost * (1.0f / FREQ_ECH_QEI);


            if (fabs(distanceRestant) < fabs(incrementDistance))
                incrementDistance = distanceRestant;


            robotState.xPosGhost += incrementDistance * dirX;
            robotState.yPosGhost += incrementDistance * dirY;

            if (fabs(distanceRestant) < 0.01f && fabs(robotState.vitesseLineaireGhost) < 0.01f && fabs(robotState.vitesseLineaireGhost) < 0.01f)
            {
                robotState.xPosGhost = robotState.consigneLineaireX;
                robotState.yPosGhost = robotState.consigneLineaireY;
                robotState.vitesseLineaireGhost = 0;
                stateGhost = IDLE;
            }

            break;
        }
    }

            
            
                    
             
}
