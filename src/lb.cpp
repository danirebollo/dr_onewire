
#include "lb.h"

const int led1 = 2;
const int pin1 = 5;
const int pin2 = 18;
const int messagesymbolms = 20;

unsigned long buffer[50];
uint8_t buffercounter_high = 0;
uint8_t buffercounter_low = 0;

bool status = false;
bool pinstatus = false;
unsigned long readtimer = (20 * messagesymbolms);
unsigned long tolerance = messagesymbolms / 3;

uint8_t getbuffersize()
{
    return buffercounter_high - buffercounter_low;
}
unsigned long gettimesincefirstisr()
{
    return millis() - buffer[buffercounter_low];
}
void isr()
{
    buffer[buffercounter_high] = millis();
    buffercounter_high++;
}

//send message
void sendmessage(uint8_t message)
{
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
}

void readmessage()
{
    if (getbuffersize() > 0 && (gettimesincefirstisr() > readtimer))
    {
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
        Serial.print("bitarray: [");
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
        if ((aVal % 2) == resultarray[11])
        {
            Serial.print("PARITY OK\n");
        }

        Serial.print("\n");
        buffercounter_high = 0;
        buffercounter_low = 0;

        //enabling ISR

        Serial.print("ReadTask: Done\n");
    }
    else
    {
        delay(100);
    }
}
