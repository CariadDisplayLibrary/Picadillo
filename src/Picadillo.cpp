
// We can only compile this code if we have both a
// DMA system and the Parallel Master Port.
#ifdef _BOARD_PICADILLO_35T_
#include <Picadillo.h>

void Picadillo::writeCommand(uint16_t c) {
    while (PMMODEbits.BUSY == 1);
    PMADDR = 0x0000;
    PMDIN = c;
}

void Picadillo::writeData(uint16_t c) {
    while (PMMODEbits.BUSY == 1);
    PMADDR = 0x0001;
    PMDIN = c;
}

//==============================================================
// Initialise HX8357 LCD Driver IC
//==============================================================
void Picadillo::initializeDevice() 
{
    PMCONbits.ON = 0;
    asm volatile("nop");

    PMCONbits.PTWREN = 1;
    PMCONbits.PTRDEN = 1;
    PMCONbits.CSF = 0;

    PMAEN = 0x0001; // Enable PMA0 pin for RS pin and CS1 as CS

    PMMODEbits.MODE16 = 1;
    PMMODEbits.MODE = 0b10;
    PMMODEbits.WAITB = 0;
    PMMODEbits.WAITM = 0;
    PMMODEbits.WAITE = 0;

    //PMADDR = 0; // Set current address to 0
    PMADDR = 0x0000; // Set current address to 0, CS1 Active

    PMCONbits.ON = 1;

    identifyDisplay();

    (this->*_initializeDevice)();
}


void Picadillo::setAddrWindow(int x0, int y0, int x1, int y1) 
{
    (this->*_setAddrWindow)(x0, y0, x1, y1);
}

void Picadillo::setPixel(int x, int y, color_t color) 
{
	if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) 
		return;
    if ((x < _clip_x0) || (x > _clip_x1) || (y < _clip_y0) || (y > _clip_y1)) 
        return;
	setAddrWindow(x,y,x+1,y+1);
    PMADDR = 0x0001;
    PMDIN = color;
}

void Picadillo::fillScreen(color_t color) 
{
	fillRectangle(0, 0,  _width, _height, color);
}

void Picadillo::fillRectangle(int x, int y, int w, int h, color_t color) 
{
    if (!clipToScreen(x, y, w, h)) {
        return;
    }
	setAddrWindow(x, y, x+w-1, y+h-1);

    PMADDR = 0x0001;
	for(y=h; y>0; y--) {
		for(x=w; x>0; x--) {
            PMDIN = color;
		}
	}
}

void Picadillo::drawHorizontalLine(int x, int y, int w, color_t color) 
{
    int h = 1;
    if (!clipToScreen(x, y, w, h)) {
        return;
    }
	setAddrWindow(x, y, x+w-1, y);

    PMADDR = 0x0001;
	while (w--) {
		PMDIN = color;
	}
}

void Picadillo::drawVerticalLine(int x, int y, int h, color_t color) 
{
    int w = 1;
    if (!clipToScreen(x, y, w, h)) {
        return;
    }
	setAddrWindow(x, y, x, y+h-1);

    PMADDR = 0x0001;
	while (h--) {
        PMDIN = color;
	}
}

void Picadillo::setRotation(int m) {
    (this->*_setRotation)(m);
}

void Picadillo::invertDisplay(bool i) {
    (this->*_invertDisplay)(i);
}

void Picadillo::displayOn() {
    (this->*_displayOn)();
}

void Picadillo::displayOff() {
	(this->*_displayOff)();
}

void Picadillo::openWindow(int x0, int y0, int x1, int y1) {
    setAddrWindow(x0, y0, x0 + x1 - 1, y0 + y1 - 1);
    PMADDR = 0x0001;
}

void Picadillo::windowData(color_t d) {
    PMDIN = d;
}

void Picadillo::windowData(color_t *d, int l) {
//    for (uint32_t i = 0; i < l; i++) {
//        while (PMMODEbits.BUSY == 1);
//        PMDIN = d[i];
//    }
//    return;
    uint32_t toXfer = l * 2;

    uint16_t *data = d;

    while (toXfer > 0) {
        while (DCH3CONbits.CHBUSY);
        uint32_t chunk = toXfer;
        if (chunk > 65534) {
            chunk = 65534;
        }

        toXfer -= chunk;

        PMMODEbits.IRQM = 0b01;

        DCH3INTbits.CHSDIE = 1;
        DCH3SSA = ((uint32_t)data) & 0x1FFFFFFF;
        DCH3DSA = ((uint32_t)&PMDIN) & 0x1FFFFFFF;
        DCH3SSIZ = chunk;
        DCH3DSIZ = 2;
        DCH3CSIZ = 2;
        DCH3ECONbits.SIRQEN = 1;
        DCH3ECONbits.CHSIRQ = _PMP_IRQ;
        DCH3CONbits.CHAEN = 0;
        DCH3CONbits.CHEN = 1;

        DMACONbits.ON=1;

        DCH3ECONbits.CFORCE = 1;
        data += (chunk >> 1);
    }
}

