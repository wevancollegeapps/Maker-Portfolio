#include <MIDI.h>
#include <LiquidCrystal.h>
#include <math.h>
// if everything is plugged in try unplugging and plugging back in the USB-B side of the arduino to get it working
// if that doesn't work then take the hat completely off of the arduino and wait then put it on
// the schematic online has midi pins 4 and 5 and its subsequent diode reversed
// 100kohm resistor is not necessary and only serves to dampen the signal
// CHECK THAT PIN 6(base) IS NOT CONNECTED TO THE COLLECTOR!!!!!!!!!!!!! YOU CAN PULL THE PIN UP IF YOU LIKE!!!!!!!!!!!!! FOR THE LOVE OF GOD!!!!

// to upload try unplugging the hat

// is it not working?
// IF LEFT OUTSIDE IT MAY NEED TO BE HEATED UP. IT DOES NOT WORK WELL COLD AND HUMID. BLAST UNDER HEAT GUN UNTIL WARM
// try pressing on the LCD screen a bit
// Make sure that the 6 pins under the hat but above the arduino are not touching
// try unpluggin the arduino!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// fiddle with the wires or some ts
// fiddle with the midi jack
// reupload code
// hold down the restart button for 30s
// check if midi is plugged in lmao
// restart laptop
// replace the opticoupler
// idk
MIDI_CREATE_DEFAULT_INSTANCE();
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

bool pixelMap[8][5];   // 8 rows × 5 columns
byte customChar[8];    // Final byte pattern

int row = 0;
int col = 0;
int currentLCDColumn = 0;  // Store the LCD column where we display the char
bool starting = true;

byte fullBlock[8] = {
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};

byte bars[9][8] = {
  {0,0,0,0,0,0,0,0},  // 0 height (blank)
  {0,0,0,0,0,0,0,31},
  {0,0,0,0,0,0,31,31},
  {0,0,0,0,0,31,31,31},
  {0,0,0,0,31,31,31,31},
  {0,0,0,31,31,31,31,31},
  {0,0,31,31,31,31,31,31},
  {0,31,31,31,31,31,31,31},
  {31,31,31,31,31,31,31,31}  // Full height
};

int data[16] = {0};  // Strengths for each column (0–8)

byte lastNote = 255; // 255 means "no note is playing"
bool activeNotes[128] = {false}; // Track which notes are active
bool noteIsPlaying = false; // Is note playing?

unsigned int midiperiod[128];                      // Array to store MIDI note periods

const int analogInPin_Periodendauer_min = A0;      // Analog input pin for minimum period potentiometer
const int analogInPin_ontime = A5;                 // Analog input pin for on-time potentiometer

int outputPsuB = 10;                               // Output pin for the PWM signal

int Irms;                                          // Minimum period duration of the PWM signal in µs
int Periodendauer_min_Intervall_unten;             // Lower limit for the minimum allowed period
int Periodendauer_min_Intervall_oben;              // Upper limit for the minimum allowed period
int ontimeGOAL;                                        // Variable for the on-time of the PWM signal
int ontime_max;                                    // Maximum on-time of the PWM signal in µs
float ontime;                                        // Real ontime transmitted to the PWM
float IrmsCalc;

unsigned long previousMillis = 0;
const unsigned long interval = 150; // Half a second
int notestrength = 0;

bool lastFlag = true;            // To detect falling edge
bool timerStarted = false;
bool inActionLoop = false;       // This becomes true after 1 second of flag == false
unsigned long startMillis = 0;
const unsigned long intervalnote = 2500; // 2.5 seconds

    float ResonantFrequency = 120000.0;
    float MMCVoltageMAX = 4800.0;
    int PeakCurrent = 750;

// =================================================
// ==================   SETUP   ====================
// =================================================

