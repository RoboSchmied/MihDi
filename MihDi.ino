/*
   MihDi.ino
   - Dance your melody!
   - A Midi device that pushes notes to synthesizer over USB.
   - The notes are relative to the distance measured by ultrasonic sensor.

*/
#include "MIDIUSB.h"
#include "NotesMh01.h"

//#define VERSION01   //  erste version MCCrypt  DJ-1/2-Umbau   , sonst mario-version

#ifdef VERSION01   //  erste version MCCrypt  DJ-1/2-Umbau
  // ENABLE ACTUATORS
  #define BUTTONS    // enable button use
  const int VMin=0; // kleiner Potti wert
  const int VMax=1023;  //grosser potti wert
  const int V5Max = 330;
  const int V5Min = 0;
  const int V6Max = 410;
  const int V6Min = 0;

#else             // zweite version mario
  const int VMin=1023; // kleiner Potti wert
  const int VMax=0;  //grosser potti wert
  const int V5Max = VMax;
  const int V5Min = VMin;
  const int V6Max = VMax;
  const int V6Min = VMin;
#endif

int BS=1;   //button state

//  DEBUG OPTIONS
//#define SerialIn   // read commands from Serial   seems not to work together with midi on one USB port
// #define DEBUG                 1
#define ShowPingData      // show distance when note changes
//#define ShowALLPingData  // show every distance
//#define SerialValues   /// Output data changes on button + potis
#define LEDFREQ      /// show Freq on front LEDs
#define TEST1   //test midi program change on button



const int PING = 3; // TRIG Pin
const int ECHO = 2; // ECHO Pin
const int LED1Pin = 14;  // Freq
const int LED2Pin = 16; //  Ton
bool LED1Status = 0;

volatile unsigned long PingZeit = 0, diffP = 0, AnzInt = 0;
unsigned long AnzLoop, AnzLoopAlt, MpSZeit;
int MpS;
int Distance = 0, DistanceLast = 0;
unsigned long lastPingStartTime = 0;
unsigned long PingInterval = 100; //ping every .. ms
const int PingIntervalMin = 20;
int DistMin = 200;
int DistMax = 1700;
const int DistMinSetable = 1200; //min Distance setable by Pot2
const int DistMaxSetable = 7000;


struct Note {
  unsigned long tStart;  //Time start millis
  bool isPlaying;
};
Note N[128] = {false}; //init to zero
unsigned long t;  //Time
unsigned long NotePlayTime = 2000;



// Constants
//
#ifdef VERSION01
//                     Anschluss auf micro  
const int POT_PIN = 0;   //A0  Volume    // Pot connected to analog pin   Drehregler oben links   
const int POT2_PIN = 1;  //A1  Dist max // Pot connected to analog pin 0  Crossfader (unten)
const int POT3_PIN = 2;  //A2  TonLaenge // Pot connected to analog pin   Drehregler oben rechts 
const int POT4_PIN = 3;  //A3  Freq    // Pot connected to analog pin   Drehregler unten links 
const int POT5_PIN = 6;  //D4  NoteMax // Pot connected to analog pin  Vertikal Slider rechts   
const int POT6_PIN = 7;  //D6  NoterMin // Pot connected to analog pin  Vertikal Slider links    
#else   // mario V02
const int POT_PIN = 2;   //A0  Volume    // Pot connected to analog pin 0  Crossfader (unten)      Volume
const int POT2_PIN = 0;  //A1  Dist         // Pot connected to analog pin   Drehregler oben links
const int POT3_PIN = 6;  //A2  TonLaenge   // Pot connected to analog pin   Drehregler oben links
const int POT4_PIN = 3;  //A3  Freq    // Pot connected to analog pin   Drehregler unten links
const int POT5_PIN = 7;  //D4  NoteMax // Pot connected to analog pin  Vertikal Slider rechts   NoteMax
const int POT6_PIN = 1;  //D6  NoteMin // Pot connected to analog pin  Vertikal Slider links    NoterMin
#endif

 int s_nLastPotValue5 = 0;
 int s_nLastMappedValue5 = 0;
 int s_nLastPotValue6 = 0;
 int s_nLastMappedValue6 = 0;
 int nCurrentPotValue5=0;
 int nCurrentPotValue6=0;

