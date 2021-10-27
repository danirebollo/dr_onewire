
#include "dr_onewire.h"
//HAL
void dr_onewire::detachinterrupt()
{
    detachInterrupt(pin1);
}

void dr_onewire::setpin2input()
{
    pinMode(pin1, inputmode);
}
void dr_onewire::setpin2outputopendrain()
{
    pinMode(pin1, OUTPUT_OPEN_DRAIN);
}
void dr_onewire::setpinlowRAW()
{
    digitalWrite(pin1, LOW);
}
void dr_onewire::setpinhighRAW()
{
    digitalWrite(pin1, HIGH);
}

void dr_onewire::setpinlow()
{
    setpinlowRAW();
    delay(messagesymbolms);
    setpinhighRAW();
    delay(messagesymbolms);
}
void dr_onewire::setpinhigh()
{
    setpinhighRAW();
    delay(messagesymbolms);
    setpinlowRAW();
    delay(messagesymbolms);
}
bool dr_onewire::getpinstatus()
{
    return digitalRead(pin1);
}

//!HAL
void dr_onewire::emptybuffer()
{
    buffercounter_high = 0;
        buffercounter_low = 0;
}
void dr_onewire::init(String name, uint8_t wrpin, void (*f)(void),void (*f2)(onewiremessage), uint8_t inputmode0)
{
    inputmode=inputmode0; //INPUT, INPUT_PULLUP
    isrcallback=*f;
    receivedmessagecallback=*f2;
    currentclass=this;
    name0=name;
    pin1=wrpin;
    setpin2input();
    isrcallback();
}

uint8_t dr_onewire::isbufferempty()
{
   if (getbuffersize() >1) 
   return false;
   else
   return true;
}
uint8_t dr_onewire::getbuffersize()
{
    return buffercounter_high - buffercounter_low;
}

void dr_onewire::showbuffercontent()
{
    Serial.print("shoubuffercontent: ");
    for(uint8_t i=buffercounter_low;i<buffercounter_high;i++)
    {
        Serial.print((String)i+": "+(String)buffer[i]+"\n");
    }
}
unsigned long dr_onewire::gettimesincefirstisr()
{
    return millis() - buffer[buffercounter_low];
}
void dr_onewire::isr()
{
    buffer[buffercounter_high] = millis();
    buffercounter_high++;
}

//send message
void dr_onewire::sendmessage_raw(onewiremessage message)
{
    detachinterrupt();
    setpin2outputopendrain();
    //Start bits /sync
    //Serial.print("SENDING BIN: '");
    //Serial.print(message,BIN);
    Serial.print("sendmessage_raw:: '"+(String)message+"'\n");
    //Serial.print("START: GO LOW 0 2T\n");
    setpinlowRAW();
    delay(messagesymbolms * 2);
    //Serial.print("GO HIGH 1 1T\n");
    setpinhighRAW();
    delay(messagesymbolms);

    //sending 8b data
    uint8_t parity = 0;
    //Serial.print("Sending: ");
    //    for (uint8_t i = 0; i < (sizeof(onewiremessage)*8); i++)
    //{
    //    Serial.print((String)bitRead(message, (sizeof(onewiremessage)*8)-1 - i));
    //}
    //Serial.print("\nENC: ");
    for (uint8_t i = 0; i < (sizeof(onewiremessage)*8); i++)
    {
        if (bitRead(message, (sizeof(onewiremessage)*8)-1 - i))
        {
            //Serial.print("10 ");
            setpinhigh();
            //delay(messagesymbolms);
            parity++;
        }
        else
        {
            //Serial.print("01 ");
            setpinlow();
            //delay(messagesymbolms);
        }
    }
    //Serial.print("\n");
    //Serial.print("Parity: ");
    //calculate parity bit
    if ((parity % 2) == 0) //par
    {
    //    Serial.print("1 (10) ");
        setpinhigh();
    }
    else
    {
    //    Serial.print("0 (01) ");
        setpinlow();
    }
    //sending ACK
    setpinlow();

    //delay(messagesymbolms);

    //Stop bit
    //Serial.print("STOP BIT 0\n");
    setpinlowRAW();
    delay(messagesymbolms);

    //Serial.print("GO HIGH 1\n");
    setpinhighRAW();
    setpin2input();
    isrcallback();
}

