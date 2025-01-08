#ifndef IR_BLASTER_H
#define IR_BLASTER_H

class IR_Blaster {
public:
    /**
     * Constructor for IR_Blaster Class, acquires the Pin used to light up the IR-Led and initializes the CommandQueue,
     * MessageQueue, AddressQueue and N_BytesQueue.
     * @param Pin used for lighting up an IR-Led.
     */
    explicit IR_Blaster(const uint8_t& Pin) : Pin(Pin), CommandQueue(xQueueCreate(10, sizeof(bool))),
                                              MessageQueue(xQueueCreate(10, sizeof(uint32_t))),
                                              AddressQueue(xQueueCreate(10, sizeof(uint8_t))),
                                              N_BytesQueue(xQueueCreate(10, sizeof(uint8_t))){};


    /**
     * Begin function for setting up the IR-Led for signaling and starting the main task.
     */
    void begin(){
        ledcSetup(PWM_CHANNEL, NEC_KHZ, PWM_RESOLUTION);
        ledcAttachPin(Pin, PWM_CHANNEL);
        xTaskCreate(Static_main, TaskName, TaskDepth, this, TaskPriority, &Task);
    };

    /**
     * Function to send a Message, Address and N_Bytes in the queue's to be send by the IR-Led.
     * @param Message to be send after the address following the NEC-protocol, can range by 1, 2, 3 or 4 bytes.
     * @param Address to be send first after the Startbit following the NEC-protocol, is 8 bits.
     * @param N_Bytes this is to know if you want to sent 1, 2, 3 or 4 bytes.
     */
    void sendMessage(const uint32_t& Message, const uint8_t& Address = 0x00, const uint8_t& N_Bytes = 1) {
        bool Command = true;
        xQueueSend(CommandQueue, &Command, portMAX_DELAY);
        xQueueSend(MessageQueue, &Message, portMAX_DELAY);
        xQueueSend(AddressQueue, &Address, portMAX_DELAY);
        xQueueSend(N_BytesQueue, &N_Bytes, portMAX_DELAY);
    }

    /**
     * Function to send the repeat code used in the NEC-protocol to the queue to be send by the IR-Led.
     */
    void sendRepeatCode(){
        bool Command = false;
        xQueueSend(CommandQueue, &Command, portMAX_DELAY);
    }
private:
    /**
     * Settings used by the IR Blaster task.
     */
    TaskHandle_t Task;
    const char* TaskName = "IR Blaster";
    const uint16_t TaskDepth = 2048;
    const uint8_t  TaskPriority = 1;

    /**
     * Values for proper PWM bursts IR-Led.
     */
    static const uint8_t BURST_ON  = 127;
    static const uint8_t BURST_OFF = 0;

    /**
     * Delays in microseconds for different bits.
     */
    static const uint16_t START_HIGH        = 9000;
    static const uint16_t START_LOW         = 4500;
    static const uint16_t ONE_HIGH          = 560;
    static const uint16_t ONE_LOW           = 1690;
    static const uint16_t ZERO_HIGH         = 560;
    static const uint16_t ZERO_LOW          = 560;
    static const uint16_t REPEAT_FIRST_BIT  = 9000;
    static const uint16_t REPEAT_PAUSE      = 2250;
    static const uint16_t REPEAT_SECOND_BIT = 560;

    /**
     * Standard ledcSetup() settings for NEC-protocol.
     */
    static const uint8_t PWM_CHANNEL       = 0;
    static const uint16_t NEC_KHZ          = 38000;
    static const uint8_t PWM_RESOLUTION    = 8;

    /**
     * Pin used by IR-Led.
     */
    uint8_t Pin;

    /**
     * Queue handles for all queue's used in this class.
     */
    QueueHandle_t CommandQueue;
    QueueHandle_t MessageQueue;
    QueueHandle_t AddressQueue;
    QueueHandle_t N_BytesQueue;

    /**
     * Function to send a byte trough IR, will send first normally and then inverted for verification on the
     * receiver-end.
     * @param Message a byte-size value to be send trough IR.
     */
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

    /**
     * Function to send the start bit used in the NEC-protocol.
     */
    static void sendStartBit() {
        ledcWrite(PWM_CHANNEL, BURST_ON);
        ets_delay_us(START_HIGH);
        ledcWrite(PWM_CHANNEL, BURST_OFF);
        ets_delay_us(START_LOW);
    };

    /**
     * Function to send a "1" using the NEC-protocol.
     */
    static void sendOneBit() {
        ledcWrite(PWM_CHANNEL, BURST_ON);
        ets_delay_us(ONE_HIGH);
        ledcWrite(PWM_CHANNEL, BURST_OFF);
        ets_delay_us(ONE_LOW);
    };

    /**
     * Function to send a "0" using the NEC-protocol.
     */
    static void sendZeroBit() {
        ledcWrite(PWM_CHANNEL, BURST_ON);
        ets_delay_us(ZERO_HIGH);
        ledcWrite(PWM_CHANNEL, BURST_OFF);
        ets_delay_us(ZERO_LOW);
    };

    /**
     * Mainloop for the IR Blaster class, Will wait for a command in the CommandQueue and depending on the command send
     * a message or repeat-code.
     */
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

    /**
     * Function to run a non-static function in a Task, through pointing 🫵.
     */
    static void Static_main(void* arg){
        IR_Blaster* runner = (IR_Blaster*)arg;
        runner->main();
    }
};

#endif