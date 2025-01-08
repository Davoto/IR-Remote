#ifndef IR_BUTTON_HANDLER_H
#define IR_BUTTON_HANDLER_H

#include "Arduino.h"

class IR_Button_Handler{
public:
    /**
     * Constructor for the IR_Button_Handler class.
     * @param Button a gpio pin to be used as button.
     */
    explicit IR_Button_Handler(const gpio_num_t& Button) : Button(Button) {}

    /**
     * Function to setup the pin used by this class and begin the class task.
     */
    void begin(){
        pinMode(Button, INPUT_PULLUP);
        xTaskCreate(Static_ButtonTask, TaskName, TaskDepth, this, TaskPriority, &Task);
    }

    /**
     * Function to stop the main task.
     */
    void stop(){
        vTaskDelete(&Task);
    }

    /**
     * Function to get the current state for the variable ButtonPressed.
     * @return ButtonPressed's value.
     */
    bool GetState(){
        return ButtonPressed;
    }

    /**
     * Function to reset the state of the button, to be used after reading a "true" state for GetState().
     */
    void ResetButton(){
        ButtonPressed = false;
    }
private:
    /**
     * Setting's used by this class's task.
     */
    TaskHandle_t Task;
    const char* TaskName = "BigRemote";
    const uint16_t TaskDepth = 1024;
    const uint8_t  TaskPriority = 1;

    /**
     * Gpio-pin used by button and state of the button.
     */
    gpio_num_t Button;
    volatile bool ButtonPressed = false;

    /**
     * Task to check if button is pressed, when button is pressed delay is extended to avoid accidental double presses
     * and to artificially debounce the button.
     */
    void ButtonTask(){
        for(;;){
            if(!digitalRead(Button)) {
                ButtonPressed = true;
                vTaskDelay(200);
            }
            vTaskDelay(100);
        }
    }

    /**
     * Function to run a non-static function in a Task, through pointing 🫵.
     */
    static void Static_ButtonTask(void* arg){
        IR_Button_Handler* runner = (IR_Button_Handler*)arg;
        runner->ButtonTask();
    }
};

#endif //IR_BUTTON_HANDLER_H