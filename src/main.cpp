// include the library code:
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SimpleKalmanFilter.h>
#include <EEPROM.h>

#define MAX_CONTDOWN_MINUTES        15
#define HEATGUN_TEMP_FAN_SHUTDOWN   60
#define MIN_HOT_HG_FAN_SPEED        50

const int PRESET_1_ADDRESS = 0X00;
const int PRESET_2_ADDRESS = 0X06;
const int PRESET_3_ADDRESS = 0X0C;

int preset_1 [3];
int preset_2 [3];
int preset_3 [3];
/*
 The circuit:
 * LCD RS pin to digital pin D12
 * LCD Enable pin to digital pin D11
 * LCD D4 pin to digital pin A5
 * LCD D5 pin to digital pin A4
 * LCD D6 pin to digital pin A3
 * LCD D7 pin to digital pin A2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K potentiometer:  ends to +5V and ground and wiper to LCD VO pin (pin 3)
 */
 // initialize the library with the numbers of the interface pins
//LiquidCrystal lcd(12, 11, A5, A4, A3, A2);
LiquidCrystal_I2C lcd(0x3f,16,2);


void ISR_2();
void processDisplay(int sI_out, int hG_out);
int processTimer();
int safeHeatGunFan(int heatGunTmp, int fanSetpoint);

const int buzzer = 11; //buzzer to arduino pin 11

//Encoder's pins
int pinA = 2;  // Connected to D3  (Rotation)
int pinB = 3;  // Connected to D4 (Rotation)
int SW = 4; // Connected to D5 (Pressing)

const int sIThermoPin = A0;  // the analog input pin for reading an amplified voltage of the soldering iron's thermocouple 
const int hGThermoPin = A1;  // the analog input pin for reading an amplified voltage of the heat gun's thermocouple
const int sIHeatingElementPin = 8; // the output pin for controlling the soldering iron's heating element
const int hGFanPWMPin = 9;  // the output pin for controlling the heat gun's fan
const int hGHeatingElementPin = 7;  // the output pin for controlling the heat gun's heating element 

int sIEncoderPosCount = 0; 
int prevSolderIronEncoderPos = 0;
int hGEncoderPosCount = 0;

int fanEncoderPosCount = 0; 
int fan_pwm_value = 0 ;

int buttonLast=HIGH;
int buttonMenuPos=0;
 
int pinALast;  
// int aVal;
boolean bCW;

int sIRawValue = 0;
int sIThermo = 0;        
int outputValue = 0;        

int hGThermo = 0;        

// New Encoder variables
int counter = 0; 
bool clk_State;
bool Last_State; 
int last_counter = 0; 
bool dt_State;  
int pushed = HIGH;


const int P_coef = 4;
const int displayRedrawCount = 1000;

// Countdown timer related
unsigned long prevMillis;    
int current_countdown = MAX_CONTDOWN_MINUTES * 60;