void Picadillo::closeWindow() {
    if (DCH3CONbits.CHEN == 1) {
        while (DCH3CONbits.CHBUSY);
        DCH3CONbits.CHEN = 0;
    }
}

uint8_t Picadillo::readByte(bool fresh) {
    static bool haveStoredByte = false;
    static uint8_t storedByte = 0;
    if (fresh) {
        haveStoredByte = false;
    }
    if (!haveStoredByte) {
        while (PMMODEbits.BUSY == 1);
        uint16_t din = PMDIN;
        storedByte = din & 0xFF;
        haveStoredByte = true;
        return din >> 8;
    }
    haveStoredByte = false;
    return storedByte;
}

color_t Picadillo::colorAt(int x, int y) {
    return (this->*_colorAt)(x, y);
}

void Picadillo::getRectangle(int x, int y, int w, int h, color_t *buf) {
    uint32_t i = 0;
    for (int py = 0; py < h; py++) {
        for (int px = 0; px < w; px++) {
            buf[i++] = colorAt(x + px, y + py);
        }
    }
}

uint16_t Picadillo::readData(bool cont) {
    uint16_t din;
    PMADDR = 0x0001;
    if (!cont) {
        while (PMMODEbits.BUSY == 1);
        din = PMDIN;
    }
    while (PMMODEbits.BUSY == 1);
    din = PMDIN;
    return din;
}

void Picadillo::enableBacklight() {
    analogWrite(PIN_BACKLIGHT, _brightness);
}

void Picadillo::disableBacklight() {
    analogWrite(PIN_BACKLIGHT, 0);
}

void Picadillo::setBacklight(int b) {
    _brightness = b;
    analogWrite(PIN_BACKLIGHT, _brightness);
}

int Picadillo::getScanLine() {
    return (this->*_getScanLine)();
}

void Picadillo::identifyDisplay() {
//Serial.begin(115200);
//    writeCommand(0x04); // HX8357 read display ID
//    Serial.printf("Dummy: %02X\r\n", readData(false));
//    Serial.printf("Blank: %02X\r\n", readData(true));
//    Serial.printf("High:  %02X\r\n", readData(true));
//    Serial.printf("Med:   %02X\r\n", readData(true));
//    Serial.printf("Low:   %02X\r\n", readData(true));

    writeCommand(0x04); // HX8357 read display ID
    readData(false);
    readData(true);
    uint32_t h = readData(true);
    uint32_t m = readData(true);
    uint32_t l = readData(true);

    uint32_t displayId = (h << 16) | (m << 8) | l;

    switch (displayId) {
        default: // Assume HX8357 - it's shaky about reporting its ID
        case DISPLAY_HX8357:
            _initializeDevice = &Picadillo::initializeDevice_HX8357;
            _setAddrWindow = &Picadillo::setAddrWindow_HX8357;
            _setRotation = &Picadillo::setRotation_HX8357;
            _invertDisplay = &Picadillo::invertDisplay_HX8357;
            _displayOn = &Picadillo::displayOn_HX8357;
            _displayOff = &Picadillo::displayOff_HX8357;
            _colorAt = &Picadillo::colorAt_HX8357;
            _getScanLine = &Picadillo::getScanLine_HX8357;

            break;
        case DISPLAY_ILI9488:
            _initializeDevice = &Picadillo::initializeDevice_ILI9488;
            _setAddrWindow = &Picadillo::setAddrWindow_ILI9488;
            _setRotation = &Picadillo::setRotation_ILI9488;
            _invertDisplay = &Picadillo::invertDisplay_ILI9488;
            _displayOn = &Picadillo::displayOn_ILI9488;
            _displayOff = &Picadillo::displayOff_ILI9488;
            _colorAt = &Picadillo::colorAt_ILI9488;
            _getScanLine = &Picadillo::getScanLine_ILI9488;
            break;
    }
}

#endif
