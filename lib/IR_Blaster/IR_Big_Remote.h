#ifndef IR_BIG_REMOTE_H
#define IR_BIG_REMOTE_H

#include "IR_Button_Handler.h"
#include <LCDWIKI_GUI.h>
#include "SSD1283A.h"
#include "IR_Blaster.h"
#include "SPIFFS.h"
#include "ArduinoJson.h"

class BigRemote {
public:
    BigRemote(gpio_num_t &ButtonSelect, gpio_num_t &ButtonUp, gpio_num_t &ButtonDown,
              gpio_num_t &ButtonBack, SSD1283A_GUI &Display, IR_Blaster &IR_Blaster_) :
            ButtonSelect(ButtonSelect), ButtonUp(ButtonUp), ButtonDown(ButtonDown), ButtonBack(ButtonBack),
            Display(Display), IR_Blaster_(IR_Blaster_) {};

    void begin() {
        ButtonSelect.begin();
        ButtonUp.begin();
        ButtonDown.begin();
        ButtonBack.begin();
        xTaskCreate(Static_main, TaskName, TaskDepth, this, TaskPriority, &Task);
    };

    void stop() {
        vTaskDelete(&Task);
    };
private:
    IR_Button_Handler ButtonSelect, ButtonUp, ButtonDown, ButtonBack;
    SSD1283A_GUI Display;
    IR_Blaster IR_Blaster_;
    JsonDocument DatabaseJson;

    TaskHandle_t Task;
    const char *TaskName = "BigRemote";
    const uint16_t TaskDepth = 16384;
    const uint8_t TaskPriority = 3;

    uint16_t Selection = 0;

    enum colours {
        Black = 0x0000,
        White = 0xffff,
        Grey = 0x6000,
        LightGrey = 0x3000,
        Red = 0xf800
    };

    enum state {
        StartScreen,
        Options,
        DatabaseUpdater,
        Zapper,
    };
    state State = StartScreen;

    bool anyButtonPressed() {
        return ButtonSelect.GetState() || ButtonUp.GetState() || ButtonDown.GetState() || ButtonBack.GetState();
    }

    void resetAllButtons() {
        ButtonSelect.ResetButton();
        ButtonUp.ResetButton();
        ButtonDown.ResetButton();
        ButtonBack.ResetButton();
    }

    void setText(const uint8_t &Size, const colours &ColourText, const colours &ColourBack) {
        Display.Set_Text_Size(Size);
        Display.Set_Text_colour(ColourText);
        Display.Set_Text_Back_colour(ColourBack);
    }

    void func_StartScreen() {
        unsigned char Title[] = "Welcome to\n  ~ Big ~\n IR-Remote!";
        unsigned char Subtitle[] = "Press any button";

        Display.Fill_Screen(Black);

        setText(2, Red, Black);
        Display.Print_String(Title, 6, 15);

        setText(1, Red, Grey);
        Display.Print_String(Subtitle, 22, 100);

        for (;;) {
            if (anyButtonPressed()) {
                resetAllButtons();
                break;
            }
            vTaskDelay(1);
        }
        ESP_LOGI(TaskName, "Pressed Button on StartScreen");
        Selection = 0;
        State = Options;
    }

