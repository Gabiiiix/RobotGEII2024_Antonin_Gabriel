
#define POSITION_DATA 0x0061

#include <xc.h>
#include "QEI.h"
#include "Robot.h"
#include <math.h>
#include "IO.h"
#include "Utilities.h"
#include "UART_Protocol.h"
#include "timer.h"
#include "asservissement.h"

void InitQEI1() {
    QEI1IOCbits.SWPAB = 1; //QEAx and QEBx are swapped
    QEI1GECL = 0xFFFF;
    QEI1GECH = 0xFFFF;
    QEI1CONbits.QEIEN = 1; // Enable QEI Module
}

void InitQEI2() {
    QEI2IOCbits.SWPAB = 1; //QEAx and QEBx are not swapped
    QEI2GECL = 0xFFFF;
    QEI2GECH = 0xFFFF;
    QEI2CONbits.QEIEN = 1; // Enable QEI Module
}

void QEIUpdateData() {
    //On sauvegarde les anciennes valeurs
    QeiDroitPosition_T_1 = QeiDroitPosition;
    QeiGauchePosition_T_1 = QeiGauchePosition;

    //On actualise les valeurs des positions
    long QEI1RawValue = POS1CNTL;
    QEI1RawValue += ((long) POS1HLD << 16);
    long QEI2RawValue = POS2CNTL;
    QEI2RawValue += ((long) POS2HLD << 16);

    //Conversion en mm (regle pour la taille des roues codeuses)
    QeiDroitPosition = 0.00001620 * QEI1RawValue;
    QeiGauchePosition = -0.00001620 * QEI2RawValue;

    //Calcul des deltas de position
    delta_d = QeiDroitPosition - QeiDroitPosition_T_1;
    delta_g = QeiGauchePosition - QeiGauchePosition_T_1;

    //Calcul des vitesses
    //attention a remultiplier par la frequence d echantillonnage
    robotState.vitesseDroitFromOdometry = delta_d * FREQ_ECH_QEI;
    robotState.vitesseGaucheFromOdometry = delta_g * FREQ_ECH_QEI;
    robotState.vitesseLineaireFromOdometry = (robotState.vitesseGaucheFromOdometry + robotState.vitesseDroitFromOdometry) / 2;
    robotState.vitesseAngulaireFromOdometry = (robotState.vitesseDroitFromOdometry - robotState.vitesseGaucheFromOdometry) / DISTROUES;

    //Mise a jour du positionnement terrain a t-1
    robotState.xPosFromOdometry_1 = robotState.xPosFromOdometry;
    robotState.yPosFromOdometry_1 = robotState.yPosFromOdometry;
    robotState.angleRadianFromOdometry_1 = robotState.angleRadianFromOdometry;

    //Calcul des positions dans le referentiel du terrain
    robotState.xPosFromOdometry = robotState.vitesseLineaireFromOdometry * cos(robotState.vitesseAngulaireFromOdometry);
    robotState.yPosFromOdometry = robotState.vitesseLineaireFromOdometry * sin(robotState.vitesseAngulaireFromOdometry);
    robotState.angleRadianFromOdometry = robotState.vitesseAngulaireFromOdometry / FREQ_ECH_QEI;
    if (robotState.angleRadianFromOdometry > PI)
        robotState.angleRadianFromOdometry -= 2 * PI;
    if (robotState.angleRadianFromOdometry < -PI)
        robotState.angleRadianFromOdometry += 2 * PI;
    
    //L'asservissement et le PID S'active uniquement lorsqu'il a reçus les coefficients du C#
    if(FlagPIDCoeff){ 
        UpdateAsservissement();
    }
    
}


void SendPositionData() {
    unsigned char positionPayload[36];
    getBytesFromInt32(positionPayload, 0, timestamp);
    getBytesFromFloat(positionPayload, 4, (float) (robotState.xPosFromOdometry));
    getBytesFromFloat(positionPayload, 8, (float) (robotState.yPosFromOdometry));
    getBytesFromFloat(positionPayload, 12, (float) (robotState.angleRadianFromOdometry));
    getBytesFromFloat(positionPayload, 16, (float) (robotState.vitesseLineaireFromOdometry));
    getBytesFromFloat(positionPayload, 20, (float) (robotState.vitesseAngulaireFromOdometry));
    getBytesFromFloat(positionPayload, 24, (float) (robotState.vitesseDroitFromOdometry));
    getBytesFromFloat(positionPayload, 28, (float) (robotState.vitesseGaucheFromOdometry));
    positionPayload[35] = (char)(timeoscillo >> 24);
    positionPayload[34] = (char)((timeoscillo >> 16) & 0x00FF);
    positionPayload[33] = (char)((timeoscillo >> 8) & 0x0000FF);
    positionPayload[32] = (char)(timeoscillo & 0x000000FF);
    UartEncodeAndSendMessage(POSITION_DATA, 36, positionPayload);
}