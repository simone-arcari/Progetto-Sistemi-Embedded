/*
 * HD44780.c
 *
 *  Created on: 20 march 2021
 *      Author: Simone Arcari
 */

#include "HD44780.h"

delay_t delayMs = nullptr;

set_pin_t registerSelectWrite = nullptr;
set_pin_t readWriteWrite = nullptr;
set_pin_t enableWrite = nullptr;
set_pin_t dataLine4Write = nullptr;
set_pin_t dataLine5Write = nullptr;
set_pin_t dataLine6Write = nullptr;
set_pin_t dataLine7Write = nullptr;

int registeredCallbacks = 0;

const int LOW = 0;
const int HIGH = 1;

void init_LCD(void) {
/*****************************************************************************
 *
 * Description:
 *    Initializes the LCD display with basic settings (4bit mode)
 *
 ****************************************************************************/
	putCommand_hf(0x30);			// sequence for initialization
	putCommand_hf(0x30);    		// ----
	putCommand_hf(0x20);    		// ----

	putCommand(FOUR_BIT_TWO_LINE_5x8_CMD);	    // selecting 16 x 2 LCD in 4bit mode with 5x7 pixels
    putCommand(DISP_ON_CUR_OFF_BLINK_OFF_CMD);	// display ON cursor OFF blinking OFF
    putCommand(DISPLAY_CLEAR_CMD);          	// clear display
    putCommand(ENTRY_MODE_INC_NO_SHIFT_CMD);    // cursor auto increment
    putCommand(0x80);          			        // 1st line 1st location of LCD 0 offset 80h
}

void clear_line(void) {
/*****************************************************************************
 *
 * Description:
 *    Sets all data lines to a low logic level
 *
 ****************************************************************************/
	if (registeredCallbacks != E_CALLBACK_NUMBER) {
        return;
    }

    dataLine4Write(LOW);
    dataLine5Write(LOW);
    dataLine6Write(LOW);
    dataLine7Write(LOW);
}

void toggle(void) {
/*****************************************************************************
 *
 * Description:
 *     Latching data in to LCD data register using high to Low signal
 *
 ****************************************************************************/
    if (registeredCallbacks != E_CALLBACK_NUMBER) {
        return;
    }

    enableWrite(HIGH);
	delayMs(2);
    enableWrite(LOW);
}

void sendUpperByte(char data_to_LCD) {
/*****************************************************************************
 *
 * Description:
 *     Sends only the upper part of a byte
 *
 * Parameters:
 *    [in] data_to_LCD - byte to be sent to the LCD
 *
 ****************************************************************************/
    if (registeredCallbacks != E_CALLBACK_NUMBER) {
        return;
    }

    if(data_to_LCD & 0x10)
    {
        dataLine4Write(HIGH);
    } else {
    	dataLine4Write(LOW);
    }

    if(data_to_LCD & 0x20)
    {
    	dataLine5Write(HIGH);
    } else {
    	dataLine5Write(LOW);
    }

    if(data_to_LCD & 0x40)
    {
    	dataLine6Write(HIGH);
    } else {
    	dataLine6Write(LOW);
    }

    if(data_to_LCD & 0x80)
    {
    	dataLine7Write(HIGH);
    } else {
    	dataLine7Write(LOW);
    }
}

void sendLowerByte(char data_to_LCD) {
/*****************************************************************************
 *
 * Description:
 *     Sends only the lower part of a byte
 *
 * Parameters:
 *    [in] data_to_LCD - byte to be sent to the LCD
 *
 ****************************************************************************/
    if(data_to_LCD & 0x01)
    {
        dataLine4Write(HIGH);
    } else {
    	dataLine4Write(LOW);
    }

    if(data_to_LCD & 0x02)
    {
    	dataLine5Write(HIGH);
    } else {
    	dataLine5Write(LOW);
    }

    if(data_to_LCD & 0x04)
    {
    	dataLine6Write(HIGH);
    } else {
    	dataLine6Write(LOW);
    }

    if(data_to_LCD & 0x08)
    {
    	dataLine7Write(HIGH);
    } else {
    	dataLine7Write(LOW);
    }
}

void putCommand_hf(char data_to_LCD) {
/*****************************************************************************
 *
 * Description:
 *    Sends only the upper part of its command
 *
 * Parameters:
 *    [in] data_to_LCD - byte to be sent to the LCD
 *
 ****************************************************************************/
    if (registeredCallbacks != E_CALLBACK_NUMBER) {
        return;
    }

    registerSelectWrite(LOW);	    // selecting register as command register
    readWriteWrite(LOW);	        // selecting write mode
    clear_line();              	    // clearing the 4 bits data line
    sendUpperByte(data_to_LCD);

    toggle();
}

void putCommand(char data_to_LCD) {
/*****************************************************************************
 *
 * Description:
 *    Sends the whole command
 *
 * Parameters:
 *    [in] data_to_LCD - byte to be sent to the LCD
 *
 ****************************************************************************/
    if (registeredCallbacks != E_CALLBACK_NUMBER) {
        return;
    }

    registerSelectWrite(LOW);	    // selecting register as command register
    readWriteWrite(LOW);	    // selecting write mode
    clear_line();                   // clearing the 4 bits data line
    sendUpperByte(data_to_LCD);

    toggle();

    clear_line();                   // clearing the 4 bits data line
    sendLowerByte(data_to_LCD);

    toggle();
}

