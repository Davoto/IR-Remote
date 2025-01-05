#ifndef IR_BIG_REMOTE_H
#define IR_BIG_REMOTE_H

#include "IR_Button_Handler.h"
#include <LCDWIKI_GUI.h>
#include "SSD1283A.h"
#include "IR_Blaster.h"
#include "SPIFFS.h"
#include "ArduinoJson.h"

class BigRemote{
public:
    BigRemote(IR_Button_Handler& ButtonSelect, IR_Button_Handler& ButtonUp, IR_Button_Handler& ButtonDown,
              IR_Button_Handler& ButtonBack, SSD1283A_GUI& Display, IR_Blaster& IR_Blaster_) :
              ButtonSelect(ButtonSelect), ButtonUp(ButtonUp), ButtonDown(ButtonDown), ButtonBack(ButtonBack),
              Display(Display), IR_Blaster_(IR_Blaster_) {};

    void begin(){
        xTaskCreate(Static_main, TaskName, TaskDepth, this, TaskPriority, &Task);
    };

    void stop(){
        vTaskDelete(&Task);
    };
private:
    IR_Button_Handler ButtonSelect, ButtonUp, ButtonDown, ButtonBack;
    SSD1283A_GUI Display;
    IR_Blaster IR_Blaster_;
    JsonDocument Database;

    TaskHandle_t Task;
    const char* TaskName = "BigRemote";
    const uint16_t TaskDepth = 16384;
    const uint8_t  TaskPriority = 2;

    enum colours{
        Black = 0x0000,
        White = 0xffff,
        Grey = 0x6000,
        Red = 0xf800
    };

    enum state{
        StartScreen,
        Options,
        DatabaseUpdater,
        Zapper,
    };
    state State = StartScreen;

    bool anyButtonPressed(){
        return ButtonSelect.GetState() || ButtonUp.GetState() || ButtonDown.GetState() || ButtonBack.GetState();
    }

    void resetAllButtons(){
        ButtonSelect.ResetButton();
        ButtonUp.ResetButton();
        ButtonDown.ResetButton();
        ButtonBack.ResetButton();
    }

    void func_StartScreen(){
        Display.Fill_Screen(Black);
        Display.Set_Text_Size(2);
        Display.Set_Text_colour(Red);
        Display.Set_Text_Back_colour(Black);
        Display.Print_String("Welcome To\nBig\nIR-Remote!", 0, 0);
        while(!ButtonSelect.GetState()){
            Serial.print(".");
            vTaskDelay(100);
        }
        ESP_LOGI(TaskName, "Pressed Button on StartScreen");
        resetAllButtons();
    }

    void main(){
        Display.setRotation(3);
        for(;;){
            switch (State) {
                case StartScreen:
                    func_StartScreen();
                    break;
                case Options:
                    break;
                case DatabaseUpdater:
                    break;
                case Zapper:
                    break;
            }
            vTaskDelay(1000);
        }
    }

    static void Static_main(void* arg){
        BigRemote* runner = (BigRemote*)arg;
        runner->main();
    }
};

#endif //IR_BIG_REMOTE_H
