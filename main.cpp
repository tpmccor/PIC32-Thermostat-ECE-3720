
#include <plib.h>
#include "LCD_Interface.h"

#define TEMP 0
#define FAN 1
#define SET 2
#define FAN_SPEED OC3RS

int mode = TEMP;
int temp = 70;
int setTemp = 70;
int fanSpeed = 50;
LCD_Interface LCD;

void init();
void initLCD();
void sampleTemp();
void updateLCD(int val, int type);
void fan();
    
extern "C"{ //C++ apparently doesn't like interrupts
    void __ISR(3,ipl1) encoderA(){
        if(INTCONbits.INT0EP == !PORTCbits.RC8){ //Encoder clockwise turn
            if(mode == TEMP){
                setTemp++;
                if(setTemp > 999){
                    setTemp = 999;
                }
            }
            else if(mode == FAN){
                fanSpeed++;
                if(fanSpeed > 100){
                    fanSpeed = 100;
                }
            }
        }
        else{ //Encoder counterclockwise turn
            if(mode == TEMP){
                setTemp--;
                if(setTemp < 0){
                    setTemp = 0;
                }
            }
            else if(mode == FAN){
                fanSpeed--;
                if(fanSpeed < 0){
                    fanSpeed = 0;
                }
            }
        }
        if(INTCONbits.INT0EP){
            INTCONbits.INT0EP = 0;
        }
        else{
            INTCONbits.INT0EP = 1;
        }
        
        if(mode == TEMP){
            updateLCD(setTemp, SET);
        }
        else if(mode == FAN){
            updateLCD(fanSpeed, FAN);
            FAN_SPEED = fanSpeed;
        }
        IFS0bits.INT0IF = 0;
    }

    void __ISR(15,ipl2) encoderB(){
        if(INTCONbits.INT3EP == PORTBbits.RB7){ //Encoder clockwise turn
            if(mode == TEMP){
                setTemp++;
                if(setTemp > 999){
                    setTemp = 999;
                }
            }
            else if(mode == FAN){
                fanSpeed++;
                if(fanSpeed > 100){
                    fanSpeed = 100;
                }
            }
        }
        else{ //Encoder counterclockwise turn
            if(mode == TEMP){
                setTemp--;
                if(setTemp < 0){
                    setTemp = 0;
                }
            }
            else if(mode == FAN){
                fanSpeed--;
                if(fanSpeed < 0){
                    fanSpeed = 0;
                }
            }
        }
        if(INTCONbits.INT3EP){
            INTCONbits.INT3EP = 0;
        }
        else{
            INTCONbits.INT3EP = 1;
        }
        if(mode == TEMP){
            updateLCD(setTemp, SET);
        }
        else if(mode == FAN){
            updateLCD(fanSpeed, FAN);
            FAN_SPEED = fanSpeed;
        }
        IFS0bits.INT3IF = 0;
    }
    
    void __ISR(7, ipl3) switchMode(){
        if(mode == TEMP){
            mode = FAN;
        }
        else if(mode == FAN){
            mode = TEMP;
        }
        initLCD();
        IFS0bits.INT1IF = 0;
    }
}

int main(){
    init();
    initLCD();
    
    while(1){
        if(mode == TEMP){
            sampleTemp();
            fan();
        }
    }
    return 0;
}

void initLCD(){
    //Sets initial display text. ß is ASCII equivalent for degrees symbol on LCD
    IEC0bits.INT0IE = 0;
    IEC0bits.INT1IE = 0;
    IEC0bits.INT3IE = 0;
    LCD.clear();
    if(mode == TEMP){
        LCD.moveCursor(0,0);
        LCD.print("Temp: ");
        updateLCD(temp, TEMP);
        LCD.print("ßF");
        LCD.moveCursor(1,0);
        LCD.print("Set Temp: ");
        updateLCD(setTemp, SET);
        LCD.print("ßF");
    }
    if(mode == FAN){
        LCD.moveCursor(0,0);
        LCD.print("Fan Speed:");
        updateLCD(fanSpeed, FAN);
        LCD.print("%");
    }
    IEC0bits.INT0IE = 1;
    IEC0bits.INT1IE = 1;
    IEC0bits.INT3IE = 1;
}

