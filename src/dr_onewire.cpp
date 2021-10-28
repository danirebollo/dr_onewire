
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
unsigned long dr_onewire::gettimefromlastisr()
{
    if(millis()>buffer[buffercounter_high-1])
    return millis() - buffer[buffercounter_high-1];
    else
    return 0;
}

unsigned long dr_onewire::gettimesincefirstisr()
{
    if(millis()>buffer[buffercounter_low])
    return millis() - buffer[buffercounter_low];
    else
    return 0;
}

void dr_onewire::isr()
{
    buffer[buffercounter_high] = millis();
    buffercounter_high++;
}

//send message
void dr_onewire::sendmessage_raw(onewiremessage message, bool isack)
{
    detachinterrupt();
    setpin2outputopendrain();
    //Start bits /sync
    //Serial.print("SENDING BIN: '");
    //Serial.print(message,BIN);
    Serial.print((String)millis()+" - "+(String)pin1+" - sendmessage_raw:: '"+(String)message+"'\n");
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
    if(isack)
        setpinhigh();
    else
        setpinlow();
    //delay(messagesymbolms);
    setpinhigh();
    //Stop bit
    //Serial.print("STOP BIT 0\n");
    setpinlowRAW();
    delay(messagesymbolms);

    //Serial.print("GO HIGH 1\n");
    setpinhighRAW();
    setpin2input();
    isrcallback();
}

