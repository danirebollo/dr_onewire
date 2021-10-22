#ifndef lb_h
#define lb_h

#include <Arduino.h>


#define ACKMESSAGE 222

class twowire_dr
{

const int messagesymbolms = 20;


bool status = false;
bool pinstatus = false;
unsigned long readtimer = (20 * messagesymbolms);
unsigned long tolerance = messagesymbolms / 3;

private:
    void detachinterrupt();
    void setpin2inputpullup();
    void setpin2outputopendrain();
    
    void setpinlow();
    void setpinhigh();

public:
    typedef uint16_t onewiremessage;
    int pin1;
    onewiremessage loopmessage;
    String name0="";
    twowire_dr *currentclass;
    unsigned long buffer[50];
    uint8_t buffercounter_high = 0;
    uint8_t buffercounter_low = 0;
    void (*isrcallback) (void);
    void init(String name, uint8_t wrpin,void (*f)(void));
    void sendmessage_raw(onewiremessage message);
    bool sendmessage(onewiremessage message);
    bool sendmessage(uint8_t cmd, uint8_t message);
    bool readmessage_raw(onewiremessage *message);
    bool loop();
    void isr(); //static
    uint8_t getbuffersize();
    unsigned long gettimesincefirstisr();
};

#endif