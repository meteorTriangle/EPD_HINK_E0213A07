#ifndef EPD_H
#define EPD_H
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"


/// QR code library is optional, source link:(https://github.com/ricmoo/QRCode)
#ifdef __QRCODE_H_
#include "qrcode.h"
#endif

#define EPD_FULL 1
#define EPD_PART 2

class EPD{
    public:
        EPD(uint epd_spiTx, uint epd_spiSck, uint epd_spiCs, uint epd_DC, uint epd_busy, uint epd_rst, uint spiID);
        #ifdef __QRCODE_H_
        void img_putqrcode(QRCode* qr, int posX, int posY, uint32_t* src, int img_hor, int img_vert);
        #endif
        void Init(int mode);
        void putImg(uint32_t* img_src);
        void putPartImg(uint32_t* img_src);
        void putPartBaseImg(uint32_t* img_src);
    private:
        void HW_reset();
        void command(uint8_t command);
        void data(uint8_t data);
        void waitUntilBusy();
        uint8_t imgfetchHex(int y, int x, uint32_t* img_src);
        uint epd_spiTx, epd_spiSck, epd_spiCs, epd_DC, epd_busy, epd_rst;
        uint spiID;


};

#endif