int cyclesCount = 0;
SimpleKalmanFilter kf = SimpleKalmanFilter(30, 30, 0.01);
SimpleKalmanFilter kf_HG = SimpleKalmanFilter(32, 32, 0.01);

 void setup() { 
   pinMode (pinA,INPUT_PULLUP); 
   pinMode (pinB,INPUT_PULLUP); 
   pinMode (SW,INPUT_PULLUP);
   attachInterrupt(digitalPinToInterrupt(pinA), ISR_2, CHANGE);
   pinMode (hGHeatingElementPin,OUTPUT);
   pinMode (sIHeatingElementPin,OUTPUT);
   /* Read Pin A
   Whatever state it's in will reflect the last position   
   */
   pinALast = digitalRead(pinA);   
   Serial.begin (115200);
   prevMillis = millis();

  // set up the LCD's number of columns and rows:
  lcd.init();
  lcd.backlight();

  // Read presets values from the EEPROM
  readPresets();
 } 

 void loop() { 
   
   // 2 step calib:
   // 1) {Xi set temperature, Yi temperature measured by an external device} :   linear fit  {80,48},{100,62},{160,92},{200,123},{300,197}
   // this gives us  0.672409 * x-8.56477,   [1] 
   // 2) {Xi set temperature, Yi temperature measured by an external device} :  linear fit  {100,136},{150,205},{200,268},{222,300}
   // this gives us 1.33191 * x + 3.48942,  [2]
   // Let's substitute [1] into [2]:  3.48942 + 1.33191 * (0.672409 * x-8.56477)  and then let's simplify => finally we have (0.895588 * x -7.91808) , [3]
   sIThermo = analogRead(sIThermoPin); // Reads the value from the specified analog pin, ADC will map input voltages between 0V and 5V  into integer values between 0 and 1023
   sIRawValue = sIThermo;
   sIThermo = kf.updateEstimate(sIThermo);
   
   sIThermo = (int) (0.833333 *sIThermo - 152); // use [3]  // linear fit {{216, 28}, {402, 183}}

   // {Xi set temperature, Yi temperature measured by an external device}:  linear fit {40,52},{80,95},{100,125},{160,195}
   //1.202 * x+2.56
   hGThermo = analogRead(hGThermoPin); // Reads the value from the specified analog pin, ADC will map input voltages between 0V and 5V  into integer values between 0 and 1023
   hGThermo = kf_HG.updateEstimate(hGThermo);
   hGThermo = (int)(2.62026 *hGThermo + 26.3331);

   int setPointDiff;  // Used for the 3 differents setPoints

   //thermostats are implemented by using an on-off controller
   //thermostat 1
   setPointDiff = sIEncoderPosCount - sIThermo;
   setPointDiff = setPointDiff > 0 ? setPointDiff : 0;
   
   int sI_out;
   
   if (setPointDiff) 
     sI_out = HIGH;
   else
     sI_out = LOW;
   
   digitalWrite(sIHeatingElementPin,sI_out);

   //thermostat 2
   setPointDiff = hGEncoderPosCount - hGThermo;
   setPointDiff = setPointDiff > 0 ? setPointDiff : 0;
   int hG_out;
   if (setPointDiff) 
     hG_out = HIGH;
   else
     hG_out = LOW;

   digitalWrite(hGHeatingElementPin,hG_out);

   //setting fan speed
  //  setPointDiff = fanEncoderPosCount;
   fanEncoderPosCount = fanEncoderPosCount > 0 ? fanEncoderPosCount : 0;
   fanEncoderPosCount = fanEncoderPosCount < 255 ? fanEncoderPosCount : 255;

   // Make sure the fan is on if the heatgun is hot.
  //  analogWrite(hGFanPWMPin,fanEncoderPosCount);
  fan_pwm_value = safeHeatGunFan(hGThermo, fanEncoderPosCount);
  analogWrite(hGFanPWMPin,fan_pwm_value);

/*************************************************************************************************
 * Handle the rotary encoder settings button using interrupt service routine (ISR_2 bellow)
 * 
 */
   int buttonStatus = digitalRead(SW);
   if ( buttonStatus == LOW && buttonStatus != buttonLast )
   {
    Serial.println ("Pushbutton");
    buttonMenuPos = buttonMenuPos == 2 ? 0 : buttonMenuPos+1 ;
   }

   if(sIEncoderPosCount) {

      if(prevSolderIronEncoderPos == sIEncoderPosCount) {
        if (!processTimer()) { 
          // If timeout expired, set the setpoint to zero (turn off the soldering Iron)
          sIEncoderPosCount = 0;
        }
      } else {
        // Reset Timer
        current_countdown = MAX_CONTDOWN_MINUTES * 60;
      }

     prevSolderIronEncoderPos = sIEncoderPosCount;

   } else {
     noTone(buzzer);     // Stop timeout alarm when the user touches the settings button ...
   }
   /***************************************************************************************************/

   // Update the display
   processDisplay(sI_out, hG_out);

  buttonLast = buttonStatus ;

  // pinALast = aVal;
 } 

/***
 * Interrupt service routine to listen/watch the rotary encoder operations by the user
 * 
 */
void ISR_2() {
    clk_State = digitalRead(pinA); //pin 2 state, clock pin? 
    dt_State =  digitalRead(pinB); //pin 3 state, DT pin? 
  
  if (clk_State != Last_State){     
     // If the data state is different to the clock state, that means the encoder is rotating clockwise
     if (dt_State != clk_State) { 
       //counter ++;
        switch(buttonMenuPos)
         {
          case 0:
           sIEncoderPosCount += 10;
           break;
          case 1:
           hGEncoderPosCount += 10;
           break;
          case 2:
           fanEncoderPosCount += 10;
           break;
         }
     }
     else {
       switch(buttonMenuPos)
         {
          case 0:
           sIEncoderPosCount -= 10;
           sIEncoderPosCount = sIEncoderPosCount > 0 ? sIEncoderPosCount : 0;
           break;
          case 1:
           hGEncoderPosCount -= 10;
           hGEncoderPosCount = hGEncoderPosCount > 0 ? hGEncoderPosCount : 0;
           break;
          case 2:
           fanEncoderPosCount -= 10;
           fanEncoderPosCount = fanEncoderPosCount > 0 ? fanEncoderPosCount : 0;
           break;
         }
     } 
   } 
   Last_State = clk_State; // Updates the previous state of the data with the current state
}


