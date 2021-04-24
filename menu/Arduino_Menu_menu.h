/*
    The code in this file uses open source libraries provided by thecoderscorner

    DO NOT EDIT THIS FILE, IT WILL BE GENERATED EVERY TIME YOU USE THE UI DESIGNER
    INSTEAD EITHER PUT CODE IN YOUR SKETCH OR CREATE ANOTHER SOURCE FILE.

    All the variables you may need access to are marked extern in this file for easy
    use elsewhere.
 */

#ifndef MENU_GENERATED_CODE_H
#define MENU_GENERATED_CODE_H

#include <tcMenu.h>

#include <Wire.h>
#include <LiquidCrystalIO.h>
#include <IoAbstractionWire.h>
#include "tcMenuLiquidCrystal.h"

// all define statements needed
#define LCD_WIDTH 16
#define LCD_HEIGHT 2
#define I2C_ADDRESS 0x3f
#define PIN_LAYOUT EN_RW_RS
#define PULLUP_LOGIC true
#define INTERRUPT_SWITCHES true
#define SWITCH_IODEVICE 
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3
#define ENCODER_PIN_OK 4

// all variables that need exporting
extern LiquidCrystal lcd;
extern LiquidCrystalRenderer renderer;

// all menu item forward references.
extern ActionMenuItem menuHome;
extern AnalogMenuItem menuPresetsHeatGunPreset2;
extern AnalogMenuItem menuPresetsHeatGunPreset1;
extern BackMenuItem menuBackPresetsHeatGun;
extern SubMenuItem menuPresetsHeatGun;
extern AnalogMenuItem menuPresetsIronPreset2;
extern AnalogMenuItem menuPresetsIronPreset1;
extern BackMenuItem menuBackPresetsIron;
extern SubMenuItem menuPresetsIron;
extern BackMenuItem menuBackPresets;
extern SubMenuItem menuPresets;
extern AnalogMenuItem menuGunCooldownMinFanSpeed;
extern AnalogMenuItem menuGunCooldownCoolToTemp;
extern BackMenuItem menuBackGunCooldown;
extern SubMenuItem menuGunCooldown;
extern AnalogMenuItem menuIdleTimeWarningDelay;
extern BooleanMenuItem menuIdleTimeWarningBeep;
extern AnalogMenuItem menuIdleTimeTimeout;
extern BackMenuItem menuBackIdleTime;
extern SubMenuItem menuIdleTime;
extern AnalogMenuItem menuMaxTempsGun;
extern AnalogMenuItem menuMaxTempsIron;
extern BackMenuItem menuBackMaxTemps;
extern SubMenuItem menuMaxTemps;
extern const ConnectorLocalInfo applicationInfo;

// Callback functions must always include CALLBACK_FUNCTION after the return type
#define CALLBACK_FUNCTION

void CALLBACK_FUNCTION onHome(int id);

void setupMenu();

#endif // MENU_GENERATED_CODE_H