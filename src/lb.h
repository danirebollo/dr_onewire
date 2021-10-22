#ifndef lb_h
#define lb_h

#include <Arduino.h>

class twowire_dr
{

const int messagesymbolms = 20;


bool status = false;
bool pinstatus = false;
unsigned long readtimer = (20 * messagesymbolms);
unsigned long tolerance = messagesymbolms / 3;

private:
public:
    int pin1;
    String name0="";
    twowire_dr *currentclass;
    unsigned long buffer[50];
    uint8_t buffercounter_high = 0;
    uint8_t buffercounter_low = 0;
    void (*isrcallback) (void);
    void init(String name, uint8_t wrpin,void (*f)(void));
    void sendmessage(uint8_t message);
    bool readmessage();
    void isr(); //static
    uint8_t getbuffersize();
    unsigned long gettimesincefirstisr();
};

#endif