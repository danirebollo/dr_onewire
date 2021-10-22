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
void setup()
{
  // Set pin mode
  pinMode(led1, OUTPUT);

  Serial.begin(9600);
  Serial.print("setup() running on core ");
  Serial.println(xPortGetCoreID());

  twdr1.init("twdr1",pin01,attach01);
  twdr2.init("twdr2",pin02,attach02);

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
      Task1code, /* Task function. */
      "Task1",   /* name of task. */
      10000,     /* Stack size of task */
      NULL,      /* parameter of the task */
      1,         /* priority of the task */
      &Task1,    /* Task handle to keep track of created task */
      0);        /* pin task to core 0 */
  delay(500);

//create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
      Task2code, /* Task function. */
      "Task2",   /* name of task. */
      10000,     /* Stack size of task */
      NULL,      /* parameter of the task */
      1,         /* priority of the task */
      &Task2,    /* Task handle to keep track of created task */
      0);        /* pin task to core 1 */
  delay(500);
}

void loop()
{
}

void Task1code(void *pvParameters)
{
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
  
  unsigned long latesttimer=millis();

  for (;;)
  {
    uint8_t message = 129;
    twdr2.sendmessage(message);
    //twdr2.readmessage();
    delay(random(5)*1000+1000);
    delay(500);
    delay(200);
    //twdr2.readmessage();
    while(!twdr2.readmessage())
    {

    }
  }
}

//Task2code: read task
void Task2code(void *pvParameters)
{
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  unsigned long latesttimer=millis();
  for (;;)
  {
    uint8_t message = 111;
    //twdr1.sendmessage(message);
    //twdr1.readmessage();

    if(twdr1.readmessage())
    {
      twdr1.sendmessage(message);
    }
    delay(500);
    delay(200);
  }
}
