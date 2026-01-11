#pragma once
#include <stdint.h>

namespace Ota {

void request();
void update();
bool active();
uint32_t secondsLeft();
const char* hostname();

} // namespace Ota

