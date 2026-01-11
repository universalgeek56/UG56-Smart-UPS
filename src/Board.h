#pragma once
#include <stdint.h>

namespace Board {

// ===== ИНИЦИАЛИЗАЦИЯ =====
void begin();
void update();   // кнопка + LED

// ===== UPS (ЖЕЛЕЗО) =====
bool upsIsOn();          // чтение состояния
bool upsButtonBusy();   // кнопка сейчас нажата?
void upsPressButton();  // неблокирующее нажатие

}