byte NoteSpMin = 0; //
byte NoteSpMax = 127; // NotenSpektrum wird von BUT2 geaendert   von 0-127  auf 0-39
byte NoteMin = 0; // wird von POT1 geaendert    Notenhöhe nah
byte NoteMax = 127; // wird von POT5 geaendert   Notenhöhe fern


const byte NoteHarMin = 0; // Harmonic notes min
const byte NoteHarMax = 39; // Harmonic notes max number
const byte NoteAllMin = 0;  // All notes min note (in header file)
const byte NoteAllMax = 108 - 12;  // All notes min note (in header file)



byte lastNote = 0; //letzte gespielte note

const int BUT1_PIN = 15; // Chan1/2 Umschalter unten
const int BUT2_PIN = 5; // digi 5  Kippschalter an Gehäuse-hinterseite
const int BUT3_PIN = 9; // digi 9  Kippschalter an Gehäuse-hinterseite
int BUT1_State = 3;
int BUT2_State = 3;
int BUT3_State = 3;
const int B1switchON = 0;
const int B2switchON = 1;
const int B3switchON = 1;

//const int BUT2_PIN=14
//const int BUT3_PIN=16

const int POT_THRESHOLD = 7;  //7      // Threshold amount to guard against false values
const int POT5_THRESHOLD = 1; //
const int POT6_THRESHOLD = 1;
byte MIDI_CHANNEL = 1;         // MIDI Channel 1
byte Volume = 100;

bool Change = false;

#ifdef SerialIn
// Serial event
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
String ss = "";
String sss = "";
#endif  //SerialIn

int MinAbw=1;

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

//    Status byte : 1001 CCCC    channel (Instrument)
//    Data byte 1 : 0PPP PPPP    pitch (Note)
//    Data byte 2 : 0VVV VVVV    velocity (Volume)

void noteOn(byte channel, byte pitch, byte velocity) {
  N[pitch].tStart = millis();
  if ( N[pitch].isPlaying == false ) {
    //Note weiterspielen statt neu anschlagen
    midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
    MidiUSB.sendMIDI(noteOn);
    N[pitch].isPlaying = true;
    // letzte Note aus (immer nur eine spielen lassen)
    if (lastNote != pitch ) { 
      noteOff(channel, lastNote, 0);
      lastNote = pitch;
    }
    
#ifdef LEDFREQ
    digitalWrite(LED2Pin, LOW);
#endif
  }
  
}

void noteOff(byte channel, byte pitch, byte velocity) {
  if ( N[pitch].isPlaying == true ) {
    midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
    MidiUSB.sendMIDI(noteOff);
    N[pitch].isPlaying = false;
#ifdef LEDFREQ
    digitalWrite(LED2Pin, HIGH);
#endif
  }

}


void allOff() {
  // turn all notes off
  for (int i = 0; i <= 127; i++) {
    //if ( N[i].isPlaying == true ){
    noteOff(MIDI_CHANNEL, i, 0);
    MidiUSB.flush();
    //Serial.print("Aus: "); Serial.println(i);
    //}
  } //for i
}

void AbwChange() {
  MinAbw = (abs(DistMax - DistMin) / abs(NoteMax - NoteMin)) / 3;
#ifdef SerialValues
  Serial.print("MinAbw=");
  Serial.println(MinAbw);
#endif

}

void setup() {
  pinMode(LED1Pin, OUTPUT);          // Sets the digital pin as output
  digitalWrite(LED1Pin, LOW);       // Turn the LED on
  pinMode(LED2Pin, OUTPUT);          // Sets the digital pin as output
  digitalWrite(LED2Pin, LOW);       // Turn the LED on

  pinMode(BUT1_PIN, INPUT);          // Sets the digital pin as output
  pinMode(BUT2_PIN, INPUT);          // Sets the digital pin as output
  pinMode(BUT3_PIN, INPUT);          // Sets the digital pin as output

  MpSZeit = millis();
  pinMode(PING, OUTPUT);    // send wave
  digitalWrite(PING, LOW);  // silent
  pinMode(ECHO, INPUT);     // recieve
  // interrupt when ECHO Pin goes HIGH
  attachInterrupt(digitalPinToInterrupt(ECHO), interruptRoutine, FALLING);  // vorher HIGH
  Serial.begin(115200);
  //Serial.println(freeRam());
  //controlChange(3, 3, 10); // Set the value of controller 10 on channel 0 to 65
  //programChange(0xc0, MIDI_CHANNEL, 3);  // Change MIDI voice to guitar
  //MidiUSB.flush();
  //startPing();
  //allOff();
  Button2Change();
  AbwChange();

}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0x0B | channel, control, value};
  //midiEventPacket_t event = {0x0B, 0x0B | channel, control, value};
  MidiUSB.sendMIDI(event);
}

