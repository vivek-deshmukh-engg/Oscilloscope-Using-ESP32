#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <math.h>

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define I2C_ADDRESS 0x3C
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ADC input
#define ANALOG_PIN 34  // ADC1_CH6 on ESP32

// Buttons
#define PIN_VDIV_INC 32
#define PIN_VDIV_DEC 33
#define PIN_TDIV_INC 25
#define PIN_TDIV_DEC 26
#define PIN_RUNSTOP 27

// Waveform box
#define WAVE_X 0
#define WAVE_Y 8
#define WAVE_WIDTH 128
#define WAVE_HEIGHT 50
#define NUM_POINTS WAVE_WIDTH

uint8_t waveform[NUM_POINTS];
float vBuffer[NUM_POINTS];   // store real voltages for measurements

// Controls
float vPerDiv;    
float tPerDiv;    
bool running = true;

// ADC constants
const float VREF = 3.3;
const int ADC_MAX = 4095;
const int hDivisions = 8;

// Scaling arrays
float vDivisions[] = {0.1, 0.2, 0.5, 1.0, 2.0, 5.0};
int vDivIndex = 3; // start 1 V/div

float tDivisions[] = {10, 20, 50, 100, 200, 500, 1000}; // ms/div
int tDivIndex = 3; // start 100 ms/div

// Debounce helper
bool readButton(int pin){
  static unsigned long lastPress[40] = {0};
  if(digitalRead(pin) && millis()-lastPress[pin]>200){
    lastPress[pin]=millis();
    return true;
  }
  return false;
}

// Averaged ADC for smoother values
int readADC(int pin){
  long sum=0;
  for(int i=0;i<32;i++) sum+=analogRead(pin);
  return sum/32;
}

// Map voltage to OLED pixel
int voltageToPixel(float voltage){
  if(voltage<0) voltage=0;
  if(voltage>VREF) voltage=VREF;
  return map(voltage*1000, 0, VREF*1000, WAVE_HEIGHT-1, 0);
}

// --- Measurement functions ---
float computeVpp(float *buffer, int n){
  float vmin = 1e9, vmax = -1e9;
  for(int i=0;i<n;i++){
    if(buffer[i] < vmin) vmin = buffer[i];
    if(buffer[i] > vmax) vmax = buffer[i];
  }
  return vmax - vmin;
}

float computeVrms(float *buffer, int n){
  double sumsq = 0;
  for(int i=0;i<n;i++) sumsq += buffer[i]*buffer[i];
  return sqrt(sumsq/n);
}

// crude frequency estimate from zero-crossings
float computeFreq(float *buffer, int n, float sampleRate){
  int crossings = 0;
  for(int i=1;i<n;i++){
    if((buffer[i-1]<VREF/2 && buffer[i]>=VREF/2) ||
       (buffer[i-1]>VREF/2 && buffer[i]<=VREF/2)){
      crossings++;
    }
  }
  if(crossings < 2) return 0;
  float periods = crossings/2.0;
  float timeWindow = n / sampleRate;
  return periods / timeWindow;
}

void setup(){
  Serial.begin(115200);
  if(!display.begin(I2C_ADDRESS,true)){ while(1); }

  display.clearDisplay();
  display.display();

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  pinMode(PIN_VDIV_INC, INPUT_PULLDOWN);
  pinMode(PIN_VDIV_DEC, INPUT_PULLDOWN);
  pinMode(PIN_TDIV_INC, INPUT_PULLDOWN);
  pinMode(PIN_TDIV_DEC, INPUT_PULLDOWN);
  pinMode(PIN_RUNSTOP, INPUT_PULLDOWN);

  for(int i=0;i<NUM_POINTS;i++){
    waveform[i]=WAVE_HEIGHT/2;
    vBuffer[i]=0;
  }

  vPerDiv = vDivisions[vDivIndex];
  tPerDiv = tDivisions[tDivIndex];
}

void loop(){
  // --- Buttons ---
  if(readButton(PIN_VDIV_INC)) { vDivIndex++; if(vDivIndex>=sizeof(vDivisions)/sizeof(vDivisions[0])) vDivIndex=sizeof(vDivisions)/sizeof(vDivisions[0])-1; }
  if(readButton(PIN_VDIV_DEC)) { vDivIndex--; if(vDivIndex<0) vDivIndex=0; }
  vPerDiv = vDivisions[vDivIndex];

  if(readButton(PIN_TDIV_INC)) { tDivIndex++; if(tDivIndex>=sizeof(tDivisions)/sizeof(tDivisions[0])) tDivIndex=sizeof(tDivisions)/sizeof(tDivisions[0])-1; }
  if(readButton(PIN_TDIV_DEC)) { tDivIndex--; if(tDivIndex<0) tDivIndex=0; }
  tPerDiv = tDivisions[tDivIndex];

  if(readButton(PIN_RUNSTOP)) running=!running;

  // --- Update waveform ---
  static float vNow = 0;
  if(running){
    for(int i=0;i<NUM_POINTS-1;i++){
      waveform[i]=waveform[i+1];
      vBuffer[i]=vBuffer[i+1];
    }
    int adcVal=readADC(ANALOG_PIN);
    vNow=(adcVal*VREF)/ADC_MAX;
    vBuffer[NUM_POINTS-1]=vNow;
    waveform[NUM_POINTS-1]=voltageToPixel(vNow);
  }

  // --- Measurements ---
  float vpp = computeVpp(vBuffer, NUM_POINTS);
  float vrms = computeVrms(vBuffer, NUM_POINTS);

  float totalScreenTime = tPerDiv*hDivisions; // ms across screen
  float sampleRate = (NUM_POINTS / (totalScreenTime/1000.0)); // Hz
  float freq = computeFreq(vBuffer, NUM_POINTS, sampleRate);

  // --- Draw OLED ---
  display.clearDisplay();

  // Grid
  display.drawLine(WAVE_X, WAVE_Y+WAVE_HEIGHT/2, WAVE_X+WAVE_WIDTH, WAVE_Y+WAVE_HEIGHT/2, SH110X_WHITE);
  display.drawLine(WAVE_X+WAVE_WIDTH/2, WAVE_Y, WAVE_X+WAVE_WIDTH/2, WAVE_Y+WAVE_HEIGHT, SH110X_WHITE);

  // Waveform
  for(int x=0;x<NUM_POINTS-1;x++){
    display.drawLine(WAVE_X+x, WAVE_Y+waveform[x], WAVE_X+x+1, WAVE_Y+waveform[x+1], SH110X_WHITE);
  }

  // Labels
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0,0); display.printf("V=%.2fV",vNow);
  display.setCursor(64,0); display.printf("V/div=%.2f",vPerDiv);
  display.setCursor(0,58); display.printf("T/div=%.0fms",tPerDiv);
  display.setCursor(96,58); display.print(running?"RUN":"STOP");

  // Measurements
  display.setCursor(0, 0+8);  display.printf("Vpp=%.2f", vpp);
  display.setCursor(64, 0+8); display.printf("Vrms=%.2f", vrms);
  display.setCursor(0, 0+16); display.printf("f=%.1fHz", freq);

  display.display();

  // --- Delay per sample ---
  float delayPerSample = (totalScreenTime/NUM_POINTS);
  delay((int)delayPerSample);
}
