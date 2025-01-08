#ifndef IR_BIG_REMOTE_H
#define IR_BIG_REMOTE_H

#include "IR_Button_Handler.h"
#include "SSD1283A.h"
#include "IR_Blaster.h"
#include "SPIFFS.h"
#include "ArduinoJson.h"

class BigRemote {
public:
    /**
     * Constructor for Big Remote, loads button handler classes, display class and IR-Blaster class.
     * @param ButtonSelect pin to be used as the selection button.
     * @param ButtonUp pin to be used as the up button.
     * @param ButtonDown pin to be used as the down button.
     * @param ButtonBack pin to be used as the back button.
     * @param Display class to be used to display menu's.
     * @param IR_Blaster_ class to be used to send message's trough IR.
     */
    BigRemote(gpio_num_t &ButtonSelect, gpio_num_t &ButtonUp, gpio_num_t &ButtonDown,
              gpio_num_t &ButtonBack, SSD1283A_GUI &Display, IR_Blaster &IR_Blaster_) :
            ButtonSelect(ButtonSelect), ButtonUp(ButtonUp), ButtonDown(ButtonDown), ButtonBack(ButtonBack),
            Display(Display), IR_Blaster_(IR_Blaster_) {};

    /**
     * Function to initialize Button Handlers used in this class, set Display rotation and to start
     * the main task.
     */
    void begin() {
        ButtonSelect.begin();
        ButtonUp.begin();
        ButtonDown.begin();
        ButtonBack.begin();
        Display.setRotation(3);
        xTaskCreate(Static_main, TaskName, TaskDepth, this, TaskPriority, &Task);
    };

    /**
     * Function to Stop the button handler tasks and main task.
     */
    void stop() {
        ButtonSelect.stop();
        ButtonUp.stop();
        ButtonDown.stop();
        ButtonBack.stop();
        vTaskDelete(&Task);
    };
private:
    /**
     * Classes used within this class.
     */
    IR_Button_Handler ButtonSelect, ButtonUp, ButtonDown, ButtonBack;
    SSD1283A_GUI Display;
    IR_Blaster IR_Blaster_;

    /**
     * Options used by this class's task.
     */
    TaskHandle_t Task;
    const char *TaskName = "BigRemote";
    const uint16_t TaskDepth = 16384;
    const uint8_t TaskPriority = 3;

    /**
     * Selection values to remember placement within different states of the main task.
     */
    uint16_t Selection = 0;
    uint16_t FolderSelected = 0;

    /**
     * Enum used by setText function to require colours within this enum.
     */
    enum colours {
        Black = 0x0000,
        White = 0xffff,
        Grey = 0x6000,
        LightGrey = 0x3000,
        Red = 0xf800
    };

    /**
     * States used within the main task.
     */
    enum state {
        StartScreen,
        MainMenu,
        DatabaseUpdater,
        ZapperFolder,
        ZapperBlast
    };
    state State = StartScreen;

    /**
     * Function to check if any button is pressed.
     * @return true when any button is pressed otherwise false.
     */
    bool anyButtonPressed() {
        return ButtonSelect.GetState() || ButtonUp.GetState() || ButtonDown.GetState() || ButtonBack.GetState();
    }

    /**
     * Function to reset all button's states.
     */
    void resetAllButtons() {
        ButtonSelect.ResetButton();
        ButtonUp.ResetButton();
        ButtonDown.ResetButton();
        ButtonBack.ResetButton();
    }

    /**
     * Function to set display text size and colour.
     * @param Size
     * @param ColourText
     * @param ColourBack
     */
    void setText(const uint8_t &Size, const colours &ColourText, const colours &ColourBack) {
        Display.Set_Text_Size(Size);
        Display.Set_Text_colour(ColourText);
        Display.Set_Text_Back_colour(ColourBack);
    }

