/* 
 * File:   LCD_Interface.h
 * Author: Travis McCormick
 *
 * Created on April 16, 2019, 3:55 PM
 * 
 * Description: This is a class used for interfacing Hitachi HD44780 chipset 2x16 LCDs
 *              with the PIC32 microcontroller. This library is intended to interface 
 *              with the LCD in 4-bit mode with Port B on the microcontroller.
 */

#ifndef LCD_INTERFACE_H
#define	LCD_INTERFACE_H

//Use lower 6 bits of Port B to control LCD
#define PORT_MASK 0xFF80
#define LCD_PORT TRISB
#define LCD_PINS ANSELB
#define LCD_DATA LATB
#define LCD_ENABLE LATBbits.LATB6
#define LCD_WRITE_RS LATBbits.LATB5
#define LCD_READ_RS LATBbits.LATB5
#define TIMER T1CONbits
#define COUNT TMR1
#define PER PR1
#define ROWS 2
#define COLS 16

class LCD_Interface {
public:
    LCD_Interface();
    void print(const char str[]);
    void print(int num);
    void moveCursor(int row, int col);
    void cursorOff();
    void cursorOn();
    void clear();
    
private:
    void sendData();
    void delay(int ms);
    void write(int data);

};

#endif

