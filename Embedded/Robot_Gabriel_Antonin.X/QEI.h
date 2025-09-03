#ifndef QEI_H
#define	QEI_H

void InitQEI1();
void InitQEI2();
void QEIUpdateData();
void SendPositionData();

#define FREQ_ECH_QEI 250

static float QeiDroitPosition_T_1 = 0.0;
static float QeiDroitPosition = 0.0;
static float QeiGauchePosition_T_1 = 0.0;
static float QeiGauchePosition = 0.0;
static float delta_d = 0.0;
static float delta_g = 0.0;

#endif	/* QEI_H */

