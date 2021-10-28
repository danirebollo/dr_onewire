# Introduction #
This is a bidirectional one wire interface between microcontrollers (ESP32 currently supported). It is not the best way to interconnect, I preffer I2C, SPI, CAN or other robust protocols, but I needed to use one wire sometimes so I did this. <br>
 <br>
I'm improving this over the time so please put some issue on github if you found some bug. <br>

# Example details #
-In github example sender and receiver is done by different core using FREERTOS tasks pinned to cores. <br>
-To use this library with another microcontroller only is need to change the first lines (HAL) of the dr_onewire.cpp file. <br>

# Characteristics #
-Manchester encoding with sync, parity and ack <br>
-Send and receive 16B value or 2x 8B value (command/message) <br>
-Each reception is cheched with parity bit <br>
-Each send is acknowledged by receiver. Otherwise throws an error. <br>
-Simple anti colision mechanism based on time is implemented. If some part detects bussy bus or has stored messages wait before try to send. <br>
-Configurable sender symbol time (dr_onewire.h:: const int messagesymbolms = 30;)<br>
-Receiver symbolrate autodetect. <br>
<br>
# Notes #
-This library uses interrupts, not RMT <br>
-I need to use the standard library folder structure <br>
