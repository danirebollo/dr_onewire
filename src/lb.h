#ifndef lb_h
#define lb_h

#include <Arduino.h>

class twowire_dr
{

const int messagesymbolms = 20;

unsigned long buffer[50];
uint8_t buffercounter_high = 0;
uint8_t buffercounter_low = 0;

bool status = false;
bool pinstatus = false;
unsigned long readtimer = (20 * messagesymbolms);
unsigned long tolerance = messagesymbolms / 3;

private:
public:
    void init(uint8_t wrpin);
    void sendmessage(uint8_t message);
    void readmessage();
    void isr();
    uint8_t getbuffersize();
    unsigned long gettimesincefirstisr();
};

#endif