void setup()
{
    Serial.begin(31250);    // Other baud rates DO NOT work!!
    lcd.begin(16,2);
    lcd.setCursor(0,0);
    lcd.print("   DRSSTC By:   ");
    lcd.setCursor(0,1);
    lcd.print("  Evan  Wright  ");

    pinMode(13, OUTPUT);
    pinMode(outputPsuB, OUTPUT);  // Set Pin_PWM as output

    for (int i = 0; i < 8; i++) {
      lcd.createChar(i, bars[i]);
    }

    lcd.createChar(0, fullBlock);
    for(int i=0; i<16; i++) data[i] = ' ';  // space means blank

    // Define on-time and boundaries of minimum period
    // ===============================================

    ontime_max = 120;            // Keep at 120µs for music but for burst mode it can be higher
    // 120µs at 500bps is a 6% duty cycle, 100µs at 500bps is a 5% duty cycle

    // CALCULATION FOR DUTY CYCLE -->   (ONTIME/1000000)/(1/BPS)*100 = DUTY CYCLE

    Periodendauer_min_Intervall_oben = 54;   // Max minimum period = 2.0 ms, f_max_lower = 500 Hz
    // Usually 200 and 80
    // (1/x)*100000 for frequency
    Periodendauer_min_Intervall_unten = 0;   // Min minimum period = 0.8 ms, f_max_upper = 1250 Hz
    // Set to 8 for higher frequencies, 800 for normal

    // ==========================
    //     Set timer
    // ==========================

    TCCR1A = B00100001;        // PWM, Phase and frequency correct - change at OCR1A
    TCCR1B = B10010;           // Prescaling by 8 of system clock

    // Value of 1 corresponds to 1 µs!
    // ===============================

    OCR1A = midiperiod[68];    // Output the initially unimportant start period of the PWM signal

    // OCR1A = 2500;           // Start period tau = e.g. 2500 µs = 2.5 ms == 400 Hz

    OCR1B = 0;                 // Start ON-Time = 0 µs, i.e. no tone is played initially!    

    // Connect the handleNoteOn function to the library,
    // so it is called upon reception of a NoteOn.    

    MIDI.setHandleNoteOn(handleNoteOn);      // Just the name of the function    

    MIDI.setHandleNoteOff(handleNoteOff);    // Same for NoteOffs

    MIDI.begin(MIDI_CHANNEL_OMNI);           // Initiate MIDI communications, listen to all channels 

    // Period durations of MIDI notes
    // ==============================

    midiperiod[0] = 122312; 
    midiperiod[1] = 115447; 
    midiperiod[2] = 108968; 
    midiperiod[3] = 102852; 
    midiperiod[4] = 97079; 
    midiperiod[5] = 91631; 
    midiperiod[6] = 86488; 
    midiperiod[7] = 81634; 
    midiperiod[8] = 77052; 
    midiperiod[9] = 72727; 
    midiperiod[10] = 68645; 
    midiperiod[11] = 64793; 
    midiperiod[12] = 61156; 
    midiperiod[13] = 57724;
    midiperiod[14] = 54484;
    midiperiod[15] = 51426; 
    midiperiod[16] = 48540; 
    midiperiod[17] = 45815; 
    midiperiod[18] = 43244; 
    midiperiod[19] = 40817; 
    midiperiod[20] = 38526; 
    midiperiod[21] = 36364; 
    midiperiod[22] = 34323; 
    midiperiod[23] = 32396;
    midiperiod[24] = 30578; 
    midiperiod[25] = 28862; 
    midiperiod[26] = 27242; 
    midiperiod[27] = 25713; 
    midiperiod[28] = 24270; 
    midiperiod[29] = 22908; 
    midiperiod[30] = 21622; 
    midiperiod[31] = 20408; 
    midiperiod[32] = 19263; 
    midiperiod[33] = 18182; 
    midiperiod[34] = 17161; 
    midiperiod[35] = 16198; 
    midiperiod[36] = 15289; 
    midiperiod[37] = 14431; 
    midiperiod[38] = 13621; 
    midiperiod[39] = 12856; 
    midiperiod[40] = 12135; 
    midiperiod[41] = 11454; 
    midiperiod[42] = 10811; 
    midiperiod[43] = 10204; 
    midiperiod[44] = 9631; 
    midiperiod[45] = 9091; 
    midiperiod[46] = 8581; 
    midiperiod[47] = 8099;
    midiperiod[48] = 7645; 
    midiperiod[49] = 7215; 
    midiperiod[50] = 6810; 
    midiperiod[51] = 6428; 
    midiperiod[52] = 6067; 
    midiperiod[53] = 5727; 
    midiperiod[54] = 5405; 
    midiperiod[55] = 5102; 
    midiperiod[56] = 4816; 
    midiperiod[57] = 4545; 
    midiperiod[58] = 4290; 
    midiperiod[59] = 4050; 
    midiperiod[60] = 3822; 
    midiperiod[61] = 3608; 
    midiperiod[62] = 3405; 
    midiperiod[63] = 3214; 
    midiperiod[64] = 3034; 
    midiperiod[65] = 2863; 
    midiperiod[66] = 2703; 
    midiperiod[67] = 2551; 
    midiperiod[68] = 2408; 
    midiperiod[69] = 2273; 
    midiperiod[70] = 2145; 
    midiperiod[71] = 2025;
    midiperiod[72] = 1911; 
    midiperiod[73] = 1804; 
    midiperiod[74] = 1703; 
    midiperiod[75] = 1607; 
    midiperiod[76] = 1517; 
    midiperiod[77] = 1432; 
    midiperiod[78] = 1351; 
    midiperiod[79] = 1276; 
    midiperiod[80] = 1204; 
    midiperiod[81] = 1136; 
    midiperiod[82] = 1073; 
    midiperiod[83] = 1012; 
    midiperiod[84] = 956; 
    midiperiod[85] = 902; 
    midiperiod[86] = 851; 
    midiperiod[87] = 804; 
    midiperiod[88] = 758; 
    midiperiod[89] = 716; 
    midiperiod[90] = 676; 
    midiperiod[91] = 638; 
    midiperiod[92] = 602; 
    midiperiod[93] = 568; 
    midiperiod[94] = 536; 
    midiperiod[95] = 506; 
    midiperiod[96] = 478; 
    midiperiod[97] = 451; 
    midiperiod[98] = 426; 
    midiperiod[99] = 402;  
    midiperiod[100] = 379; 
    midiperiod[101] = 358; 
    midiperiod[102] = 338; 
    midiperiod[103] = 319; 
    midiperiod[104] = 301; 
    midiperiod[105] = 284; 
    midiperiod[106] = 268; 
    midiperiod[107] = 253; 
    midiperiod[108] = 239; 
    midiperiod[109] = 225; 
    midiperiod[110] = 213; 
    midiperiod[111] = 201; 
    midiperiod[112] = 190; 
    midiperiod[113] = 179; 
    midiperiod[114] = 169; 
    midiperiod[115] = 159; 
    midiperiod[116] = 150; 
    midiperiod[117] = 142; 
    midiperiod[118] = 134; 
    midiperiod[119] = 127; 
    midiperiod[120] = 119; 
    midiperiod[121] = 113; 
    midiperiod[122] = 106; 
    midiperiod[123] = 100; 
    midiperiod[124] = 95; 
    midiperiod[125] = 89; 
    midiperiod[126] = 84; 
    midiperiod[127] = 50; 
}