bool dr_onewire::readmessage_raw(onewiremessage *message)
{
    if (getbuffersize() > 0 && (gettimesincefirstisr() > readtimer))
    {
        bool result=true;
        //TODO fix buffer to allow isr...
        //Serial.print("("+(String)pin1+") readmessage_raw disabling ISR \nreading message\n");
        detachinterrupt();
        bool status = false;
        bool resultarray[((sizeof(onewiremessage)*8)*2)+1+2+3]; //sizeof(onewiremessage)+1+2+3 size: max transitions. 8b=9, start=2, parity+stop=3
        bool resultarray2[((sizeof(onewiremessage)*8)*2)+2]; //sizeof(onewiremessage)+1+2+3 size: max transitions. 8b=9, start=2, parity+stop=3
        uint8_t racounter = 0;
        uint8_t ms=(buffer[1] - buffer[0])/2;
        //uint8_t messagesize=0;
        for (uint8_t j = buffercounter_low + 1; j < buffercounter_high; j++)
        {
            unsigned long symbol = ((buffer[j] - buffer[j - 1]) + tolerance) / ms;

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
       //Serial.print("Reading: \nStart: "+(String)resultarray[0]+(String)resultarray[1]+(String)resultarray[2]+"\n");
        if (!(resultarray[0] == 0 && resultarray[1] == 0 && resultarray[2] == 1))
        {
            Serial.print("Start NOK \n");
            result=false;
        }
        

        onewiremessage aVal = 0;   
         uint8_t parity=0;
        //Serial.print("Readed bits: ");
        //
        //for (uint8_t i = 0; i < ((sizeof(onewiremessage)*8)*2); i++)
        //{
        //    //aVal = aVal << 1 | resultarray[i + 3];
        //    Serial.print((String)resultarray[i + 3] + "");
        //    //if(resultarray[i + 3]==1)
        //    //parity++;
        //}
        //Serial.print(" \n");

        //Serial.print("Readed bits2: ");

        for (uint8_t i = 0; i < ((sizeof(onewiremessage)*8)*2); )
        {
            bool value=false;
            if(resultarray[i + 3]==true&&resultarray[i +1 + 3]==false)
                resultarray2[i]=1;
            else if (resultarray[i + 3]==false&&resultarray[i +1 + 3]==true)
                resultarray2[i]=0;
            else
                Serial.print(" (i: "+(String)i+")[CAUTION!! ERROR DECODING] ("+(String)resultarray[i + 3]+(String)resultarray[i +1+ 3]+")");
            
        //    Serial.print((String)resultarray2[i] );

            aVal = aVal << 1 | resultarray2[i];
            //Serial.print((String)resultarray2[i + 3] + "");
            if(resultarray2[i]==1)
            parity++;
i++;
i++;
        }
        //Serial.print(" \n");

        Serial.print("readmessage_raw:: Readed Data: " + (String)aVal + "\n");
        *message=aVal;

        //Serial.print("("+(String)pin1+") Readed Data: " + (String)aVal + "/ "+(String)*message+"\n");
        
        //Serial.print("PARITY READED: "+(String)resultarray[(sizeof(onewiremessage)*8)+3]+", calculated: "+parity+" ("+(String)(parity % 2)+") \n");

        if ((!(parity % 2)) != resultarray[((sizeof(onewiremessage)*8)*2)+3])
        {
           Serial.print("PARITY NOK\n");
           result=false;
        }
        if (resultarray[((sizeof(onewiremessage)*8)*2)+3+1]==1)
        {
           Serial.print("ACK\n");
        }
        else
        Serial.print("NOT ACK\n");
        //else
        //Serial.print("PARITY OK\n");

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

bool dr_onewire::sendmessage(uint8_t cmd, uint8_t message)
{
    return sendmessage(((uint16_t)message << 8) | cmd);
}


bool dr_onewire::sendmessage_addtobuff(onewiremessage message)
{
    messagebuffpos++;
    if(messagebuffpos>=messagebufflen)
    {
        return false;
    }
    messagebuff[messagebuffpos]=message;
    
    return true;
}
bool dr_onewire::sendmessage_getfrombuff(onewiremessage *message)
{
    if(messagebuffpos>0)
    {
        *message=messagebuff[messagebuffpos];
        messagebuffpos--;
        return true;
    }
    return false;
}
uint8_t dr_onewire::sendmessage_getbuffpos()
{
    return messagebuffpos;
}
bool dr_onewire::sendmessage(onewiremessage message)
{
    if(!sendmessage_addtobuff(message))
    {
        Serial.print((String)message+"\t-\t"+(String)pin1+"\t-\t"+(String)millis()+" - "+(String)pin1+" - sendmessage ERROR 1 message "+(String)message+". buff: "+(String)getbuffersize()+"("+(String)buffercounter_high+"/"+(String)buffercounter_low+"), pinstatus: "+(String)getpinstatus()+"\n");
        return false;
    }
    return true;
}

bool dr_onewire::sendmessage_r(onewiremessage message)
{
    delay(messagesymbolms*8);
    bool result=true;
    unsigned long timer=millis();
    uint16_t readedmessage=0;
    bool timeout=false;
    int counter=0;

   //Serial.print((String)message+"\t-\t"+(String)pin1+"\t-\t"+(String)millis()+" - "+(String)pin1+" - sendmessage 1 message "+(String)message+". buff: "+(String)getbuffersize()+"("+(String)buffercounter_high+"/"+(String)buffercounter_low+"), pinstatus: "+(String)getpinstatus()+"\n");
       
   while(getbuffersize() >1 || !getpinstatus())
   {
       delay(100);
       //Serial.print((String)millis()+" - "+"sendmessage ("+(String)pin1+") emptying buffer before sendmessage "+(String)message+". buff: "+(String)getbuffersize()+"\n");
       loop();
       delay(100);
       if(timer+20000<millis())
       {
           Serial.print((String)message+"\t-\t"+(String)pin1+"\t-\t"+(String)millis()+" - "+"sendmessage ("+(String)pin1+") getbuffersize timeout error. message "+(String)message+"\n");
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
                    //Serial.print((String)message+"\t-\t"+(String)pin1+"\t-\t"+(String)millis()+" - "+(String)pin1+" - sendmessage ACK error. received message "+(String)readedmessage+"\n");
                    //Serial.print((String)readedmessage+"\t-\t"+(String)pin1+"\t-\t"+(String)millis()+" - "+(String)pin1+" - sendmessage sending ACK."+"\n");            
                    sendmessage_raw(ACKMESSAGE);  
                    receivedmessagecallback(readedmessage);
                    delay(messagesymbolms*8);
                    return false;
                }
            }


            if(millis()>timer+10000)
            {
                timeout=true;
                Serial.print((String)message+"\t-\t"+(String)pin1+"\t-\t"+(String)millis()+" - "+(String)pin1+" - sendmessage TIMEOUT, message: "+(String)message+"\n");      
            }
                
            
            //if(readedmessage!=0 )
            //    Serial.print((String)message+"\t-\t"+(String)pin1+"\t-\t"+(String)millis()+" - "+(String)pin1+" - sendmessage loop readed: "+(String)readedmessage+", bool: "+a1+", counter: "+(String)counter+"\n");
            counter++;
            delay(500);
        }
        //Serial.print("sendmessage ("+(String)pin1+") received message "+(String)readedmessage+"\n");
                
        if(timeout)
        {
            Serial.print((String)message+"\t-\t"+(String)pin1+"\t-\t"+(String)millis()+" - "+(String)pin1+" - sendmessage timeour error. no received message \n");     
        }
    }

    if(!result)
    {
        Serial.print((String)message+"\t-\t"+(String)pin1+"\t-\t"+(String)millis()+" - "+(String)pin1+" - sendmessage READDING TO SEND BUF message "+(String)message+". buff: "+(String)getbuffersize()+"("+(String)buffercounter_high+"/"+(String)buffercounter_low+"), pinstatus: "+(String)getpinstatus()+"\n");
        sendmessage_addtobuff(message);
    }

    return result;
}


bool dr_onewire::loop()
{
    if(readmessage_raw(&loopmessage))
    {
        //Serial.print((String)loopmessage+"\t-\t"+(String)pin1+"\t-\t"+(String)millis()+" - "+(String)pin1+" - sending ack \n");  
        sendmessage_raw(ACKMESSAGE);  
        //Serial.print((String)loopmessage+"\t-\t"+(String)pin1+"\t-\t"+(String)millis()+" - "+" end receiving message '"+loopmessage+"'\n"); 
        receivedmessagecallback(loopmessage);
    }
    else{
        onewiremessage message;
        if(sendmessage_getfrombuff(&message))
        {
            sendmessage_r(message);
            //Serial.print((String)message+"\t-\t"+(String)pin1+"\t-\t"+(String)millis()+" - "+(String)pin1+" - sendmessage ERROR 2 message "+(String)message+". buff: "+(String)getbuffersize()+"("+(String)buffercounter_high+"/"+(String)buffercounter_low+"), pinstatus: "+(String)getpinstatus()+"\n");
        }
    }
    
    delay(messagesymbolms*8);
    return true;
}