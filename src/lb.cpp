
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
bool twowire_dr::getpinstatus()
{
    return digitalRead(pin1);
}

//!HAL
void twowire_dr::emptybuffer()
{
    buffercounter_high = 0;
        buffercounter_low = 0;
}
void twowire_dr::init(String name, uint8_t wrpin, void (*f)(void),void (*f2)(onewiremessage))
{
    isrcallback=*f;
    receivedmessagecallback=*f2;
    currentclass=this;
    name0=name;
    pin1=wrpin;
    setpin2inputpullup();
    isrcallback();
}

uint8_t twowire_dr::isbufferempty()
{
   if (getbuffersize() >1) 
   return false;
   else
   return true;
}
uint8_t twowire_dr::getbuffersize()
{
    return buffercounter_high - buffercounter_low;
}

void twowire_dr::showbuffercontent()
{
    Serial.print("shoubuffercontent: ");
    for(uint8_t i=buffercounter_low;i<buffercounter_high;i++)
    {
        Serial.print((String)i+": "+(String)buffer[i]+"\n");
    }
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
        //Serial.print("Readed bits: ");
        uint8_t parity=0;
        for (uint8_t i = 0; i < (sizeof(onewiremessage)*8); i++)
        {
            aVal = aVal << 1 | resultarray[i + 3];
            //Serial.print((String)resultarray[i + 3] + "");
            if(resultarray[i + 3]==1)
            parity++;
        }
        //Serial.print(" \n");

        //Serial.print("Readed Data: " + (String)aVal + "\n");
        *message=aVal;

        //Serial.print("("+(String)pin1+") Readed Data: " + (String)aVal + "/ "+(String)*message+"\n");
        
        //Serial.print("PARITY READED: "+(String)resultarray[(sizeof(onewiremessage)*8)+3]+", calculated: "+parity+" ("+(String)(parity % 2)+") \n");

        if ((!(parity % 2)) != resultarray[(sizeof(onewiremessage)*8)+3])
        {
           //Serial.print("PARITY NOK\n");
           result=false;
        }

        //Serial.print("\n");
        

        //enabling ISR

        //Serial.print("ReadTask: Done\n");

        //TODO fix buffer to allow isr...
        
                isrcallback();
        delay(50);
        emptybuffer();
        
        return result;
    }
    else
    {
        *message=0;
        return false;
    }
}

bool twowire_dr::sendmessage(uint8_t cmd, uint8_t message)
{
    return sendmessage(((uint16_t)message << 8) | cmd);
}

bool twowire_dr::sendmessage(onewiremessage message)
{
    delay(messagesymbolms*8);
    bool result=true;
    unsigned long timer=millis();
    uint16_t readedmessage=0;
    bool timeout=false;
    int counter=0;

   Serial.print((String)millis()+" - "+(String)pin1+" - sendmessage 1 message "+(String)message+". buff: "+(String)getbuffersize()+"("+(String)buffercounter_high+"/"+(String)buffercounter_low+"), pinstatus: "+(String)getpinstatus()+"\n");
       
   while(getbuffersize() >1 || !getpinstatus())
   {
       delay(100);
       //Serial.print((String)millis()+" - "+"sendmessage ("+(String)pin1+") emptying buffer before sendmessage "+(String)message+". buff: "+(String)getbuffersize()+"\n");
       loop();
       delay(100);
       if(timer+20000<millis())
       {
           Serial.print((String)millis()+" - "+"sendmessage ("+(String)pin1+") getbuffersize timeout error. message "+(String)message+"\n");
           result=false;
           break;
       }
   }
   
    //Serial.print((String)millis()+" - "+(String)pin1+" - sendmessage 2 sendmessage "+(String)message+". buff: "+(String)getbuffersize()+"\n");

    if(result)
    {
        result=false;
        sendmessage_raw(message);
        delay(messagesymbolms*8);

        timer=millis();
        while(!timeout)
        {
            bool a1=readmessage_raw(&readedmessage);
            if(a1)
            {
                if(readedmessage==ACKMESSAGE)
                {
                    result=true;
                    break;
                }
                else
                {
                    Serial.print((String)millis()+" - "+(String)pin1+" - sendmessage ACK error. received message "+(String)readedmessage+"\n");
                    result=false;
                }
            }
            if(millis()>timer+3000)
                timeout=true;
            
            if(readedmessage!=0 )
                Serial.print((String)millis()+" - "+(String)pin1+" - sendmessage loop readed: "+(String)readedmessage+", bool: "+a1+", counter: "+(String)counter+"\n");
            counter++;
        }
        //Serial.print("sendmessage ("+(String)pin1+") received message "+(String)readedmessage+"\n");
                
        if(timeout)
        {
            Serial.print((String)millis()+" - "+(String)pin1+" - sendmessage timeour error. no received message \n");     
        }
    }

    if(result)
        Serial.print((String)millis()+" - "+(String)pin1+" - # Success sending message '"+(String)message+"'\n\n");
    else
        Serial.print((String)millis()+" - "+(String)pin1+" - ## CAUTION!! LOST MESSAGE '"+(String)message+"' \n");
    
    return result;
}


bool twowire_dr::loop()
{
    if(readmessage_raw(&loopmessage))
    {
        Serial.print((String)millis()+" - "+(String)pin1+" - loop sending ack \n");  
      sendmessage_raw(ACKMESSAGE);
        receivedmessagecallback(loopmessage);
    }

    delay(messagesymbolms*8);
    return true;
}