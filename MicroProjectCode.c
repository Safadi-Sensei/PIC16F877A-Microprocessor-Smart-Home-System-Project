// light and motion control system - immediate response example
#include <xc.h>  // include xc8 compiler header for pic16f877a, we need this for all register definitions

// now we set up the microcontroller configuration bits for our project
#pragma config FOSC = HS        // we chose high-speed external oscillator for precise timing
#pragma config WDTE = OFF       // we decided to disable the watchdog timer for simplicity
#pragma config PWRTE = ON       // we enable power-up timer so system stabilizes on boot
#pragma config BOREN = ON       // we keep brown-out reset on to protect against low voltage issues
#pragma config LVP = OFF        // we turn off low-voltage programming to free up RB3 pin
#pragma config CPD = OFF        // we don’t need data memory code protection for this demo
#pragma config WRT = OFF        // we don’t require writing protection on flash memory
#pragma config CP = OFF         // we disable code protection so we can reprogram easily

#define _XTAL_FREQ 20000000     // define system clock frequency (20MHz) for __delay macros

// lcd pin definitions - we wire our lcd in 4-bit mode to save pins
#define LCD_RS PORTDbits.RD2    // register select pin for lcd commands vs data
#define LCD_EN PORTDbits.RD3    // lcd enable pin, we toggle this to latch data
#define LCD_D4 PORTDbits.RD4    // data pins D4-D7 for 4-bit mode
#define LCD_D5 PORTDbits.RD5    
#define LCD_D6 PORTDbits.RD6    
#define LCD_D7 PORTDbits.RD7    

// sensor and led control pins - we have an LDR and PIR sensor
#define LIGHT_LED PORTBbits.RB4      // led we flash when it's dark
#define MOTION_LED PORTBbits.RB2     // led we flash when motion is detected
#define MOTION_SENSOR PORTBbits.RB0  // input from our motion (PIR) sensor

// function prototypes - here we list everything we’ll write below
void LCD_Init(void);                          // here we are initializing the lcd hardware
void LCD_Cmd(unsigned char cmd);              // here we are sending commands to the lcd
void LCD_Char(unsigned char data);            // here we write a single character to lcd
void LCD_String(const char* str);             // here we write a full string
void LCD_Clear(void);                         // here we clear the lcd screen
void LCD_Goto(unsigned char row, unsigned char col); // here we move cursor to row,col
void ADC_Init(void);                          // here we set up the adc to read sensors
unsigned int ADC_Read(unsigned char channel); // here we read a value from chosen adc channel
void Display_Number(unsigned int num);        // here we convert number to ascii and print
void Clear_Line(unsigned char line);          // here we blank out a full lcd line

// global state variables - we use these to avoid redundant updates
unsigned char motion_detected = 0;     // current motion sensor reading
unsigned char previous_motion = 0;     // last motion state so we only update display when changed
unsigned int previous_light = 0;       // last light percentage measured
unsigned char previous_light_led = 0;  // last state of light-led
unsigned char previous_motion_led = 0; // last state of motion-led

// adc initialization - we do this once in setup
void ADC_Init(void) {
    // here we configure the adc control registers for channel 0
    ADCON1 = 0x8E;  //0b10001110 set voltage reference and digital config only AN0 as A
    ADCON0 = 0x41;  //0b01000001 enable adc & select channel 0 (AN0) & FOSC/8
    TRISA = 0x01;   // make RA0 an input for our photoresistor (ldr)
}

// adc read helper - returns 10-bit value from chosen channel
unsigned int ADC_Read(unsigned char channel) {
    // here we switch adc channel by masking and shifting
    ADCON0 = (ADCON0 & 0xC7) | (channel << 3); ///0b01000001 & 0b11000111
    __delay_ms(2);          // tiny wait for acquisition to complete
    GO_nDONE = 1;           // kick off conversion
    while(GO_nDONE);        // wait for adc to finish
    // now we combine high and low result registers
    return ((ADRESH << 8) + ADRESL);
}

// lcd initialization sequence - do this once after power-up
void LCD_Init(void) {
    // trisd controls direction for lcd pins - we set all to output
    TRISDbits.TRISD2 = 0; TRISDbits.TRISD3 = 0;
    TRISDbits.TRISD4 = 0; TRISDbits.TRISD5 = 0;
    TRISDbits.TRISD6 = 0; TRISDbits.TRISD7 = 0;
    
    // start with all lcd data/control lines low
    LCD_RS = 0; LCD_EN = 0; LCD_D4 = 0;
    LCD_D5 = 0; LCD_D6 = 0; LCD_D7 = 0;
    
    __delay_ms(100);        // wait a bit after power
    LCD_Cmd(0x02);          // set 4-bit interface mode
    LCD_Cmd(0x28);          // use 2 lines and 5x7 font
    LCD_Cmd(0x0C);          // display on, cursor off
    LCD_Cmd(0x06);          // move cursor right after char
    LCD_Cmd(0x01);          // clear display
    __delay_ms(10);         // allow clear to finish
}

// send a command to lcd - low-level helper
void LCD_Cmd(unsigned char cmd) {
    LCD_RS = 0;  // we select command register
    // send upper nibble bits 4-7
    LCD_D4 = (cmd >> 4) & 0x01;// 11110000.0000 0001
    LCD_D5 = (cmd >> 5) & 0x01;
    LCD_D6 = (cmd >> 6) & 0x01;
    LCD_D7 = (cmd >> 7) & 0x01;
    LCD_EN = 1;  __delay_us(10); LCD_EN = 0;  // latch data
    __delay_ms(2);
    // send lower nibble bits 0-3
    LCD_D4 = cmd & 0x01;
    LCD_D5 = (cmd >> 1) & 0x01;
    LCD_D6 = (cmd >> 2) & 0x01;
    LCD_D7 = (cmd >> 3) & 0x01;
    LCD_EN = 1;  __delay_us(10); LCD_EN = 0;
    __delay_ms(10);
}

