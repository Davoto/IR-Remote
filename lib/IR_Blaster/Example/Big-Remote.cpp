#include "Arduino.h"
#include "IR_Blaster.h"
#include <LCDWIKI_GUI.h>
#include "SSD1283A.h"
#include "IR_Big_Remote.h"
#include "IR_Button_Handler.h"

gpio_num_t IR_Pin = GPIO_NUM_27;
IR_Blaster IR_Blaster_(IR_Pin);

SSD1283A_GUI Display(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*LED=*/ 4); //hardware spi,cs,cd,reset,led

IR_Button_Handler SelectButton(GPIO_NUM_25);
IR_Button_Handler UpButton(GPIO_NUM_26);
IR_Button_Handler DownButton(GPIO_NUM_14);
IR_Button_Handler BackButton(GPIO_NUM_12);
BigRemote BigRemote_(SelectButton, UpButton, DownButton, BackButton, Display, IR_Blaster_);

void setup(){
    Serial.begin(115200);
    IR_Blaster_.begin();
    Display.init();
    SelectButton.begin();
    UpButton.begin();
    DownButton.begin();
    BackButton.begin();
    BigRemote_.begin();
}

void loop(){
}