void volumeChange(byte channel, byte Volume) {
  //byte control=0x07;  //Volume Command
  //byte value=Volume & 0x7F;
  midiEventPacket_t event = {0xB0 | (channel & 0x0F), 0x07, Volume};
  
  MidiUSB.sendMIDI(event);
}

//  change the voice
void programChange(byte cmd, byte channel, byte data1) {
  //programChange(0xc0, byte(MIDI_CHANNEL), 11);  // Change MIDI voice to guitar
  //programChange(0xc0, 25);  // Change MIDI voice to guitar
  //cmd = cmd | channel;  // merge channel number
  //midiEventPacket_t event = {cmd | channel, data1};
  //midiEventPacket_t event = {cmd , channel, data1};
  midiEventPacket_t event1 = {0xc0, 0x0c | channel, data1};
  //midiEventPacket_t event1 = {cmd, channel, data1};
  MidiUSB.sendMIDI(event1);
}




int N1 = 25, N1alt = 0;
//int N2=14;

void MidiVolume(int channel, byte volume)
{
  //Serial.print("Changing Volume to ");
  //Serial.println(volume);
  volumeChange(channel, volume);
  //noteOn(channel, N1, volume);   // Channel 0, middle C, normal velocity
  //noteOff(1, N2, 0);   // Channel 0, middle C, normal velocity
  //MidiUSB.flush();

}

void Button2Change(){
      if ( BUT2_State == B2switchON) {   // only harmonic notes
        NoteSpMin = NoteHarMin; // Harmonic notes min
        NoteSpMax = NoteHarMax; // Harmonic notes max number
      }
      else {
        NoteSpMin = NoteAllMin; // Alle Noten
        NoteSpMax = NoteAllMax;
      }
      // Effect.. laesst Potis (5+6 die TonGrenzen festlegen) neu einlesen (Schwelle austrixen)
      s_nLastPotValue6 = nCurrentPotValue6 + ( POT6_THRESHOLD*2); 
      s_nLastPotValue5 = nCurrentPotValue5 + ( POT5_THRESHOLD*2);
      //AbwChange();
}

