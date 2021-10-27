#ifndef lb_h
#define lb_h

#include <Arduino.h>

#define ENABLEPULLUPS
#define messagebufflen 10
#define ACKMESSAGE 222

#define PACKETSIZE (((1*8)*2)+1+2+3) //sizeof(onewiremessage)=1B
class dr_onewire
{



const int messagesymbolms = 30;

uint8_t inputmode=0;
bool status = false;
bool pinstatus = false;
unsigned long readtimer = (PACKETSIZE*2 * messagesymbolms);
unsigned long tolerance = messagesymbolms / 3;


private:
    void detachinterrupt();
    void setpin2input();
    void setpin2outputopendrain();
    
    void setpinlow();
    void setpinhigh();
    bool getpinstatus();
    void showbuffercontent();
    uint8_t isbufferempty();
    void emptybuffer();
    void setpinlowRAW();
    void setpinhighRAW();

public:
    typedef uint16_t onewiremessage;
    onewiremessage messagebuff[messagebufflen];
    uint8_t messagebuffpos=0;
    
    int pin1;
    onewiremessage loopmessage;
    String name0="";
    dr_onewire *currentclass;
    unsigned long buffer[50];
    uint8_t buffercounter_high = 0;
    uint8_t buffercounter_low = 0;
    void (*isrcallback) (void);
    void (*receivedmessagecallback) (onewiremessage);
    void init(String name, uint8_t wrpin,void (*f)(void),void (*f2)(onewiremessage), uint8_t inputmode0=INPUT);
    void sendmessage_raw(onewiremessage message);
    bool sendmessage(onewiremessage message);
    bool sendmessage(uint8_t cmd, uint8_t message);
    bool readmessage_raw(onewiremessage *message);
    bool loop();
    void isr(); //static
    uint8_t getbuffersize();
    unsigned long gettimesincefirstisr();
    bool sendmessage_addtobuff(onewiremessage message);
    bool sendmessage_getfrombuff(onewiremessage *message);
    uint8_t sendmessage_getbuffpos();
    bool sendmessage_r(onewiremessage message);
};

#endif