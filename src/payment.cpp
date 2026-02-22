#include <Globals.h>
#include <qrcode.h>
#include <WiFiClientSecure.h>
// ==================== QR БУФЕР ====================

static uint8_t* qr_buffer = nullptr;

// ==================== ЗАГРУЗКА/СОХРАНЕНИЕ ЦЕН ====================
void loadFromPrefs() {
    prefs.begin("parfum", true);
    dior3 = prefs.getLong("dior", dior3);
    dolce_gabbana4 = prefs.getLong("dolce", dolce_gabbana4);
    tom_ford1 = prefs.getLong("tom", tom_ford1);
    lanvin2 = prefs.getLong("lanvin", lanvin2);
    prefs.end();
    
    Serial.println("📦 Prices loaded from memory:");
    Serial.printf("   Tom Ford: %d\n", tom_ford1);
    Serial.printf("   Lanvin: %d\n", lanvin2);
    Serial.printf("   Dior: %d\n", dior3);
    Serial.printf("   Dolce Gabbana: %d\n", dolce_gabbana4);
}

void saveToPrefs() {
    prefs.begin("parfum", false);
    prefs.putLong("dior", dior3);
    prefs.putLong("dolce", dolce_gabbana4);
    prefs.putLong("tom", tom_ford1);
    prefs.putLong("lanvin", lanvin2);
    prefs.end();
}

int findIndexByName(const String &needle, JsonArray &jnames) {
    for (int i = 0; i < (int)jnames.size(); ++i) {
        String nm = String((const char*)jnames[i]).c_str();
        String a = nm;
        String b = needle;
        a.toLowerCase();
        b.toLowerCase();
        if (a == b) return i;
    }
    return -1;
}

void pollPrices() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("❌ WiFi not connected, skip pollPrices");
        return;
    }
    
    WiFiClientSecure client;
    client.setInsecure();
    
    HTTPClient http;
    String url = String(SERVER_URL) + "/prices";
    http.begin(client, url);
    
    int code = http.GET();
    
    if (code != HTTP_CODE_OK) {
        Serial.printf("❌ GET /api/prices failed, code=%d\n", code);
        http.end();
        return;
    }

    String payload = http.getString();
    http.end();

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (err) {
        Serial.print("❌ JSON parse error: ");
        Serial.println(err.c_str());
        return;
    }

    JsonArray jnames = doc["names"].as<JsonArray>();
    JsonArray jprices = doc["prices"].as<JsonArray>();

    // Tom Ford
    int idx = findIndexByName(String(NAME_TOM), jnames);
    if (idx >= 0 && idx < (int)jprices.size()) {
        long newv = jprices[idx].as<long>();
        if (newv != tom_ford1) {
            tom_ford1 = newv;
            Serial.printf("📝 Tom Ford updated -> %d\n", tom_ford1);
        }
    }

    // Lanvin
    idx = findIndexByName(String(NAME_LANVIN), jnames);
    if (idx >= 0 && idx < (int)jprices.size()) {
        long newv = jprices[idx].as<long>();
        if (newv != lanvin2) {
            lanvin2 = newv;
            Serial.printf("📝 Lanvin updated -> %d\n", lanvin2);
        }
    }

    // Dior
    idx = findIndexByName(String(NAME_DIOR), jnames);
    if (idx >= 0 && idx < (int)jprices.size()) {
        long newv = jprices[idx].as<long>();
        if (newv != dior3) {
            dior3 = newv;
            Serial.printf("📝 Dior updated -> %d\n", dior3);
        }
    }

    // Dolce Gabbana
    idx = findIndexByName(String(NAME_DOLCE), jnames);
    if (idx >= 0 && idx < (int)jprices.size()) {
        long newv = jprices[idx].as<long>();
        if (newv != dolce_gabbana4) {
            dolce_gabbana4 = newv;
            Serial.printf("📝 Dolce Gabbana updated -> %d\n", dolce_gabbana4);
        }
    }

    saveToPrefs();

    Serial.println("✅ Current prices:");
    Serial.printf("   Tom Ford: %d\n   Lanvin: %d\n   Dior: %d\n   Dolce Gabbana: %d\n", 
                  tom_ford1, lanvin2, dior3, dolce_gabbana4);
    
    // === ОБНОВЛЯЕМ LABELS НА ЭКРАНЕ ===
    updatePriceLabels();
}

