#include<Globals.h>

void servo_c1 () {
    if (button1_count.isSingle() && counts > 0) {
        counts--;
        servo1.write(90);
        delay(300);
        servo1.write(0);
    }
}
void servo_c2 () {
    if (button2_count.isSingle() && counts > 0) {
        counts--;
        servo1.write(90);
        delay(300);
        servo1.write(0);
    }
}
void servo_c3 () {
    if (button3_count.isSingle() && counts > 0) {
        counts--;
        servo1.write(90);
        delay(300);
        servo1.write(0);
    }
}
void servo_c4 () {
    if (button4_count.isSingle() && counts > 0) {
        counts--;
        servo1.write(90);
        delay(300);
        servo1.write(0);
    }
}