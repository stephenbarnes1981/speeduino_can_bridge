#define SPEEDUINO_CANID 0x00
#define SPEEDUINO_R_COMMAND 0x30

#define MILLIS_BETWEEN_READS 1000

#define DATA_TO_REQUEST 16
#define COOLANT_OFFSET 7
#define RPM_OFFSET 14

byte dataCoolant; // current coolant
byte dataRpmLo;   // current rpm lo byte
byte dataRpmHi;   // current rpm hi byte

byte requiredBytes = 0;             // required number of bytes
byte receivedBytes = 0;             // received number of bytes
byte serialBuffer[DATA_TO_REQUEST]; // buffer for storing serial data

unsigned long lastMillis;

enum State {
  state_waiting,
  state_request_data,
  state_reading_data,
  state_storing_data,
  state_writing_canbus
};

State currentState = state_waiting;

void setup() {
  Serial.begin(9600);
  // TODO: initialise canbus
  lastMillis = millis();
}

void loop() {
  switch (currentState) {
    case state_waiting:
      if (timeout()) {
        currentState = state_request_data;
      }
      break;
    case state_request_data:
      speeduinoRequestRealtime(0, DATA_TO_REQUEST);
      currentState = state_reading_data;
      break;
    case state_reading_data:
      if (readSerial()) {
        currentState = state_storing_data;
      }
      break;
    case state_storing_data:
      // first byte should be r and second should be command
      if (serialBuffer[0] == (byte)'r' && serialBuffer[1] == SPEEDUINO_R_COMMAND) {
        dataCoolant = serialBuffer[2 + COOLANT_OFFSET];
        dataRpmLo = serialBuffer[2 + RPM_OFFSET + 0];
        dataRpmHi = serialBuffer[2 + RPM_OFFSET + 1];
        currentState = state_writing_canbus;
      } else {
        // error state?
      }
      break;
    case state_writing_canbus:
      stateWritingCanbus();
      currentState = state_waiting;
      break;
    default:
      // shouldn't get here...
      break;
  }
}

// check how many millis have elapsed and return true if we
// have reached the required number
bool timeout() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastMillis >= MILLIS_BETWEEN_READS) {
    lastMillis = currentMillis;
    return true;
  }
  return false;
}

// send a request for realtime data
// reset received bytes and set required data length
void speeduinoRequestRealtime(byte data_offset, byte data_length) {
  Serial.print('r');
  Serial.print(SPEEDUINO_CANID);
  Serial.print(SPEEDUINO_R_COMMAND);
  Serial.print(data_offset);
  Serial.print(data_length);

  receivedBytes = 0;
  requiredBytes = data_length + 2;
}

// read from serial, return true if read enough bytes into buffer
bool readSerial() {
  while (Serial.available()) {
    serialBuffer[receivedBytes] = Serial.read();
    receivedBytes++;
  }
  return requiredBytes == receivedBytes;
}

void stateWritingCanbus() {
  Serial.print("coolant: ");
  Serial.print(dataCoolant);
  Serial.println();
  Serial.print("rpm: ");
  Serial.print(dataRpmLo);
  Serial.print(dataRpmHi);
  Serial.println();

  // TODO: write to data canbus
}