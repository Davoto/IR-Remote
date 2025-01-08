#ifndef IR_BLASTER_H
#define IR_BLASTER_H

class IR_Blaster {
public:
    explicit IR_Blaster(const uint8_t& Pin) : Pin(Pin), CommandQueue(xQueueCreate(10, sizeof(bool))),
                                              MessageQueue(xQueueCreate(10, sizeof(uint32_t))),
                                              AddressQueue(xQueueCreate(10, sizeof(uint8_t))),
                                              N_BytesQueue(xQueueCreate(10, sizeof(uint8_t))){};

    void begin(){
        ledcSetup(PWM_CHANNEL, NEC_KHZ, PWM_RESOLUTION);
        ledcAttachPin(Pin, PWM_CHANNEL);
        xTaskCreate(Static_main, TaskName, TaskDepth, this, TaskPriority, &Task);
    };

    void sendMessage(const uint32_t& Message, const uint8_t& Address = 0x00, const uint8_t& N_Bytes = 1) {
        bool Command = true;
        xQueueSend(CommandQueue, &Command, portMAX_DELAY);
        xQueueSend(MessageQueue, &Message, portMAX_DELAY);
        xQueueSend(AddressQueue, &Address, portMAX_DELAY);
        xQueueSend(N_BytesQueue, &N_Bytes, portMAX_DELAY);
    }

    void sendRepeatCode(){
        bool Command = false;
        xQueueSend(CommandQueue, &Command, portMAX_DELAY);
    }
private:
    TaskHandle_t Task;
    const char* TaskName = "IR Blaster";
    const uint16_t TaskDepth = 2048;
    const uint8_t  TaskPriority = 1;

    // Values for proper PWM bursts
    static const uint8_t BURST_ON  = 127;
    static const uint8_t BURST_OFF = 0;

    // Delays in microseconds for different bits.
    static const uint16_t START_POSITIVE       = 9000;
    static const uint16_t START_NEGATIVE       = 4500;
    static const uint16_t ONE_POSITIVE         = 560;
    static const uint16_t ONE_NEGATIVE         = 1690;
    static const uint16_t ZERO_POSITIVE        = 560;
    static const uint16_t ZERO_NEGATIVE        = 560;
    static const uint16_t REPEAT_FIRST_BIT     = 9000;
    static const uint16_t REPEAT_PAUSE         = 2250;
    static const uint16_t REPEAT_SECOND_BIT    = 560;

    // Standard ledcSetup() settings.
    static const uint8_t PWM_CHANNEL       = 0;
    static const uint16_t NEC_KHZ          = 38000;
    static const uint8_t PWM_RESOLUTION    = 8;

    uint8_t Pin;

    QueueHandle_t CommandQueue;
    QueueHandle_t MessageQueue;
    QueueHandle_t AddressQueue;
    QueueHandle_t N_BytesQueue;

    static void sendByte(const uint8_t& Message) {
        uint8_t PositiveMsg = Message;
        uint8_t NegativeMsg = ~Message;

        // Send positive version of byte.
        for (int j = 0; j < 8; j++) {
            if (PositiveMsg % 2) {
                sendOneBit();
            } else sendZeroBit();

            PositiveMsg >>= 1;
        }

        // Send negative version of byte as check.
        for (int k = 0; k < 8; k++) {
            if (NegativeMsg % 2) {
                sendOneBit();
            } else sendZeroBit();

            NegativeMsg >>= 1;
        }
    }

    static void sendStartBit() {
        ledcWrite(PWM_CHANNEL, BURST_ON);
        ets_delay_us(START_POSITIVE);
        ledcWrite(PWM_CHANNEL, BURST_OFF);
        ets_delay_us(START_NEGATIVE);
    };

    static void sendOneBit() {
        ledcWrite(PWM_CHANNEL, BURST_ON);
        ets_delay_us(ONE_POSITIVE);
        ledcWrite(PWM_CHANNEL, BURST_OFF);
        ets_delay_us(ONE_NEGATIVE);
    };

    static void sendZeroBit() {
        ledcWrite(PWM_CHANNEL, BURST_ON);
        ets_delay_us(ZERO_POSITIVE);
        ledcWrite(PWM_CHANNEL, BURST_OFF);
        ets_delay_us(ZERO_NEGATIVE);
    };

    void main(){
        for(;;){
            bool Command;
            xQueueReceive(CommandQueue, &Command, portMAX_DELAY);
            if(Command){
                uint32_t Message;
                uint8_t Address;
                uint8_t N_Bytes;

                xQueueReceive(MessageQueue, &Message, portMAX_DELAY);
                xQueueReceive(AddressQueue, &Address, portMAX_DELAY);
                xQueueReceive(N_BytesQueue, &N_Bytes, portMAX_DELAY);

                sendStartBit();
                sendByte(Address);

                switch (N_Bytes) {
                    case 1:
                        sendByte(Message);
                        break;
                    case 2:
                        sendByte(Message);
                        sendByte(Message >> 8);
                        break;
                    case 3:
                        sendByte(Message);
                        sendByte(Message >> 8);
                        sendByte(Message >> 16);
                        break;
                    case 4:
                        sendByte(Message);
                        sendByte(Message >> 8);
                        sendByte(Message >> 16);
                        sendByte(Message >> 24);
                        break;
                }

                sendZeroBit();
            }else{
                ledcWrite(PWM_CHANNEL, BURST_ON);
                ets_delay_us(REPEAT_FIRST_BIT);
                ledcWrite(PWM_CHANNEL, BURST_OFF);
                ets_delay_us(REPEAT_PAUSE);
                ledcWrite(PWM_CHANNEL, BURST_ON);
                ets_delay_us(REPEAT_SECOND_BIT);
                ledcWrite(PWM_CHANNEL, BURST_OFF);
            }
        }
    }

    static void Static_main(void* arg){
        IR_Blaster* runner = (IR_Blaster*)arg;
        runner->main();
    }
};

#endif