void updatePriceLabels() {
    // Обновляем Label'ы с ценами
    lv_label_set_text(ui_web1, String(tom_ford1).c_str());
    lv_label_set_text(ui_web2, String(lanvin2).c_str());
    lv_label_set_text(ui_web3, String(dior3).c_str());
    lv_label_set_text(ui_web4, String(dolce_gabbana4).c_str());
    
    Serial.println("🏷️ Price labels updated!");
}
// ==================== WiFi ====================
void setup_wifi() {
    Serial.println();
    Serial.print("📡 Connecting to WiFi: ");
    Serial.println(ssid);
    
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ WiFi connected!");
        Serial.print("📍 IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\n❌ WiFi failed!");
    }
}

// ==================== MQTT Callback ====================
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    Serial.println("\n📩 MQTT Message received!");
    
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    
    Serial.println("Message: " + message);
    
    // Парсим JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        Serial.println("❌ JSON parse error");
        return;
    }
    
    String status = doc["status"].as<String>();
    
    // === CREATED — показываем QR ===
    if (status == "created") {
        String qr_url = doc["qr_url"].as<String>();
        qr_url1 = qr_url;
        int amount = doc["amount"].as<int>();
        
        Serial.println("📝 Order created!");
        Serial.println("🔗 QR URL: " + qr_url);
        Serial.println("💰 Amount: " + String(amount));
        
        // Генерируем QR и показываем на Screen2
        isDone = false;
        generateQrToImage(qr_url);
        lv_scr_load(ui_Screen2);
        
        orderPending = true;
    }
    
    // === CONFIRMED — оплата успешна ===
    else if (status == "confirmed") {
        int amount = doc["amount"].as<int>();
        
        Serial.println("✅ Payment confirmed: " + String(amount) + " sum");
        
        clearQrImage();
        lv_scr_load(ui_Screen5);  // Экран успеха
        delay(3000);
        timer2 = millis();
        counts = 2;
        paymentReceived = true;
        orderPending = false;
    }
    
    // === CANCELLED — отмена ===
    else if (status == "cancelled") {
        Serial.println("❌ Order cancelled");
        
        clearQrImage();
        lv_scr_load(ui_Screen6);  // Экран ошибки
        
        orderPending = false;
        parfum_num = 0;
        parfum_mode(0);
    }
}

// ==================== MQTT Reconnect ====================
void mqtt_reconnect() {
    static unsigned long lastAttempt = 0;
    
    if (!client.connected() && millis() - lastAttempt > 3000) {
        Serial.print("🔌 Connecting to MQTT...");
        
        String clientId = "ESP32-Perfume-" + String(random(0xffff), HEX);
        
        if (client.connect(clientId.c_str())) {
            error_mode = false;
            lv_scr_load(ui_Screen1);
            Serial.println(" connected!");
            client.subscribe(mqtt_topic);
            Serial.print("📡 Subscribed to: ");
            Serial.println(mqtt_topic);
        } else {
            error_mode = true;
            lv_scr_load(ui_Screen3);
            Serial.print(" failed, rc=");
            Serial.println(client.state());
        }
        
        lastAttempt = millis();
    }
}

// ==================== Setup MQTT ====================
void setup_mqtt() {
    client.setClient(espClient);
    client.setServer(mqtt_server, 1883);
    client.setCallback(mqtt_callback);
    client.setBufferSize(1024);
}

// ==================== MQTT Loop ====================
void mqtt_loop() {
    if (!client.connected()) {
        mqtt_reconnect();
    }
    client.loop();
}