void processDisplay(int sI_out, int hG_out) {
  char str[16];
  char sISeparator = '-';
  char hGSeparator = '-';
  char fanSeparator = '=';
  char timeUnit = 'M';
  int  time;

  //refresh display and serial output 
  if (cyclesCount++ % displayRedrawCount == 0) {  // skips some cycles, otherwise output processing will block input processing

        Serial.print("sIenc = ");
        Serial.print(sIEncoderPosCount);
        Serial.print(" sIThermo = ");
        Serial.print(sIThermo);
        Serial.print(" sI_out = ");
        Serial.print(sI_out);
        Serial.print(" hG_out = ");
        Serial.print(hG_out);
        Serial.print(" hGenc = ");
        Serial.print(hGEncoderPosCount);
        Serial.print(" hGThermo = ");
        Serial.print(hGThermo);
        Serial.print(" fanEnc = ");
        Serial.print(fanEncoderPosCount);
        Serial.print(" hgFan = ");
        Serial.print(fan_pwm_value);
        Serial.print(" buttonMenuPos = ");
        Serial.println(buttonMenuPos);
        
        switch (buttonMenuPos)
        {
          case 0:
          sISeparator = '*'; break;
          case 1:
          hGSeparator = '*'; break;
          case 2:
          fanSeparator = '*'; break;
        }

        if(current_countdown >= 60) {
            timeUnit = 'M';  // Minutes
            time = current_countdown / 60;
        } else {
            timeUnit = 'S'; // Sec.
            time = current_countdown;
        }
              
        sprintf(str, "S=%3d%c%3d  T-%2d%c",sIThermo,sISeparator,sIEncoderPosCount, time, timeUnit);
        lcd.setCursor(0, 0);
        lcd.print(str);
        sprintf(str, "H=%3d%c%3d F%c%3d%%",hGThermo,hGSeparator,hGEncoderPosCount,fanSeparator,fanEncoderPosCount*100/255);
        // sprintf(str, "H=%3d%c%3d F%c%3d%%",sIRawValue,hGSeparator,hGEncoderPosCount,fanSeparator,fan_pwm_value*100/255); //DEBUG
        
        lcd.setCursor(0, 1);
        lcd.print(str);
  }
}


/**
 * Handle the timout timer to power off the heating elements if left idle for MAX_CONTDOWN_MINUTES 
 * 
 */
int processTimer() {
  
    //  if one second has passed
    if(millis() - prevMillis >= 1000){
        current_countdown --;
        prevMillis += 1000;
  
      if(current_countdown <= 30){
          //beep
          Serial.println("Beep!");
            tone(buzzer, 1000); // Send 1KHz sound signal...
            delay(250);        // ...for 1 sec
            noTone(buzzer);     // Stop sound...
      } else {
        noTone(buzzer);     // Stop sound...
      }

    current_countdown = current_countdown > 0 ? current_countdown : 0;
  } 
  return current_countdown;

}

/**
 * Used to check if it is safe to shutdown the heatgun fan
 * Force the fan to saty ON if the heatgun is still too hot
 * to prevent damages
 */
int safeHeatGunFan(int heatGunTmp, int fanSetpoint) {

  if(heatGunTmp >= HEATGUN_TEMP_FAN_SHUTDOWN) {
    return MIN_HOT_HG_FAN_SPEED;
  } else {
    return fanSetpoint;
  }

}

/**
 * Read the Presets values for the 3 buttons from the EEPROM non volatile memory
 * Each button as 3 values for SolderingIron, HeatGun, and Fan blower
 * 
 */
void readPresets(void) {
  EEPROM.get( PRESET_1_ADDRESS, preset_1 );
  EEPROM.get( PRESET_2_ADDRESS, preset_2 );
  EEPROM.get( PRESET_3_ADDRESS, preset_3 );
}


/**
 * Save/Update the Presets values for the 3 buttons to the EEPROM non volatile memory
 * Each button as 3 values for SolderingIron, HeatGun, and Fan blower
 * 
 */
void savePresets(void) {
  EEPROM.put( PRESET_1_ADDRESS, preset_1 );
  EEPROM.put( PRESET_2_ADDRESS, preset_2 );
  EEPROM.put( PRESET_3_ADDRESS, preset_3 );
}