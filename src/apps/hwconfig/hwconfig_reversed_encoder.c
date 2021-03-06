#include "HardwareConfig.h"

const struct HardwareConfigData hwconfig __attribute__((section(".hwconfig"))) = {
    .magic = HARDWARE_CONFIG_MAGIC,
    .version = HARDWARE_CONFIG_VERSION,
    .reversedEncoder = 1,
};
