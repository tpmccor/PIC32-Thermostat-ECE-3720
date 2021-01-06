/* 
 * File:   LCD_Interface.cpp
 * Author: Travis
 * 
 * Created on April 16, 2019, 3:55 PM
 */

#include "LCD_Interface.h"
#include <plib.h>

LCD_Interface::LCD_Interface() {
    //Initialize delay timer
    TIMER.ON = 0;
    TIMER.TCKPS = 1;
    TIMER.TCS = 0;
    COUNT = 0;
    PER = 60;
    
    //Configure lower 7 bits of port
    LCD_PORT &= PORT_MASK;
    LCD_PINS &= PORT_MASK;
    LCD_DATA &= PORT_MASK;
    LCD_WRITE_RS = 0;
    
    //Requires wait at least 15 ms after Vcc gets to 4.5V
    delay(20);
    //Data sequence to set to LCD to 4-bit mode
    LCD_DATA = (LCD_DATA & PORT_MASK) | 0x03;
    sendData();
    delay(5);
    sendData();
    delay(5);
    sendData();
    delay(1);
    LCD_DATA = (LCD_DATA & PORT_MASK) | 0x02;
    sendData();
    delay(1);
    write(0x02); 
    
    //Set LCD settings: Font size, number of lines, cursor, etc.  
    write(0x08); //2 lines and 5x8 character size
    write(0x08); //Turn display off
    write(0x01); //Clear display (takes more time)
    delay(10);
    write(0x06); //Allow cursor to move with prints
    write(0x0C); //Turn display on
}

void LCD_Interface::delay(int ms){
    //Time delay in milliseconds
    for(int i = 0; i < ms; i++){
        COUNT = 0;
        TIMER.ON = 1;
        while(COUNT < PER);
        TIMER.ON = 0;
    }
}

void LCD_Interface::sendData(){
    //Create pulse for LCD driver's enable bit
    LCD_ENABLE = 1;
    delay(1);
    LCD_ENABLE = 0;
    delay(1);
}

void LCD_Interface::write(int data){
    int _rs = LCD_READ_RS;
    
    /*
        Send upper 4 bits, then send lower 4 bits
     
        1 Zero out 7 bits for LCD
        2 Copy 4 bits of data to lower 4 bits of port
        3 Write RS bit before sending data
     */
    LCD_DATA = (LCD_DATA & PORT_MASK); //1
    LCD_DATA = (LCD_DATA & PORT_MASK) | ((data >> 4) & 0x0F); //2
    LCD_WRITE_RS = _rs; //3                 
    sendData();
    LCD_DATA = (LCD_DATA & PORT_MASK); //1
    LCD_DATA = (LCD_DATA & PORT_MASK) | (data & 0x0F); //2
    LCD_WRITE_RS = _rs; //3
    sendData();
}

void LCD_Interface::print(const char str[]){
    /*
        RS = 1 means data is sent
        RS = 0 means LCD commands are sent
     */
    LCD_WRITE_RS = 1; 
    while(*str != '\0'){
        write(*str);
        str++;
    }
    LCD_WRITE_RS = 0; 
}

void LCD_Interface::print(int num){
    LCD_WRITE_RS = 1;
    /*
        1 Count how many digits are in the number
        2 Print each individual digit
     */
    //1
    int digits = 0;
    int temp = num;
    while(temp > 0){
        digits++;
        temp /= 10;
    }
    //2
    while(digits > 1){
        int mult = 1;
        for(int i = 1; i < digits; i++){
            mult *= 10;
        }
        write((num / (mult)) + '0');
        num %= (mult);
        digits--;
    }
    write(num + '0');
    LCD_WRITE_RS = 0;
}

void LCD_Interface::clear(){
    //Clear display
    write(0x01);
    delay(10);
}

void LCD_Interface::cursorOn(){
    //Turns on display with blinking cursor
    write(0x0F);
}

void LCD_Interface::cursorOff(){
    //Turns off blinking cursor
    write(0x0C);
}

void LCD_Interface::moveCursor(int row, int col){
    //Makes sure cursor coordinate is within boundaries
    if(row >= ROWS){
        row = ROWS - 1;
    }
    if(col >= COLS){
        col = COLS - 1;
    }
    if(row < 0){
        row = 0;
    }
    if(col < 0){
        col = 0;
    }
    
    /*  
        First row = 0x00 DD RAM offset
        Second row = 0x40 DD RAM offset
       
        Set row offset bit, then set column offset
        Upper bit tells it to write DD RAM address (cursor position);
     */
    int pos = (row << 6) | col; 
    write(0x80 | pos);          
    delay(10);
}