#ifndef TOOLBOX_H
#define TOOLBOX_H
#define PI 3.141592653589793
float Abs(float value);
float Max(float value, float value2);
float Min(float value, float value2);
float LimitToInterval(float value, float lowLimit, float highLimit);
float RadianToDegree(float value);
float DegreeToRadian(float value);
float ModuloByAngle(float from, float to);
float PythagorTheorem(float x, float y);
float distancePointToSegment(float px, float py, float ax, float ay, float bx, float by);
float NormalizeAngle(float angle);
#endif /* TOOLBOX_H */