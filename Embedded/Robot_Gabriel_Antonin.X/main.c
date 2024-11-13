#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <math.h>
#include "ChipConfig.h"
#include "IO.h"
#include"timer.h"
#include "PWM.h"
#include "ADC.h"
#include "Robot.h"
#include "main.h"
#include "UART.h"


unsigned char stateRobot;
unsigned int timerstarted = 0;

int main(void) {


    /********Initialisation oscillateur*******************************/
    InitOscillator();
    /*****************Configuration des input et output (IO)************************************/
    InitIO();
    InitTimer4();
    InitTimer1();

    SetFreqTimer4(1000);

    InitPWM();
    InitADC1();
    InitUART1();

    //PWMSetSpeedConsigne(-10, MOTEUR_GAUCHE);
    //PWMSetSpeedConsigne(-10, MOTEUR_DROIT);


    LED_BLANCHE_1 = 0;
    LED_BLEUE_1 = 0;
    LED_ORANGE_1 = 0;
    LED_ROUGE_1 = 0;
    LED_VERTE_1 = 0;


    LED_BLANCHE_2 = 0;
    LED_BLEUE_2 = 0;
    LED_ORANGE_2 = 0;
    LED_ROUGE_2 = 0;
    LED_VERTE_2 = 0;


    /***********************************************************************************************
            Boucle Principale
     ***********************************************************************************************/
    while (1) {

        if (INTER1 == 1) {
            timerstarted = 1;
            timestop = 0;
        }

//        SendMessageDirect((unsigned char*) "Bonjour", 7);;;
//        __delay32(40000000);

        if (ADCIsConversionFinished() == 1) {
            ADCClearConversionFinishedFlag();
            unsigned int * result = ADCGetResult();
            float volts = ((float) result [0])* 3.3 / 4096;
            robotState.distanceTelemetreExtremeGauche = 34 / volts - 5;
            volts = ((float) result [1])* 3.3 / 4096;
            robotState.distanceTelemetreGauche = 34 / volts - 5;
            volts = ((float) result [2])* 3.3 / 4096;
            robotState.distanceTelemetreCentre = 34 / volts - 5;
            volts = ((float) result [3])* 3.3 / 4096;
            robotState.distanceTelemetreDroit = 34 / volts - 5;
            volts = ((float) result [4])* 3.3 / 4096;
            robotState.distanceTelemetreExtremeDroit = 34 / volts - 5;
        }

    }

}

void OperatingSystemLoop(void) {
    if (timestop <= 60000 && timerstarted) {

        switch (stateRobot) {
            case STATE_ATTENTE:
                timestamp = 0;
                PWMSetSpeedConsigne(0, MOTEUR_DROIT);
                PWMSetSpeedConsigne(0, MOTEUR_GAUCHE);
                stateRobot = STATE_ATTENTE_EN_COURS;
            case STATE_ATTENTE_EN_COURS:
                if (timestamp > 1000)
                    stateRobot = STATE_AVANCE;
                break;
            case STATE_AVANCE:
                PWMSetSpeedConsigne(24, MOTEUR_DROIT);
                PWMSetSpeedConsigne(24, MOTEUR_GAUCHE);
                stateRobot = STATE_AVANCE_EN_COURS;
                break;
            case STATE_AVANCE_EN_COURS:

                SetNextRobotStateInAutomaticMode();
                break;
            case STATE_TOURNE_LENTEMENT_GAUCHE:
                PWMSetSpeedConsigne(11, MOTEUR_DROIT);
                PWMSetSpeedConsigne(18, MOTEUR_GAUCHE);
                stateRobot = STATE_TOURNE_GAUCHE_EN_COURS;
                break;
            case STATE_TOURNE_GAUCHE:
                PWMSetSpeedConsigne(6, MOTEUR_DROIT);
                PWMSetSpeedConsigne(20, MOTEUR_GAUCHE);
                stateRobot = STATE_TOURNE_GAUCHE_EN_COURS;
                break;

            case STATE_TOURNE_GAUCHE_EN_COURS:
                SetNextRobotStateInAutomaticMode();
                break;
            case STATE_TOURNE_LENTEMENT_DROITE:
                PWMSetSpeedConsigne(18, MOTEUR_DROIT);
                PWMSetSpeedConsigne(11, MOTEUR_GAUCHE);
                stateRobot = STATE_TOURNE_GAUCHE_EN_COURS;
                break;
            case STATE_TOURNE_DROITE:
                PWMSetSpeedConsigne(20, MOTEUR_DROIT);
                PWMSetSpeedConsigne(6, MOTEUR_GAUCHE);
                stateRobot = STATE_TOURNE_DROITE_EN_COURS;
                break;

            case STATE_TOURNE_DROITE_EN_COURS:
                SetNextRobotStateInAutomaticMode();
                break;

            case STATE_TOURNE_SUR_PLACE_GAUCHE:
                PWMSetSpeedConsigne(-13, MOTEUR_DROIT);
                PWMSetSpeedConsigne(13, MOTEUR_GAUCHE);
                stateRobot = STATE_TOURNE_SUR_PLACE_GAUCHE_EN_COURS;
                timestamp = 0;
                break;
            case STATE_TOURNE_SUR_PLACE_GAUCHE_EN_COURS:
                if (timestamp > 600) {
                    SetNextRobotStateInAutomaticMode();
                }
                break;
            case STATE_TOURNE_SUR_PLACE_DROITE:
                PWMSetSpeedConsigne(13, MOTEUR_DROIT);
                PWMSetSpeedConsigne(-13, MOTEUR_GAUCHE);
                stateRobot = STATE_TOURNE_SUR_PLACE_DROITE_EN_COURS;
                timestamp = 0;
                break;
            case STATE_TOURNE_SUR_PLACE_DROITE_EN_COURS:
                if (timestamp > 300) {
                    SetNextRobotStateInAutomaticMode();
                }
                break;
            default:
                stateRobot = STATE_ATTENTE;
                break;
        }
    } else {
        timerstarted = 0;
        PWMSetSpeedConsigne(0, MOTEUR_DROIT);
        PWMSetSpeedConsigne(0, MOTEUR_GAUCHE);

    }
}

