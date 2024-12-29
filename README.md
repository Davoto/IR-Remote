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

Als dit gelukt is is het project compile-baar.