
#include "mbed.h"
#include <map>
#include "C12832.h"

/*
Execute in ISR's apart from initial set up
2 Int input calculator with + - * / ^ operands 
Blinking under cursors and led color iindicates states.
fire button cycles modifier left to right. 
incr/drcr values using up/down buttons.

Program Implementation:
ticker triggers ISR "loop" Updates cursor blink
ticker triggers ISR "loop" Updates Calculations & display
timer used for debouncing pins

Trig ISR:
Cursor cords right shifted with wrap around
LED & State incremented.
*/

#define RED    255, 0, 0
#define GREEN  0, 255, 0
#define BLUE   0, 0, 255
#define YELLOW 255, 255, 0
#define CYAN   0, 255, 255
#define MAGENTA 255, 0, 255
#define WHITE  255, 255, 255
#define BLACK  0, 0, 0

typedef enum FSM{
    INIT,
    NUM1,
    OPERAND,
    NUM2,
} FSM;

C12832 lcd(D11, D13, D12, D7, D10); // LCD Instance

class RGB{
    //@Breif PWM Based RGB Led Driver
    public:
        void setWhite(void){        
            r.write(0.0f);
            g.write(0.0f);
            b.write(0.0f);
        }
        void setOff(void){
            r.write(1.0f);
            g.write(1.0f);
            b.write(1.0f);
        }
        void set(uint8_t red, uint8_t green, uint8_t blue){
            r.write((255-red)/255.0f);
            g.write((255-green)/255.0f);
            b.write((255-blue)/255.0f);
        }

    RGB(PinName red, PinName green, PinName blue, float pwm_period = 0.001f) : r(red), g(green), b(blue) {
        
        r.period(pwm_period);
        g.period(pwm_period);
        b.period(pwm_period);
        r.write(1.0f);
        g.write(1.0f);
        b.write(1.0f);
        }// Constructor

    private:
        PwmOut r, g, b;
};


class Cursor{
    //@Breif: Status indicator 
    private:
        int coords[4];
        bool toggle;

    public:
        void erase(){
            lcd.line(coords[0], coords[1], coords[2], coords[3], 0);
        }    

        void blink(){
            toggle = !toggle;
            lcd.line(coords[0], coords[1], coords[2], coords[3], toggle);
        }
        void shift(){
            erase(); // clear old line
            coords[0] += 15;
            coords[2] += 15;
        }
        void cursorReset(){
            erase(); // clear old line
            coords[0] = 0;
            coords[2] = 10; 
        }
        inline int getCursorX(){return coords[0];}
        inline int getCursorY(){return coords[1] - 10;}

    Cursor(){
        coords[0] = 0;  //x1
        coords[1] = 16; //y1
        coords[2] = 10; //x2
        coords[3] = 16; //y2
        toggle = false;
        } // Constructor
};

class Num{
    //@Breif: Interger 0-9
    private:
        int min, max;
    public:
        int val;
        void incr(void){
            if(val < max){
                val++;
            }else{
                val = min; // wrap around
            }
        }
        void decr(void){
            if(val > min){
                val--;
            }else{
                val = max; // wrap around;
            }
        }
        inline char getC(){return '0' + val;}

    Num(){
        val = 1;
        min = 0;
        max = 9;
    }

};

class Operator{
    //@Breif: Mathematical Operators
    private:
        char types[5];
        char *op;
    public:
        void incr(void){
            if(op == types + sizeof(types) - 1){
                op = &types[0]; // first opperato in array 
            }else{
                op++; // next operator in array
            }
        }
        void decr(void){
            if(op == &types[0]){
                op = &types[4]; // Last opperator in array 
            }else{
                op--; // prev operator in array
            }
        }
        inline char getC(void){return *op;}

        void run(Num n1, Num n2, float & result){
            switch (*op){
                case '+':
                    result = n1.val + n2.val;
                    break;
                case '-':
                    result = n1.val - n2.val;
                    break;
                case '*':
                    result = n1.val * n2.val;
                    break;
                case '/':
                    result =  (float)n1.val / (float)n2.val;
                    break;
                case '^':
                    result = pow((float)n1.val,(float)n2.val); 
                    break;
                default:
                    break;
            }

        }

    Operator(){
        types[0] = '+';
        types[1] = '-';
        types[2] = '*';
        types[3] = '/';
        types[4] = '^';
        op = &types[0];
    }// Constructor
}; 


class Calc{
    //@Breif Main Calculator application
    private:
        float result;
    public:
        int grid[5];
        Num n1;
        Operator op;
        Num n2;
        inline float getRes(void) {return result;}

        void update(){
            //@breif: computes result 
            op.run(n1, n2, result);
        }
    Calc(){
        grid[0] = 0; // Num1
        grid[1] = 15; // Operand
        grid[2] = 30; // Num2
        grid[3] = 45; // =
        grid[4] = 60; // Res
        result = 0.00;
    }

};

// ******** Globals ***************** //
Ticker calcTick;
Ticker blinkTick;
Timer debounce;

Calc calc;
RGB led(D5, D9, D8);
Cursor cursor;
FSM state;

InterruptIn fire(D4);
InterruptIn up(A2); 
InterruptIn down(A3);


//********* ISR's ****************** //
void calcISR(void){
    //@Breif Updates LCD with Values
    lcd.cls(); //clear lcd hot fix
    lcd.character(calc.grid[0] + 2, 5, calc.n1.getC()); 
    lcd.character(calc.grid[1] + 2, 5, calc.op.getC());
    lcd.character(calc.grid[2] + 2, 5, calc.n2.getC());
    lcd.character(calc.grid[3] + 2, 5, '=');
    lcd.locate(calc.grid[4], 5);
    lcd.printf("%.2f", calc.getRes()); // result 
}

void blinkISR(){
    //@Breif: Blink Cursor
    cursor.blink();
}

void fireISR(void){
    //@Breif: Controls the state machine 
    if(debounce.read_ms() < 500){
        debounce.reset();
        return;
    }
    switch (state){
        case INIT:
            calcTick.attach(&calcISR, 0.25); // 250ms calc update loop
            blinkTick.attach(&blinkISR, 0.5); //500ms blink update loop
            state = NUM1; 
            break;
        case NUM1:
            cursor.shift(); 
            led.set(BLUE);
            state = OPERAND; // next state
            break;
        case OPERAND:
            cursor.shift();
            led.set(GREEN);
            state =NUM2;
            break;
        case NUM2:
            cursor.cursorReset(); // shift cursto to begingin 
            led.set(RED);
            state = NUM1;   
            break;
        default:
            state = INIT;
            cursor.cursorReset(); // shift cursto to begingin 
            led.set(RED);
            break;
    }
}

void upISR(){
    //@Breif: Incremenets user input 
    switch(state){
        case NUM1:
            calc.n1.incr();
            break;
        case OPERAND:
            calc.op.incr();
            break;
        case NUM2:
            calc.n2.incr();
        default:
            break;
    }
}

void downISR(){
    //@Breif: Decrements user input
    switch(state){
        case NUM1:
            calc.n1.decr();
            break;
        case OPERAND:
            calc.op.decr();
            break;
        case NUM2:
            calc.n2.decr();
        default:
            break;
    }
}



// ****** MAIN *********** //
int main(void){
    state = INIT;
    led.setOff();
    debounce.start();
    fire.rise(&fireISR);   
    up.rise(&upISR);
    down.rise(&downISR);

    for(;;){}
}
