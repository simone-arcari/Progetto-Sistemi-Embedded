#include "mbed.h"
#include "pid.h"
#include "callbacks.h"
#include "HD44780.h"

#define DAY_TO_NIGHT_THRESHOLD	0.15
#define NIGHT_TO_DAY_THRESHOLD  0.25

#define MAX_DAYLIGHT    0.85
#define MIN_DAYLIGHT    0.4

#define MAX_UMIDITY    1.0
#define MIN_UMIDITY    0.4

#define UMIDITY_REFERENCE   0.6

typedef enum
{
	E_DAY,
	E_NIGHT
}E_DAY_NIGHT_STATE;

typedef enum
{
	E_START,
    E_PULL_UP,
	E_PULL_DOWN,
	E_PASSIVE,
    E_PULL_UP_NIGHT
}E_STATE;

typedef float light_t;
typedef float umidity_t;
typedef float duty_t;

const light_t DAYLIGHT_REFERENCE = (MAX_DAYLIGHT+MIN_DAYLIGHT) / 2;

/*
 *  GPIO
 */
AnalogIn externalSensorLight(A0);
AnalogIn internalSensorLight(A1);
AnalogIn umiditySensor(A2);
AnalogIn userLightReference(A3);
AnalogIn userUmidityReference(A4);

PwmOut artificialLight(D3);
PwmOut electrochromicGlass(D5);
PwmOut nebulizer(D6);

// Create a PID object
PID pid1(1.0, 0.0, 0.0);    // pull up light    -->     artificialLight
PID pid2(1.0, 0.0, 0.5);    // pull down light  -->     electrochromicGlass
PID pid3(1.0, 0.0, 0.0);    // umidity          -->     nebulizer

bool pid1Running = false;
bool pid2Running = false;
bool pid3Running = false;

Ticker sensorsTicker;     // Ticker to read sensor data every 5 minutes
Ticker pidTicker;         // Ticker to call PID at regular intervals

Timer umidityTimer;       // Timer 1min for umidity

volatile bool sensorReadAllowed = false; // Flag to indicate data readiness

light_t     externalLight = 0;
light_t     internalLight = 0;
umidity_t   umidity = 0;
light_t     lightReference = 0;
umidity_t   umidityReference = 0;

// Display
DigitalOut registerSelect(D7);
DigitalOut readWrite(D8);
DigitalOut enable(D9);
DigitalOut dataLine4(D10);
DigitalOut dataLine5(D11);
DigitalOut dataLine6(D12);
DigitalOut dataLine7(D13);

// Forward declarations
E_DAY_NIGHT_STATE getCurrentDayNightState(E_DAY_NIGHT_STATE prevState, light_t externalLight);
void read_sensor_data();
void update_pid();
void startState(E_DAY_NIGHT_STATE &dayNightState, E_STATE &state);
void pullUpState(E_DAY_NIGHT_STATE &dayNightState, E_STATE &state);
void passiveState(E_DAY_NIGHT_STATE &dayNightState, E_STATE &state);
void pullDownState(E_DAY_NIGHT_STATE &dayNightState, E_STATE &state);
void pullUpNightState(E_DAY_NIGHT_STATE &dayNightState, E_STATE &state);
void newPrintDisplay(unsigned char* str);
void printSensorsSecondLine(int s1, int s2);