    /**
     * Function to render startscreen, proceeds to the wait in own loop for anyButtonPressed() to proceed to state
     * Menu.
     */
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
            vTaskDelay(100);
        }
        ESP_LOGI(TaskName, "Pressed Button on StartScreen");
        Selection = 0;
        State = MainMenu;
    }

    /**
     * Function renders the main menu with two options, Zapper and Update. Zapper goes to the ZapperFolder State and
     * Update goes to the DatabaseUpdate state. You can select these options by using the up and down buttons in
     * combination with the select button to proceed to these states, the back button is used to return to the
     * StartScreen state.
     */
    void func_MainMenu() {
        unsigned char OptionChoose[13] = "   Choose:  ";
        unsigned char OptionZapper[13] = "   Zapper   ";
        unsigned char OptionUpdate[13] = "   Update   ";

        Display.Fill_Screen(Black);

        setText(2, White, Black);
        Display.Print_String(OptionChoose, 0, 0);

        Selection = 0;
        for(;;){
            if(Selection){
                setText(2, White, LightGrey);
                Display.Print_String(OptionZapper, 0, 40);

                setText(2, Red, Grey);
                Display.Print_String(OptionUpdate, 0, 80);
            }else{
                setText(2, Red, Grey);
                Display.Print_String(OptionZapper, 0, 40);

                setText(2, White, LightGrey);
                Display.Print_String(OptionUpdate, 0, 80);
            }

            for (;;) {
                vTaskDelay(100);

                if (ButtonSelect.GetState()) {
                    ESP_LOGI(TaskName, "Pressed button for Zapper");
                    if(Selection){
                        State = DatabaseUpdater;
                    }else State = ZapperFolder;
                    resetAllButtons();
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

                if (ButtonDown.GetState()) {
                    ESP_LOGI(TaskName, "Pressed button to change selection");
                    Selection = 1;
                    resetAllButtons();
                    break;
                }
            }
            if(State != MainMenu) break;
        }
    }
    
    void func_DatabaseUpdater() {
        JsonDocument DatabaseJson;

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
            // uint32_t databasePercentage = databaseSize / 1200; uncommented because use is not working currently.

            ESP_LOGI(TaskName, "Current filesize in bytes (Max possible 120000~): %d", databaseSize);
            database.close();

            setText(2, White, LightGrey);
            Display.Print_String(LowerText, 0, 80);
            // Display.Print_String(String(databasePercentage), databasePercentage < 10 ? 72 : 60, 112); Not working for unknown reason.


            deserializeJson(DatabaseJson, Serial);
            if (!DatabaseJson.isNull()) {
                database = SPIFFS.open("/database.json", FILE_WRITE);
                serializeJson(DatabaseJson, database);
                database.close();

                setText(2, Red, Grey);
                Display.Print_String(NewDatabase, 0, 48);
            }

            vTaskDelay(100);

            if (ButtonBack.GetState()) {
                ESP_LOGI(TaskName, "Pressed button to go back");
                State = MainMenu;
                resetAllButtons();
                break;
            }
        }
    }

    void func_ZapperFolder() {
        unsigned char Title[] = "  Folders  ";

        Selection = FolderSelected;
        JsonDocument DatabaseJson;
        File database = SPIFFS.open("/database.json", FILE_READ);
        deserializeJson(DatabaseJson, database);
        database.close();

        uint16_t MaxSize = DatabaseJson.size();

        Display.Fill_Screen(Black);
        setText(2, White, Black);
        Display.Print_String(Title, 0, 0);

        bool Changed = true;
        for (;;) {
            if (Changed) {
                Changed = false;
                for (uint16_t i = 0; i < MaxSize; i++) {
                    if (i == Selection) {
                        setText(2, Red, Grey);
                    } else setText(2, White, LightGrey);
                    int16_t y = i * 16 + 16;
                    Display.Print_String(DatabaseJson[i][0]["subject"].as<String>(), 0, y);
                    Serial.println(DatabaseJson[i][0]["subject"].as<String>());
                }
            }

            vTaskDelay(100);

            if (ButtonSelect.GetState()) {
                ESP_LOGI(TaskName, "Pressed button to proceed to blast");
                resetAllButtons();
                State = ZapperBlast;
                break;
            }

            if (ButtonUp.GetState()) {
                ESP_LOGI(TaskName, "Pressed button up");
                resetAllButtons();
                if (Selection > 0) {
                    Changed = true;
                    Selection--;
                }
            }

            if (ButtonDown.GetState()) {
                ESP_LOGI(TaskName, "Pressed button down");
                resetAllButtons();
                if (Selection < MaxSize-1) {
                    Changed = true;
                    Selection++;
                }
            }

            if (ButtonBack.GetState()) {
                ESP_LOGI(TaskName, "Pressed button to go back");
                State = MainMenu;
                resetAllButtons();
                break;
            }

        }
    }

    void func_ZapperBlast(){
        unsigned char Title[] = "    Zaps   ";

        FolderSelected = Selection;
        Selection = 1;
        JsonDocument DatabaseJson;
        File database = SPIFFS.open("/database.json", FILE_READ);
        deserializeJson(DatabaseJson, database);
        database.close();

        uint16_t MaxSize = DatabaseJson[FolderSelected].size();

        Display.Fill_Screen(Black);
        setText(2, White, Black);
        Display.Print_String(Title, 0, 0);

        bool Changed = true;
        for (;;) {
            if (Changed) {
                Changed = false;
                for (int i = 1; i < MaxSize; i++) {
                    if (i == Selection) {
                        setText(2, Red, Grey);
                    } else setText(2, White, LightGrey);
                    int16_t y = i * 16;
                    Display.Print_String(DatabaseJson[FolderSelected][i]["name"].as<String>(), 0, y);
                    Serial.println(DatabaseJson[FolderSelected][i]["name"].as<String>());
                }
            }

            vTaskDelay(100);

            if (ButtonSelect.GetState()) {
                ESP_LOGI(TaskName, "Pressed button to go back");
                resetAllButtons();
                IR_Blaster_.sendMessage(DatabaseJson[FolderSelected][Selection]["id"],
                                        DatabaseJson[FolderSelected][0]["address"],
                                        DatabaseJson[FolderSelected][Selection]["n_bytes"]);
                Display.Fill_Screen(White);
                vTaskDelay(100);
                Display.Fill_Screen(Black);
                setText(2, White, Black);
                Display.Print_String(Title, 0, 0);
                Changed = true;
            }

            if (ButtonUp.GetState()) {
                ESP_LOGI(TaskName, "Pressed button up");
                resetAllButtons();
                if (Selection > 1) {
                    Changed = true;
                    Selection--;
                }
            }

            if (ButtonDown.GetState()) {
                ESP_LOGI(TaskName, "Pressed button down");
                resetAllButtons();
                if (Selection < MaxSize-1) {
                    Changed = true;
                    Selection++;
                }
            }

            if (ButtonBack.GetState()) {
                ESP_LOGI(TaskName, "Pressed button to go back");
                State = ZapperFolder;
                resetAllButtons();
                break;
            }
        }
    }

    /**
     * Main task initializes SPIFFS path, switches between 5 states see the functions per state for further explanation
     * of main task.
     */
    void main() {
        if (!SPIFFS.begin(true, "/minidb")) ESP_LOGE(TaskName, "Spiffs failed to mount.");
        for (;;) {
            switch (State) {
                case StartScreen:
                    func_StartScreen();
                    break;
                case MainMenu:
                    func_MainMenu();
                    break;
                case DatabaseUpdater:
                    func_DatabaseUpdater();
                    break;
                case ZapperFolder:
                    func_ZapperFolder();
                    break;
                case ZapperBlast:
                    func_ZapperBlast();
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