// =========================================================
// ==================   MAIN LOOP   ========================
// =========================================================

const char* getNoteNameFromPeriod(unsigned int period) {
  const char* noteNames[128] = {
  "C-1", "C#-1", "D-1", "D#-1", "E-1", "F-1", "F#-1", "G-1", "G#-1", "A-1", "A#-1", "B-1",
  "C0",  "C#0",  "D0",  "D#0",  "E0",  "F0",  "F#0",  "G0",  "G#0",  "A0",  "A#0",  "B0",
  "C1",  "C#1",  "D1",  "D#1",  "E1",  "F1",  "F#1",  "G1",  "G#1",  "A1",  "A#1",  "B1",
  "C2",  "C#2",  "D2",  "D#2",  "E2",  "F2",  "F#2",  "G2",  "G#2",  "A2",  "A#2",  "B2",
  "C3",  "C#3",  "D3",  "D#3",  "E3",  "F3",  "F#3",  "G3",  "G#3",  "A3",  "A#3",  "B3",
  "C4",  "C#4",  "D4",  "D#4",  "E4",  "F4",  "F#4",  "G4",  "G#4",  "A4",  "A#4",  "B4",
  "C5",  "C#5",  "D5",  "D#5",  "E5",  "F5",  "F#5",  "G5",  "G#5",  "A5",  "A#5",  "B5",
  "C6",  "C#6",  "D6",  "D#6",  "E6",  "F6",  "F#6",  "G6",  "G#6",  "A6",  "A#6",  "B6",
  "C7",  "C#7",  "D7",  "D#7",  "E7",  "F7",  "F#7",  "G7",  "G#7",  "A7",  "A#7",  "B7",
  "C8",  "C#8",  "D8",  "D#8",  "E8",  "F8",  "F#8",  "G8",  "G#8",  "A8",  "A#8",  "B8",
  "C9",  "C#9",  "D9",  "D#9",  "E9",  "F9",  "F#9",  "G9"
  };
  for (int i = 0; i < 128; i++) {
    if (midiperiod[i] == period) {
      return noteNames[i];
    }
  }
  return "Unknown";
}