int main(void)
{
	/* Initial state */
    E_DAY_NIGHT_STATE dayNightState = E_DAY;
	E_STATE state = E_START;

    /* 
     *  Actuators 
     */

	artificialLight.period(0.001f);
    artificialLight.write(1.0);

    electrochromicGlass.period(0.001f);
    electrochromicGlass.write(1.0);

    nebulizer.period(0.001f);
    nebulizer.write(1.0);

    /* Sensors */
    read_sensor_data(); // Perform an initial sensor data reading
    sensorsTicker.attach(&read_sensor_data, 1min);

    /* PID Controller */
    Thread threadPID;
    threadPID.start(update_pid);

    /* 
     *  Display Callbacks & Init
     */
    register_callback(setRegisterSelect,    E_CALLBACK_RS);
    register_callback(setReadWrite,         E_CALLBACK_RW);
    register_callback(setEnable,            E_CALLBACK_EN);
    register_callback(setDataLine4,         E_CALLBACK_DATA4);
    register_callback(setDataLine5,         E_CALLBACK_DATA5);
    register_callback(setDataLine6,         E_CALLBACK_DATA6);
    register_callback(setDataLine7,         E_CALLBACK_DATA7);
    register_callback(displayDelay,         E_DELAY);

    init_LCD();
    putCommand(DISPLAY_CLEAR_CMD);          		        // clear display
	setCursor(0,0);    				                        // first line, first column, 1st position
	writeString((unsigned char*)"Display LCD 4bit", false); // message

    ThisThread::sleep_for(chrono::seconds(3));  // Sleep 3 seconds

    /* Infinite loop */
	while (true)
	{
		dayNightState = getCurrentDayNightState(dayNightState, externalLight);
        if (dayNightState == E_DAY)
        {
            lightReference = DAYLIGHT_REFERENCE;
            umidityReference = UMIDITY_REFERENCE;
        }
        else if (dayNightState == E_NIGHT)
        {
            lightReference = userLightReference.read();
            umidityReference = userUmidityReference.read();
        }
        
        /* Updated every 5 minutes */
        if (sensorReadAllowed)  
        {
            printf("Reading data from sensors...\n");

            externalLight = externalSensorLight.read();
            internalLight = internalSensorLight.read();
            umidity = umiditySensor.read();
            
            sensorReadAllowed = false;
        }
        
        
		/* State machine for controlling brightness */
        switch (state)
		{
			case E_START:
                startState(dayNightState, state);

			break;

			case E_PULL_UP:
                pullUpState(dayNightState, state);
				break;

            case E_PASSIVE:
                passiveState(dayNightState, state);
				break;

            case E_PULL_DOWN:
                pullDownState(dayNightState, state);
				break;

            case E_PULL_UP_NIGHT:
                pullUpNightState(dayNightState, state);
                break;

			default:
				break;
		}

        /* State machine for controlling umidity */
        static bool timerRunning = false;
        switch (dayNightState)
	    {
            case E_DAY:
                printf("Day\n");
                
                // Gestire umidita per 1 minuto  (TIMER)
                if (umidity < MIN_UMIDITY && !timerRunning)
                {
                    umidityTimer.start();
                    timerRunning = true;
                    pid3Running = true;
                }
                else {
                    pid3Running = false;
                }

                if (umidityTimer.elapsed_time().count() >= 60)
                {
                    umidityTimer.stop();
                    umidityTimer.reset();
                    timerRunning = false;
                    pid3Running = false;
                }

                break;

            case E_NIGHT:
                printf("Night\n");
                
                umidityTimer.stop();
                umidityTimer.reset();
                timerRunning = false;

                pid3Running = true;

                break;

            default:
				break;
        }
	}
	
	return 0;
}

E_DAY_NIGHT_STATE getCurrentDayNightState(E_DAY_NIGHT_STATE prevState, light_t externalLight)
{
	E_DAY_NIGHT_STATE currState;

	switch (prevState)
	{
		case E_DAY:
			if (externalLight < DAY_TO_NIGHT_THRESHOLD)
				currState = E_NIGHT;
			else
				currState = E_DAY;
			break;

		case E_NIGHT:
			if (externalLight > NIGHT_TO_DAY_THRESHOLD)
				currState = E_DAY;
			else
				currState = E_NIGHT;
			break;

		default:
			currState = prevState;
			break;
	}

	return currState;
}

void read_sensor_data() 
{
    sensorReadAllowed = true;
}

void update_pid()
{
    float output;

    while (true) {
        // feedback
        light_t internalLight = internalSensorLight.read();
        umidity_t umidity = umiditySensor.read();

        /*
        *  PID 1
        */
        if (pid1Running) {
            output = pid1.calculate(lightReference, internalLight); // Calculate the PID output
            artificialLight.write(output); // Write the PID output to the actuator (0.0 to 1.0)
        } else {
            artificialLight.write(0.0);
        }
        
        /*
        *  PID 2
        */
        if (pid2Running) {
            output = pid2.calculate(lightReference, internalLight); // Calculate the PID output
            electrochromicGlass.write(output); // Write the PID output to the actuator (0.0 to 1.0)
        } else {
            electrochromicGlass.write(0.0);
        }

        /*
        *  PID 3
        */
        if (pid3Running) {
            output = pid3.calculate(umidityReference, umidity); // Calculate the PID output
            nebulizer.write(output); // Write the PID output to the actuator (0.0 to 1.0)
        } else {
            nebulizer.write(0.0);
        }
    }
}

