/*
 
 Battery Series Resistance Meter v4
 by Francesco Meschia, January 2016
 
 */

#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <LiquidCrystal.h>

// Digital output pin connected to the switching MOSFET gate
#define PIN_GATE 6
// Analog input pin connected to low side of voltage sense
#define PIN_VSL A1
// Analog input pin connected to high side of voltage sense
#define PIN_VSH A0
// Analog input pin connected to low side of current sense
#define PIN_IL A2
// Analog input pin connected to high side of current sense
#define PIN_IH A3
// Digital input pin connected to display option jumper 
#define PIN_OPT 7
// Internal ADC reference voltage
#define ADC_REF_VALUE 1.1
// Sample size for averaging
#define AVG_SIZE 5
// Measurement duty cycle
#define DUTY_CYCLE 0.25
// Measurement period
#define PERIOD 2000
// Delay between switching and measurement
#define MEASUREMENT_DELAY_US 1000

int state = 0;
boolean sleep_entered = false;
float v0_l, v0_h, v1_l, v1_h;
float i0_l, i0_h, i1_l, i1_h;

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  analogReference(INTERNAL);
  pinMode(PIN_VSL, INPUT);
  pinMode(PIN_VSH, INPUT);
  pinMode(PIN_IL, INPUT);
  pinMode(PIN_IH, INPUT);
  pinMode(PIN_OPT, INPUT_PULLUP);
  pinMode(PIN_GATE, OUTPUT);
  digitalWrite(PIN_GATE, HIGH);
  lcd.begin(16, 2);
  lcd.print("ESR Meter v0.4");
  Serial.begin(9600);
  Serial.println("ESR Meter v0.4 ready");
  delay(1000);
  for (int i=0; i<100; i++) {
    analogRead(PIN_VSL); 
    analogRead(PIN_VSH); 
    analogRead(PIN_IL); 
    analogRead(PIN_IH);
  }
  //configure_wdt();
}

inline void configure_wdt() {
  cli();
  wdt_reset();
  MCUSR &= ~_BV(WDRF);
  WDTCSR |= (_BV(WDCE) | _BV(WDE));   // Enable the WD Change Bit
  WDTCSR =   _BV(WDIE) |              // Enable WDT Interrupt
  _BV(WDP2) | _BV(WDP1) | _BV(WDP0);   // Set Timeout to ~1 seconds
  sei();
}

ISR(WDT_vect)
{
  /* Check if we are in sleep mode or it is a genuine WDR. */
  if(sleep_entered == false)
  {
    /* The app has locked up, force a WDR. */
    wdt_enable(WDTO_15MS);
    while(1);
  }
  else
  {
    wdt_reset();
    /* Service the timer if necessary. */
    sleep_entered = false;
    sleep_disable();

    /* Inline function so OK to call from here. */
    configure_wdt();
  }
}

