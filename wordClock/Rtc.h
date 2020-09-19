#ifndef I_RTC_H
#define I_RTC_H
#include <Time.h>

class Rtc
{
  public:
    void setup();
    void set(time_t ts);
    void hctosys();
};

extern Rtc rtc;

#endif  /* !I_RTC_H */