void startState(E_DAY_NIGHT_STATE &dayNightState, E_STATE &state)
{
    printf("Start\n");

    /* Check if we should pass to the next state */
	if (dayNightState == E_DAY) {
        if (internalLight < DAYLIGHT_REFERENCE)
        {
            state = E_PULL_UP; // edge 1
            newPrintDisplay((unsigned char*)"Pull Up");
        }
        else {
            state = E_PULL_DOWN; // edge 8
            newPrintDisplay((unsigned char*)"Pull Down");
        }
    } 

    if (dayNightState == E_NIGHT) 
    {
        state = E_PULL_UP_NIGHT;  // edge 7
        newPrintDisplay((unsigned char*)"Pull Up Night");
    }

    /*
     *  This state does nothing
     */
    pid1Running = false;
    pid2Running = false;
}

void pullUpState(E_DAY_NIGHT_STATE &dayNightState, E_STATE &state)
{
    printf("Pull Up\n");
    /* Check if we should pass to the next state */
    if (dayNightState == E_DAY)
    {
        if (internalLight > DAYLIGHT_REFERENCE)
        {
            state = E_PASSIVE;  // edge 2
            newPrintDisplay((unsigned char*)"Passive");
            return;
        }
    }
    else if (dayNightState == E_NIGHT)
    {
        state = E_PULL_UP_NIGHT;  // edge 9
        newPrintDisplay((unsigned char*)"Pull Up Night");
        return;
    }

    /* Handle the current state */
    pid1Running = true;
    pid2Running = false;
}

void passiveState(E_DAY_NIGHT_STATE &dayNightState, E_STATE &state)
{
    printf("Passive\n");

    /* Check if we should pass to the next state */
    if (dayNightState == E_DAY)
    {
        if (internalLight < MIN_DAYLIGHT)
        { 
            state = E_PULL_UP;  // edge 3
            newPrintDisplay((unsigned char*)"Pull Up");
            return;
        }
        else if (internalLight > MAX_DAYLIGHT)
        {
            state = E_PULL_DOWN;    // edge 4
            newPrintDisplay((unsigned char*)"Pull Down");
            return;
        }
    }
    else if (dayNightState == E_NIGHT)
    {
        state = E_PULL_UP_NIGHT;  // edge 10
        newPrintDisplay((unsigned char*)"Pull Up Night");
        return;
    }
    
    /* Handle the current state */
    pid1Running = false;
    pid2Running = false;
}

void pullDownState(E_DAY_NIGHT_STATE &dayNightState, E_STATE &state)
{
    printf("Pull Down\n");

    /* Check if we should pass to the next state */
    if (dayNightState == E_DAY)
    {
        if (internalLight < DAYLIGHT_REFERENCE)
        {
            state = E_PASSIVE;  // edge 5
            newPrintDisplay((unsigned char*)"Passive");
        }
    }
    else if (dayNightState == E_NIGHT)
    {
        state = E_PULL_UP_NIGHT;  // edge 11
        newPrintDisplay((unsigned char*)"Pull Up Night");
    }

    /* Handle the current state */
    pid1Running = false;
    pid2Running = true;
}

void pullUpNightState(E_DAY_NIGHT_STATE &dayNightState, E_STATE &state)
{
    printf("Pull Up Nigth\n");

    /* Check if we should pass to the next state */
    if (dayNightState == E_DAY)
    {
        state = E_START;    // edge 6
        newPrintDisplay((unsigned char*)"Start");
        return;
    }
    
    /* Handle the current state */
    pid1Running = true;
    pid2Running = false;
}

void newPrintDisplay(unsigned char* str)
{
    putCommand(DISPLAY_CLEAR_CMD);      // clear display
	setCursor(0,0);                     // first line, first column, 1st position
	writeString(str, false);            // message
}

void printSensorsSecondLine(int s1, int s2)
{
    //putCommand(DISPLAY_CLEAR_CMD);
    setCursor(1, 0);
    writeString((unsigned char*)"s1:", false);
    writeNumber(s1);
    writeString((unsigned char*)"% s2:", false);
    writeNumber(s2);
    writeString((unsigned char*)"%", false);
}