int columnHeights[16];  // 0 to 8 (height of bar)

bool emiwarn = false;             // trigger variable
unsigned long emiwarnStart = 0;   // when emiwarn became true
bool emiwarnOneSecPassed = false; // flag to track 1 second elapsed

void loop()
{
    int rawValue = analogRead(analogInPin_ontime);
    rawValue = 1023 - rawValue;  // invert
    ontimeGOAL = map(rawValue, 0, 1023, 0, ontime_max); // Read on-time from potentiometer and Map on-time to range [0, ontime_max] µs

    int rawIrms = analogRead(analogInPin_Periodendauer_min);
    rawIrms = 1023 - rawIrms;  // invert
    Irms = map(rawIrms, 0, 1023, Periodendauer_min_Intervall_unten, Periodendauer_min_Intervall_oben); // Read minimum period from potentiometer and Map minimum period to range [min, max] µs
        
    MIDI.read(); // Call MIDI.read as fast as possible for real-time performance.

  bool flag = noteIsPlaying;

    if (lastFlag && !flag) {
    startMillis = millis();
    timerStarted = true;
  }
  lastFlag = flag;

  // After 1 second of being false, activate action mode
  if (timerStarted && (millis() - startMillis >= intervalnote)) {
    timerStarted = false;
    inActionLoop = true;
  }

  // Exit action loop if flag goes back to true
  if (flag && inActionLoop) {
    inActionLoop = false;
  }

  if (emiwarn == false){
  if (inActionLoop) {
    lcd.setCursor(0, 0);
    lcd.print("Max");
    lcd.print(Irms);
    lcd.print("A    ");
    if (ontimeGOAL >= 100) {
    lcd.setCursor(8,0);
    lcd.print("Max ");
  } else {
    lcd.setCursor(9,0);
    lcd.print("Max ");
  }
  }
  }

  unsigned long currentMillis = millis();

if (currentMillis - previousMillis >= interval) {
  previousMillis = currentMillis;

    // Shift all data left
    for (int i = 0; i < 15; i++) {
      data[i] = data[i + 1];
    }

    // Simulated new note strength (random for demo)
    data[15] = notestrength;

    // Draw bar graph
    for (int col = 0; col < 16; col++) {
      lcd.setCursor(col, 1);
      lcd.write(data[col] + 1);
    }
  }

  if (noteIsPlaying == false) {
    notestrength = 0;
  }
  if (starting == true) {
    delay(intervalnote);
    starting = false;
  }

  if (emiwarn && emiwarnStart == 0) {
    emiwarnStart = millis();      // start the timer
    emiwarnOneSecPassed = false;  // reset the 1s flag
  }

  // Check if 1 second has passed since emiwarn turned true
  if (emiwarn && !emiwarnOneSecPassed && millis() - emiwarnStart >= 2000) {
    lcd.setCursor(0, 0);
    lcd.print("Max");
    lcd.print(Irms);
    lcd.print("A    ");
    if (ontimeGOAL >= 100) {
    lcd.setCursor(8,0);
    lcd.print("Max ");
    } else {
    lcd.setCursor(9,0);
    lcd.print("Max ");
    }
    emiwarn = false;
    emiwarnOneSecPassed = true;   // prevent repeating
    // Place any action here that should occur after 1 second
  }

  // Reset the timer when emiwarn goes false
  if (!emiwarn) {
    emiwarnStart = 0;
    emiwarnOneSecPassed = false;
  }

  if (emiwarn == false){
    if (ontimeGOAL >= 100) {
    lcd.setCursor(11,0);
    lcd.print(ontimeGOAL);
    lcd.print("us");
  } else {
    lcd.setCursor(12,0);
    lcd.print(ontimeGOAL);
    lcd.print("us ");
  }
  if ((round(ontimeGOAL))<100) {
    if (inActionLoop == false) {
      lcd.setCursor(11, 0);
      lcd.print(" ");
    }
  }
  }
}


