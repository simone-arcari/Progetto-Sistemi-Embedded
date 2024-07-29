#include "pid.h"

PID::PID(float kp, float ki, float kd) 
    : kp_(kp), ki_(ki), kd_(kd), previous_error_(0.0), integral_(0.0) {
    timer_.start();  // Avvia il timer
}

float PID::calculate(float setpoint, float measured_value) {
    float error = setpoint - measured_value;

    float dt = timer_.elapsed_time().count();  // Usa elapsed_time e count
    timer_.reset();  // Resetta il timer per il prossimo ciclo

    float Pout = kp_ * error;

    integral_ += error * dt;
    float Iout = ki_ * integral_;

    float derivative = (error - previous_error_) / dt;
    float Dout = kd_ * derivative;

    float output = Pout + Iout + Dout;

    previous_error_ = error;

    return output;
}
