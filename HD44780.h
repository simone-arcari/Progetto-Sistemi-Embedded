/*
 * HD44780.h
 *
 *  Created on: 20 march 2021
 *      Author: Simone Arcari
 */

#ifndef __HD44780_H
#define __HD44780_H

#define TOTAL_CHARACTERS_OF_LCD 32
#define LCD_LINE_LENGHT 16

#define IR 0
#define DR 1

// Display commands
#define DISPLAY_CLEAR_CMD               0b00000001  //  Clears display and set DDRAM address 0
#define RETURN_HOME_CMD                 0b00000010  //  DDRAM address 0, display in original position

#define ENTRY_MODE_DEC_NO_SHIFT_CMD     0b00000100  //  Entry mode decrement, no display shift
#define ENTRY_MODE_DEC_SHIFT_CMD        0b00000101  //  Entry mode decrement, display shift
#define ENTRY_MODE_INC_NO_SHIFT_CMD     0b00000110  //  Entry mode increment, no display shift
#define ENTRY_MODE_INC_SHIFT_CMD        0b00000111  //  Entry mode increment, display shift

#define DISP_OFF_CUR_OFF_BLINK_OFF_CMD  0b00001000  //  Display off, cursor off, blinking off
#define DISP_OFF_CUR_OFF_BLINK_ON_CMD   0b00001001  //  Display off, cursor off, blinking on
#define DISP_OFF_CUR_ON_BLINK_OFF_CMD   0b00001010  //  Display off, cursor on, blinking off
#define DISP_OFF_CUR_ON_BLINK_ON_CMD    0b00001011  //  Display off, cursor on, blinking on
#define DISP_ON_CUR_OFF_BLINK_OFF_CMD   0b00001100  //  Display on, cursor off, blinking off
#define DISP_ON_CUR_OFF_BLINK_ON_CMD    0b00001101  //  Display on, cursor off, blinking on
#define DISP_ON_CUR_ON_BLINK_OFF_CMD    0b00001110  //  Display on, cursor on, blinking off
#define DISP_ON_CUR_ON_BLINK_ON_CMD     0b00001111  //  Display on, cursor on, blinking on

#define CURSOR_MOVE_SHIFT_LEFT_CMD      0b00010000  //  Cursor move, shift to the left
#define CURSOR_MOVE_SHIFT_RIGHT_CMD     0b00011100  //  Cursor move, shift to the right
#define DISPLAY_MOVE_SHIFT_LEFT_CMD     0b00011000  //  Display move, shift to the left
#define DISPLAY_MOVE_SHIFT_RIGHT_CMD    0b00011100  //  Display move, shift to the right

#define FOUR_BIT_ONE_LINE_5x8_CMD       0b00100000  //  4 bit, 1 line, 5x8 font size
#define FOUR_BIT_ONE_LINE_5x10_CMD      0b00100100  //  4 bit, 1 line, 5x10 font size
#define FOUR_BIT_TWO_LINE_5x8_CMD       0b00101000  //  4 bit, 2 line, 5x8 font size
#define FOUR_BIT_TWO_LINE_5x10_CMD      0b00101100  //  4 bit, 2 line, 5x10 font size
#define EIGHT_BIT_ONE_LINE_5x8_CMD      0b00110000  //  8 bit, 1 line, 5x8 font size
#define EIGHT_BIT_ONE_LINE_5x10_CMD     0b00110100  //  8 bit, 1 line, 5x10 font size
#define EIGHT_BIT_TWO_LINE_5x8_CMD      0b00111000  //  8 bit, 2 line, 5x8 font size
#define EIGHT_BIT_TWO_LINE_5x10_CMD     0b00111100  //  8 bit, 2 line, 5x10 font size

#define CGRAM_ADDRESS(addr) (addr|=0x40)            //  CGRAM addressing
#define DDRAM_ADDRESS(addr) (addr|=0x80)            //  DDRAM addressing

#define NUM_TO_CODE(num) (num+0x30)                 //  0-9 ROM codes
#define CODE_TO_NUM(code) (code-0x30)               //  0-9 ROM num

typedef enum
{
    E_CALLBACK_RS = 0,
    E_CALLBACK_RW,
    E_CALLBACK_EN,
    E_CALLBACK_DATA4,
    E_CALLBACK_DATA5,
    E_CALLBACK_DATA6,
    E_CALLBACK_DATA7,
    E_DELAY,
    E_CALLBACK_NUMBER   // total callback number
} E_CALLBACK_TYPE;

typedef void (*set_pin_t)(int);
typedef void (*delay_t)(int);


/* NORMAL DISPLAY IN 4BIT MODE */
//void lcd_pin(int,int,int,int,int,int,int,pin_port,pin_port,pin_port,pin_port,pin_port,pin_port,pin_port);   // LCD pin declaration function
void init_LCD(void);        					                                                            // LCD initializing function
void clear_line(void);                                                                                      // function to clear data lines
void toggle(void);                                                                                          // latching function of LCD
void sendUpperByte(char);                                                                                   // masking higher 4 bits and sending to LCD
void sendLowerByte(char);                                                                                   // masking lower 4 bits and sending to LCD
void putCommand_hf(char);                                                                                   // function to send half byte command to LCD
void putCommand(char);                                                                                      // function to send Command to LCD
void writeByte(char);                                                                                       // function to send data to LCD
void writeString(unsigned char LineOfCharacters[TOTAL_CHARACTERS_OF_LCD],char);	                            // function to send string to LCD
void writeNumber(int);							                                                            // function to send number to LCD
void lcd_lef_sh(void); 							                                                            // left shifting function
void lcd_rig_sh(void);  						                                                            // right shifting function
void setCursor(unsigned char,unsigned char);
bool register_callback(void (*callback)(int), E_CALLBACK_TYPE type);	                                                            // function to select the position of cursor


#endif /* __HD44780_H*/