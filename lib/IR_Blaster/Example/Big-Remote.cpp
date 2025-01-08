#include "Arduino.h"
#include "IR_Blaster.h"
#include <LCDWIKI_GUI.h>
#include "SSD1283A.h"
#include "IR_Big_Remote.h"

gpio_num_t IR_Pin = GPIO_NUM_27;
IR_Blaster IR_Blaster_(IR_Pin);

SSD1283A_GUI Display(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*LED=*/ 4); //hardware spi,cs,cd,reset,led

gpio_num_t SelectButton = GPIO_NUM_25;
gpio_num_t UpButton = GPIO_NUM_26;
gpio_num_t DownButton = GPIO_NUM_14;
gpio_num_t BackButton = GPIO_NUM_12;
BigRemote BigRemote_(SelectButton, UpButton, DownButton, BackButton, Display, IR_Blaster_);

void setup(){
    Serial.begin(115200);
    IR_Blaster_.begin();
    Display.init();
    BigRemote_.begin();
}

void loop(){
}