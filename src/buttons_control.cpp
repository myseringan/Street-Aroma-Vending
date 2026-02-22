#include <Globals.h>

void buttons_control() {
    // Если уже ждём оплату — только отмена работает
    if (orderPending) {
        if (button_cancel.isSingle()) {
            Serial.println("🔘 Cancel pressed");
            cancelOrder();
        }
        return;
    }
    if (block) {
    // Выбор парфюма 1 - Tom Ford
    if (button1.isSingle()) {
        Serial.println("🔘 Parfum 1 selected - Tom Ford");
        parfum_mode(1);
        timer1 = millis();
        flag_time = true;
        parfum_num = 1;
        isDone = true;
    }
    
    // Выбор парфюма 2 - Lanvin
    if (button2.isSingle()) {
        Serial.println("🔘 Parfum 2 selected - Lanvin");
        parfum_mode(2);
        parfum_num = 2;
        timer1 = millis();
        flag_time = true;
        isDone = true;
    }
    
    // Выбор парфюма 3 - Dior
    if (button3.isSingle()) {
        Serial.println("🔘 Parfum 3 selected - Dior");
        parfum_mode(3);
        parfum_num = 3;
        timer1 = millis();
        flag_time = true;
        isDone = true;
    }
    
    // Выбор парфюма 4 - Dolce Gabbana
    if (button4.isSingle()) {
        Serial.println("🔘 Parfum 4 selected - Dolce Gabbana");
        parfum_mode(4);
        parfum_num = 4;
        timer1 = millis();
        flag_time = true;
        isDone = true;
    }
    
    if (button_cancel.isSingle()) {
        Serial.println("🔘 Cancel pressed");
        parfum_num = 0;
        parfum_mode(0);
    }
}
}

void parfum_mode(int param) {
    lv_obj_t* images[] = {ui_Image3, ui_Image2, ui_Image5, ui_Image4};
    int total_images = 4;

    for (int i = 0; i < total_images; i++) {
        if (i == (param - 1)) {
            lv_obj_add_state(images[i], LV_STATE_PRESSED);
        } else {
            lv_obj_clear_state(images[i], LV_STATE_PRESSED);
        }
    }
}

void parfum1 () {
if (isDone == true && (millis() - timer1 >= 3000)) {
        payment_time = millis();
        if (flag_time) {
            lv_scr_load(ui_Screen4);
            timer3 = millis();
            flag_time = false;
            FLAG = true;
        }
        if (FLAG) {
        if (millis() - timer3 > 3000){
        createOrder(1, tom_ford1);  // Цена Tom Ford
        FLAG = false;
        }
    }
        }
}
void parfum2 () {
if (isDone == true && (millis() - timer1 >= 3000)) {
    payment_time = millis();
        if (flag_time) {
            lv_scr_load(ui_Screen4);
            timer3 = millis();
            flag_time = false;
            FLAG = true;
        }
        if (FLAG) {
    if (millis() - timer3 > 3000){
    createOrder(2, lanvin2);  // Цена Lanvin
    FLAG = false;
    }
}
}
}
void parfum3 () {
if (isDone == true && (millis() - timer1 >= 3000)) {
    payment_time = millis();
        if (flag_time) {
            lv_scr_load(ui_Screen4);
            timer3 = millis();
            flag_time = false;
            FLAG = true;
        }
        if (FLAG) {
    if (millis() - timer3 > 3000){
    createOrder(3, dior3);  // Цена Dior
    FLAG = false;
    }
}
}
}
void parfum4 () {
if (isDone == true && (millis() - timer1 >= 3000)) {
    payment_time = millis();
        if (flag_time) {
            lv_scr_load(ui_Screen4);
            timer3 = millis();
            flag_time = false;
            FLAG = true;
        }
        if (FLAG) {
    if (millis() - timer3 > 3000){
    createOrder(4, dolce_gabbana4);  // Цена Dolce Gabbana
    FLAG = false;
    }
}
}
}