void fillBlankColumns() {
  for (int col = 0; col < 16; col++) {
    if (data[col] == ' ' || data[col] == 0) {  // blank or char 0
      lcd.setCursor(col, 1);       // second row, 0-based index
      lcd.write(byte(0));          // write full block char
      data[col] = 0;               // update array to track that it's filled
    }
  }
}

void drawColumn(int height, int col) {
  // height: 0 (blank) to 8 (full)
  // Top row (char row 0) shows height > 5
  // Bottom row (char row 1) shows height 1–5

  byte topChar = 0;
  byte bottomChar = 0;

  if (height > 5) {
    topChar = height - 5;  // e.g., height=8 -> top=3
    bottomChar = 5;        // Bottom is maxed
  } else {
    topChar = 0;
    bottomChar = height;   // All height in bottom row
  }
  lcd.setCursor(col, 1);
  lcd.write(bottomChar);
}

// ========================
// MIDI note is played:
// ========================

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
  noteIsPlaying = true;
  ontime = ontimeGOAL;

  float lastDisplayOntime = -1;
  float freqHz = 1.0 / (midiperiod[pitch] * 0.000001);

  while (true) {
    if (pitch <= 10) {
      ontime = 0;
      lcd.setCursor(0, 0);
      emiwarn = true;
      lcd.print("High EMI Likely ");
      memset(activeNotes, 0, sizeof(activeNotes));
      lastNote = 255;  // no note is playing
      OCR1B = 0;       // turn off PWM
      noteIsPlaying = false;
      return;
    } else {
    IrmsCalc = 0.5 * PeakCurrent * sqrt(freqHz * ontime * 0.000001);

    if (IrmsCalc < (Irms - 5)) {
      ontime += 0.5;
    }
    else if (IrmsCalc > (Irms + 5)) {
      ontime -= 0.5;
    }
    else {
      break;  // Close enough, switch to fine tune
    }

    if (abs(ontime - lastDisplayOntime) >= 1) {
      lastDisplayOntime = ontime;
    }

    if (ontime <= 0 || ontime >= 65535) {
      emiwarn = true;
      lcd.setCursor(0, 0);
      lcd.print("Cant match Irms ");
      ontime = 0;
      return;
    }
    }
  }

  while (true) {
    if (pitch <= 10) {
      ontime = 0;
      lcd.setCursor(0, 0);
      emiwarn = true;
      lcd.print("High EMI Likely ");
      memset(activeNotes, 0, sizeof(activeNotes));
      lastNote = 255;  // no note is playing
      OCR1B = 0;       // turn off PWM
      noteIsPlaying = false;
      return;
    } else {
    IrmsCalc = 0.5 * PeakCurrent * sqrt(freqHz * ontime * 0.000001);

    if (IrmsCalc < (Irms - 1)) {
      ontime += 0.1;
    }
    else if (IrmsCalc > (Irms + 1)) {
      ontime -= 0.1;
    }
    else {
      break;
    }

    if (abs(ontime - lastDisplayOntime) >= 0.5) {
      lastDisplayOntime = ontime;
    }

    if (ontime <= 0 || ontime >= 65535) {
      emiwarn = true;
      lcd.setCursor(0, 0);
      lcd.print("Cant match Irms ");
      ontime = 0;
      return;
    }
    }
  }

  if (ontime >= ontimeGOAL){
    ontime = ontimeGOAL;
    IrmsCalc = 0.5 * PeakCurrent * sqrt(freqHz * ontime * 0.000001);
  }

  OCR1A = midiperiod[pitch];
  float Dfreq = OCR1A;
  notestrength = (ontime/ontimeGOAL)*7;
  if (emiwarn == false){
    lcd.setCursor(0, 0);
    lcd.print(round(ontime));
    lcd.print("us");
    if ((round(ontimeGOAL))<100) {
      lcd.print(" ");
    }
    if ((round(1 / (Dfreq * 0.000001)))<1000) {
      lcd.print(" ");
    }
    lcd.print(round(1 / (Dfreq * 0.000001)));
    lcd.print("Hz      ");
    //lcd.print(pitch);
  }
    activeNotes[pitch] = true;
    lastNote = pitch;
    OCR1B = ontime;
}


