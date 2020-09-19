#include <Time.h>

#include "DiyHueController.h"
#include "HttpController.h"
#include "HttpContent.h"
#include "PersistentStorage.h"
#include "Rtc.h"

extern PersistentStorage persistentStorage;

static ESP8266WebServer server(80);
HttpController httpController;

#define readCharParam(setting,param) \
  if (server.hasArg(#param)) { \
    server.arg(#param).toCharArray(setting, sizeof(setting)); \
  }

#define readBoolParam(setting,param) \
  do { \
    setting = false; \
    if (server.hasArg(#param)) { \
      setting = server.arg(#param) == "1"; \
    } \
  } while(0)

#define readIntParam(setting,param)  \
  if (server.hasArg(#param)) { \
    setting = server.arg(#param).toInt(); \
  }


String form_input(const String& name, const String& info, const String& value, const int length) {
    String s = F("<div>{i}: <input type='text' name='{n}' id='{n}' placeholder='{i}' value='{v}' maxlength='{l}'/></div>");
    String t_value = value;
    t_value.replace("'", "&#39;");
    s.replace("{i}", info);
    s.replace("{n}", name);
    s.replace("{v}", t_value);
    s.replace("{l}", String(length));
    return s;
}

String form_password(const String& name, const String& info, const String& value, const int length) {
    String s = F("<div>{i}: <input type='password' name='{n}' id='{n}' placeholder='{i}' value='{v}' maxlength='{l}'/></div>");
    String t_value = value;
    t_value.replace("'", "&#39;");
    s.replace("{i}", info);
    s.replace("{n}", name);
    s.replace("{v}", t_value);
    s.replace("{l}", String(length));
    return s;
}

String form_number(const String& name, const String& info, const int value, const int min, const int max) {
    String s = F("<div>{i} ({min}..{max}): <input type='number' name='{n}' id='{n}' placeholder='{i} ({min}..{max})' value='{v}' min='{min}' max='{max}'/></div>");
    s.replace("{i}", info);
    s.replace("{n}", name);
    s.replace("{v}", String(value));
    s.replace("{min}", String(min));
    s.replace("{max}", String(max));
    return s;
}

String form_checkbox(const String& name, const String& info, const bool checked) {
    String s = F("<div><label for='{n}'><input type='checkbox' name='{n}' value='1' id='{n}' {c}/> {i}</label></div>");
    if (checked) {
        s.replace("{c}", F(" checked='checked'"));
    } else {
        s.replace("{c}", "");
    };
    s.replace("{i}", info);
    s.replace("{n}", name);
    return s;
}

String form_submit() {
    String s = F("<div><input type='submit' name='submit' value='Save &amp; Apply Changes' /></div>");
    return s;
}

static void handleRoot() {
  String content = FPSTR(WEB_PAGE_HEADER);
  content.replace("{title}", F("WordClock Configuration"));

  if (server.method() == HTTP_GET) {
    content += F("<script>let r=new XMLHttpRequest();r.open('POST','/date',true);r.setRequestHeader('Content-type','application/x-www-form-urlencoded');r.send('timestamp='+parseInt(new Date().getTime()/1000));</script>");
    content += F("<form method='POST'>");

    content += F("<h4>Wifi Settings</h4>");
    content += form_input("wifissid", F("SSID"), persistentStorage.wifi.ssid, 32);
    content += form_password("wifipassword", F("Password"), persistentStorage.wifi.password, 64);

    content += F("<h4>Primary Text Color</h4>");
    content += form_number("red", F("Red"), persistentStorage.red, 0, 255);
    content += form_number("green", F("Green"), persistentStorage.green, 0, 255);
    content += form_number("blue", F("Blue"), persistentStorage.blue, 0, 255);

    content += F("<h4>NTP Settings</h4>");
    content += form_input("ntphost", F("Host"), persistentStorage.ntp.domain, sizeof(persistentStorage.ntp.domain));
    
    content += F("<h4>MQTT Settings</h4>");
    content += form_checkbox("mqttenabled", F("Enabled"), persistentStorage.flags.mqttEnabled);
    content += form_input("mqtthost", F("Host"), persistentStorage.mqtt.domain, 16);
    content += form_input("mqttclientid", F("Client-ID"), persistentStorage.mqtt.clientId, 16);
    content += form_input("mqttuser", F("User"), persistentStorage.mqtt.user, 16);
    content += form_password("mqttpassword", F("Password"), persistentStorage.mqtt.password, 64);

    content += form_submit();
    content += F("</form>");
  } else if (server.method() == HTTP_POST) {
    readCharParam(persistentStorage.wifi.ssid, wifissid);
    readCharParam(persistentStorage.wifi.password, wifipassword);
    readIntParam(persistentStorage.red, red);
    readIntParam(persistentStorage.green, green);
    readIntParam(persistentStorage.blue, blue);
    readBoolParam(persistentStorage.flags.mqttEnabled, mqttenabled);
    readCharParam(persistentStorage.ntp.domain, ntphost);
    readCharParam(persistentStorage.mqtt.domain, mqtthost);
    readCharParam(persistentStorage.mqtt.clientId, mqttclientid);
    readCharParam(persistentStorage.mqtt.user, mqttuser);
    readCharParam(persistentStorage.mqtt.password, mqttpassword);
    Serial.println("updated mqtt domain to: " + String(persistentStorage.mqtt.domain));
    persistentStorage.commit();
    ESP.reset();
  } else {
    server.send(405, "text/plain", "405 Method Not Allowed");
    return;
  }

  content += FPSTR(WEB_PAGE_FOOTER);
  server.send(200, "text/html", content);
}

static void handleDate() {
  if (server.method() == HTTP_POST) {
    if (server.hasArg("timestamp")) {
      time_t timestamp = server.arg("timestamp").toInt();
      if (timestamp > 0) {
        rtc.set(timestamp);
        setTime(timestamp);
        rtc.hctosys();
      }
      server.send(200, "text/plain", "Changed date successfully");
    } else {
      server.send(400, "text/plain", "400 Bad Request");
    }
  } else {
    server.send(405, "text/plain", "405 Method Not Allowed");
    return;
  }
}

static void handleDiyHueSet() {
  float transitiontime = 4;

  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "on") {
      if (server.arg(i) == "True" || server.arg(i) == "true") {
        diyHueController.setLightState(true);
      }
      else {
        diyHueController.setLightState(false);
      }
    }
    else if (server.argName(i) == "r") {
      diyHueController.setRed(server.arg(i).toInt());
    }
    else if (server.argName(i) == "g") {
      diyHueController.setGreen(server.arg(i).toInt());
    }
    else if (server.argName(i) == "b") {
      diyHueController.setBlue(server.arg(i).toInt());
    }
    else if (server.argName(i) == "x") {
      diyHueController.setColorX(server.arg(i).toFloat());
    }
    else if (server.argName(i) == "y") {
      diyHueController.setColorY(server.arg(i).toFloat());
    }
    else if (server.argName(i) == "bri") {
      diyHueController.setBri(server.arg(i).toInt());
    }
    else if (server.argName(i) == "bri_inc") {
      diyHueController.incBri(server.arg(i).toInt());
    }
    else if (server.argName(i) == "ct") {
      diyHueController.setCt(server.arg(i).toInt());
    }
    else if (server.argName(i) == "sat") {
      diyHueController.setSat(server.arg(i).toInt());
    }
    else if (server.argName(i) == "hue") {
      diyHueController.setHue(server.arg(i).toInt());
    }
    else if (server.argName(i) == "alert" && server.arg(i) == "select") {
      diyHueController.alert();
    }
    else if (server.argName(i) == "transitiontime") {
      transitiontime = server.arg(i).toInt();
    }
  }

  server.send(200, "text/plain", "OK");
  //server.send(200, "text/plain", "OK, x: " + (String)x[light] + ", y:" + (String)y[light] + ", bri:" + (String)bri[light] + ", ct:" + ct[light] + ", colormode:" + color_mode[light] + ", state:" + light_state[light]);
  diyHueController.process_lightdata(transitiontime);
}

static void handleDiyHueGet() {
  String colormode;
  String power_status = diyHueController.getLightState() ? "true" : "false";
  
  if (diyHueController.getColorMode() == 1)
    colormode = "xy";
  else if (diyHueController.getColorMode() == 2)
    colormode = "ct";
  else if (diyHueController.getColorMode() == 3)
    colormode = "hs";

  server.send(200, "text/plain", "{\"on\": " + power_status
    + ", \"bri\": " + (String)diyHueController.getBri()
    + ", \"xy\": [" + (String)diyHueController.getColorX() + ", " + (String)diyHueController.getColorY()
    + "], \"ct\":" + (String)diyHueController.getCt()
    + ", \"sat\": " + (String)diyHueController.getSat()
    + ", \"hue\": " + (String)diyHueController.getHue()
    + ", \"colormode\": \"" + colormode + "\"}");
}

static void handleDiyHueDetect() {
  server.send(200, "text/plain", "{\"hue\": \"bulb\",\"lights\": 1,\"modelid\": \"LCT015\",\"name\": \"wordClock\",\"mac\": \"" + WiFi.macAddress() + "\"}");
}

static void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  if (server.method() == HTTP_GET) {
    message += "GET";
  } else if (server.method() == HTTP_POST) {
    message += "POST";
  } else {
    message += "unknown";
  }
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i ++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}

void HttpController::setup() {
  server.on("/", handleRoot);
  server.on("/set", handleDiyHueSet);
  server.on("/get", handleDiyHueGet);
  server.on("/detect", handleDiyHueDetect);
  server.on("/date", handleDate);

  server.onNotFound(handleNotFound);
  server.begin();
}

void HttpController::maintain() {
  server.handleClient();
}
