#include "Toolbox.h"
#include <math.h>

float Abs(float value) {
    if (value >= 0)
        return value;
    else return -value;
}

float Max(float value, float value2) {
    if (value > value2)
        return value;
    else
        return value2;
}

float Min(float value, float value2) {
    if (value < value2)
        return value;
    else
        return value2;
}

float LimitToInterval(float value, float lowLimit, float highLimit) {
    if (value > highLimit)
        value = highLimit;
    else if (value < lowLimit)
        value = lowLimit;
    return value;
}

float RadianToDegree(float value) {
    return value / PI * 180.0;
}

float DegreeToRadian(float value) {
    return value * PI / 180.0;
}

float ModuloByAngle(float from, float to) {
    float diff = to - from;
    while (diff > PI) diff -= 2 * PI;
    while (diff < -PI) diff += 2 * PI;
    return diff;
}

float PythagorTheorem(float x, float y){
    float x_square = x*x;
    float y_square = y*y;
    float result = sqrt(x_square+y_square);
    return result;
}


float distancePointToSegment(float px, float py, float ax, float ay, float bx, float by)
{
    float A = px - ax;
    float B = py - ay;
    float C = bx - ax;
    float D = by - ay;

    float dot = A * C + B * D;
    float len_sq = C * C + D * D;

    float param = -1.0;
    if (len_sq != 0.0)
        param = dot / len_sq;

    float xx, yy;

    if (param < 0.0) {
        xx = ax;
        yy = ay;
    }
    else if (param > 1.0) {
        xx = bx;
        yy = by;
    }
    else {
        xx = ax + param * C;
        yy = ay + param * D;
    }

    float dx = px - xx;
    float dy = py - yy;

    return sqrt(dx * dx + dy * dy);
}

float waypointDevant(float rx, float ry, float theta, float wx, float wy)
{
    float dirx = cosf(theta);
    float diry = sinf(theta);

    float vx = wx - rx;
    float vy = wy - ry;

    float dot = dirx * vx + diry * vy;

    return (dot >= 0.0f);  // true = waypoint devant
}

