//#include "BleKeyboard.h"
#include <Arduino_JSON.h>
#include <WebSocketsClient_Generic.h>
#include <ArduinoWebsockets.h>
#include <arduino-timer.h>
#include <M5StickC.h>
#include "AXP192.h"
#include <WiFi.h>

int battery = 100;
float batVoltage = 0.000;
float refVoltage = 4.090;
bool btdevice = false;
//BleKeyboard bleKeyboard;
auto timer = timer_create_default();
using namespace websockets;

WebsocketsClient client;

const char* ssid = "2.4GHz-BHDes";
const char* password = "7ubie473";
const String player = "Bora";
const char* websockets_server_host = "192.168.4.28";
const uint16_t websockets_server_port = 80;

uint16_t emosnd = 1;

void prtxtlcd(String inpt) {    
    M5.Axp.SetLDO2(true);
    M5.Axp.SetLDO3(true);
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setCursor(0, 0, 1);
    M5.Lcd.println("\r\n");
    M5.Lcd.setTextColor(TFT_YELLOW);
    battery = getBatteryLevel();
    M5.Lcd.print("Battery: "); M5.Lcd.print(battery); M5.Lcd.print("% ");
    M5.Lcd.print("\r\nSOCKET STATUS: "); M5.Lcd.println(btdevice);
    M5.Lcd.setTextColor(TFT_MAGENTA);
    M5.Lcd.print("\r\nPLAYER: "); M5.Lcd.println(player);
    M5.Lcd.println("\r\n");
    M5.Lcd.setTextColor(TFT_GREEN);
    M5.Lcd.println(inpt);
}

void setup() {
    Serial.begin(115200);
    //Serial.println("Starting BLE work!");
    M5.begin();
    M5.Lcd.setRotation(3);
    //bleKeyboard.begin();
    timer.every(5000, displayOff);
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
    prtxtlcd(String(WiFi.localIP()));
    client.addHeader("Sec-WebSocket-Protocol", "echo-protocol");
    btdevice = client.connect(websockets_server_host, websockets_server_port, "/");

    if (btdevice) {
        Serial.println("Connected!");

        JSONVar myArray;
        myArray[0] = "playerjoin";
        myArray[1] = player;

        client.send(JSON.stringify(myArray));
        prtxtlcd("SOCKET CONNECTED");
    }
    else {
        Serial.println("Not Connected!");
    }

    client.onMessage([&](WebsocketsMessage message) {
            //Serial.print("Got Message: ");
            //Serial.println(message.data());
    });

}


bool displayOff(void *) {
    batVoltage = M5.Axp.GetBatVoltage();
    battery = getBatteryLevel();
    //M5.Axp.SetSleep();
    M5.Axp.SetLDO2(false);
    M5.Axp.SetLDO3(false);
    return true;
}

int getBatteryLevel(void)
{
    float tbattery = ((batVoltage / refVoltage) * 100.000);
    Serial.println(" BAT%:" + (int)tbattery);
    if (((int)tbattery) > 100)
        tbattery = 100.000;
    return (int)tbattery;
}

void toggleEmoSound(void) {
    if (emosnd == 1 || emosnd == 2)
        emosnd++;
    else if (emosnd == 3)
        emosnd = 1;
}

String getEmoSoundName(void) {
    if (emosnd == 1)
        return "emosoundpos";
    else if (emosnd == 2)
        return "emosoundneg";
    else
        return "emosoundluck";
}

void loop() {
    //bleKeyboard.isConnected() == true ? btdevice = true : btdevice = false;     
    M5.update();    
    Serial.println(" BATV:" + String(batVoltage));
    // 0x01 long press(1s), 0x02 press
    if (M5.Axp.GetBtnPress() == 0x02)
    {
        //Serial.println("btnpress");
        toggleEmoSound();
        prtxtlcd(" EMOSOUND: " + String(emosnd));
    }

    if (M5.BtnA.wasPressed())
    {
        //Serial.println(" BTN A ");
        //if (btdevice) {
            //bleKeyboard.press(KEY_DOWN_ARROW);
            //bleKeyboard.releaseAll();
        //}
        //else {
            //prtxtlcd("Sending Poiint");
            JSONVar myArray;
            myArray[0] = "point";
            myArray[1] = player;
            client.send(JSON.stringify(myArray));
        //}
    }

    if (M5.BtnB.wasPressed())
    {
        JSONVar myArray;
        myArray[0] = getEmoSoundName();
        myArray[1] = player;
        client.send(JSON.stringify(myArray));
        //}
    }
    timer.tick();
    if (client.available()) {
        client.poll();
    }    
    delay(200);
}
