#ifndef FALLING_STAR_ANIMATOR_H
#define FALLING_STAR_ANIMATOR_H

#include <Arduino.h>
#include "IDisplayDriver.h"
#include "IAnimator.h"

#define FALLING_STAR_LIMIT 20

class FallingStar
{
  private:
    IDisplayDriver *driver;

    uint8_t x, y;
    int delay;
    uint32_t color;
    bool applied;
    bool finished;

  public:
    FallingStar() { }
    FallingStar(IDisplayDriver *driver, uint8_t x, uint8_t y, int delay);

    bool animate();
};

class FallingStarAnimator: public IAnimator
{
  private:
    IDisplayDriver *driver;
    FallingStar stars[FALLING_STAR_LIMIT];
    uint8_t starCount = 0;

  public:
    FallingStarAnimator(IDisplayDriver *driver): driver(driver) { }

    virtual void setDots(uint8_t count, uint8_t red, uint8_t green, uint8_t blue);
    virtual void setPixel(uint8_t x, uint8_t y, uint8_t red, uint8_t green, uint8_t blue);
    virtual void clearPixel(uint8_t x, uint8_t y);
    virtual void commit();
};


#endif  /* !FALLING_STAR_ANIMATOR_H */

