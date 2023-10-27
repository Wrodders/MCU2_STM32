#include "mbed.h"


class Joystick{

    private:    
        DigitalIn up, down, left, right, fire;
    public:
        Joystick(PinName u, PinName d, PinName l, PinName r, PinName f) : up(u), down(d), left(l), right(r), fire(f){};

        bool upPressed(void){
            return up;
        }
        bool downPressed(void){
            return down;
        }
        bool leftPressed(void){
            return left;
        }
        bool rightPressed(void){
            return right;
        }
        bool firePressed(void){
            return fire;
        }
};


class RGB{

    private:
        PwmOut r, g, b;
    public:
        RGB(PinName red, PinName green, PinName blue, float pwm_period = 0.001f) : r(red), g(green), b(blue) {
        
        r.period(pwm_period);
        g.period(pwm_period);
        b.period(pwm_period);
        r.write(1.0f);
        g.write(1.0f);
        b.write(1.0f);
    }

        void setRed(void){
            r.write(0.0f);
        }
        void setGreen(void){
            g.write(0.0f);
        }
        void setBlue(void){
            b.write(0.0f);
        }

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
};



#define RED    255, 0, 0
#define GREEN  0, 255, 0
#define BLUE   0, 0, 255
#define YELLOW 255, 255, 0
#define CYAN   0, 255, 255
#define MAGENTA 255, 0, 255
#define WHITE  255, 255, 255



// main() runs in its own thread in the OS
int main()
{
    Joystick js(A2, A3, A4, A5, D4);


    RGB led(D5, D9, D8);
    while (true) {

        if(js.upPressed()){
            led.set(YELLOW);
        }
        else if (js.downPressed()){
            led.setGreen();
        }
        else if (js.leftPressed()){
            led.setRed();
        }
        else if(js.rightPressed()){
            led.setWhite();
        }
        else if (js.firePressed()){
            led.setBlue();
        }
        else {
            led.setOff();
        }
        
    }
}


