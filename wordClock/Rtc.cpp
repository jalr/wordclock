//#include <Wire.h>
#include "Rtc.h"
#include "RTClib.h"

RTC_DS1307 ds1307;

Rtc rtc;

void Rtc::setup() {
  if (! ds1307.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }
  if (! ds1307.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    ds1307.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  rtc.hctosys();
}

void Rtc::hctosys() {
  Serial.print("Reading timestamp from RTC: ");
  Serial.println(ds1307.now().unixtime());
  setTime(ds1307.now().unixtime());
}

void Rtc::set(time_t ts) {
  DateTime dt = DateTime(ts);
  Serial.print("Saving timestamp to RTC: ");
  Serial.println(ts);
  ds1307.adjust(dt);
}
