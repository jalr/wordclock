#ifndef DISPLAY_DRIVER_FABLAB_NEA_CLOCK_H
#define DISPLAY_DRIVER_FABLAB_NEA_CLOCK_H

#include <Adafruit_NeoPixel.h>
#include "IDisplayDriver.h"
#include "IAnimator.h"

class DisplayDriverFablabNeaClock : public IDisplayDriver
{
  private:
    uint8_t pin = 5;
    Adafruit_NeoPixel pixels = { 114, pin, NEO_GRB + NEO_KHZ800 };

  public:
    void setup();

    virtual int width();
    virtual int height();
    
    virtual void setDots(uint8_t count, uint8_t red, uint8_t green, uint8_t blue);

    virtual uint32_t getPixel(uint8_t x, uint8_t y);
    virtual void setPixel(uint8_t x, uint8_t y, uint8_t red, uint8_t green, uint8_t blue);
    virtual void clearPixel(uint8_t x, uint8_t y);

    void clear();
    
    virtual void show();

  private:
    uint8_t getPixelIndex(uint8_t x, uint8_t y);
};

#endif  /* !DISPLAY_DRIVER_FABLAB_NEA_CLOCK_H */





