
#include "lb.h"
//HAL
void twowire_dr::detachinterrupt()
{
    detachInterrupt(pin1);
}

void twowire_dr::setpin2inputpullup()
{
    pinMode(pin1, INPUT_PULLUP);
}
void twowire_dr::setpin2outputopendrain()
{
    pinMode(pin1, OUTPUT_OPEN_DRAIN);
}
void twowire_dr::setpinlow()
{
    digitalWrite(pin1, LOW);
}
void twowire_dr::setpinhigh()
{
    digitalWrite(pin1, HIGH);
}
//!HAL

void twowire_dr::init(String name, uint8_t wrpin, void (*f)(void))
{
    isrcallback=*f;
    currentclass=this;
    name0=name;
    pin1=wrpin;
    setpin2inputpullup();
    isrcallback();
}

uint8_t twowire_dr::getbuffersize()
{
    return buffercounter_high - buffercounter_low;
}
unsigned long twowire_dr::gettimesincefirstisr()
{
    return millis() - buffer[buffercounter_low];
}
void twowire_dr::isr()
{
    buffer[buffercounter_high] = millis();
    buffercounter_high++;
}

//send message
void twowire_dr::sendmessage_raw(onewiremessage message)
{
    detachinterrupt();
    setpin2outputopendrain();
    //Start bits /sync
    setpinlow();
    delay(messagesymbolms * 2);
    setpinhigh();
    delay(messagesymbolms);

    //sending 8b data
    uint8_t parity = 0;
    for (uint8_t i = 0; i < (sizeof(onewiremessage)*8); i++)
    {
        if (bitRead(message, (sizeof(onewiremessage)*8)-1 - i))
        {
            setpinhigh();
            delay(messagesymbolms);
            parity++;
        }
        else
        {
            setpinlow();
            delay(messagesymbolms);
        }
    }

    //calculate parity bit
    if ((parity % 2) == 0) //par
    {
        setpinhigh();
    }
    else
    {
        setpinlow();
    }
    delay(messagesymbolms);

    //Stop bit
    setpinlow();
    delay(messagesymbolms);

    setpinhigh();
    setpin2inputpullup();
    isrcallback();
}

bool twowire_dr::readmessage_raw(onewiremessage *message)
{
    if (getbuffersize() > 0 && (gettimesincefirstisr() > readtimer))
    {
        bool result=true;
        //TODO fix buffer to allow isr...
        //Serial.print("("+(String)pin1+") readmessage_raw disabling ISR \nreading message\n");
        detachinterrupt();
        bool status = false;
        bool resultarray[(sizeof(onewiremessage)*8)+1+2+3]; //sizeof(onewiremessage)+1+2+3 size: max transitions. 8b=9, start=2, parity+stop=3
        uint8_t racounter = 0;
        for (uint8_t j = buffercounter_low + 1; j < buffercounter_high; j++)
        {
            unsigned long symbol = ((buffer[j] - buffer[j - 1]) + tolerance) / messagesymbolms;

            for (uint8_t k = 0; k < symbol; k++)
            {
                resultarray[racounter] = status;
                racounter++;
            }
            status = !status;
        }
        /*
        //Serial.print("readingmessage on "+name0+": bitarray: [");
        for (uint8_t k = 0; k < WORDSIZE; k++)
        {
            //Serial.print("" + (String)resultarray[k] + (String) "");
        }
        */
       // Serial.print("]\n");
        if (!(resultarray[0] == 0 && resultarray[1] == 0 && resultarray[2] == 1))
        {
            //Serial.print("Start NOK \n");
            result=false;
        }
        

        onewiremessage aVal = 0;

        for (uint8_t i = 0; i < (sizeof(onewiremessage)*8); i++)
        {
            aVal = aVal << 1 | resultarray[i + 3];
        }
        //Serial.print("Data: " + (String)aVal + "\n");
        message=&aVal;

        if ((aVal % 2) != resultarray[(sizeof(onewiremessage)*8)+3])
        {
           // Serial.print("PARITY NOK\n");
           result=false;
        }

        //Serial.print("\n");
        buffercounter_high = 0;
        buffercounter_low = 0;

        //enabling ISR

        //Serial.print("ReadTask: Done\n");

        //TODO fix buffer to allow isr...
        
        //Serial.print("("+(String)pin1+") readmessage_raw Enabling ISR \n");
        setpin2inputpullup();
        isrcallback();
        return result;
    }
    else
    {
        delay(500);
        return false;
    }
}

bool twowire_dr::sendmessage(uint8_t cmd, uint8_t message)
{
    return sendmessage(((uint16_t)message << 8) | cmd);
}

bool twowire_dr::sendmessage(onewiremessage message)
{
    bool result=false;
    sendmessage_raw(message);
    delay(200);
    unsigned long timer=millis();
    onewiremessage readedmessage=0;
    while((millis()<timer+10000))
    {
        if(readmessage_raw(&readedmessage))
        {
            if(readedmessage==ACKMESSAGE)
            {

            }
            //Serial.print("sendmessage ACK: "+(String)readedmessage+"\n");
            result=true;
            break;
        }
    }
    return result;
}


bool twowire_dr::loop()
{
    if(readmessage_raw(&loopmessage))
    {
      sendmessage_raw(ACKMESSAGE);
    }
    //delay(300);
}