bool dr_onewire::readmessage_raw(onewiremessage *message, bool *isack)
{
    *isack=0;
    if ( getbuffersize() > 0 && (gettimefromlastisr() > WAITUNTILREAD)) // (gettimefromlastisr() > 500) // getbuffersize() > 10 && gettimesincefirstisr readtimer WAITUNTILREAD
    {
        //Serial.print((String)millis()+" - "+(String)pin1+" - buffersize: "+getbuffersize()+", gettimesincefirstisr():"+gettimesincefirstisr()+", gettimefromlastisr():"+gettimefromlastisr()+", readtimer: "+readtimer+" \n");
        //Serial.print((String)millis()+" - "+(String)pin1+" - buffercounter_low: "+buffercounter_low+", buffer[buffercounter_low]:"+(String)buffer[buffercounter_low]+"\n");
        //Serial.print((String)millis()+" - "+(String)pin1+" - buffercounter_high-1: "+(buffercounter_high-1)+", buffer[buffercounter_high]-1:"+(String)(buffer[buffercounter_high-1])+"\n");
        //Serial.print((String)millis()+" - "+(String)pin1+" - millis: "+(millis())+"\n");

        bool result=true;
        //TODO fix buffer to allow isr...
        detachinterrupt();
        bool status = false;
        bool resultarray[((sizeof(onewiremessage)*8)*2)+1+2+3+2]; //sizeof(onewiremessage)+1+2+3 size: max transitions. 8b=9, start=2, parity+stop=3
        uint8_t racounter = 0;
        uint8_t ms=(buffer[1] - buffer[0])/2;
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
            Serial.print((String)millis()+" - "+(String)pin1+" - Start NOK \n");
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
        bool paritybit=0;
        bool ackbit=0;
        uint8_t messagebuffersize=(sizeof(onewiremessage)*8*2)+2;
        uint8_t p = 0;

        //Serial.print("Decoding: ");
        while (p < ((messagebuffersize)+1))
        {
            bool value=false;
            if(resultarray[p + 3]==true&&resultarray[p +1 + 3]==false)
                value=1;
            else if (resultarray[p + 3]==false&&resultarray[p +1 + 3]==true)
                value=0;
            else
            {
                Serial.print((String)millis()+" - "+(String)pin1+" - (i: "+(String)p+")[CAUTION!! ERROR DECODING] ("+(String)resultarray[p + 3]+(String)resultarray[p +1+ 3]+")\n");
                result=false;
                value=0;
                paritybit=0;
                ackbit=0;
                break;
            }

            if(p<((sizeof(onewiremessage)*8)*2))
            {
              aVal = aVal << 1 | value;  
              if(value==1 )
                parity++;
            }
            else if(p==((sizeof(onewiremessage)*8)*2))
                paritybit=value;
            else if(p==((sizeof(onewiremessage)*8)*2)+2)
            {
                ackbit=value;
            }            
            p=p+2;
        }

        String msg="readmessage_raw:: Readed Data: " + (String)aVal + ", ";
        *message=aVal;

        if ((!(parity % 2)) != paritybit)
        {
           msg+="PARITY NOK, ";
           result=false;
        }
        if (ackbit)
        {
            *isack=1;
           msg+="ACK, ";
        }
        else
        msg+="NOT ACK";

        Serial.println((String)millis()+" - "+(String)pin1+" - "+ msg);

        //enabling ISR

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


bool dr_onewire::sendmessage_addtobuff(messagestruct message)
{
    messagebuffpos++;
    if(messagebuffpos>=messagebufflen)
    {
        return false;
    }
    messagebuff[messagebuffpos]=message;
    
    return true;
}
bool dr_onewire::sendmessage_getfrombuff(messagestruct *message)
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
    messagestruct ms1;
    ms1.message=message;
    if(!sendmessage_addtobuff(ms1))
    {
        Serial.print((String)message+"\t-\t"+(String)pin1+"\t-\t"+(String)millis()+" - "+(String)pin1+" - sendmessage ERROR 1 message "+(String)message+". buff: "+(String)getbuffersize()+"("+(String)buffercounter_high+"/"+(String)buffercounter_low+"), pinstatus: "+(String)getpinstatus()+"\n");
        return false;
    }
    return true;
}

bool dr_onewire::ismessagewaitingforack()
{

    return txmsg1.waitingforack;
}
bool dr_onewire::sendmessage_r(messagestruct message)
{
    delay(messagesymbolms*8);
    bool result=true;
    if(getbuffersize() >1 || !getpinstatus())
    {
        result=false;
        Serial.print((String)millis()+" - "+"sendmessage ("+(String)pin1+") buffer not empty> "+(String)message.message+". buff: "+(String)getbuffersize()+"\n");
    }
    if(result)
    {
        Serial.print((String)millis()+" - "+(String)pin1+" - sendmessage 2 sendmessage "+(String)message.message+". ACK: "+(String)message.ack+" buff: "+(String)getbuffersize()+"\n");
        result=false;
        sendmessage_raw(message.message,message.ack);
        delay(messagesymbolms*8);
        txmsg1.message=message;
        if(txmsg1.message.ack==0)
        {
            Serial.print((String)millis()+" - "+(String)pin1+" - sendmessage 3 sendmessage "+(String)message.message+". ACK: "+(String)message.ack+" setting waitingforack to 1\n");
            Serial.print((String)millis()+" - "+(String)pin1+" - setting waitingforack to 1\n");
            txmsg1.waitingforack=1;
            txmsg1.txtimestamp=millis();
        }
    }
    return result;
}


bool dr_onewire::loop()
{
    bool isack;
    //Timeout TX messages recollect
    if((txmsg1.txtimestamp+TXACKTIMEOUT<millis())&&txmsg1.waitingforack==1)
    {
        Serial.print((String)millis()+" - "+(String)pin1+" - setting waitingforack to 0\n");
        txmsg1.waitingforack=0;
        Serial.print((String)millis()+" - "+(String)pin1+" - LOST TX message: "+(String)txmsg1.message.message+" [timeout ("+(String)txmsg1.txtimestamp+" / "+(String)millis()+" ) ]\n");
        txmsg1.message.retrycounter++;
        if(txmsg1.message.retrycounter<2)
        {
            Serial.print((String)millis()+" - "+(String)pin1+" - "+(String)txmsg1.message.message+"\t-\t"+(String)pin1+"\t-\t"+(String)millis()+" - "+(String)pin1+" - sendmessage READING TO SEND BUF message "+(String)txmsg1.message.message+". buff: "+(String)getbuffersize()+"("+(String)buffercounter_high+"/"+(String)buffercounter_low+"), pinstatus: "+(String)getpinstatus()+"\n");
            sendmessage_addtobuff(txmsg1.message);  
        }
        else
            Serial.print((String)millis()+" - "+(String)pin1+" - "+ (String)txmsg1.message.message+"\t-\t"+(String)pin1+"\t-\t"+(String)millis()+" - "+(String)pin1+" - sendmessage LOST MESSAGE (exceed retries)\n"); 
   
    }

    if(readmessage_raw(&loopmessage,&isack))
    {
        bool testbool=0;
        Serial.print((String)millis()+" - "+(String)pin1+" - Received message:"+(String)loopmessage+", message ACK: "+(String)isack+"("+(String)testbool+"), txmessage: "+(String)txmsg1.message.message+", waitingACK: "+(String)txmsg1.waitingforack+"\n");
        if(!isack)
        {   
            //send ACK and call callback
            Serial.print((String)millis()+" - "+(String)pin1+" - Received message:"+(String)loopmessage+", ACKing\n");
            messagestruct ms2;
            ms2.ack=1;
            ms2.message=loopmessage;
            sendmessage_addtobuff(ms2); 
            receivedmessagecallback(loopmessage);
        }
        else if(isack && (loopmessage==txmsg1.message.message) &&txmsg1.waitingforack==1)//readedmessage==ACKMESSAGE)
        {
            Serial.print((String)millis()+" - "+(String)pin1+" - setting waitingforack to 0\n");
            txmsg1.waitingforack=0;
            Serial.print((String)millis()+" - "+(String)pin1+" - Received ACK for sended message: "+(String)loopmessage+"\n");
        }
    }
    else{
        //onewiremessage message;
        if(txmsg1.waitingforack==0)
        {
            messagestruct ms1;
            if(sendmessage_getfrombuff(&ms1))
            {
                //delay(500);
                Serial.print((String)millis()+" - "+(String)pin1+" - sendmessage_getfrombuff READING: "+(String)ms1.message+"\n");
                //Serial.print("");
                sendmessage_r(ms1);
                //Serial.print((String)message+"\t-\t"+(String)pin1+"\t-\t"+(String)millis()+" - "+(String)pin1+" - sendmessage ERROR 2 message "+(String)message+". buff: "+(String)getbuffersize()+"("+(String)buffercounter_high+"/"+(String)buffercounter_low+"), pinstatus: "+(String)getpinstatus()+"\n");
            }
        }
    }
    
    delay(messagesymbolms*8);
    return true;
}