void loop() {
  static int s_nLastPotValue = 0;
  static int s_nLastMappedValue = 0;
  static int s_nLastPotValue2 = 0;
  static int s_nLastMappedValue2 = 0;
  static int s_nLastPotValue3 = 0;
  static int s_nLastMappedValue3 = 0;
  static int s_nLastPotValue4 = 0;
  static int s_nLastMappedValue4 = 0;
 

  static int nMappedValue;
  static int nMappedValue2;
  static int nMappedValue3;
  static int nMappedValue4;
  static int nMappedValue5;
  static int nMappedValue6;

#ifdef SerialIn
  // Serial event
  if (stringComplete) {

    ss = inputString.substring(0, 1);
    if ( ss == "i" ) {
      Serial.print("mc=");
      Serial.println(MIDI_CHANNEL);

      Serial.print("M1=");
      Serial.println(nMappedValue);
      Serial.print("M2=");
      Serial.println(nMappedValue2);
      Serial.print("M3=");
      Serial.println(nMappedValue3);
      Serial.print("M4=");
      Serial.println(nMappedValue4);
      Serial.print("M5=");
      Serial.println(nMappedValue5);
      Serial.print("M6=");
      Serial.println(nMappedValue6);


    }

    ss = inputString.substring(0, 3);
    if ( ss == "mc=" ) {
      // interval An
      Serial.println(MIDI_CHANNEL);
      sss = inputString.substring(3);
      MIDI_CHANNEL = sss.toInt();
      Serial.println(sss);
    }
    Serial.println(inputString);
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
#endif  // SerialIn

  //PING
  if (diffP > 0)
  {
    // Formula: uS / 58 = centimeters ; 500 = offset HC-SR0; 615 = offset US-015
    if (millis() - MpSZeit >= 1000)
    {
      MpSZeit = millis();
      MpS = AnzLoop - AnzLoopAlt;
      AnzLoopAlt = AnzLoop;
    }
    Distance = int((diffP - 615) / 5.8); //in mm
    if ((Distance <= DistMax) and (Distance >= DistMin)) { // Trigger event with dist data

      N1 = map(Distance, DistMin, DistMax, NoteMin, NoteMax); // Map the value to 0-127   to NOTE
      //if ( BUT2_State==B2switchON ){
      //   N1=noteMh[N1];
      //}


      if (abs(N1 - N1alt) == 1) { //Aenderung um nur eine Note
        //MinAbw=((DistMax-DistMin)/(NoteMax-NoteMin))/3;
        if ((abs(Distance - DistanceLast) <= MinAbw) and (BUT3_State == B3switchON)) {
          N1 = N1alt; // Wenn Aenderung
        }  //MinAbw
        else {
          DistanceLast = Distance;
        }
      } //um nur eine Note
      else {
        DistanceLast = Distance;
      }

      Change = true; //make new sound
      N1alt = N1;

#ifdef ShowPingData
      Serial.print(Distance); //distance in mm
      Serial.print(" ");
      Serial.print(N1); //uS
      Serial.print(" ");
      Serial.println(MinAbw);
      //Serial.print(MpS); //uS
      //Serial.print("\t");
      //Serial.println(AnzInt);  //interrupt count
#endif

    }
    else
    {
#ifdef ShowALLPingData
      Serial.println(Distance); //distance in mm
#endif
    }

    AnzLoop += 1;
    diffP = 0;

  }
  t = millis();
  // Start new Ping every .. ms
  if ((diffP == 0) and (t - lastPingStartTime >= PingInterval)) {
    lastPingStartTime = t;
    startPing();
  }



  //Spielende Toeen Stoppen nach...
  for (int i = 0; i <= 127; i++) {
    if ( N[i].isPlaying == true ) {
      //if (N[i].tStart + NotePlayTime < t) {
      if (t-N[i].tStart >= NotePlayTime ) {
        noteOff(MIDI_CHANNEL, i, 0);
        MidiUSB.flush();
        //Serial.print("Aus: "); Serial.println(i);
      }
    }
  } //for i

#ifdef BUTTONS  
    BS = digitalRead(BUT1_PIN);
#endif
    if (BS != BUT1_State ) {
      BUT1_State = BS;
  #ifdef SerialValues
      Serial.print("B1=");
      Serial.println(BUT1_State);
  #endif
    }
  #ifdef BUTTONS  
    BS = digitalRead(BUT2_PIN);
  #endif
    if (BS != BUT2_State ) {
      BUT2_State = BS;
      Button2Change();
      
  #ifdef SerialValues
      Serial.print("B2=");
      Serial.println(BUT2_State);
  #endif
    }
  #ifdef BUTTONS  
    BS = digitalRead(BUT3_PIN);
  #endif
    if (BS != BUT3_State ) {
      BUT3_State = BS;
  #ifdef SerialValues
      Serial.print("B3=");
      Serial.println(BUT3_State);
  #endif
#ifdef TEST1
    programChange(0xc0,MIDI_CHANNEL,4);
    MidiUSB.flush();
#endif

  }


  // Potti 1
  int nCurrentPotValue = analogRead(POT_PIN);
  if (abs(nCurrentPotValue - s_nLastPotValue) > POT_THRESHOLD)
  {
    s_nLastPotValue = nCurrentPotValue;
    //int nMappedValue = map(nCurrentPotValue, 0, 1023, 0, 127); // Map the value to 0-127
    nMappedValue = map(nCurrentPotValue, VMin, VMax, 0, 127); // Map the value to 0-127
    if (nMappedValue != s_nLastMappedValue)
    {
      s_nLastMappedValue = nMappedValue;  //Volume
      Volume = nMappedValue;

#ifdef SerialValues
      Serial.print("P1="); Serial.print(nCurrentPotValue); Serial.print(" Volume="); Serial.println(nMappedValue);
#endif
#ifdef BUTTONS
      if ( BUT1_State == B1switchON ) {
        N1 = NoteMin;
        Change = true;
      }
#endif
    }
  }
  // Potti 2  //nunten Crossfadder
  int nCurrentPotValue2 = analogRead(POT2_PIN);
  if (abs(nCurrentPotValue2 - s_nLastPotValue2) > POT_THRESHOLD)
  {
    s_nLastPotValue2 = nCurrentPotValue2;
    //int nMappedValue = map(nCurrentPotValue, 0, 1023, 0, 127); // Map the value to 0-127
    nMappedValue2 = map(nCurrentPotValue2, VMin, VMax, DistMinSetable, DistMaxSetable); // Map the value to 0-127
    if (nMappedValue2 != s_nLastMappedValue2)
    {
      s_nLastMappedValue2 = nMappedValue2;
      DistMax = nMappedValue2; //Note
#ifdef SerialValues
      Serial.print("P2 "); Serial.print(nCurrentPotValue2); Serial.print(" DistMax(mm)="); Serial.println(nMappedValue2);
#endif
#ifdef BUTTONS
      if ( BUT1_State==B1switchON ) { Change=true; }
#endif
    }
  }

  // Potti 3
  int nCurrentPotValue3 = analogRead(POT3_PIN);
  if (abs(nCurrentPotValue3 - s_nLastPotValue3) > POT_THRESHOLD)
  {
    s_nLastPotValue3 = nCurrentPotValue3;

    //int nMappedValue = map(nCurrentPotValue, 0, 1023, 0, 127); // Map the value to 0-127
    //nMappedValue3 = map(nCurrentPotValue3, 0, 1023, 0, 16); // Midi channel to 0-144
    nMappedValue3 = map(nCurrentPotValue3, VMin, VMax, 1, 1000); // Midi channel to 0-144
    if (nMappedValue3 != s_nLastMappedValue3)
    {
      s_nLastMappedValue3 = nMappedValue3;
      //allOff();  //alle toene aus
      //MIDI_CHANNEL=nMappedValue3;
      NotePlayTime = nMappedValue3;
#ifdef SerialValues
      Serial.print("P3 "); Serial.print(nCurrentPotValue3); Serial.print(" NotePlayTime="); Serial.println(nMappedValue3);
#endif
#ifdef BUTTONS
      if ( BUT1_State == B1switchON ) {
        Change = true;
      }
#endif
    }
  }

  // Potti 4
  int nCurrentPotValue4 = analogRead(POT4_PIN);
  if (abs(nCurrentPotValue4 - s_nLastPotValue4) > POT_THRESHOLD)
  {
    s_nLastPotValue4 = nCurrentPotValue4;

    //int nMappedValue = map(nCurrentPotValue, 0, 1023, 0, 127); // Map the value to 0-127
    //nMappedValue4 = map(nCurrentPotValue4, 0, 1023, 0, 16); // Midi channel to 0-144
    nMappedValue4 = map(nCurrentPotValue4, VMin, VMax, 500, PingIntervalMin); // Midi channel to 0-144
    if (nMappedValue4 != s_nLastMappedValue4)
    {
      s_nLastMappedValue4 = nMappedValue4;
      //allOff();  //alle toene aus
      //MIDI_CHANNEL=nMappedValue4;
      PingInterval = nMappedValue4;
#ifdef SerialValues
      Serial.print("P4 "); Serial.print(nCurrentPotValue4); Serial.print(" PingInterval="); Serial.println(nMappedValue4);
#endif
      //if ( BUT1_State==B1switchON ) { Change=true; }
    }
  }

  // Potti 5
  nCurrentPotValue5 = analogRead(POT5_PIN);
  if (abs(nCurrentPotValue5 - s_nLastPotValue5) > POT5_THRESHOLD)
  {
    s_nLastPotValue5 = nCurrentPotValue5;
    //int nMappedValue = map(nCurrentPotValue, 0, 1023, 0, 127); // Map the value to 0-127
#ifdef VERSION01    
    if (nCurrentPotValue5 < V5Min) {
      nCurrentPotValue5 = V5Min;
    }
    if (nCurrentPotValue5 > V5Max) {
      nCurrentPotValue5 = V5Max;
    }
#endif    
    nMappedValue5 = map(nCurrentPotValue5, V5Min, V5Max, NoteSpMax, NoteSpMin); // Map the value to 0-127
    if (nMappedValue5 != s_nLastMappedValue5)
    {
      s_nLastMappedValue5 = nMappedValue5;
      NoteMax = nMappedValue5; //Ping-Note Max
      AbwChange();
#ifdef SerialValues
      Serial.print("P5 ");
      Serial.print(nCurrentPotValue5);
      Serial.print(" NoteMax=");
      Serial.println(nMappedValue5);
#endif
#ifdef BUTTONS
      if ( BUT1_State == B1switchON ) {
        N1 = NoteMax;
        Change = true;
      }
#endif
    }
  }

  // Potti 6
  nCurrentPotValue6 = analogRead(POT6_PIN);
  if (abs(nCurrentPotValue6 - s_nLastPotValue6) > POT6_THRESHOLD)
  {
    s_nLastPotValue6 = nCurrentPotValue6;
    //int nMappedValue = map(nCurrentPotValue, 0, 1023, 0, 127); // Map the value to 0-127
#ifdef VERSION01
    if (nCurrentPotValue6 < V6Min) {
      nCurrentPotValue6 = V6Min;
    }
    if (nCurrentPotValue6 > V6Max) {
      nCurrentPotValue6 = V6Max;
    }
#endif
    nMappedValue6 = map(nCurrentPotValue6, V6Min, V6Max, NoteSpMax, NoteSpMin); // Map the value to 0-127

    if (nMappedValue6 != s_nLastMappedValue6)
    {
      s_nLastMappedValue6 = nMappedValue6;  //Volume
      NoteMin = nMappedValue6;
      AbwChange();
#ifdef SerialValues
      Serial.print("P6 "); Serial.print(nCurrentPotValue6); Serial.print(" NoteMin="); Serial.println(nMappedValue6);
#endif
#ifdef BUTTONS
      if ( BUT1_State == B1switchON ) {
        N1 = NoteMin;
        Change = true;
      }
#endif
    }
  }


  if (Change == true)
  {
    //MidiVolume(MIDI_CHANNEL, nMappedValue);
    //programChange(0xc0, byte(MIDI_CHANNEL), nMappedValue);  // Change MIDI voice to guitar
    if ( BUT2_State == B2switchON ) {
      N1 = noteMh[N1];
      //N1=noteMh[map(N1, NoteMin, NoteMax,0, 39)];
    } else {
      N1 = note[N1];
    }
    noteOn(byte(MIDI_CHANNEL), byte(N1), byte(Volume));
    MidiUSB.flush();
    Change = false;
  }


  delayMicroseconds(1);

  /*Serial.println("Sending note on");
    noteOn(1, N1, 104);   // Channel 0, middle C, normal velocity
    noteOff(1, N2, 0);   // Channel 0, middle C, normal velocity
    MidiUSB.flush();
    delay(1000);
    noteOn(1, N1, 99);   // Channel 0, middle C, normal velocity
    //noteOn(1, 28, 0);   // Channel 0, middle C, normal velocity
    MidiUSB.flush();
    delay(500);

    Serial.println("Sending note off");
    noteOn(1, N2, 124);   // Channel 0, middle C, normal velocity
    noteOff(1, N1, 104);  // Channel 0, middle C, normal velocity
    MidiUSB.flush();
    delay(3500);
  */
  //controlChange(0, 10, 65); // Set the value of controller 10 on channel 0 to 65
}
void interruptRoutine() {
  diffP = (micros() - PingZeit);
  AnzInt += 1;
}
void startPing() {
  PingZeit = micros();
  digitalWrite(PING, HIGH);
#ifdef LEDFREQ
  LED1Status = !LED1Status;
  digitalWrite(LED1Pin, LED1Status);
#endif
  //Interrupts();
  delayMicroseconds(1);
  digitalWrite(PING, LOW);


}

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

#ifdef SerialIn
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
#endif   // SerialIn
