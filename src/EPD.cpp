#include "qrcode.h"
#include "EPD.h"

const unsigned char lut_full_update[]= {
    0x80,0x60,0x40,0x00,0x00,0x00,0x00,             //LUT0: BB:     VS 0 ~7
    0x10,0x60,0x20,0x00,0x00,0x00,0x00,             //LUT1: BW:     VS 0 ~7
    0x80,0x60,0x40,0x00,0x00,0x00,0x00,             //LUT2: WB:     VS 0 ~7
    0x10,0x60,0x20,0x00,0x00,0x00,0x00,             //LUT3: WW:     VS 0 ~7
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,             //LUT4: VCOM:   VS 0 ~7

    0x03,0x03,0x00,0x00,0x05,                       // TP0 A~D RP0
    0x09,0x09,0x00,0x00,0x05,                       // TP1 A~D RP1
    0x03,0x03,0x00,0x00,0x0C,                       // TP2 A~D RP2
    0x00,0x00,0x00,0x00,0x00,                       // TP3 A~D RP3
    0x00,0x00,0x00,0x00,0x00,                       // TP4 A~D RP4
    0x00,0x00,0x00,0x00,0x00,                       // TP5 A~D RP5
    0x00,0x00,0x00,0x00,0x00,                       // TP6 A~D RP6

    0x17,0x41,0x9B,0x3A,0x0F,0x04,
};

const unsigned char lut_partial_update[]= { //20 bytes
    0x40,0x40,0x00,0x00,0x00,0x00,0x00,             //LUT0: BB:     VS 0 ~7
    0x00,0x00,0x80,0x00,0x00,0x00,0x00,             //LUT1: BW:     VS 0 ~7
    0x00,0x00,0x40,0x00,0x00,0x00,0x00,             //LUT2: WB:     VS 0 ~7
    0x80,0x80,0x00,0x00,0x00,0x00,0x00,             //LUT3: WW:     VS 0 ~7
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,             //LUT4: VCOM:   VS 0 ~7

    0x02,0x00,0x00,0x00,0x00,                       // TP0 A~D RP0
    0x05,0x00,0x00,0x00,0x00,                       // TP1 A~D RP1
    0x0E,0x00,0x00,0x00,0x01,                       // TP2 A~D RP2
    0x00,0x00,0x00,0x00,0x00,                       // TP3 A~D RP3
    0x00,0x00,0x00,0x00,0x00,                       // TP4 A~D RP4
    0x00,0x00,0x00,0x00,0x00,                       // TP5 A~D RP5
    0x00,0x00,0x00,0x00,0x00,                       // TP6 A~D RP6

    0x15,0x41,0x9B,0x3A,0x0F,0x04,
};

EPD::EPD(uint epd_spiTx, uint epd_spiSck, uint epd_spiCs, uint epd_DC, uint epd_busy, uint epd_rst, uint spiID){
    this->epd_busy = epd_busy;
    this->epd_DC = epd_DC;
    this->epd_rst = epd_rst;
    this->epd_spiCs = epd_spiCs;
    this->epd_spiSck = epd_spiSck;
    this->epd_spiTx = epd_spiTx;
    this->spiID = spiID;

    gpio_set_function(epd_spiTx, GPIO_FUNC_SPI);
    gpio_set_function(epd_spiSck, GPIO_FUNC_SPI);

    gpio_init(epd_DC);
    gpio_init(epd_rst);
    gpio_init(epd_busy);
    gpio_init(epd_spiCs);
    gpio_set_dir(epd_spiCs, true);
    gpio_set_dir(epd_DC, true);
    gpio_set_dir(epd_rst, true);
    gpio_set_dir(epd_busy, false);
    if(spiID == 1){
        spi_init(spi1, 10*1000*1000);
    }else {
        spi_init(spi0, 10*1000*1000);
    }

}


void EPD::command(uint8_t command){
    gpio_put(epd_DC, false);
    gpio_put(epd_spiCs, false);
    sleep_us(1);
    if(spiID == 1){
        spi_write_blocking(spi1, &command, sizeof(uint8_t));
    }else {
        spi_write_blocking(spi0, &command, sizeof(uint8_t));
    }
    gpio_put(epd_spiCs, true);
}

void EPD::data(uint8_t data){
    gpio_put(epd_DC, true);
    gpio_put(epd_spiCs, false);
    sleep_us(1);
    if(spiID == 1){
        spi_write_blocking(spi1, &data, sizeof(uint8_t));
    }else {
        spi_write_blocking(spi0, &data, sizeof(uint8_t));
    }
    gpio_put(epd_spiCs, true);
}

void EPD::waitUntilBusy(){
    sleep_us(100);
    while(true){
        if(gpio_get(epd_busy) == 0){
            break;
        }
    }
}

void EPD::HW_reset(){
    sleep_ms(20);
    gpio_put(epd_rst, false);
    sleep_ms(40);
    gpio_put(epd_rst, true);
    sleep_ms(50);
}



uint8_t EPD::imgfetchHex(int y, int x, uint32_t* img_src){
    int hsize = 212/32 + (212%32==0 ? 0 : 1);
    if(y >=103 || x>=211){
        return 0b1;
    }
    else{
        uint32_t target = *(img_src+hsize*y + (x/32));
        return (target)>>((31 - x%32)) & 0b1;
    }
}