void init(){
    INTEnableSystemMultiVectoredInt();    
     
    //Configure optical encoder and interrupts
    TRISCbits.TRISC8 = 1;
    TRISBbits.TRISB7 = 1;
    PPSInput(2, INT3, RPC8);
    INTCONbits.INT0EP = 0;
    IEC0bits.INT0IE = 1;
    IFS0bits.INT0IF = 0;
    IPC0bits.INT0IP = 1;
    INTCONbits.INT3EP = 1;
    IPC3bits.INT3IP = 2;
    IFS0bits.INT3IF = 0;
    IEC0bits.INT3IE = 1;
    
    //Configure ADC
    TRISAbits.TRISA1 = 1;
    ANSELAbits.ANSA1 = 1;
    AD1CHSbits.CH0SA = 1;
    AD1CON1bits.FORM = 0;
    AD1CON1bits.SSRC = 7;
    AD1CON1bits.ASAM = 0;
    AD1CON2bits.VCFG = 0;
    AD1CON2bits.CSCNA = 0;
    AD1CON2bits.SMPI = 0;
    AD1CON2bits.BUFM = 0;
    AD1CON2bits.ALTS = 0;
    AD1CON3bits.ADRC = 0;
    AD1CON3bits.SAMC = 16;
    AD1CON3bits.ADCS = 1;
    AD1CON1bits.ON = 1;
    
    //Configure PWM for fan
    T2CONbits.ON = 0;
    T2CONbits.T32 = 0;
    TMR2 = 0;
    T2CONbits.TCKPS = 0;
    T2CONbits.TCS = 0;
    PR2 = 100;
    T2CONbits.ON = 1;    
    TRISBbits.TRISB9 = 0;
    LATBbits.LATB9 = 0;
    PPSOutput(4, RPB9, OC3);
    OC3CONbits.ON = 0;
    OC3CONbits.OC32 = 0;
    OC3CONbits.OCTSEL = 0;
    OC3CONbits.OCM = 0b110;
    FAN_SPEED = fanSpeed;
    OC3CONbits.ON = 1;
    
    //Switch modes push button interrupt
    TRISCbits.TRISC4 = 1;
    PPSInput(4 ,INT1, RPC4);
    INTCONbits.INT1EP = 1;
    IPC1bits.INT1IP = 3;
    IFS0bits.INT1IF = 0;
    IEC0bits.INT1IE = 1;
}

void updateLCD(int val, int type){
    /*
        Turning the encoder fast will result in an interrupt while the LCD is printing and the interrupt will
            trigger another print before the first can finish. This will send corrupt data to the LCD.
     */
    IEC0bits.INT0IE = 0;
    IEC0bits.INT1IE = 0;
    IEC0bits.INT3IE = 0;
    if(type == TEMP){
        LCD.moveCursor(0,6);
    }
    else if(type == SET){
        LCD.moveCursor(1,10);
    }
    else if(type == FAN){
        LCD.moveCursor(1,4);
    }

    LCD.print(val);
    //Override previous digits on LCD only when number of digits change
    if(val == 99 || val == 9 || val == 100 || val == 10){
        if(type == TEMP || type == SET){
            LCD.print("ßF ");  
        } 
        else if(type == FAN){
            LCD.print("%  ");
        }
    }
    IEC0bits.INT0IE = 1;
    IEC0bits.INT1IE = 1;
    IEC0bits.INT3IE = 1;
}

void sampleTemp(){
    /*
        1 Poll for ADC conversion
        2 Convert sample to voltage, then to degrees C, then to degrees F
     */
    AD1CON1bits.SAMP = 1;
    while(!AD1CON1bits.DONE); //1
    temp = ADC1BUF0;
    temp = (((temp  * 3.3 * 100) / 1024) - 50) * 1.8 + 32; //2
    if(temp < 0){
        temp = 0;   //Just avoiding negative temperatures, too much work and likely not going to happen
    }else if(temp > 999){
        temp = 999; //Not going to happen either
    }
    updateLCD(temp, TEMP);    
    AD1CON1bits.DONE = 0;
}

void fan(){
    //Turn fan on when temperature is greater than set temp + noise error threshold
    if(temp >= setTemp + 2){
        FAN_SPEED = fanSpeed;
    }
    else{
        FAN_SPEED = 0;
    }
}
