# S3-sub-project IR-Remote

Dit is een onderdeel van het project snelheidsdetector, de nec-infrarood afstandsbediening.

## Installatie

Eerst de repo clonen.

```bash
git clone git@github.com:Davoto/IR-Remote.git
```

Vervolgens initialiseer je de repo door het te openen als platformio-project in bijvoorbeeld VSCode of CLion. Dit is
nodig om de dependencies te installeren. Er is vervolgens één probleem, een bug in één van de dependencies, namelijk in 
".pio/libdeps/esp32dev/LCDWIKI GUI Library/LCDWIKI_font.c" staat dit op lijn 12:
```c
static const unsigned char lcd_font[] PROGMEM =
```

Dit moet dit zijn:
```c
const unsigned char lcd_font[] PROGMEM =
```

Dit moet omdat platformio/esp-idf een compile-error geeft als "lcd_font[]" static is.

Verder zijn er ook nog 2 lijnen die aangepast moeten worden in "lib/SSD1283A/src/SSD1283A.h"

Op lijn 27 moet dit:
```c++
// #include <LCDWIKI_GUI.h>
```

Dit worden:
```c++
#include <LCDWIKI_GUI.h>
```

En bij lijn 107, is een functie nooit afgemaakt en geeft daardoor een compile-error. Aangezien in dit programma deze
functie niet essentïeel is, los je dit zo op:

Origineel:
```c++
    virtual int16_t Read_GRAM(int16_t x, int16_t y, uint16_t *block, int16_t w, int16_t h) {};
```

Aangepast:
```c++
    virtual int16_t Read_GRAM(int16_t x, int16_t y, uint16_t *block, int16_t w, int16_t h) {return 0;};
```

Als dit gelukt is is het project compile-baar.

## Handige weetjes
De zelfgeschreven code voor dit project is te vinden in "lib/IR_Blaster", hierin zit ook een Example folder met wat test
apps naast Big-Remote.cpp wat de main app van dit project is. In Big-Remote.cpp kun je ook in de code ook lezen welke
pins nodig zijn om dit te recreëren.