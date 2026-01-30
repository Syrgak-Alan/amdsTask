#include <main.h>

#define SIZE 100
#define ROWS SIZE
#define COLS (SIZE * 2)
#define LED_PIN 13

float dataPool[ROWS][COLS];
float* rowPtrs[ROWS];
char ringbuf[50];

uint32_t blinkInterval = 500; 
unsigned long lastBlinkTime = 0;
bool ledState = LOW;
int currentK = -1; 

String inputBuffer = "";

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  for (int i = 0; i < ROWS; i++) rowPtrs[i] = dataPool[i];

  while (!Serial && millis() < 3000);
  Serial.println("\r\n--- Teensy OS v1.0 ---");
  Serial.print("> ");
}

void printInverse() {
  Serial.println("RESULT_START");
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < ROWS; j++) {
      Serial.print(rowPtrs[i][j + ROWS], 6);
      Serial.print(" ");
    }
    Serial.println(); 
    Serial.flush();
    delay(5);
  }
  Serial.println("RESULT_END");
}

void loadMatrix() {
  for (int i = 0; i < ROWS; i++) {
    rowPtrs[i] = &dataPool[i][0];
  }

  Serial.println("READY_FOR_DATA");
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < ROWS; j++) {

      // updating LED while parsing data
      while (Serial.available() == 0) { updateLED(); }
      rowPtrs[i][j] = Serial.parseFloat();
      
      // init of identity part of augmented matrix
      rowPtrs[i][j + ROWS] = (i == j) ? 1.0f : 0.0f;
    }
  }
  Serial.println("LOAD_COMPLETE");
}

void updateLED() {
  if (millis() - lastBlinkTime >= blinkInterval) {
    lastBlinkTime = millis();
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }
}

void stepInversion() {
  if (currentK < 0 || currentK >= ROWS) return;

  int pivotRow = currentK;
  for (int i = currentK + 1; i < ROWS; i++) {
    if (abs(rowPtrs[i][currentK]) > abs(rowPtrs[pivotRow][currentK])) pivotRow = i;
  }

  float* temp = rowPtrs[currentK];
  rowPtrs[currentK] = rowPtrs[pivotRow];
  rowPtrs[pivotRow] = temp;

  float pVal = rowPtrs[currentK][currentK];
  if (abs(pVal) < 1e-9) { Serial.println("ERROR: Singular"); currentK = -1; return; }
  
  for (int j = currentK; j < COLS; j++) rowPtrs[currentK][j] /= pVal;

  for (int i = 0; i < ROWS; i++) {
    if (i != currentK) {
      float factor = rowPtrs[i][currentK];
      for (int j = currentK; j < COLS; j++) rowPtrs[i][j] -= factor * rowPtrs[currentK][j];
    }
  }

  currentK++;
  if (currentK == ROWS) {
    Serial.println("FINISH");
    printInverse(); 
    currentK = -1;
    Serial.println("\nCalculation Complete.");
    Serial.print("> "); 
  }
}

void processCommand(String cmd) {
  cmd.trim();
  
  if (cmd == "cmd load") {
    Serial.println("\r\nDropping to raw mode for CSV load...");
    loadMatrix();
  } 
  else if (cmd == "cmd start") {
    if (currentK == -1) {
      currentK = 0;
      Serial.println("\r\nInversion started.");
    }
  } 

  if (cmd.startsWith("cmd frequency=")) {
      // extract everything after the '='
      String valStr = cmd.substring(14); 
      long periodMs = valStr.toInt();
      
      if (periodMs > 0) {
          // if freq is 100, blinkInterval is 50 (50ms ON, 50ms OFF = 100ms cycle)
          blinkInterval = (uint32_t)(periodMs / 2); 
          Serial.printf("\r\nBlink period set to %ld ms (Interval: %d ms)\n", periodMs, blinkInterval);
      }
  }
  else if (cmd != "") {
    Serial.println("\r\nUnknown command.");
  }
  
  Serial.print("> ");
}

void loop() {
  updateLED();
  stepInversion();

  while (Serial.available()) {
    char c = Serial.read();
    
    if (c != '\r' && c != '\n') Serial.print(c);

    if (c == '\r' || c == '\n') {
      Serial.println(); 
      processCommand(inputBuffer);
      inputBuffer = ""; 
    } 
    else if (c == '\b' || c == 127) {
      if (inputBuffer.length() > 0) {
        inputBuffer.remove(inputBuffer.length() - 1);
        Serial.print("\b \b");
      }
    } 
    else {
      inputBuffer += c;
    }
  }
}