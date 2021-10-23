#ifndef lb_h
#define lb_h

#include <Arduino.h>


#define ACKMESSAGE 222

class dr_onewire
{

const int messagesymbolms = 30;


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
    bool getpinstatus();
    void showbuffercontent();
    uint8_t isbufferempty();
    void emptybuffer();
public:
    typedef uint16_t onewiremessage;
    int pin1;
    onewiremessage loopmessage;
    String name0="";
    dr_onewire *currentclass;
    unsigned long buffer[50];
    uint8_t buffercounter_high = 0;
    uint8_t buffercounter_low = 0;
    void (*isrcallback) (void);
    void (*receivedmessagecallback) (onewiremessage);
    void init(String name, uint8_t wrpin,void (*f)(void),void (*f2)(onewiremessage));
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