#include <Arduino.h>
#include "lb.h"

#define LED 2
TaskHandle_t Task1;
TaskHandle_t Task2;
void Task1code(void *pvParameters);
void Task2code(void *pvParameters);

//digitalWrite(pin1, HIGH);
//delay(1000);
//digitalWrite(pin1, LOW);
//delay(1000);

////////////////////////////////////////////////
const int led1 = 2;
const int pin1 = 5;
const int pin2 = 18;

////////////////////////////////////////////////
void setup()
{
  // Set pin mode
  pinMode(led1, OUTPUT);
  pinMode(pin1, OUTPUT_OPEN_DRAIN);
  pinMode(pin2, INPUT_PULLUP);

  Serial.begin(9600);
  Serial.print("setup() running on core ");
  Serial.println(xPortGetCoreID());

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
      1);        /* pin task to core 1 */
  delay(500);
  attachInterrupt(pin2, isr, CHANGE);
}

void loop()
{
}

void Task1code(void *pvParameters)
{
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;)
  {
    uint8_t message = 129;
    sendmessage(message);
    delay(5000);
  }
}

//Task2code: read task
void Task2code(void *pvParameters)
{
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;)
  {
    readmessage();
  }
}
