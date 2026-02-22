#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <GyverButton.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <lvgl.h>
#include <ui.h>
#include <string>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h>
#include <qrcode.h>

// ==================== КОНФИГУРАЦИЯ ====================
// WiFi
#define WIFI_SSID "Temur"
#define WIFI_PASSWORD "11111111"

// MQTT
#define MQTT_SERVER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_TOPIC "payments/"

// Сервер
#define SERVER_URL "/perfume-api"
#define DEVICE_ID "perfume_001"

// ==================== EXTERN ПЕРЕМЕННЫЕ ====================
extern Servo servo1;
extern Servo servo2;
extern Servo servo3;
extern Servo servo4;

void servo_c1 ();
void servo_c2 ();
void servo_c3 ();
void servo_c4 ();
extern TFT_eSPI tft;
extern Preferences prefs;

extern const char* ssid;
extern const char* password;
extern const char* mqtt_server;
extern const char* mqtt_topic;

extern WiFiClient espClient;
extern PubSubClient client;

extern GButton button1;
extern GButton button2;
extern GButton button3;
extern GButton button4;
extern GButton button1_count;
extern GButton button2_count;
extern GButton button3_count;
extern GButton button4_count;
extern GButton button_cancel;

// Состояние
extern unsigned long payment_time;
extern int parfum_num;
extern bool orderPending;
extern bool paymentReceived;
extern bool isDone;
extern bool error_mode;
extern bool servo_cflag;
extern byte counts;
extern bool countsflag;
extern bool block;
// Цены парфюмов (загружаются с сервера)
extern int tom_ford1;
extern int lanvin2;
extern int dior3;
extern int dolce_gabbana4;

// Имена парфюмов для API
extern const char* NAME_TOM;
extern const char* NAME_LANVIN;
extern const char* NAME_DIOR;
extern const char* NAME_DOLCE;

// Таймер pollPrices
extern unsigned long lastPoll;
extern unsigned long timer1;
extern unsigned long timer2;
extern unsigned long timer3;
extern bool FLAG;
extern bool flag_time;
// LVGL буфер
static lv_color_t buf1[320 * 20];

// ==================== ФУНКЦИИ ====================
// TFT
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
extern String qr_url1;
// Кнопки
void buttons_control();
void parfum_mode(int param);

// WiFi & MQTT
void setup_wifi();
void setup_mqtt();
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void mqtt_reconnect();
void mqtt_loop();

// QR код
void generateQrToImage(String url);
void clearQrImage();

// Заказы
void createOrder(int parfumId, int price);
void cancelOrder();

// Парфюмы
void parfum1();
void parfum2();
void parfum3();
void parfum4();
void dispenseParfum(int num);

// Загрузка цен с сервера
void pollPrices();
void updatePriceLabels();
void loadFromPrefs();
void saveToPrefs();

#endif
