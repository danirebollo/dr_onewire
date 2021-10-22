
#include "lb.h"


void twowire_dr::init(String name, uint8_t wrpin, void (*f)(void))
{
    isrcallback=*f;
    currentclass=this;
    name0=name;
    pin1=wrpin;
    pinMode(pin1, INPUT_PULLUP);
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
void twowire_dr::sendmessage_raw(uint8_t message)
{
    detachInterrupt((pin1));
    pinMode(pin1, OUTPUT_OPEN_DRAIN);
    //Start bits /sync
    digitalWrite(pin1, LOW);
    delay(messagesymbolms * 2);
    digitalWrite(pin1, HIGH);
    delay(messagesymbolms);

    //sending 8b data
    uint8_t parity = 0;
    for (uint8_t i = 0; i < 8; i++)
    {
        if (bitRead(message, 7 - i))
        {
            digitalWrite(pin1, HIGH);
            delay(messagesymbolms);
            parity++;
        }
        else
        {
            digitalWrite(pin1, LOW);
            delay(messagesymbolms);
        }
    }

    //parity bit
    if ((parity % 2) == 0) //par
    {
        digitalWrite(pin1, HIGH);
    }
    else
    {
        digitalWrite(pin1, LOW);
    }
    delay(messagesymbolms);

    //Stop bit
    digitalWrite(pin1, LOW);
    delay(messagesymbolms);

    digitalWrite(pin1, HIGH);
    pinMode(pin1, INPUT_PULLUP);
    isrcallback();
}

bool twowire_dr::readmessage_raw(uint8_t *message)
{
    if (getbuffersize() > 0 && (gettimesincefirstisr() > readtimer))
    {
        //TODO fix buffer to allow isr...
        //Serial.print("("+(String)pin1+") readmessage_raw disabling ISR \nreading message\n");
        detachInterrupt((pin1));
        bool status = false;
        uint8_t WORDSIZE = 13;
        bool resultarray[WORDSIZE];
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
        Serial.print("readingmessage on "+name0+": bitarray: [");
        for (uint8_t k = 0; k < WORDSIZE; k++)
        {
            Serial.print("" + (String)resultarray[k] + (String) "");
        }
        Serial.print("]\n");
        if (resultarray[0] == 0 && resultarray[1] == 0 && resultarray[2] == 1)
        {
            Serial.print("Start OK \n");
        }

        uint8_t aVal = 0;

        for (uint8_t i = 0; i < 8; i++)
        {
            aVal = aVal << 1 | resultarray[i + 3];
        }
        Serial.print("Data: " + (String)aVal + "\n");
        message=&aVal;

        if ((aVal % 2) == resultarray[11])
        {
            Serial.print("PARITY OK\n");
        }

        Serial.print("\n");
        buffercounter_high = 0;
        buffercounter_low = 0;

        //enabling ISR

        Serial.print("ReadTask: Done\n");

        //TODO fix buffer to allow isr...
        
        //Serial.print("("+(String)pin1+") readmessage_raw Enabling ISR \n");
        pinMode(pin1, INPUT_PULLUP);
        isrcallback();
        return true;
    }
    else
    {
        delay(500);
        return false;
    }
}

bool twowire_dr::sendmessage(uint8_t message)
{
    bool result=false;
    sendmessage_raw(message);
    delay(200);
    unsigned long timer=millis();
    uint8_t readedmessage=0;
    while((millis()<timer+10000))
    {
        if(readmessage_raw(&readedmessage))
        {
            if(readedmessage==ACKMESSAGE)
            {

            }
            Serial.print("sendmessage ACK: "+(String)readedmessage+"\n");
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
    delay(300);
}