void writeByte(char data_to_LCD) {
/*****************************************************************************
 *
 * Description:
 *    Sends the whole data to LCD (one byte)
 *
 * Parameters:
 *    [in] data_to_LCD - byte to be sent to the LCD
 *
 ****************************************************************************/
    if (registeredCallbacks != E_CALLBACK_NUMBER) {
        return;
    }

    registerSelectWrite(HIGH);      // selecting register as data register
    readWriteWrite(LOW);	        // selecting write mode    
    clear_line();                   // clearing the 4 bits data line
    sendUpperByte(data_to_LCD);

    toggle();

    clear_line();                   // clearing the 4 bits data line
    sendLowerByte(data_to_LCD);

    toggle();
}

void writeString(unsigned char LineOfCharacters[TOTAL_CHARACTERS_OF_LCD], char OverLenghtCharacters) {
/*****************************************************************************
 *
 * Description:
 *    Prints the desired string on the LCD display (multiple byte)
 *
 * Parameters:
 *    [in] LineOfCharacters[] - array containing the string to be printed
 *    [in] OverLenghtCharacters - flag to enable wrapping (true = wrapping ON) (false = wrapping OFF)
 *
 ****************************************************************************/
	unsigned char i=0, line=-1;

	while(LineOfCharacters[i] && i<TOTAL_CHARACTERS_OF_LCD) {
		if(OverLenghtCharacters) {
			if((i%LCD_LINE_LENGHT)==0)
				setCursor(++line, 0);
		}

		writeByte(LineOfCharacters[i]);
		i++;
	}
}

void writeNumber(int number) {
/*****************************************************************************
 *
 * Description:
 *    Prints the desired number (decimal integer) on the LCD display (multiple byte)
 *
 * Parameters:
 *    [in] number - number to be printed
 *
 ****************************************************************************/
    int index = 0, MAX_NUM_LEN = 10, temp;
    int digit[MAX_NUM_LEN];

    while(number != 0) {
        digit[index] = number%10;
        number/=10;
        index++;
    }

    if(index == 0) {
        writeByte(NUM_TO_CODE(0));  // print number zero
    }else {
        /* array inversion algorithm */
        int cycle;
        int comodo = index-1;
        for(cycle = 0; cycle < index-index/2; cycle++) {
            temp = digit[comodo];
            digit[comodo] = digit[cycle];
            digit[cycle] = temp;
            comodo--;
        }

        for(cycle = 0; cycle < index; cycle++) {
            writeByte(NUM_TO_CODE(digit[cycle]));
        }
    }
}

void lcd_rig_sh(void) {
/*****************************************************************************
 *
 * Description:
 *    At each function call the entire content printed on the LCD is shifted
 *    to the right of a box
 *
 ****************************************************************************/
    if (registeredCallbacks != E_CALLBACK_NUMBER) {
        return;
    }

	putCommand(DISPLAY_MOVE_SHIFT_RIGHT_CMD);  // command for right shift
	delayMs(100);
}

void lcd_lef_sh(void) {
/*****************************************************************************
 *
 * Description:
 *    At each function call the entire content printed on the LCD is shifted
 *    to the left of a box
 *
 ****************************************************************************/
    if (registeredCallbacks != E_CALLBACK_NUMBER) {
        return;
    }
    
	putCommand(DISPLAY_MOVE_SHIFT_LEFT_CMD);  // command for left shift
    delayMs(100);
}

void setCursor(unsigned char line, unsigned char col) {
/*****************************************************************************
 *
 * Description:
 *    Place the cursor in the desired position
 *
 * Parameters:
 *    [in] line - number of the line in which to place the cursor
 *    [in] col - number of the column in which to place the cursor
 *
 ****************************************************************************/
	unsigned char address;

	switch(line){
		case 0:
			address = 0x00;
			break;

		case 1:
			address = 0x40;
			break;

		case 2:
			address = 0x10;
			break;

		case 3:
			address = 0x50;
			break;

		default:
			address = 0x00;
			break;
	}

	if(col >= 0 && col <= LCD_LINE_LENGHT)
		address += col;

	putCommand(DDRAM_ADDRESS(address));
}

bool register_callback(void (*callback)(int), E_CALLBACK_TYPE type) {
    bool res = false;

    if (callback == nullptr) {
        return false;
    }

    if (type < 0 || type >= E_CALLBACK_NUMBER) {
        return false;
    }
    
    switch (type)
    {
        case E_CALLBACK_RS:
            if (registerSelectWrite == nullptr)
            {
                registerSelectWrite = callback;
                registeredCallbacks++;
                res = true;
            }
            break;

        case E_CALLBACK_RW:
            if (readWriteWrite == nullptr)
            {
                readWriteWrite = callback;
                registeredCallbacks++;
                res = true;
            }
            break;

        case E_CALLBACK_EN:
            if (enableWrite == nullptr)
            {
                enableWrite = callback;
                registeredCallbacks++;
                res = true;
            }
            break;

        case E_CALLBACK_DATA4:
            if (dataLine4Write == nullptr)
            {
                dataLine4Write = callback;
                registeredCallbacks++;
                res = true;
            }
            break;

        case E_CALLBACK_DATA5:
            if (dataLine5Write == nullptr)
            {
                dataLine5Write = callback;
                registeredCallbacks++;
                res = true;
            }
            break;

        case E_CALLBACK_DATA6:
            if (dataLine6Write == nullptr)
            {
                dataLine6Write = callback;
                registeredCallbacks++;
                res = true;
            }
            break;

        case E_CALLBACK_DATA7:
            if (dataLine7Write == nullptr)
            {
                dataLine7Write = callback;
                registeredCallbacks++;
                res = true;
            }
            break;

        case E_DELAY:
            if (delayMs == nullptr)
            {
                delayMs = callback;
                registeredCallbacks++;
                res = true;
            }
            break;
    }

    return res;
}