#include <Globals.h>

// ==================== ОБЪЕКТЫ ====================
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
TFT_eSPI tft;
Preferences prefs;

WiFiClient espClient;
PubSubClient client;

// WiFi & MQTT настройки
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* mqtt_server = MQTT_SERVER;
const char* mqtt_topic = MQTT_TOPIC;

// Кнопки
GButton button1(22);
GButton button2(23);
GButton button3(32);
GButton button4(33);
GButton button1_count(19);
GButton button2_count(21);
GButton button3_count(26);
GButton button4_count(27);
GButton button_cancel(25);

// Состояние
unsigned long payment_time = 0;
int parfum_num = 0;
bool orderPending = false;
bool paymentReceived = false;
bool isDone = false;
bool error_mode = false;
short wait_sec_var = 9000;
unsigned long wait_sec_timer = 0;
bool wait_flag = false;
bool wait_flag1 = true;
bool wait_flag2 = true;
bool servo_cflag = true;
String qr_url1;
byte counts = 0;
bool countsflag = true;
bool block = true;
bool FLAG = true;
bool flag_time = true;
// Цены парфюмов (загружаются с сервера)
int tom_ford1 = 5000;
int lanvin2 = 6000;
int dior3 = 7000;
int dolce_gabbana4 = 8000;

// Для pollPrices
unsigned long lastPoll = 0;
const char* NAME_TOM = "Tom Ford";
const char* NAME_LANVIN = "Lanvin";
const char* NAME_DIOR = "Dior";
const char* NAME_DOLCE = "Dolce Gabbana";

unsigned long timer1 = 0;
unsigned long timer2 = 0;
unsigned long timer3 = 0;
// ==================== SETUP ====================
void setup() {
    Serial.begin(115200);
    Serial.println("\n🚀 Perfume Automat Starting...");
    
    // TFT
    tft.init();
    tft.setSwapBytes(true);
    tft.setRotation(0);
    
    // LVGL
    lv_init();
    
    static lv_disp_draw_buf_t draw_buf;
    lv_disp_draw_buf_init(&draw_buf, buf1, NULL, 320 * 20);
    
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 320;
    disp_drv.ver_res = 480;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
    
    ui_init();
    
    // Сервоприводы
    servo1.attach(18);
    servo2.attach(-1);
    servo3.attach(-1);
    servo4.attach(-1);
    
    // Загружаем цены из памяти
    loadFromPrefs();
    
    // Показываем загрузку
    lv_scr_load(ui_Screen4);
    
    // WiFi
    setup_wifi();
    
    // MQTT
    setup_mqtt();
    
    // Загружаем цены с сервера
    pollPrices();
    updatePriceLabels();
    lv_scr_load(ui_Screen1);
    
    Serial.println("✅ Setup complete!");
}

// ==================== LOOP ====================
void loop() {
    // MQTT
    mqtt_loop();
    // LVGL
    static uint32_t last_tick = 0;
    uint32_t now = millis();
    lv_tick_inc(now - last_tick);
    last_tick = now;
    lv_timer_handler();
    if (error_mode == false) {
    lv_label_set_text(ui_Label1, std::to_string(counts).c_str());
    // Таймаут оплаты (5 минут)
    if (orderPending && (millis() - payment_time > 300000)) {
          if (!wait_flag) {
        wait_flag1 = false;
        lv_scr_load(ui_Screen7);
        lv_label_set_text(ui_Label5, "9");
        wait_sec_timer = millis();
        wait_flag = true;
    }
        Serial.println("⏰ Payment timeout!");
        if (millis() - wait_sec_timer > 1000) {
        wait_sec_var -= 1000;
        lv_label_set_text(ui_Label5, std::to_string(wait_sec_var / 1000).c_str());
        wait_sec_timer = millis();
  }
        if (wait_sec_var == 0) {
        wait_flag1 = true;
        wait_flag = false;
        cancelOrder();
        lv_scr_load(ui_Screen1);
        wait_sec_var = 9000;
        }
        if (button1.isSingle() || button2.isSingle() || button3.isSingle() || button4.isSingle() || button_cancel.isSingle()) {
        wait_flag1 = true;
        wait_flag = false;
        wait_sec_var = 9000;
        payment_time = millis();
        generateQrToImage(qr_url1);
        lv_scr_load(ui_Screen2);

  }
    }
    // Обновляем цены каждые 30 секунд
    if (millis() - lastPoll > 30000) {
        pollPrices();
        lastPoll = millis();
    }
    
    // Кнопки
    button1.tick();
    button2.tick();
    button3.tick();
    button4.tick();
    button_cancel.tick();
    button1_count.tick();
    button2_count.tick();
    button3_count.tick();
    button4_count.tick();
    if (wait_flag1) {
    // Обработка состояний парфюма
    if (block) {
    switch (parfum_num) {
        case 0:lv_scr_load(ui_Screen1);isDone = false;break;
        case 1: parfum1(); break;
        case 2: parfum2(); break;
        case 3: parfum3(); break;
        case 4: parfum4(); break;
    }}
    // Оплата получена — выдаём парфюм
    if (paymentReceived) {
        if (millis() - timer2 > 300000) {
        if (!wait_flag) {
        wait_flag2 = false;
        lv_scr_load(ui_Screen7);
        lv_label_set_text(ui_Label5, "9");
        wait_sec_timer = millis();
        wait_flag = true;
    }
    if (millis() - wait_sec_timer > 1000) {
        wait_sec_var -= 1000;
        lv_label_set_text(ui_Label5, std::to_string(wait_sec_var / 1000).c_str());
        wait_sec_timer = millis();
  }
        if (wait_sec_var == 0) {
        wait_flag2 = true;
        wait_flag = false;
        parfum_mode(0);
        counts = 0;
        lv_scr_load(ui_Screen1);
        wait_sec_var = 9000;
        }
        if (button1.isSingle() || button2.isSingle() || button3.isSingle() || button4.isSingle() || button_cancel.isSingle()) {
        wait_flag2 = true;
        wait_flag = false;
        wait_sec_var = 9000;
        timer2 = millis();
        lv_scr_load(ui_Screen1);

  }
}       if (wait_flag2) {
        block = false;
        lv_scr_load(ui_Screen1);
        switch (parfum_num)
        {
        case 1:servo_c1(); break;
        case 2:servo_c2(); break;
        case 3:servo_c3(); break;
        case 4:servo_c4(); break;
        }
        if (counts == 0) {
        paymentReceived = false;
        parfum_mode(0);
        parfum_num = 0;
        block = true;
        }
    }
}
}
buttons_control();
}
    if (counts < 0) {
    counts = 0;
    }
vTaskDelay(pdMS_TO_TICKS(5));
}