// write one char to lcd display
void LCD_Char(unsigned char data) {
    LCD_RS = 1;  // we select data register now
    // send upper nibble
    LCD_D4 = (data >> 4) & 0x01;
    LCD_D5 = (data >> 5) & 0x01;
    LCD_D6 = (data >> 6) & 0x01;
    LCD_D7 = (data >> 7) & 0x01;
    LCD_EN = 1;  __delay_us(10); LCD_EN = 0;
    __delay_ms(2);
    // send lower nibble
    LCD_D4 = data & 0x01;
    LCD_D5 = (data >> 1) & 0x01;
    LCD_D6 = (data >> 2) & 0x01;
    LCD_D7 = (data >> 3) & 0x01;
    LCD_EN = 1;  __delay_us(10); LCD_EN = 0;
    __delay_ms(2);
}

// write a whole string to lcd until we hit null terminator
void LCD_String(const char* str) {
    while(*str) {  // loop until end of string
        LCD_Char(*str++);
    }
}

// clear the lcd display - handy when changing pages
void LCD_Clear(void) {
    LCD_Cmd(0x01);    // send clear command
    __delay_ms(10);   // let it finish
}

// move cursor to a particular row and column
void LCD_Goto(unsigned char row, unsigned char col) {
    unsigned char address;
    if(row == 1) {
        address = 0x80 + (col - 1);  // first row base address -- 1000 xxxx 
    } else {
        address = 0xC0 + (col - 1);  //second row base address -- 1100 xxxx 
    }
    LCD_Cmd(address);
}

// clear entire line by overwriting with spaces
void Clear_Line(unsigned char line) {
    LCD_Goto(line, 1);
    LCD_String("                "); // 16 blanks
}

// convert integer to ascii and print on lcd
void Display_Number(unsigned int num) {
    unsigned char buffer[6];
    unsigned char i = 0;
    
    if(num == 0) {  // handle the zero case nicely
        LCD_Char('0');
        return;
    }
    // pack digits in reverse
    while(num > 0) {
        buffer[i++] = (num % 10) + '0';
        num /= 10;
    }
    // now print in correct order
    while(i > 0) {
        LCD_Char(buffer[--i]);
    }
}

// our main entry point - we set things up then loop forever
void main(void) {
    unsigned int adc_value;
    unsigned int light_percentage;

    // configure ports for lcd and sensors
    TRISD = 0x00;           // portd outputs for lcd
    PORTD = 0x00;           // clear portd
    TRISBbits.TRISB0 = 1;   // rb0 input from motion sensor
    TRISBbits.TRISB2 = 0;   // rb2 output to motion led
    TRISBbits.TRISB4 = 0;   // rb4 output to light led
    PORTB = 0x00;           // clear portb

    // init peripherals
    LCD_Init();             // here we initialize lcd
    ADC_Init();             // here we initialize adc
    __delay_ms(200);        // wait for stable readings

    // show startup screen
    LCD_Goto(1, 1);
    LCD_String("SMART LIGHTING");
    LCD_Goto(2, 1);
    LCD_String("& MOTION SYS");
    __delay_ms(3000);

    // calibration screen
    LCD_Clear();
    LCD_Goto(1, 1);
    LCD_String("CALIBRATING...");
    LCD_Goto(2, 1);
    LCD_String("PLEASE WAIT");
    __delay_ms(3000);

    // initialize leds off
    LIGHT_LED = 0;
    MOTION_LED = 0;
    // reset previous states
    previous_motion = 0;
    previous_light = 0;
    previous_light_led = 0;
    previous_motion_led = 0;

    LCD_Clear();  // clear display for main loop

    while(1) {
        // read light sensor and convert to percentage
        adc_value = ADC_Read(0);
        light_percentage = 100 - (unsigned int)((unsigned long)adc_value * 100 / 1023);
        // decide if we need the light led on
        if(light_percentage < 50) {
            LIGHT_LED = 1;
        } else {
            LIGHT_LED = 0;
        }
        // read motion sensor state
        motion_detected = MOTION_SENSOR;
        // decide if we need motion led on
        if(motion_detected) {
            MOTION_LED = 1;
        } else {
            MOTION_LED = 0;
        }
        // update lcd when something changed on line 1
        if((light_percentage != previous_light) || (motion_detected != previous_motion)) {
            Clear_Line(1);        // blank line 1
            LCD_Goto(1, 1);
            LCD_String("L:");
            Display_Number(light_percentage);
            LCD_String("% M:");
            if(motion_detected) LCD_String("YES"); else LCD_String("NO ");
            previous_light = light_percentage;
            previous_motion = motion_detected;
        }
        // update lcd when leds changed on line 2
        if((LIGHT_LED != previous_light_led) || (MOTION_LED != previous_motion_led)) {
            Clear_Line(2);
            LCD_Goto(2, 1);
            LCD_String("LT:");
            if(LIGHT_LED) LCD_String("ON "); else LCD_String("OFF");
            LCD_String(" MT:");
            if(MOTION_LED) LCD_String("ON"); else LCD_String("OFF");
            previous_light_led = LIGHT_LED;
            previous_motion_led = MOTION_LED;
        }
        __delay_ms(200);  // small delay to limit update rate
    }
}
