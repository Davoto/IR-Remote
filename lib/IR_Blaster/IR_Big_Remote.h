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
    BigRemote(gpio_num_t& ButtonSelect, gpio_num_t& ButtonUp, gpio_num_t& ButtonDown,
              gpio_num_t& ButtonBack, SSD1283A_GUI& Display, IR_Blaster& IR_Blaster_) :
              ButtonSelect(ButtonSelect), ButtonUp(ButtonUp), ButtonDown(ButtonDown), ButtonBack(ButtonBack),
              Display(Display), IR_Blaster_(IR_Blaster_) {};

    void begin(){
        ButtonSelect.begin();
        ButtonUp.begin();
        ButtonDown.begin();
        ButtonBack.begin();
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
    const uint8_t  TaskPriority = 3;

    uint16_t Selection = 0;

    enum colours{
        Black = 0x0000,
        White = 0xffff,
        Grey = 0x6000,
        LightGrey = 0x3000,
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
        Display.Print_String("Welcome to\n  ~ Big ~\n IR-Remote!", 6, 15);

        Display.Set_Text_Size(1);
        Display.Set_Text_Back_colour(Grey);
        Display.Print_String("Press any button", 22, 100);

        for(;;){
            if(anyButtonPressed()) {
                resetAllButtons();
                break;
            }
            vTaskDelay(1);
        }
        ESP_LOGI(TaskName, "Pressed Button on StartScreen");
        Selection = 0;
        State = Options;
    }

    void func_Options(){
        unsigned char OptionChoose[13] = "   Choose:  ";
        unsigned char OptionZapper[13] = "   Zapper   ";
        unsigned char OptionUpdate[13] = "   Update   ";

        Selection = 0;
        for(;;){
            switch(Selection){
                case 0:
                    Display.Fill_Screen(Black);

                    Display.Set_Text_Size(2);
                    Display.Set_Text_colour(White);
                    Display.Set_Text_Back_colour(Black);
                    Display.Print_String(OptionChoose, 1, 0);

                    Display.Set_Text_colour(Red);
                    Display.Set_Text_Back_colour(Grey);
                    Display.Print_String(OptionZapper, 0, 40);

                    Display.Set_Text_colour(White);
                    Display.Set_Text_Back_colour(LightGrey);
                    Display.Print_String(OptionUpdate, 0, 80);

                    for(;;){
                        if(ButtonSelect.GetState()){
                            State = Zapper;
                            resetAllButtons();
                            break;
                        }
                        if(ButtonBack.GetState()){
                            State = StartScreen;
                            resetAllButtons();
                            break;
                        }
                        if(ButtonDown.GetState()){
                            Selection = 1;
                            resetAllButtons();
                            break;
                        }
                        vTaskDelay(100);
                    }
                    break;
                case 1:
                    Display.Fill_Screen(Black);

                    Display.Set_Text_Size(2);
                    Display.Set_Text_colour(White);
                    Display.Set_Text_Back_colour(Black);
                    Display.Print_String(OptionChoose, 1, 0);

                    Display.Set_Text_colour(White);
                    Display.Set_Text_Back_colour(LightGrey);
                    Display.Print_String(OptionZapper, 0, 40);

                    Display.Set_Text_colour(Red);
                    Display.Set_Text_Back_colour(Grey);
                    Display.Print_String(OptionUpdate, 0, 80);

                    for(;;){
                        if(ButtonSelect.GetState()){
                            State = DatabaseUpdater;
                            resetAllButtons();
                            Selection = 0;
                            break;
                        }
                        if(ButtonBack.GetState()){
                            State = StartScreen;
                            resetAllButtons();
                            break;
                        }
                        if(ButtonUp.GetState()){
                            Selection = 0;
                            resetAllButtons();
                            break;
                        }
                        vTaskDelay(100);
                    }
                    break;
            }
            if(State != Options) break;
        }
        vTaskDelay(1);
    }

    void main(){
        Display.setRotation(3);
        for(;;){
            switch (State) {
                case StartScreen:
                    func_StartScreen();
                    break;
                case Options:
                    func_Options();
                    break;
                case DatabaseUpdater:
                    vTaskDelay(100);
                    break;
                case Zapper:
                    vTaskDelay(125);
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