// ===============================
// MIDI note is no longer played:
// ===============================

void handleNoteOff(byte channel, byte pitch, byte velocity) {
    activeNotes[pitch] = false; // Mark note as inactive

    // If the last note released was the currently playing one, find a new one
    if (lastNote == pitch) {
        lastNote = 255; // Reset lastNote temporarily
        for (int i = 0; i < 128; i++) {
            if (activeNotes[i]) {
                lastNote = i;
                OCR1A = midiperiod[i]; // Set frequency to new note
                  ontime = ontimeGOAL;

  float lastDisplayOntime = -1;
  float freqHz = 1.0 / (midiperiod[i] * 0.000001);

  while (true) {
    if (pitch <= 10) {
      ontime = 0;
      lcd.setCursor(0, 0);
      emiwarn = true;
      lcd.print("High EMI Likely ");
      memset(activeNotes, 0, sizeof(activeNotes));
      lastNote = 255;  // no note is playing
      OCR1B = 0;       // turn off PWM
      noteIsPlaying = false;
      return;
    } else {
    IrmsCalc = 0.5 * PeakCurrent * sqrt(freqHz * ontime * 0.000001);

    if (IrmsCalc < (Irms - 5)) {
      ontime += 0.5;
    }
    else if (IrmsCalc > (Irms + 5)) {
      ontime -= 0.5;
    }
    else {
      break;  // Close enough, switch to fine tune
    }

    if (abs(ontime - lastDisplayOntime) >= 1) {
      lastDisplayOntime = ontime;
    }

    if (ontime <= 0 || ontime >= 65535) {
      emiwarn = true;
      lcd.setCursor(0, 0);
      lcd.print("Cant match Irms ");
      ontime = 0;
      return;
    }
    }
  }

  while (true) {
    if (pitch <= 10) {
      ontime = 0;
      lcd.setCursor(0, 0);
      emiwarn = true;
      lcd.print("High EMI Likely ");
      memset(activeNotes, 0, sizeof(activeNotes));
      lastNote = 255;  // no note is playing
      OCR1B = 0;       // turn off PWM
      noteIsPlaying = false;
      return;
    } else {
    IrmsCalc = 0.5 * PeakCurrent * sqrt(freqHz * ontime * 0.000001);

    if (IrmsCalc < (Irms - 1)) {
      ontime += 0.1;
    }
    else if (IrmsCalc > (Irms + 1)) {
      ontime -= 0.1;
    }
    else {
      break;
    }

    if (abs(ontime - lastDisplayOntime) >= 0.5) {
      lastDisplayOntime = ontime;
    }

    if (ontime <= 0 || ontime >= 65535) {
      emiwarn = true;
      lcd.setCursor(0, 0);
      lcd.print("Cant match Irms ");
      ontime = 0;
      return;
    }
    }
  }

  if (ontime >= ontimeGOAL){
    ontime = ontimeGOAL;
    IrmsCalc = 0.5 * PeakCurrent * sqrt(freqHz * ontime * 0.000001);
  }
  
  OCR1B = ontime;
  float Dfreq = OCR1A;
  notestrength = (ontime/ontimeGOAL)*7;
  lcd.setCursor(0, 0);
  lcd.print(round(ontime));
  lcd.print("us");
  if ((round(ontimeGOAL))<100) {
    lcd.print(" ");
  }
  if ((round(1 / (Dfreq * 0.000001)))<1000) {
    lcd.print(" ");
  }
  lcd.print(round(1 / (Dfreq * 0.000001)));
  lcd.print("Hz    ");
                noteIsPlaying = true;
                return; // Keep playing
            }
        }

        // If no active notes remain, stop the PWM signal
        OCR1B = 0;
        noteIsPlaying = false;
    }
}
