#ifndef Pushsafer_h
#define Pushsafer_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Client.h>

#define HOST "www.pushsafer.com"
/*#define SSL_PORT 443*/
#define PORT 80

struct PushSaferInput{
  String message;
  String title;
  String sound;
  String vibration;
  String icon;
  String iconcolor;
  String device;
  String url;
  String urlTitle;
  String time2live;
  String priority;
  String retry;
  String expire;
  String confirm;
  String answer;
  String answeroptions;
  String answerforce;
  String picture;
  String picture2;
  String picture3;
};

class Pushsafer
{
  public:
    Pushsafer (String key, Client &client);
    String sendEvent(struct PushSaferInput input);
    bool debug = false;

  private:
    //JsonObject * parseUpdates(String response);
    String buildString(String boundary, String name, String value);

    String _key;
    Client *client;
};

#endif