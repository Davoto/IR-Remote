#include <Arduino.h>
#include "Big-Remote.cpp"
// #include "Button-To-Signal.cpp"

extern "C" {
	void app_main();
}

void app_main(void){
	setup();
	while(true){
		loop();
		vTaskDelay(1);  // prevent the watchdog timer to kick in for this thread.
	}
}