unsigned char nextStateRobot = 0;

void SetNextRobotStateInAutomaticMode() {

    unsigned char positionObstacle = PAS_D_OBSTACLE;

    // Dtermination de la position des obstacles en fonction des ééètlmtres
    if (EtatDroiteExtreme && !EtatDroite && !EtatCentre && !EtatGauche && !EtatGaucheExtreme)
        positionObstacle = OBSTACLE_TRES_A_DROITE;

    else if (!EtatDroiteExtreme && !EtatDroite && !EtatCentre && !EtatGauche && EtatGaucheExtreme)
        positionObstacle = OBSTACLE_TRES_A_GAUCHE;

    else if (EtatDroite && !EtatCentre && !EtatGauche) //Obstacle àdroite
        positionObstacle = OBSTACLE_A_DROITE;

    else if (!EtatDroite && !EtatCentre && EtatGauche) //Obstacle àgauche
        positionObstacle = OBSTACLE_A_GAUCHE;

    else if (EtatCentre || (EtatDroite && EtatGauche)) {
        if (robotState.distanceTelemetreDroit > robotState.distanceTelemetreGauche) {
            positionObstacle = OBSTACLE_CENTRE_GAUCHE;
        } else {
            positionObstacle = OBSTACLE_EN_FACE;
        }
    } else if (!EtatDroite && !EtatCentre && !EtatGauche) //pas d?obstacle
        positionObstacle = PAS_D_OBSTACLE;

    //éDtermination de lé?tat àvenir du robot
    if (positionObstacle == PAS_D_OBSTACLE)
        nextStateRobot = STATE_AVANCE;
    else if (positionObstacle == OBSTACLE_TRES_A_DROITE)
        nextStateRobot = STATE_TOURNE_LENTEMENT_GAUCHE;
    else if (positionObstacle == OBSTACLE_TRES_A_GAUCHE)
        nextStateRobot = STATE_TOURNE_LENTEMENT_DROITE;
    else if (positionObstacle == OBSTACLE_A_DROITE)
        nextStateRobot = STATE_TOURNE_GAUCHE;
    else if (positionObstacle == OBSTACLE_A_GAUCHE)
        nextStateRobot = STATE_TOURNE_DROITE;
    else if (positionObstacle == OBSTACLE_EN_FACE)
        nextStateRobot = STATE_TOURNE_SUR_PLACE_GAUCHE;
    else if (positionObstacle == OBSTACLE_CENTRE_GAUCHE)
        nextStateRobot = STATE_TOURNE_SUR_PLACE_DROITE;


    //Si l?on n?est pas dans la transition de lé?tape en cours
    if (nextStateRobot != stateRobot - 1)
        stateRobot = nextStateRobot;


}

void DetectionCapteur(void) {

    if (robotState.distanceTelemetreExtremeGauche < 28) {
        LED_BLANCHE_1 = 1;
        EtatGaucheExtreme = 1;
    } else {
        LED_BLANCHE_1 = 0;
        EtatGaucheExtreme = 0;
    }
    if (robotState.distanceTelemetreGauche < 39) {
        LED_BLEUE_1 = 1;
        EtatGauche = 1;
    } else {
        LED_BLEUE_1 = 0;
        EtatGauche = 0;
    }
    if (robotState.distanceTelemetreCentre < 30) {
        LED_ORANGE_1 = 1;
        EtatCentre = 1;
    } else {
        LED_ORANGE_1 = 0;
        EtatCentre = 0;
    }
    if (robotState.distanceTelemetreDroit < 39) {
        LED_ROUGE_1 = 1;
        EtatDroite = 1;
    } else {
        LED_ROUGE_1 = 0;
        EtatDroite = 0;
    }
    if (robotState.distanceTelemetreExtremeDroit < 28) {
        LED_VERTE_1 = 1;
        EtatDroiteExtreme = 1;
    } else {
        LED_VERTE_1 = 0;
        EtatDroiteExtreme = 0;
    }
}
