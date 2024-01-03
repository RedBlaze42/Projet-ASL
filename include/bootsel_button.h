#ifndef INCLUDE_BOOTSEL_BUTTON

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"

bool get_bootsel_button();

#define INCLUDE_BOOTSEL_BUTTON
#endif