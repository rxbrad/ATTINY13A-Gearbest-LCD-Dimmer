// Gearbest 3.5" LCD Dimming Program for ATTiny - By RxBrad
// See here for more info:
// https://www.tinkerboy.xyz/how-to-add-brightness-control-for-the-3-5-gearbest-screen/

// USER-DEFINABLE VARIABLES
// ========================

// Define pins being used - NOTE ATTiny digital 0,1,2,3,4 =  board 5,6,7,2,3
  const byte pwmPin = 0; // digital pin for PWM output to LCD
  const byte dimPin =  3; // digital pin for Dim Button
  const byte brightPin = 4; // digital pin for Bright Button

// timeFix corrects delays when using non-default F_CPU frequency on CLK0
// Suggested Values: 9.6MHz (Default) --> 1, 4.8MHz --> 0.5, 1.2MHz --> 0.125, 600kHz --> 0.0625
// Frequency Reference
// ===================
// F_CPU 600kHz Prescaler: 1 => 2,344Hz, 8 => 293Hz, 64 => 37Hz, 256 => 9Hz
// F_CPU 1.2MHz Prescaler: 1 => 4,687Hz, 8 => 585Hz, 64 => 73Hz, 256 => 18Hz
// F_CPU 4.8MHz Prescaler: 1 => 18,750Hz, 8 => 2,344Hz, 64 => 293Hz, 256 => 73Hz
// F_CPU 9.6MHz Prescaler: 1 => 37,500Hz, 8 => 4,687Hz, 64 => 585Hz, 256 => 146Hz
// ===================
// To avoid speaker noise, optimal is 9.6MHz @ 1x prescale (37.5kHz).
// 9.6MHz @ 256x (146Hz) is passable but noisy (if you can't get 37.5kHz working).
 const int timeFix = 1;

// On startup, screen defaults to max brightness & fades to desired default
  int dutycycle = 30;      // starting LED brightness (0 = off / 255 = max)
  const int initialFadeTime = 1000; // after delay. time in millis to fade to default

// Behavior on each button press -- NOTE: If you're getting lots of unintended
// double-presses, increase dimStepTime and/or btnWait
  const byte maxDuty = 255;  // maximum allowed brightness (0 = off / 255 = max)
  const byte minDuty = 0;  // minimum allowed brightness (0 = off / 255 = max)
  const unsigned int dimStep = 5;  // lowest brightness change per button press (increases as brightness increases)
  const int dimStepTime = 250; // Time to fade to next brightness
  const int btnWait = 50; // milis delay from end of fade to next allowed button press
  const bool allowSmartSteps = 1; // adjust brightness steps to seem more equal (values: 0, 1)
  const byte maxFactor = 15; // (maxFactor * dimStep) is the max-allowed brightness change per button press

// Variables used by program: DON'T CHANGE THESE!!!
  int dimState[2];
  unsigned int startduty = dutycycle;
  unsigned int actualStep = dimStep;
  
void setup() {  // Initial setup that runs once on power-on
  // Use built-in pullup resistors for buttons
  pinMode(brightPin, INPUT_PULLUP); 
  pinMode(dimPin, INPUT_PULLUP);
  // Output from pwmPin
  pinMode(pwmPin, OUTPUT);

  if (dutycycle < 255) { // fade from full brightness to default brightness
    ChangeBrightness(255, dutycycle, initialFadeTime);
  }

  // Serial Output for Troubleshooting
  // =================================
  //Serial.begin(57600);
  //Serial.print("Starting Duty Cycle: ");
  //Serial.println(dutycycle);

} // End of setup()

void ChangeBrightness(byte startDuty, byte endDuty, int dutyChangeTime) {
    int iFadeStep = dutyChangeTime / abs(endDuty - startDuty);
    byte plusMinus = 1;
    if (endDuty < startDuty) { plusMinus = -1; }
    for (byte i = startDuty; i != endDuty; i += plusMinus) {
      analogWrite(pwmPin, i);
      delay(iFadeStep * timeFix);
    }
    analogWrite(pwmPin, endDuty);
    
    // Serial Output for Troubleshooting
    // =================================
    //Serial.print("Duty Cycle Updated: ");
    //Serial.println(endDuty);
    //Serial.print("actualStep: ");
    //Serial.print(actualStep);
    //Serial.print(" = (");
    //Serial.print(startDuty);
    //Serial.print("^2 / 850) + 1) * ");
    //Serial.print(dimStep);
    //Serial.print(" [max ");
    //Serial.print(maxFactor * dimStep);
    //Serial.println("]");
  } // End of ChangeBrightness()

void ButtonHandler(bool fMode) { // fMode: 0 = dim, 1 = bright
    int convF = 1; // add to dutycycle on bright
    if (fMode == 0) {convF = -1;} // subtract from dutycycle on dim

    if (allowSmartSteps == 1) {
      // Perceived change in brightness is much less at higher dutycycle than lower.
      // Actual change is adjusted to make each step seem similar.
      // Near dutycycle=0, change=dimStep; and near dutycycle=255, change is (dimStep * maxFactor).
      actualStep = ((startduty * startduty / 850UL) + 1) * dimStep;
      if (actualStep > (dimStep * maxFactor) ) { actualStep = dimStep * maxFactor; }
    }

    dutycycle += (actualStep * convF); // set target brightness
    if (dutycycle < minDuty) {dutycycle = minDuty;} // Don't go past min/max
    if (dutycycle > maxDuty) {dutycycle = maxDuty;}

    ChangeBrightness(startduty, dutycycle, dimStepTime);
    delay(btnWait * timeFix); // Pause before allowing next button press
    startduty = dutycycle; // Reset startduty for next button press
    
} // End of ButtonHandler()
  
void loop() { // Program constantly loops through this section
  dimState[0] = {digitalRead(dimPin)}; // Read from dimmer pin
  dimState[1] = {digitalRead(brightPin)}; // Read from brighten pin

  if (dimState[0] == LOW && startduty > minDuty) { // DECREASE BRIGHTNESS Button Pressed
    ButtonHandler(0);
    }  

  if (dimState[1] == LOW && startduty < maxDuty) { // INCREASE BRIGHTNESS Button Pressed
    ButtonHandler(1);  
    } 
    
} // End of loop()
