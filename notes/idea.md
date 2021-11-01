# Notes and Ideas

## Push buttons
* Add 4 push buttons (@@Todo: Verify we have enough digital pins (4))
  * Menu (The first button would show a menu)
  * Preset 1
  * Preset 2
  * Preset 3 (Could be used for Power Off / Cool down)

## Presets
* Implement Presets in EEPROM
* The Arduino nano has 1024 bytes of EEPROM Memory
* Writes operations are limitted to 100,000

## Menu
* The menu could allow to set the presets for each buttons
* The countdown timer could also be set there (and saved to EEPROM also).

### Menu Structure
Toggle with rotary button,  Exit with Menu button.  Menu should timeout after a few minutes/seconds.
* Regular Display
* Presets
  * Preset 1
    * Iron: 00  HeatGun: 00  Fan: 00   (Press rotary button to skip between each and save)
  * Preset 2
    * Iron: 00  HeatGun: 00  Fan: 00   (Press rotary button to skip between each and save)
  * Preset 3
    * Iron: 00  HeatGun: 00  Fan: 00   (Press rotary button to skip between each and save)
* Calibration (Future)
  * Calibrate Iron
    * Param 1
    * Param 2 
  * Calibrate HeatGun
    * Param 1
    * Param 2 
## EEPROM
* EEPROM will contain

  * Preset-1 (adress 0x00 x 6 bytes)
  * Preset-2 (adress 0x06 x 6 bytes)
  * Preset-3 (adress 0x0C x 6 bytes)
  * Shutdown Timeout (adress 0x18 x 1 byte) 
  * Maybe save the last setpoint of the soldering iron, most likely it would be the same. (or maybe not, if the user want's to use the heatgun and not the iron).
  * Future
    * The calibration offsets for the Iron and Heatgun could also be saved in the EEPROM.
    * This should be accessible from the menu.


## Cooling Fan (Noise control)
The cooling fan inside the enclosure is quite noizy and it would me nice to have it run only if needed and not necessary full blast all the time.  To do this maybe implement some kind of P.I.D controll with a thermistor of a better with a ds18b20 digital temperature sensor.

The hot parts are the Mini-360 buck converter and the mosfets and triac.  Strategically placing the temperature sensor might be tricky.

Heat sinks could also be used for the the Mosfets and Triac, but it is less cool :-)


