// We can only compile this code if we have both a
// DMA system and the Parallel Master Port.
#ifdef _BOARD_PICADILLO_35T_
#include <Picadillo.h>

//==============================================================
// Initialise HX8357 LCD Driver IC
//==============================================================
void Picadillo::initializeDevice_ILI9488() 
{
    _width  = Picadillo::Width;
    _height = Picadillo::Height;


    /* PGAMCTRL (Positive Gamma Control) (E0h) */
    writeCommand(0xE0);
    writeData(0x04);
    writeData(0x67);
    writeData(0x35);
    writeData(0x04);
    writeData(0x08);
    writeData(0x06);
    writeData(0x24);
    writeData(0x1);
    writeData(0x37);
    writeData(0x40);
    writeData(0x03);
    writeData(0x10);
    writeData(0x8);
    writeData(0x80);
    writeData(0x0);
    /* NGAMCTRL (Negative Gamma Control) (E1h)  */
    writeCommand(0xE1);
    writeData(0x00);
    writeData(0x22);
    writeData(0x24);
    writeData(0x06);
    writeData(0x12);
    writeData(0x07);
    writeData(0x36);
    writeData(0x47);
    writeData(0x47);
    writeData(0x06);
    writeData(0x0a);
    writeData(0x07);
    writeData(0x30);
    writeData(0x37);
    writeData(0x0f);
    /* Power Control 1 (C0h)  */
    writeCommand(0xC0);
    writeData(0x10);
    writeData(0x10);

    /* Power Control 2 (C1h)  */
    writeCommand(0xC1);
    writeData(0x41);

    /* VCOM Control (C5h)  */
    writeCommand(0xC5);
    writeData(0x00);
    writeData(0x2c); // VCOM
    writeData(0x80);

    /* Memory Access Control (36h)  */
    writeCommand(0x36);
    writeData(0x08);
    //- writeData(0x08);

    /* Interface Pixel Format (3Ah) */
    writeCommand(0x3A);  //Interface Mode Control
    writeData(0x55);

    writeCommand(0xB0);  //Interface Mode Control
    writeData(0x00);
    writeCommand(0xB1);   //Frame rate 70HZ
    writeData(0xB0);
    /* Display Inversion Control (B4h) */
    writeCommand(0xB4);
    writeData(0x02);

    /* Display Function Control (B6h)  */
    writeCommand(0xB6);  //RGB/MCU Interface Control
    writeData(0x02);
    writeData(0x22);

    /* Set Image Function (E9h)  */
    writeCommand(0xE9);
    writeData(0x00);

    /* Adjust Control 3 (F7h)  */
    writeCommand(0XF7);
    writeData(0xA9);
    writeData(0x51);
    writeData(0x2C);
    writeData(0x82);

    /* Sleep Out (11h) */
    writeCommand(0x11);
    delay(150);

    /* Display ON (29h) */
    writeCommand(0x29);

	
    clearClipping();
}

void Picadillo::setAddrWindow_ILI9488(int x0, int y0, int x1, int y1) 
{
    writeCommand(0x002A);
    writeData(x0 >> 8);
    writeData(x0 & 0xFF);
    writeData(x1 >> 8);
    writeData(x1 & 0xFF);
    writeCommand(0x002B);
    writeData(y0 >> 8);
    writeData(y0 & 0xFF);
    writeData(y1 >> 8);
    writeData(y1 & 0xFF);
    writeCommand(0x002c);
}

void Picadillo::setRotation_ILI9488(int m) 
{

    writeCommand(0x36);
    rotation = m % 4; // can't be higher than 3
    switch (rotation) {
        case 0:
            writeData(0x08);
            _width  = Picadillo::Width;
            _height = Picadillo::Height;
            break;
        case 1:
            writeData(0x68);
            _width  = Picadillo::Height;
            _height = Picadillo::Width;
            break;
        case 2:
            writeData(0xC8);
            _width  = Picadillo::Width;
            _height = Picadillo::Height;
            break;
        case 3:
            writeData(0xA8);
            _width  = Picadillo::Height;
            _height = Picadillo::Width;
            break;
    }
    clearClipping();
}

void Picadillo::invertDisplay_ILI9488(bool i) 
{
    writeCommand(i ? 0x21 : 0x20);
}

void Picadillo::displayOn_ILI9488() 
{
    writeCommand(0x29);
}

void Picadillo::displayOff_ILI9488() 
{
    writeCommand(0x28);
}

color_t Picadillo::colorAt_ILI9488(int x, int y) {
	if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) 
		return 0;
    setAddrWindow(x,y,x,y);
    writeCommand(0x002E);
    (void)readData();
    color_t color = readData();
    color_t color1 = 0;
    color1 |= ((color & 0xF800) >> 11);
    color1 |= (color & 0x07E0);
    color1 |= ((color & 0x001F) << 11);
    return color1;
}


int Picadillo::getScanLine_ILI9488() {
    writeCommand(0x45);
    readData(false);
    uint8_t h = readData(true);
    uint8_t l = readData(true);
    return (h << 8) | l;
}


void Picadillo::identifyDisplay() {


    writeCommand(0x04); // HX8357 read display ID
    readData(false);
    readData(true);
    uint32_t h = readData(true);
    uint32_t m = readData(true);
    uint32_t l = readData(true);

    uint32_t displayId = (h << 16) | (m << 8) | l;

    Serial.printf("Display ID: %06X\r\n", displayId);
}

#endif
