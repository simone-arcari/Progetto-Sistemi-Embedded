#ifndef PID_H
#define PID_H

#include "mbed.h"

class PID {
public:
    PID(float kp, float ki, float kd);
    float calculate(float setpoint, float measured_value);

private:
    float kp_;  // Guadagno proporzionale
    float ki_;  // Guadagno integrale
    float kd_;  // Guadagno derivativo
    float previous_error_;
    float integral_;
    Timer timer_;  // Timer di Mbed
};

#endif // PID_H
