
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Servo.h>
#include <WiFiManager.h> 
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h> 
#include <max6675.h>
#include <Button2.h>
#include <ESPRotary.h>

static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;

// Wifi network station credentials

#define BOT_TOKEN "00000000:sdgdghdfhfhfghfghfghfghfghfh"
const unsigned long BOT_MTBS = 800; // mean time between scan messages

unsigned long bot_lasttime; // last time messages' scan has been done
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
Servo servo1;
Servo servo2;
WiFiManager wifiManager;

int ktcCLK = PIN_SPI_SCK; //D5
int ktcCS = PIN_SPI_MOSI; //D7
int ktcSO = PIN_SPI_MISO; //D6
#define CLICKS_PER_STEP  1

MAX6675 ts(ktcCLK, ktcCS, ktcSO);


const String keyboard = "[[\"/servo1 0\",\"/servo1 90\",\"/servo1 180\"],[\"/servo2 0\",\"/servo2 90\",\"/servo2 180\"]]";
void handleNewMessages(int numNewMessages)
{
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);
  
  String answer;
  for (int i = 0; i < numNewMessages; i++)
  {

    telegramMessage &msg = bot.messages[i];
    Serial.println("FromId: " + bot.messages[i].from_id);
    Serial.println("msg.type: " + bot.messages[i].type);
   

    Serial.println("Received " + msg.text);
    
    if (msg.text == "/help")
      answer = "So you need _help_, uh? me too! use /start or /status";
    else if (msg.text == "/start")
      answer = " Welcome my friend! My ip is: " + WiFi.localIP().toString();
      else if (msg.text.indexOf("/servo1") == 0){
            msg.text.replace("/servo1","");
            int pos = msg.text.toInt();
            servo1.write(pos);
            answer = "Servo1 set to: " + String(servo1.read());
      }
        else if (msg.text.indexOf("/servo2") == 0){
            msg.text.replace("/servo2","");
            int pos = msg.text.toInt();
            servo2.write(pos);
            answer = "Servo 2 set to: " + String(servo2.read());
      }  
    else
      answer = "Say what?";
  

   
    bot.sendMessageWithReplyKeyboard(msg.chat_id, answer, "", keyboard);
  }
}



void bot_setup()
{

  const String commands = F("["
                            "{\"command\":\"help\",  \"description\":\"Ha ha!\"},"
                            "{\"command\":\"start\", \"description\":\"Message sent when you open a chat with a bot\"},"
                            "]");

                          
  bot.setMyCommands(commands);
  //bot.sendMessage("25235518", "Hola amigo!", "Markdown");
}

 
   ESPRotary r = ESPRotary(D2, D1, CLICKS_PER_STEP, 0, 60);
   Button2 b = Button2(D4);
   
void setup()
{


    servo1.attach(D3);
    servo1.write(0);
//    servo2.attach(D8); 
//    servo2.write(0);


    r.setChangedHandler(rotate);
    b.setTapHandler(click);
    b.setLongClickHandler(resetPosition);


    
   
  Serial.begin(115200);
  Serial.println();

//  WiFiManager wifiManager;
//  wifiManager.autoConnect("HiStranger!");

  
  
  configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
  secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  // Check NTP/Time, usually it is instantaneous and you can delete the code below.
//  Serial.print("Retrieving time: ");
//  time_t now = time(nullptr);
//  while (now < 24 * 3600)
//  {
//    Serial.print(".");
//    delay(750);
//    now = time(nullptr);
//  }
//  Serial.println(now);

  
  bot_setup();

}


// on change
void rotate(ESPRotary& r) {
   Serial.println(r.getPosition());
   servo1.write(r.getPosition()*3);
}

// single click
void click(Button2& btn) {
  Serial.println("temperature: " + (String)ts.readCelsius());
}

// long click
void resetPosition(Button2& btn) {
  r.resetPosition();
  
  Serial.println("Reset!");
}



void loop()
{

  r.loop();
  b.loop();
  
  if (millis() - bot_lasttime > BOT_MTBS)
  {

    
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }
}
