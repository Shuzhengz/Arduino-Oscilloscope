// USE 115200 BAUD

#define ul unsigned long
#define armed true                                                       // trigger and pushbutton state
#define continuous true                                                  // sweep mode (free running)
#define falling false                                                    // trigger slope positive
#define rising true                                                      // trigger slope negative
#define triggered false                                                  // sweep mode (sweep starts when triggered)
#define sweeping false                                                   // sweeping for TriggeredSweepInterval msecs
#define superimposed true                                                // signals superimposed on display
#define channeled false    

// adjustable variables
int   Beams = 2;                                                         // number of beams read
bool  DisplayMode = channeled;                                           // 'superimposed' or 'channeled'
ul    SampleInterval = 200;                                              // units: msecs * 10; 0.1 <= SampleInterval
bool  SweepMode = continuous;                                             // 'continuous' or 'triggered'
ul    TriggeredSweepInterval = 40000;                                    // total sweep time, units: seconds * 10,000
float TriggerDirection = rising;                                         // 'rising' or 'falling'
float TriggerVolts = 3.5;                                                // trigger vdc; 0 <= TriggerVolts <= 5

// interrupt controlled variables
ul    LastSample = -1;                                                   // initially, no last sample
bool  LED = HIGH;
int   BlinkCount = 0;
ul    SweepCount = 0;                                                    // counts up to Sweep Interval
bool  Tick = false;                                                      // set to true when TickCount = SampleRate
ul    TickCount = 0;                                                     // counts up to SampleRate
bool  TimingMark;                                                        // toggles every 10th 'Sample Intewrval'
ul    TimingMarkCount = 0;                                               // counts up to SampleInterval * 10
bool  TriggerState;                                                      // 'armed' or 'sweeping'
bool  TriggerOnset = false;                                              // marks first tick after trigger occurs

// loop procedure variables
float ChannelFloor;
float ChannelHeight;
bool  freeze = false;                                                    // true: stop; false: run
int   PBPin = 12;                                                        // grounded push button on pin 12 (optional)
int   PBLastState = HIGH;                                                // LOW (pressed) or HIGH (released)
int   PBVal;                                                             // inverse logic, push button tied to ground
float ChannelScale;                                                      // proportion og signal to display
float TriggerDisplay;                                                    // vertical position of trigger
int   TriggerLevel;                                                      // calculated from 'TriggerVolts'
float Value;

void interruptSetup() {
  noInterrupts();                                                        // generate an interrupt every 0.1 msec
  TCCR2A = 2;                                                            // clear timer on compare, OCR2
  TCCR2B = 2;                                                            // 16,000,000Hz/8=2,000,000 Hz; T2 clock ticks every 0.0005 msecs
  OCR2A = 199;                                                           // interrupt = 0.0005*200 = 0.1 msec
  TIMSK2 = 2;                                                            // set the ISR COMPA vect
  interrupts();
}

ISR(TIMER2_COMPA_vect) {                                                 // interrupt every 0.1 msecs
  if (TickCount < SampleInterval) {
    TickCount++;
    BlinkCount++;
    if (BlinkCount >= 500) {
      BlinkCount = 0.0;
      LED = !LED;                                                         // toggle LED every 50 msecs
    }
  }
  else {                                                                 // 'SampleInterval' has elapsed
    Tick = true;
    TickCount = 0;
    TimingMarkCount++;                                                   // update Timing mark
    if (TimingMarkCount >= 10) {                                         // 10th 'SDampleInterval' has occurred
      TimingMark = !TimingMark;
      TimingMarkCount = 0;
    }
    if (SweepMode == triggered) {
      if (TriggerState == sweeping) {                                    // sweeping, update sweep time
        SweepCount +=  SampleInterval;
        if (SweepCount >= TriggeredSweepInterval) {                      // sweep complete
          TriggerState = armed;
          LastSample = -1;
        }
      }
      else {                                                            // armed, look for trigger
        Value = analogRead(A0);
        if (LastSample > 0 and
            ((TriggerDirection == rising and Value >= TriggerLevel and LastSample < TriggerLevel) or
             (TriggerDirection == falling and Value <= TriggerLevel and LastSample > TriggerLevel))) {
          TriggerState = sweeping;                                     // triggered
          SweepCount = 0;
          TriggerOnset = true;
          TimingMarkCount = 0;
          TimingMark = true;
        }
        LastSample = Value;
      }
    }
  }
}

void setup() {
  pinMode (LED_BUILTIN, OUTPUT);
  pinMode(PBPin, INPUT);                                                 // connected to a grounded push button
  digitalWrite(PBPin, HIGH);                                             // pullup push button pin
  Serial.begin(115200);
  if (SweepMode == continuous) {
    TriggerState = sweeping;
  }
  else {
    TriggerState = armed;
  }
  TriggerLevel = (TriggerVolts / 5.0 ) * 1023;
  ChannelHeight = 5.0 / Beams;
  ChannelScale = 5.0 / 1024.0 / Beams;
  TriggerDisplay = TriggerVolts * ChannelScale + 5.0 - ChannelHeight;
  interruptSetup();
}

void loop() {
  if (freeze) {
    digitalWrite(LED_BUILTIN, LOW);
  }
  else if (TriggerState == armed) {
    digitalWrite(LED_BUILTIN, LED);
  }
  else  {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  PBVal = digitalRead(PBPin);
  if (PBVal == LOW and PBLastState == HIGH) {                            // falling edge
    freeze = !freeze;
    delay (2);                                                           // ignore contact bounce
  }
  PBLastState = PBVal;
  if  (!freeze and TriggerState  == sweeping) {
    if (Tick) {                                                          // sample if Sample Interval msecs have elapsed
      if (TimingMark) {                                                  // display timing marks and trigger, if present
        if (TriggerOnset) {
          Serial.print(5.0);
          TriggerOnset = false;
        }
        else {
          Serial.print(0.1);
        }
      }
      else {
        Serial.print(0.0);
      }
      Serial.print(" ");
      ChannelFloor = 5.0 - ChannelHeight;                                // display trigger level, if applicable
      if (SweepMode == triggered) {
        Serial.print(TriggerLevel * ChannelScale + ChannelFloor);
        Serial.print (" ");
      }
      for (int AnalogPin = 0; AnalogPin <= Beams - 1; AnalogPin++) {     // sample 1-6 analog signals and display them
        Value = analogRead(AnalogPin);
        Value = Value * ChannelScale + ChannelFloor;
        Serial.print(Value);
        Serial.print(" ");
        ChannelFloor -= ChannelHeight;
      }
      Tick = false;
      Serial.println("");
    }
  }
}