#ifdef __QRCODE_H_
void EPD::img_putqrcode(QRCode* qr, int posX, int posY, uint32_t* src, int img_hor, int img_vert){
    int hor_s = (img_hor/32) + ((img_hor%32 != 0)? 1:0);
    for(int y=0; y<qr->size; y++){
        int src_y = y+posY;
        for(int x=0; x<qr->size; x++){
            int src_x = x+posX;
            uint32_t write_bit = qrcode_getModule(qr, x, y)? 0:1;
            *(src+hor_s*src_y+(src_x/32)) = ((*(src+hor_s*src_y+(src_x/32)) & (~(0b1<<(31-src_x%32)))) | (write_bit<<(31-src_x%32))); 
        }
    }
}
#endif



void EPD::Init(int mode){
    this->HW_reset();
    
    if(mode == EPD_FULL){
        this->waitUntilBusy();
        this->command(0x12);
        this->waitUntilBusy();

        this->command(0x01);
        this->data(0xF9);
        this->data(0x00);
        this->data(0x00);

        this->command(0x11);
        this->data(0x01);

        this->command(0x44);
        this->data(0x00);
        this->data(12);

        this->command(0x45);
        this->data(0b0);
        this->data(0x00);
        this->data(212);//104x212
        this->data(0x00);
        
        this->command(0x4E);
        this->data(0x00);
        this->command(0x4F);
        this->data(0xD3);
        this->data(0x00);


        this->command(0x03);
        this->data(lut_full_update[70]);

        this->command(0x04); //
        this->data(lut_full_update[71]);
        this->data(lut_full_update[72]);
        this->data(lut_full_update[73]);

        this->command(0x3A);     //Dummy Line
        this->data(lut_full_update[74]);
        this->command(0x3B);     //Gate time
        this->data(lut_full_update[75]);

        this->command(0x32);
        for(int count = 0; count < 70; count++) {
            this->data(lut_full_update[count]);
        }
        this->command(0x3C);
        this->data(0x05);

        this->command(0x21);
        this->data(0x00);
        this->data(0x80);

        this->command(0x18);
        this->data(0x80);
        this->waitUntilBusy();

    }
    if(mode == EPD_PART){
        this->HW_reset();
        
        this->command(0x2C);
        this->data(0x26);
        this->waitUntilBusy();

        
        this->command(0x03);
        this->data(lut_partial_update[70]);

        this->command(0x04); //
        this->data(lut_partial_update[71]);
        this->data(lut_partial_update[72]);
        this->data(lut_partial_update[73]);

        this->command(0x3A);     //Dummy Line
        this->data(lut_partial_update[74]);
        this->command(0x3B);     //Gate time
        this->data(lut_partial_update[75]);

        this->command(0x32);
        for(int count = 0; count < 70; count++) {
            this->data(lut_partial_update[count]);
        }

        this->command(0x37);
        this->data(0x00);
        this->data(0x00);
        this->data(0x00);
        this->data(0x00);
        this->data(0x40);
        this->data(0x80);
        this->data(0x00);

        
        this->command(0x11);
        this->data(0x01);

        this->command(0x44);
        this->data(0x00);
        this->data(12);

        this->command(0x45);
        this->data(0x00);
        this->data(0x00);
        this->data(212);//104x212
        this->data(0x00);
        
        this->command(0x4E);
        this->data(0x00);
        this->command(0x4F);
        this->data(0xD3);
        this->data(0x00);

        this->command(0x22);
        this->data(0xC0);

        this->command(0x20);
        this->waitUntilBusy();

        this->command(0x3C);
        this->data(0x01);
        
    }

}



void EPD::putImg(uint32_t* img_src){
    this->command(0x24);

    for(uint i=0;i<212;i++)
    {
        for(uint j=0;j<13;j++){
            uint8_t ty=0;
            for(int ij1=0; ij1<8; ij1++){
                ty = (this->imgfetchHex(j*8+ij1, i, img_src) << (7-ij1) ) | ty; 
            }
            this->data(ty);
        }
    }


    this->command(0x22);
    this->data(0xCF);
    this->command(0x20);
    this->waitUntilBusy();
}



void EPD::putPartImg(uint32_t* img_src){
    this->command(0x24);

    for(uint i=0;i<212;i++)
    {
        for(uint j=0;j<13;j++){
            uint8_t ty=0;
            for(int ij1=0; ij1<8; ij1++){
                ty = (EPD::imgfetchHex(j*8+ij1, i, img_src) << (7-ij1) ) | ty; 
            }
            this->data(ty);
        }
    }


    this->command(0x22);
    this->data(0x0C);
    this->command(0x20);
    this->waitUntilBusy();
}

void EPD::putPartBaseImg(uint32_t* img_src){
    this->command(0x24);

    for(uint i=0;i<212;i++)
    {
        for(uint j=0;j<13;j++){
            uint8_t ty=0;
            for(int ij1=0; ij1<8; ij1++){
                ty = (EPD::imgfetchHex(j*8+ij1, i, img_src) << (7-ij1) ) | ty; 
            }
            this->data(ty);
        }
    }

    this->command(0x26);
    for(uint i=0;i<212;i++)
    {
        for(uint j=0;j<13;j++){
            uint8_t ty=0;
            for(int ij1=0; ij1<8; ij1++){
                ty = (EPD::imgfetchHex(j*8+ij1, i, img_src) << (7-ij1) ) | ty; 
            }
            this->data(ty);
        }
    }


    this->command(0x22);
    this->data(0xC7);
    this->command(0x20);
    this->waitUntilBusy();
}