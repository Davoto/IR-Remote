#ifndef IR_BUTTON_HANDLER_H
#define IR_BUTTON_HANDLER_H

#include "Arduino.h"

class IR_Button_Handler{
public:
    explicit IR_Button_Handler(const gpio_num_t& Button) : Button(Button) {}

    void begin(){
        pinMode(Button, INPUT_PULLUP);
        xTaskCreate(Static_ButtonTask, TaskName, TaskDepth, this, TaskPriority, &Task);
    }

    bool GetState(){
        return ButtonPressed;
    }

    void ResetButton(){
        ButtonPressed = false;
    }

private:
    gpio_num_t Button;
    volatile bool ButtonPressed = false;

    TaskHandle_t Task;
    const char* TaskName = "BigRemote";
    const uint16_t TaskDepth = 1024;
    const uint8_t  TaskPriority = 1;

    static void Static_ButtonTask(void* arg){
        IR_Button_Handler* runner = (IR_Button_Handler*)arg;
        runner->ButtonTask();
    }

    void ButtonTask(){
        for(;;){
            if(!digitalRead(Button)) ButtonPressed = true;
            vTaskDelay(100);
        }
    }
};

#endif //IR_BUTTON_HANDLER_H