// ==================== Генерация QR в ui_Image10 ====================
void generateQrToImage(String url) {
    Serial.println("📱 Generating QR: " + url);
    static lv_img_dsc_t qr_img_dsc;
    // Создаём QR код
    QRCode qrcode;
    uint8_t qrcodeData[qrcode_getBufferSize(6)];
    qrcode_initText(&qrcode, qrcodeData, 6, ECC_MEDIUM, url.c_str());
    
    // Размеры области
    int imgWidth = 172;
    int imgHeight = 192;
    
    // Размер модуля QR
    int moduleSize = min(imgWidth, imgHeight) / qrcode.size;
    int qrPixelSize = qrcode.size * moduleSize;
    
    // Центрирование
    int offsetX = (imgWidth - qrPixelSize) / 2;
    int offsetY = (imgHeight - qrPixelSize) / 2;
    
    // Выделяем память (RGB565 - 2 байта на пиксель)
    size_t bufferSize = imgWidth * imgHeight * 2;
    
    if (qr_buffer != nullptr) {
        free(qr_buffer);
    }
    qr_buffer = (uint8_t*)heap_caps_malloc(bufferSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    
    // Если нет PSRAM, используем обычную память
    if (qr_buffer == nullptr) {
        qr_buffer = (uint8_t*)malloc(bufferSize);
    }
    
    if (qr_buffer == nullptr) {
        Serial.println("❌ Failed to allocate QR buffer!");
        return;
    }
    
    // Заполняем белым
    uint16_t* pixels = (uint16_t*)qr_buffer;
    for (int i = 0; i < imgWidth * imgHeight; i++) {
        pixels[i] = 0xFFFF;  // Белый
    }
    
    // Рисуем QR код (чёрные модули)
    for (uint8_t y = 0; y < qrcode.size; y++) {
        for (uint8_t x = 0; x < qrcode.size; x++) {
            if (qrcode_getModule(&qrcode, x, y)) {
                int startX = offsetX + x * moduleSize;
                int startY = offsetY + y * moduleSize;
                
                for (int py = 0; py < moduleSize; py++) {
                    for (int px = 0; px < moduleSize; px++) {
                        int idx = (startY + py) * imgWidth + (startX + px);
                        if (idx < imgWidth * imgHeight) {
                            pixels[idx] = 0x0000;  // Чёрный
                        }
                    }
                }
            }
        }
    }
    
    // Настраиваем LVGL image descriptor
    qr_img_dsc.header.always_zero = 0;
    qr_img_dsc.header.w = imgWidth;
    qr_img_dsc.header.h = imgHeight;
    qr_img_dsc.header.cf = LV_IMG_CF_TRUE_COLOR;
    qr_img_dsc.data_size = bufferSize;
    qr_img_dsc.data = qr_buffer;
    
    // Присваиваем ui_Image10
    lv_img_set_src(ui_Image10, &qr_img_dsc);
    Serial.println("✅ QR displayed on ui_Image10!");
}

// ==================== Очистка QR ====================
void clearQrImage() {
    lv_img_set_src(ui_Image10, NULL);
    
    if (qr_buffer != nullptr) {
        free(qr_buffer);
        qr_buffer = nullptr;
    }
    
    Serial.println("🗑️ QR cleared");
}

// ==================== Создание заказа на сервере ====================
void createOrder(int parfumId, int price) {
    block = false;
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("❌ No WiFi!");
        return;
    }
    
    Serial.printf("📤 Creating order: Parfum %d, Price %d\n", parfumId, price);
    
    WiFiClientSecure client;
    client.setInsecure();  // Пропускаем проверку сертификата
    
    HTTPClient http;
    http.begin(client, String(SERVER_URL) + "/create-perfume-order");
    http.addHeader("Content-Type", "application/json");
    
    JsonDocument doc;
    doc["device_id"] = DEVICE_ID;
    doc["parfum_id"] = parfumId;
    doc["amount"] = price;
    
    String jsonStr;
    serializeJson(doc, jsonStr);
    
    int httpCode = http.POST(jsonStr);
    
    if (httpCode == 200) {
        String response = http.getString();
        Serial.println("✅ Server response: " + response);
        orderPending = true;
        payment_time = millis();
    } else {
        Serial.printf("❌ HTTP Error: %d\n", httpCode);
    }
    
    http.end();
}

// ==================== Отмена заказа ====================
void cancelOrder() {
    Serial.println("🚫 Order cancelled");
    block = true;
    clearQrImage();
    orderPending = false;
    parfum_num = 0;
    parfum_mode(0);
    lv_scr_load(ui_Screen1);
}
