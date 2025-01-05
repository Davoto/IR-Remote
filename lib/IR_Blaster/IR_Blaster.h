#ifndef IR_BLASTER_H
#define IR_BLASTER_H

class IR_Blaster {
public:
    explicit IR_Blaster(const uint8_t& Pin) : Pin(Pin) {};

    void begin() const {
        ledcSetup(PWM_CHANNEL, NEC_KHZ, PWM_RESOLUTION);
        ledcAttachPin(Pin, PWM_CHANNEL);
    };

    void sendMessage(uint32_t Message, uint8_t Adress = 0x00, uint8_t N_Bytes = 1) {
        sendStartBit();
        sendByte(Adress);

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
    }

    void sendRepeatCode(){
        ledcWrite(PWM_CHANNEL, BURST_ON);
        ets_delay_us(REPEAT_FIRST_BIT);
        ledcWrite(PWM_CHANNEL, BURST_OFF);
        ets_delay_us(REPEAT_PAUSE);
        ledcWrite(PWM_CHANNEL, BURST_ON);
        ets_delay_us(REPEAT_SECOND_BIT);
        ledcWrite(PWM_CHANNEL, BURST_OFF);
    }
private:
    // Values for proper PWM bursts
    static const uint8_t BURST_ON  = 50;
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
};

#endif