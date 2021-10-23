#include <Arduino.h>
#include "lb.h"

#define LED 2
TaskHandle_t Task1;
TaskHandle_t Task2;
void Task1code(void *pvParameters);
void Task2code(void *pvParameters);

//digitalWrite(pin02, HIGH);
//delay(1000);
//digitalWrite(pin02, LOW);
//delay(1000);

////////////////////////////////////////////////
const int led1 = 2;

const int pin02 = 5;
const int pin01 = 4;

twowire_dr twdr1;
twowire_dr twdr2;

void isr01()
{
  twdr1.isr();
}
void attach01()
{
  attachInterrupt(pin01, isr01, CHANGE);
}

void isr02()
{
  twdr2.isr();
}

void attach02()
{
  attachInterrupt(pin02, isr02, CHANGE);
}

////////////////////////////////////////////////
void receivedmessagecallback1(twowire_dr::onewiremessage message)
{
  uint8_t message8=highByte(message);
  uint8_t cmd=lowByte(message);

    Serial.println("CB1: Received message: "+(String)message+" (cmd:"+(String)cmd+" / message:"+(String)message8+")");  
}
void receivedmessagecallback2(twowire_dr::onewiremessage message)
{
  uint8_t message8=highByte(message);
  uint8_t cmd=lowByte(message);

    Serial.println("CB2: Received message: "+(String)message+" (cmd:"+(String)cmd+" / message:"+(String)message8+")");  
}

void setup()
{
  // Set pin mode
  pinMode(led1, OUTPUT);

  Serial.begin(9600);
  Serial.print("setup() running on core ");
  Serial.println(xPortGetCoreID());

  twdr1.init("twdr1",pin01,attach01,receivedmessagecallback1);
  twdr2.init("twdr2",pin02,attach02,receivedmessagecallback2);

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
      Task1code, /* Task function. */
      "Task1",   /* name of task. */
      10000,     /* Stack size of task */
      NULL,      /* parameter of the task */
      1,         /* priority of the task */
      &Task1,    /* Task handle to keep track of created task */
      0);        /* pin task to core 0 */
  //delay(500);

//create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
      Task2code, /* Task function. */
      "Task2",   /* name of task. */
      10000,     /* Stack size of task */
      NULL,      /* parameter of the task */
      1,         /* priority of the task */
      &Task2,    /* Task handle to keep track of created task */
      1);        /* pin task to core 1 */
  delay(500);
}

void loop()
{
}

int failedmessagecounter1=0;
int failedmessagecounter2=0;

void Task1code(void *pvParameters)
{
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
  
  unsigned long latesttimer=millis();
uint16_t message = 1;
  for (;;)
  {
    
    //delay(200);
    twdr2.loop();
    if(latesttimer+(random(1000)+500)<millis())
    {
      
      if(twdr2.sendmessage(message))
      {
        Serial.print("# Success sending message '"+(String)message+"' from twdr2 to twdr1 failedcounter("+(String)failedmessagecounter2+")\n\n");
      }
      else
      {
        delay(random(500));
        Serial.print("## CAUTION!! LOST MESSAGE '"+(String)message+"' from twdr2 to twdr1 failedcounter("+(String)failedmessagecounter2+")\n\n");
        failedmessagecounter2++;
      }
      message++;
      latesttimer=millis();
    }
    //delay(random(1000));
  }
}

//Task2code: read task
void Task2code(void *pvParameters)
{
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  unsigned long latesttimer=millis();
//uint16_t message = 350;
uint8_t message = 0x33;
  for (;;)
  {
    //delay(200);
    twdr1.loop();
    if(latesttimer+(random(1000)+1000)<millis())
    {
      uint8_t cmd = 0x01;
      
      if(twdr1.sendmessage(cmd, message))
      //if(twdr1.sendmessage(message))
      {
        //Serial.print("# Success sending message '"+(String)message+"' from twdr2 to twdr1 failedcounter("+(String)failedmessagecounter2+")\n");
        Serial.print("## Success sending cmd: '"+(String)cmd+"', message '"+(String)message+"' from twdr1 to twdr2\n\n");
      }
      else
      {
        delay(random(500));
        //Serial.print("## CAUTION!! LOST MESSAGE '"+(String)message+"' from twdr2 to twdr1 failedcounter("+(String)failedmessagecounter2+")\n");
        Serial.print("## CAUTION!! LOST MESSAGE cmd: '"+(String)cmd+"', message '"+(String)message+"' from twdr1 to twdr2\n\n");
        failedmessagecounter1++;
      }
      latesttimer=millis();
      message++;
    }
  }
}
