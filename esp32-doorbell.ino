#include <WiFi.h>
#include <OneButton.h>
#include "freertos/event_groups.h"
#include <Wire.h>
#include "esp_camera.h"
#include "esp_wifi.h"


#define ENABLE_SSD1306
//#define SOFTAP_MODE       //The comment will be connected to the specified ssid

#define WIFI_SSID   "your-wifi-ssid"
#define WIFI_PASSWD "your-wifi-password"

#define PWDN_GPIO_NUM 26
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 32
#define SIOD_GPIO_NUM 13
#define SIOC_GPIO_NUM 12

#define Y9_GPIO_NUM 39
#define Y8_GPIO_NUM 36
#define Y7_GPIO_NUM 23
#define Y6_GPIO_NUM 18
#define Y5_GPIO_NUM 15
#define Y4_GPIO_NUM 4
#define Y3_GPIO_NUM 14
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 27
#define HREF_GPIO_NUM 25
#define PCLK_GPIO_NUM 19

#define I2C_SDA 21
#define I2C_SCL 22

#ifdef ENABLE_SSD1306
#include "SSD1306.h"
#include "OLEDDisplayUi.h"
#define SSD1306_ADDRESS 0x3c
SSD1306 oled(SSD1306_ADDRESS, I2C_SDA, I2C_SCL);
OLEDDisplayUi ui(&oled);
#endif

#define AS312_PIN 33
#define BUTTON_1 34
String ip;
EventGroupHandle_t evGroup;

const char *host = "maker.ifttt.com";
const char *privateKey = "XXX";

OneButton button1(BUTTON_1, true);

void startCameraServer();

void send_event(const char *event)

{
  Serial.print("Connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("Connection failed");
    return;
  }
  
  // We now create a URI for the request
  String url = "/trigger/";
  url += event;
  url += "/with/key/";
  url += privateKey;

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  while(client.connected())
  {
    if(client.available())
    {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    } else {
      // No data yet, wait a bit
      delay(50);
    };
  }

  Serial.println();
  Serial.println("closing connection");
  client.stop();
}

void button1Func()
{
  // TODO: send button press notification
    static bool en = false;
    xEventGroupClearBits(evGroup, 1);
    sensor_t *s = esp_camera_sensor_get();
    en = en ? 0 : 1;
    s->set_vflip(s, en);
    delay(200);
    xEventGroupSetBits(evGroup, 1);
    send_event("button_pressed");
}

#ifdef ENABLE_SSD1306
void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
    display->setFont(ArialMT_Plain_16);
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->drawString(64 + x, 35 + y, ip);

    if (digitalRead(AS312_PIN)) {
        display->drawString(64 + x, 10 + y, "AS312 Trigger");
        // TODO: motion sensor notification
        send_event("movement");
    }
}

void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{

}

FrameCallback frames[] = {drawFrame1, drawFrame2};
#define FRAMES_SIZE (sizeof(frames) / sizeof(frames[0]))
#endif

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();

    pinMode(AS312_PIN, INPUT);

#ifdef ENABLE_SSD1306
    oled.init();
    oled.setFont(ArialMT_Plain_16);
    oled.setTextAlignment(TEXT_ALIGN_CENTER);
    delay(50);
    oled.drawString(oled.getWidth() / 2, oled.getHeight() / 2, "TTGO Camera");
    oled.display();
#endif

    if (!(evGroup = xEventGroupCreate())) {
        Serial.println("evGroup Fail");
        while (1);
    }
    xEventGroupSetBits(evGroup, 1);

    /*camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    //init with high specs to pre-allocate larger buffers
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;*/

    camera_config_t config;

    config.pin_pwdn = 26;
    config.pin_reset = -1;
    
    config.pin_xclk = 32;
    
    config.pin_sscb_sda = 13;
    config.pin_sscb_scl = 12;
    
    config.pin_d7 = 39;
    config.pin_d6 = 36;
    config.pin_d5 = 23;
    config.pin_d4 = 18;
    config.pin_d3 = 15;
    config.pin_d2 = 4;
    config.pin_d1 = 14;
    config.pin_d0 = 5;
    config.pin_vsync = 27;
    config.pin_href = 25;
    config.pin_pclk = 19;
    config.xclk_freq_hz = 20000000;
    config.ledc_timer = LEDC_TIMER_0;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12; //0-63 lower numbers are higher quality
    config.fb_count = 2; // if more than one i2s runs in continous mode. Use only with jpeg


    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init Fail");
#ifdef ENABLE_SSD1306
        oled.clear();
        oled.drawString(oled.getWidth() / 2, oled.getHeight() / 2, "Camera init Fail");
        oled.display();
#endif
        while (1);
    }

    //drop down frame size for higher initial frame rate
    sensor_t *s = esp_camera_sensor_get();
    s->set_framesize(s, FRAMESIZE_QVGA);

    button1.attachClick(button1Func);

#ifdef SOFTAP_MODE
    uint8_t mac[6];
    char buff[128];
    WiFi.mode(WIFI_AP);
    IPAddress apIP = IPAddress(2, 2, 2, 1);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    esp_wifi_get_mac(WIFI_IF_AP, mac);
    sprintf(buff, "TTGO-CAMERA-%02X:%02X", mac[4], mac[5]);
    Serial.printf("Device AP Name:%s\n", buff);
    if (!WiFi.softAP(buff, NULL, 1, 0)) {
        Serial.println("AP Begin Failed.");
        while (1);
    }
#else
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
#endif

#ifdef ENABLE_SSD1306
    oled.clear();
    oled.drawString(oled.getWidth() / 2, oled.getHeight() / 2, "WiFi Connected");
    oled.display();
#endif

    startCameraServer();

    delay(50);

#ifdef ENABLE_SSD1306
    ui.setTargetFPS(60);
    ui.setIndicatorPosition(BOTTOM);
    ui.setIndicatorDirection(LEFT_RIGHT);
    ui.setFrameAnimation(SLIDE_LEFT);
    ui.setFrames(frames, FRAMES_SIZE);
    ui.setTimePerFrame(6000);
    ui.init();
#endif

#ifdef SOFTAP_MODE
    ip = WiFi.softAPIP().toString();
    Serial.printf("\nAp Started .. Please Connect %s hotspot\n", buff);
#else
    ip = WiFi.localIP().toString();
#endif

    Serial.print("Camera Ready! Use 'http://");
    Serial.print(ip);
    Serial.println("' to connect");
}

void loop()
{
#ifdef ENABLE_SSD1306
    if (ui.update()) {
#endif
        button1.tick();
#ifdef ENABLE_SSD1306
    }
#endif
}
