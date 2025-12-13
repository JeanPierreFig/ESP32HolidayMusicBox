// ============================================================================
// DACController.cpp
// ============================================================================
#include "DACController.h"
#include <Arduino.h>

DACController::DACController() {
}

bool DACController::begin() {

    Serial.println("\n=== Initializing DAC System ===");
    
    // Initialize I2C
    // Uses I2C_SDA (47) and I2C_SCL (48) from DACController.h
    Wire.begin(I2C_SDA, I2C_SCL);
    delay(100);
    
    // Initialize codec using Adafruit library with hardware reset pin
    // Uses DAC_RST_PIN (7) from DACController.h
    Serial.println("Initializing codec with hardware reset...");
    if (!codec.begin()) {
        Serial.println("ERROR: Failed to initialize TLV320DAC3100!");
        Serial.println("Check wiring:");
        Serial.printf("  RST pin: GPIO %d to DAC RST\n", DAC_RST_PIN);
        Serial.printf("  I2C SDA: GPIO %d to DAC SDA\n", I2C_SDA);
        Serial.printf("  I2C SCL: GPIO %d to DAC SCL\n", I2C_SCL);
        return false;
    }
    
    Serial.println("✓ TLV320DAC3100 initialized with hardware reset");
    
    // Configure the DAC with proper settings
    if (!configureDAC()) {
        Serial.println("ERROR: Failed to configure DAC!");
        return false;
    }
    
    // Uses BCLK_PIN (9), LRCK_PIN (10), DOUT_PIN (11), I2C_SDA (47), I2C_SCL (48), and DAC_RST_PIN (7) from DACController.h
    Serial.printf("✓ I2S Pins: BCLK=%d, WSEL=%d, DIN=%d\n", BCLK_PIN, LRCK_PIN, DOUT_PIN);
    Serial.printf("✓ I2C Pins: SDA=%d, SCL=%d\n", I2C_SDA, I2C_SCL);
    Serial.printf("✓ Reset Pin: GPIO %d\n", DAC_RST_PIN);
    Serial.println("=== DAC System Ready ===\n");

    return true;
}

bool DACController::configureDAC() {
    Serial.println("Configuring TLV320DAC3100...");
    
    // Reset codec (hardware reset already done in begin())
    codec.reset();
    delay(100);
    
    Serial.println("  [1/6] Configuring codec interface...");
    if (!codec.setCodecInterface(TLV320DAC3100_FORMAT_I2S, TLV320DAC3100_DATA_LEN_16)) {
        Serial.println("Failed to configure codec interface!");
        return false;
    }
    
    Serial.println("  [2/6] Configuring codec clocks...");
    if (!codec.setCodecClockInput(TLV320DAC3100_CODEC_CLKIN_PLL) ||
        !codec.setPLLClockInput(TLV320DAC3100_PLL_CLKIN_BCLK)) {
        Serial.println("Failed to configure codec clocks!");
        return false;
    }
    
    Serial.println("  [3/6] Configuring PLL...");
    if (!codec.setPLLValues(1, 1, 8, 0)) {
        Serial.println("Failed to configure PLL values!");
        return false;
    }
    
    Serial.println("  [4/6] Configuring DAC dividers...");
    if (!codec.setNDAC(true, 8) ||
        !codec.setMDAC(true, 2) ||
        !codec.setDOSR(128)) {
        Serial.println("Failed to configure DAC dividers!");
        return false;
    }
    
    if (!codec.powerPLL(true)) {
        Serial.println("Failed to power up PLL!");
        return false;
    }
    
    Serial.println("  [5/6] Configuring DAC path...");
    if (!codec.setDACDataPath(true, true, 
                             TLV320_DAC_PATH_NORMAL,
                             TLV320_DAC_PATH_NORMAL,
                             TLV320_VOLUME_STEP_1SAMPLE)) {
        Serial.println("Failed to configure DAC data path!");
        return false;
    }
    
    if (!codec.configureAnalogInputs(TLV320_DAC_ROUTE_MIXER,
                                     TLV320_DAC_ROUTE_MIXER,
                                     false, false, false,
                                     false)) {
        Serial.println("Failed to configure DAC routing!");
        return false;
    }
    
    Serial.println("  [6/6] Setting volume and powering up speaker...");
    if (!codec.setDACVolumeControl(
            false, false, TLV320_VOL_INDEPENDENT) ||
        !codec.setChannelVolume(false, 18) ||
        !codec.setChannelVolume(true, 18)) {
        Serial.println("Failed to configure DAC volume control!");
        return false;
    }
    
    if (!codec.setChannelVolume(false, 5.0) ||
        !codec.setChannelVolume(true, 5.0)) {
        Serial.println("Failed to set DAC channel volumes!");
        return false;
    }
    
    if (!codec.enableSpeaker(true) ||
        !codec.configureSPK_PGA(TLV320_SPK_GAIN_6DB,
                                true) ||
        !codec.setSPKVolume(true, 0)) {
        Serial.println("Failed to configure speaker output!");
        return false;
    }
    
    delay(100);
    
    Serial.println("✓ DAC Configuration Complete!");
    Serial.println("\nHardware Checklist (Based on Example Wiring):");
    Serial.println("  □ Board 5V to DAC VIN (power the DAC with 5V, NOT 3.3V!)");
    Serial.println("  □ Board GND to DAC GND");
    Serial.printf("  □ Board SCL to DAC SCL (I2C clock) **(GPIO %d)**\n", I2C_SCL);
    Serial.printf("  □ Board SDA to DAC SDA (I2C data) **(GPIO %d)**\n", I2C_SDA);
    Serial.printf("  □ Board GPIO %d to DAC BCK (I2S bit clock)\n", BCLK_PIN);
    Serial.printf("  □ Board GPIO %d to DAC WSEL (I2S word select)\n", LRCK_PIN);
    Serial.printf("  □ Board GPIO %d to DAC DIN (I2S data)\n", DOUT_PIN);
    Serial.printf("  □ Board GPIO %d to DAC RST (hardware reset)\n", DAC_RST_PIN);
    Serial.println("  □ DAC JST-PH to speaker (4-8Ω speaker)\n");
    
    return true;
}



