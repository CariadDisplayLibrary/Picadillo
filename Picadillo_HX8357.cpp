
// We can only compile this code if we have both a
// DMA system and the Parallel Master Port.
#ifdef _BOARD_PICADILLO_35T_
#include <Picadillo.h>

void Picadillo::initializeDevice_HX8357() {
    _width  = Picadillo::Width;
    _height = Picadillo::Height;
	
	colstart = 0;  //NEED TO CONFIRM
    rowstart = 0;  //NEED TO CONFIRM
	
    writeCommand(HX8357_EXIT_SLEEP_MODE); //Sleep Out
    delay(150);
	writeCommand(HX8357_ENABLE_EXTENSION_COMMAND);
	writeData(0xFF);
	writeData(0x83);
	writeData(0x57);
	delay(1);
	writeCommand(HX8357_SET_POWER_CONTROL);
	writeData(0x00);
	writeData(0x12);
	writeData(0x12);
	writeData(0x12);
	writeData(0xC3);
	writeData(0x44);
	delay(1);
	writeCommand(HX8357_SET_DISPLAY_MODE);
	writeData(0x02);
	writeData(0x40);
	writeData(0x00);
	writeData(0x2A);
	writeData(0x2A);
	writeData(0x20);
	writeData(0x91);
	delay(1);
	writeCommand(HX8357_SET_VCOM_VOLTAGE);
	writeData(0x38);
	delay(1);
	writeCommand(HX8357_SET_INTERNAL_OSCILLATOR);
	writeData(0x68);
	writeCommand(0xE3); //Unknown Command
	writeData(0x2F);
	writeData(0x1F);
	writeCommand(0xB5); //Set BGP
	writeData(0x01);
	writeData(0x01);
	writeData(0x67);
	writeCommand(HX8357_SET_PANEL_DRIVING);
	writeData(0x70);
	writeData(0x70);
	writeData(0x01);
	writeData(0x3C);
	writeData(0xC8);
	writeData(0x08);
	delay(1);
	writeCommand(0xC2); // Set Gate EQ
	writeData(0x00);
	writeData(0x08);
	writeData(0x04);
	delay(1);
	writeCommand(HX8357_SET_PANEL_CHARACTERISTIC);
	writeData(0x09);
	delay(1);
	writeCommand(HX8357_SET_GAMMA_CURVE);
	writeData(0x01);
	writeData(0x02);
	writeData(0x03);
	writeData(0x05);
	writeData(0x0E);
	writeData(0x22);
	writeData(0x32);
	writeData(0x3B);
	writeData(0x5C);
	writeData(0x54);
	writeData(0x4C);
	writeData(0x41);
	writeData(0x3D);
	writeData(0x37);
	writeData(0x31);
	writeData(0x21);
	writeData(0x01);
	writeData(0x02);
	writeData(0x03);
	writeData(0x05);
	writeData(0x0E);
	writeData(0x22);
	writeData(0x32);
	writeData(0x3B);
	writeData(0x5C);
	writeData(0x54);
	writeData(0x4C);
	writeData(0x41);
	writeData(0x3D);
	writeData(0x37);
	writeData(0x31);
	writeData(0x21);
	writeData(0x00);
	writeData(0x01);
	delay(1);
	writeCommand(HX8357_SET_PIXEL_FORMAT); //COLMOD RGB888
	writeData(0x55);
	writeCommand(HX8357_SET_ADDRESS_MODE);
	writeData(0x00);
	writeCommand(HX8357_SET_TEAR_ON); //TE ON
	writeData(0x00);
	delay(10);
	writeCommand(HX8357_SET_DISPLAY_ON); //Display On
	delay(10);
    clearClipping();
}

void Picadillo::setAddrWindow_HX8357(int x0, int y0, int x1, int y1) 
{
    writeCommand(HX8357_SET_COLUMN_ADDRESS); // Column addr set
    writeData((x0+colstart) >> 8);
    writeData(x0+colstart);     // XSTART 
    writeData((x1+colstart) >> 8);
    writeData(x1+colstart);     // XEND

    writeCommand(HX8357_SET_PAGE_ADDRESS); // Row addr set
    writeData((y0+rowstart) >> 8);
    writeData(y0+rowstart);     // YSTART
    writeData((y1+rowstart) >> 8);
    writeData(y1+rowstart);     // YEND
	writeCommand(HX8357_WRITE_MEMORY_START); //Write SRAM Data
}

void Picadillo::setRotation_HX8357(int m) 
{
	writeCommand(HX8357_SET_ADDRESS_MODE);
	rotation = m % 4; // can't be higher than 3
	switch (rotation) 
	{
		case 0:
			//PORTRAIT
			writeData(0x0000);
			_width  = Picadillo::Width;
			_height = Picadillo::Height;
			break;
		case 1:
		    //LANDSCAPE
			writeData(0x0060);
			_width  = Picadillo::Height;
			_height = Picadillo::Width;
			break;
		case 2:	
			//UPSIDE DOWN PORTRAIT
			writeData(0x00C0);
			_width  = Picadillo::Width;
			_height = Picadillo::Height;
			break;
		case 3:
			//UPSIDE DOWN LANDSCAPE
			writeData(0x00A0);
			_width  = Picadillo::Height;
			_height = Picadillo::Width;
			break;
	}
    clearClipping();
}

void Picadillo::invertDisplay_HX8357(bool i) 
{
	writeCommand(i ? HX8357_INVON : HX8357_INVOFF);
}

void Picadillo::displayOn_HX8357() 
{
	writeCommand(HX8357_DISPLAYON);
}

void Picadillo::displayOff_HX8357() 
{
	writeCommand(HX8357_DISPLAYOFF);
}

color_t Picadillo::colorAt_HX8357(int x, int y) {
	if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) 
		return 0;
    setAddrWindow(x, y, x, y);
    writeCommand(HX8357_READ_MEMORY_START);
    readData(false);
    readByte(true);
    readByte();
    readByte();
    readByte();
    readByte();

    uint8_t r = readByte();
    uint8_t g = readByte();
    uint8_t b = readByte();

    return rgb(r, g, b);
}

int Picadillo::getScanLine_HX8357() {
    writeCommand(HX8357_GETSCAN);
    uint8_t h = readData(false);
    uint8_t l = readByte(true);
    return (h << 8) | l;
}

#endif