void loop() {
  // when loop() is entered, MOSFET gate is high and load is on
  // wait until duty cycle is expired
  delay(PERIOD * DUTY_CYCLE);
  // measure current and voltages at the sense resistor and high-pass voltage sense lines
  // (duty cycle needs to be set so that voltages at the output of the high-pass sense lines
  // have reached zero at this time, as ADC can't mesure negative voltages)
  v0_h = 0.0; 
  v0_l = 0.0; 
  i0_l = 0.0;
  i0_h = 0.0;
  for (int i=0; i<AVG_SIZE; i++) {
    v0_l += analogRead(PIN_VSL);
    v0_h += analogRead(PIN_VSH);
    i0_l += analogRead(PIN_IL);
    i0_h += analogRead(PIN_IH);
  }
  v0_h /= AVG_SIZE;
  v0_h *= 1000.0 * ADC_REF_VALUE / 1024.0;
  v0_l /= AVG_SIZE; 
  v0_l *= 1000.0 * ADC_REF_VALUE / 1024.0;
  i0_l /= AVG_SIZE;
  i0_l *= ADC_REF_VALUE / 1024.0;
  i0_h /= AVG_SIZE;
  i0_h *= ADC_REF_VALUE / 1024.0;

  // set MOSFET gate to low (disconnect load)
  digitalWrite(PIN_GATE, LOW);
  /*
  lcd.clear();
   lcd.setCursor(0,0);
   lcd.print("LOW");
   */
  // delay just enough to suppress ringing
  delayMicroseconds(MEASUREMENT_DELAY_US);

  // measure current and voltages at the sense resistor and high-pass voltage sense lines
  v1_l = 0.0;
  v1_h = 0.0;
  i1_l = 0.0;
  i1_h = 0.0;
  for (int i=0; i<AVG_SIZE; i++) {
    v1_l += analogRead(PIN_VSL);
    v1_h += analogRead(PIN_VSH);
    i1_l += analogRead(PIN_IL);
    i1_h += analogRead(PIN_IH);
  }
  v1_h /= AVG_SIZE;
  v1_h *= 1000.0 * ADC_REF_VALUE / 1024.0;
  v1_l /= AVG_SIZE; 
  v1_l *= 1000.0 * ADC_REF_VALUE / 1024.0; 
  i1_l /= AVG_SIZE;
  i1_l *= ADC_REF_VALUE / 1024.0;
  i1_h /= AVG_SIZE;
  i1_h *= ADC_REF_VALUE / 1024.0;

  /*
  lcd.clear();
   lcd.setCursor(0,0);
   lcd.print("HIGH");
   */

  // compute resistance using the dV/dI method. Use absolute values to cope with
  // the possibility of reverse polarity in the sense line connections
  int res = fabs((v1_h - v1_l) - (v0_h - v0_l)) / fabs((i1_h - i1_l) - (i0_h - i0_l));

  // The display option pin is used to show raw measurements (when shorted to ground)
  if (digitalRead(PIN_OPT) == LOW) {
    if (state == 0) {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("V0 = ");
      lcd.print(v0_l,0);
      lcd.setCursor(10,0);
      lcd.print(v0_h,0);
      lcd.setCursor(14,0);
      lcd.print("mV");
      lcd.setCursor(0,1);
      lcd.print("V1 = ");
      lcd.print(v1_l,0);
      lcd.setCursor(10,1);
      lcd.print(v1_h,0);
      lcd.setCursor(14,1);
      lcd.print("mV");
    } 
    else if (state == 1) {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("I0 = ");
      lcd.print(1000*i0_l,0);
      lcd.setCursor(10,0);
      lcd.print(1000*i0_h,0);
      lcd.setCursor(14,0);
      lcd.print("mA");
      lcd.setCursor(0,1);
      lcd.print("I1 = ");
      lcd.print(1000*i1_l,0);
      lcd.setCursor(10,1);
      lcd.print(1000*i1_h,0);
      lcd.setCursor(14,1);
      lcd.print("mA");
    } 
    else if (state == 2) {
      if (i0_h > 0.01) {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Rs = ");
        lcd.print(res);
        lcd.print(" m\364");
      } 
    }
    state++;
    state %= (i0_h > 0.01 ? 3 : 2);
    //flip = !flip;
  } 
  else {
    state = 0;
    lcd.clear();
    lcd.setCursor(0,0);
    if (i0_h > 0.01) {
      lcd.print("Rs = ");
      lcd.print(res);
      lcd.print(" m\364");
    } 
    else {
      lcd.print("Connect Battery");
    } 
  }
  delay(PERIOD * (1-DUTY_CYCLE));

  /*
  set_sleep_mode(SLEEP_MODE_PWR_iDOWN);
   sleep_enable();
   sleep_entered = true;
   sleep_mode();
   sleep_disable();
   power_all_enable();
   */

  digitalWrite(PIN_GATE, HIGH);
}

