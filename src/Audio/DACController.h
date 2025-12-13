// ============================================================================
// DACController.h
// ============================================================================
#ifndef DAC_CONTROLLER_H
#define DAC_CONTROLLER_H

#include <Adafruit_TLV320DAC3100.h>
#include <Wire.h>

// I2S Pins for Adafruit TLV320DAC3100 (avoiding SD card pins: 21, 39, 42, 45)
static constexpr int BCLK_PIN = 9;   // Bit Clock (D9 in example)
static constexpr int LRCK_PIN = 10;   // Word Select (WSEL) (D10 in example)
static constexpr int DOUT_PIN = 11;  // Data Out to DAC (DIN on DAC) (D11 in example)

// Hardware reset pin for DAC
static constexpr int DAC_RST_PIN = 7; // Hardware reset (D7 in example)

// I2C Pins (STEMMA QT connector on Metro ESP32-S3)
static constexpr int I2C_SDA = 47;   // STEMMA QT SDA
static constexpr int I2C_SCL = 48;   // STEMMA QT SCL

// TLV320DAC3100 I2C address (handled by Adafruit library)
static constexpr uint8_t DAC_I2C_ADDR = 0x18;

class DACController {
public:
    DACController();
    
    // Initialize I2C and DAC hardware
    bool begin();
    
    // Get the codec instance
    Adafruit_TLV320DAC3100& getCodec() { return codec; }
    
private:
    Adafruit_TLV320DAC3100 codec;
    
    // Configure the DAC registers
    bool configureDAC();
};

#endif // DAC_CONTROLLER_H
