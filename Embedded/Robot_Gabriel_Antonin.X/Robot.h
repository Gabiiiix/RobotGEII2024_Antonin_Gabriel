#ifndef ROBOT_H
#define ROBOT_H

typedef struct robotStateBITS {

    union {

        struct {
            unsigned char taskEnCours;
            float vitesseGaucheConsigne;
            float vitesseGaucheCommandeCourante;
            float vitesseDroiteConsigne;
            float vitesseDroiteCommandeCourante;
            float distanceTelemetreExtremeGauche;
            float distanceTelemetreGauche;
            float distanceTelemetreCentre;
            float distanceTelemetreDroit;
            float distanceTelemetreExtremeDroit;

            float vitesseDroitFromOdometry;
            float vitesseGaucheFromOdometry;
            float vitesseLineaireFromOdometry;
            float vitesseAngulaireFromOdometry;
            float xPosFromOdometry_1;
            float xPosFromOdometry;
            float yPosFromOdometry_1;
            float yPosFromOdometry;
            float angleRadianFromOdometry_1;
            float angleRadianFromOdometry;
            
            float vitesseAngulaireGhost;
            float vitesseLineaireGhost;
            float angleGhost;
            float xPosGhost;
            float yPosGhost;
            float accelerationAngulaireGhost;
            float accelerationLineaireGhost;
            float vitesseAngulaireMax;
            float vitesseLineaireMax;
            
            float consigneVitesseLineaire;
            float consigneVitesseAngulaire;
            float consigneLineaireX;
            float consigneLineaireY;
            float consigneTheta;
            
            double CorrectionVitesseLineaire;
            double CorrectionVitesseAngulaire;

        };
    };
} ROBOT_STATE_BITS;
extern volatile ROBOT_STATE_BITS robotState;




#endif /* ROBOT_H */