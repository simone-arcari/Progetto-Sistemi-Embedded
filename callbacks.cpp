#include "mbed.h"
#include "callbacks.h"

extern DigitalOut registerSelect;
extern DigitalOut readWrite;
extern DigitalOut enable;
extern DigitalOut dataLine4;
extern DigitalOut dataLine5;
extern DigitalOut dataLine6;
extern DigitalOut dataLine7;


/*
 *  Callbacks for High/Low state level
*/
void setRegisterSelect(int state)
{
    if (state)
    {
        registerSelect = 1;
    } else {
        registerSelect = 0;
    }
}

void setReadWrite(int state)
{
    if (state)
    {
        readWrite = 1;
    } else {
        readWrite = 0;
    }
}

void setEnable(int state)
{
    if (state)
    {
        enable = 1;
    } else {
        enable = 0;
    }
}

void setDataLine4(int state)
{
    if (state)
    {
        dataLine4 = 1;
    } else {
        dataLine4 = 0;
    }
}

void setDataLine5(int state)
{
    if (state)
    {
        dataLine5 = 1;
    } else {
        dataLine5 = 0;
    }
}

void setDataLine6(int state)
{
    if (state)
    {
        dataLine6 = 1;
    } else {
        dataLine6 = 0;
    }
}

void setDataLine7(int state)
{
    if (state)
    {
        dataLine7 = 1;
    } else {
        dataLine7 = 0;
    }
}

void displayDelay(int ms)
{
    ThisThread::sleep_for(chrono::milliseconds(ms));
}