#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include "ArduinoJson.h"

#include "WebSecret.h"

#define DISPLAY_SDA 2
#define DISPLAY_SCL 0

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


const unsigned char btc_logo [] PROGMEM = {

  0x00, 0x0f, 0xf0, 0x00, 0x00, 0x7f, 0xfe, 0x00, 0x01, 0xff, 0xff, 0x80, 0x03, 0xff, 0xff, 0xc0,
  0x07, 0xff, 0xff, 0xe0, 0x0f, 0xff, 0xff, 0xf0, 0x1f, 0xfe, 0x5f, 0xf8, 0x3f, 0xfe, 0x4f, 0xfc,
  0x3f, 0xe0, 0xcf, 0xfc, 0x7f, 0xe0, 0x0f, 0xfe, 0x7f, 0xf8, 0x07, 0xfe, 0x7f, 0xf8, 0xc1, 0xfe,
  0xff, 0xf0, 0xe1, 0xff, 0xff, 0xf1, 0xf1, 0xff, 0xff, 0xf1, 0xe1, 0xff, 0xff, 0xf0, 0x03, 0xff,
  0xff, 0xe1, 0x07, 0xff, 0xff, 0xe3, 0xc3, 0xff, 0xff, 0xe3, 0xe1, 0xff, 0xff, 0x03, 0xe1, 0xff,
  0x7f, 0x01, 0xc3, 0xfe, 0x7f, 0xe0, 0x03, 0xfe, 0x7f, 0xe4, 0x07, 0xfe, 0x3f, 0xe4, 0xff, 0xfc,
  0x3f, 0xec, 0xff, 0xfc, 0x1f, 0xfc, 0xff, 0xf8, 0x0f, 0xff, 0xff, 0xf0, 0x07, 0xff, 0xff, 0xe0,
  0x03, 0xff, 0xff, 0xc0, 0x01, 0xff, 0xff, 0x80, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0x0f, 0xf0, 0x00
};

const unsigned char eth_logo [] PROGMEM = {
  0x00, 0x0f, 0xf0, 0x00, 0x00, 0x7f, 0xfe, 0x00, 0x01, 0xff, 0xff, 0x80, 0x03, 0xff, 0xff, 0xc0,
  0x07, 0xff, 0xff, 0xe0, 0x0f, 0xff, 0xff, 0xf0, 0x1f, 0xfe, 0x7f, 0xf8, 0x3f, 0xfe, 0x7f, 0xfc,
  0x3f, 0xfc, 0x3f, 0xfc, 0x7f, 0xf8, 0x1f, 0xfe, 0x7f, 0xf8, 0x1f, 0xfe, 0x7f, 0xf0, 0x0f, 0xfe,
  0xff, 0xf0, 0x0f, 0xff, 0xff, 0xe1, 0x87, 0xff, 0xff, 0xc4, 0x23, 0xff, 0xff, 0xe0, 0x07, 0xff,
  0xff, 0xc0, 0x03, 0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xd8, 0x1b, 0xff, 0xff, 0xe6, 0x67, 0xff,
  0x7f, 0xe3, 0xc7, 0xfe, 0x7f, 0xf0, 0x0f, 0xfe, 0x7f, 0xf8, 0x1f, 0xfe, 0x3f, 0xfc, 0x3f, 0xfc,
  0x3f, 0xfc, 0x3f, 0xfc, 0x1f, 0xfe, 0x7f, 0xf8, 0x0f, 0xff, 0xff, 0xf0, 0x07, 0xff, 0xff, 0xe0,
  0x03, 0xff, 0xff, 0xc0, 0x01, 0xff, 0xff, 0x80, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0x0f, 0xf0, 0x00
};

const int logo_size_h = 32;
const int logo_size_w = 32;

const char *ssid = WIFI_SSID;
const char *pass = WIFI_PASS;
const char *server_host = "economia.awesomeapi.com.br";
const char *fingerprint = FINGERPRINT;

void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.println();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected to: ");
  Serial.print(ssid);
  Serial.print(" IP address: ");
  Serial.println(WiFi.localIP());

  Wire.begin(DISPLAY_SDA, DISPLAY_SCL); //INICIALIZA A BIBLIOTECA WIRE

  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.setTextColor(WHITE);
  display.clearDisplay();
}

void loop() {
  display.clearDisplay();

  const String payload = payloadRequestSecure();

  StaticJsonDocument<1024> json;

  deserializeJson(json, payload);

  String btcUSD;
  btcUSD.reserve(512);

  serializeJson(json["BTCUSD"], btcUSD);

  drawCoin(btcUSD);

  String ethUSD;
  ethUSD.reserve(512);

  serializeJson(json["ETHUSD"], ethUSD);

  drawCoin(ethUSD);


  Serial.println(payload);
  Serial.println(btcUSD);
  Serial.println(ethUSD);

  delay(100); // New Request in 10 seconds

}

void drawCoin(String payload) {

  StaticJsonDocument<512> json;

  deserializeJson(json, payload);

  String create_date = json["create_date"];
  String code = json["code"];
  String codeIn = json["codein"];
  String high = json["high"];
  String low = json["low"];
  String pctChange = json["pctChange"];
  String ask = json["ask"];


  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(create_date);
  display.display();
  delay(1500);

  display.setCursor(logo_size_w + 8, 18);
  display.setTextSize(1);
  display.print(String(code) + " to " + String(codeIn));
  display.display();
  display.setCursor(logo_size_w + 8, 30);
  display.setTextSize(1);
  display.print("$: " + String(ask));
  display.display();

  display.setCursor(logo_size_w + 8, 40);
  display.setTextSize(1);
  display.print("%: " + String(pctChange) );
  display.display();



  display.setCursor(logo_size_w + 80, 40);
  display.setTextSize(1);

  int arrow_up = 0x18;
  int arrow_down = 0x19;

  if (String(pctChange).toFloat() > 0) {
    display.write(arrow_up);
  }
  else {
    display.write(arrow_down);
  }


  display.display();

  if (code == "BTC") {
    display.drawBitmap(0, 18, btc_logo, logo_size_w, logo_size_h, 1);
  }
  else if (code == "ETH") {
    display.drawBitmap(0, 18, eth_logo, logo_size_w, logo_size_h, 1);
  }

  display.display();
  delay(3500);
  display.clearDisplay();
}




String payloadRequestSecure() {
  String line;
  int r = 0;

  WiFiClientSecure https;

  https.setFingerprint(fingerprint);
  https.setTimeout(5000); // 15 Seconds
  delay(1000);

  Serial.print("Try connecting");
  int retryCounter = 0;
  while ((!https.connect(server_host, 443)) && (retryCounter < 30)) { // 30 trys of connection
    delay(100);
    Serial.print(".");
    retryCounter++;
  }
  if (r >= 30) {
    Serial.println("Connection failed");
  }
  else {
    Serial.println("Connected, starting request.");

    https.print("GET /last/BTC-USD,ETH-USD"); https.print(" HTTP/1.1\r\n");
    https.print("Host: "); https.print(server_host); https.print("\r\n");
    https.print("Content-Type: application/json"); https.print("\r\n");
    https.print("Connection: close"); https.print("\r\n\r\n");

    Serial.println("request sent");

    while (https.connected()) {
      String line = https.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("headers received");
        break;
      }
    }

    Serial.println("Response payload >>>>");
    Serial.println();

    while (https.available()) {

      line = https.readStringUntil('\n');

      if (line.indexOf("{") > -1) {
        Serial.println(line); //Print response
        break;
      }
    }
    Serial.println();
    Serial.println("<<<<.");
    Serial.println("End of connection.");
  }

  return line;
}