    void func_Options() {
        unsigned char OptionChoose[13] = "   Choose:  ";
        unsigned char OptionZapper[13] = "   Zapper   ";
        unsigned char OptionUpdate[13] = "   Update   ";

        Selection = 0;
        for (;;) {
            switch (Selection) {
                case 0:
                    Display.Fill_Screen(Black);

                    setText(2, White, Black);
                    Display.Print_String(OptionChoose, 0, 0);

                    setText(2, Red, Grey);
                    Display.Print_String(OptionZapper, 0, 40);

                    setText(2, White, LightGrey);
                    Display.Print_String(OptionUpdate, 0, 80);

                    for (;;) {
                        if (ButtonSelect.GetState()) {
                            ESP_LOGI(TaskName, "Pressed button for Zapper");
                            State = Zapper;
                            resetAllButtons();
                            break;
                        }
                        if (ButtonBack.GetState()) {
                            ESP_LOGI(TaskName, "Pressed button to go back");
                            State = StartScreen;
                            resetAllButtons();
                            break;
                        }
                        if (ButtonDown.GetState()) {
                            ESP_LOGI(TaskName, "Pressed button to change selection");
                            Selection = 1;
                            resetAllButtons();
                            break;
                        }
                        vTaskDelay(100);
                    }
                    break;
                case 1:
                    Display.Fill_Screen(Black);

                    setText(2, White, Black);
                    Display.Print_String(OptionChoose, 0, 0);

                    setText(2, White, LightGrey);
                    Display.Print_String(OptionZapper, 0, 40);

                    setText(2, Red, Grey);
                    Display.Print_String(OptionUpdate, 0, 80);

                    for (;;) {
                        if (ButtonSelect.GetState()) {
                            ESP_LOGI(TaskName, "Pressed button for database updater");
                            State = DatabaseUpdater;
                            resetAllButtons();
                            Selection = 0;
                            break;
                        }
                        if (ButtonBack.GetState()) {
                            ESP_LOGI(TaskName, "Pressed button to go back");
                            State = StartScreen;
                            resetAllButtons();
                            break;
                        }
                        if (ButtonUp.GetState()) {
                            ESP_LOGI(TaskName, "Pressed button to change selection");
                            Selection = 0;
                            resetAllButtons();
                            break;
                        }
                        vTaskDelay(100);
                    }
                    break;
                default:
                    Selection = 0;
                    break;
            }
            if (State != Options) break;
        }
        vTaskDelay(1);
    }

    void func_DatabaseUpdater() {
        unsigned char TitleBar[24] = " Database- \n  updater  ";
        unsigned char LowerText[36] = "  Storage  \n   used    \n       %   ";
        unsigned char NewDatabase[12] = "  Updated! ";

        Display.Fill_Screen(Black);

        setText(2, White, Black);
        Display.Print_String(TitleBar, 0, 0);

        for (;;) {
            File database = SPIFFS.open("/database.json", FILE_READ);
            if (!database) ESP_LOGE(TaskName, "File failed to open.");

            uint32_t databaseSize = database.size();
            uint32_t databasePercentage = databaseSize / 1200;

            ESP_LOGI(TaskName, "Current filesize in bytes (Max possible 120000~): %d", databaseSize);
            database.close();

            setText(2, White, LightGrey);
            Display.Print_String(LowerText, 0, 80);
            Display.Print_String(String(databasePercentage), databasePercentage < 10 ? 72 : 60, 112);


            deserializeJson(DatabaseJson, Serial);
            if (!DatabaseJson.isNull()) {
                database = SPIFFS.open("/database.json", FILE_WRITE);
                serializeJson(DatabaseJson, database);
                database.close();

                setText(2, Red, Grey);
                Display.Print_String(NewDatabase, 0, 48);
            }

            if (ButtonBack.GetState()) {
                ESP_LOGI(TaskName, "Pressed button to go back");
                State = Options;
                resetAllButtons();
                break;
            }
            vTaskDelay(100);
        }
    }

    void main() {
        if (!SPIFFS.begin(true, "/minidb")) ESP_LOGE(TaskName, "Spiffs failed to mount.");
        Display.setRotation(3);
        for (;;) {
            switch (State) {
                case StartScreen:
                    func_StartScreen();
                    break;
                case Options:
                    func_Options();
                    break;
                case DatabaseUpdater:
                    func_DatabaseUpdater();
                    break;
                case Zapper:
                    vTaskDelay(125);
                    break;
            }
        }
    }

    static void Static_main(void *arg) {
        BigRemote *runner = (BigRemote *) arg;
        runner->main();
    }
};

#endif //IR_BIG_REMOTE_H
