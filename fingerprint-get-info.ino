#include <Adafruit_Fingerprint.h>

HardwareSerial serialPort(2);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&serialPort);

void setup() {
  Serial.begin(9600);
  delay(2000);

  while (!Serial);
  delay(100);

  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor password accepted");
  }
    else {
    Serial.println("Fingerprint sensor password incorrect");
  }

  delay(2000); 
}

void loop() {
  Serial.println("Masukkan Sidik Jari!");
  delay(2000);

  while (!getFingerPrintUser());

  delay(4000);
}

void printHex(int num, int precision) {
  char tmp[16];
  char format[128];

  sprintf(format, "%%.%dX", precision);

  sprintf(tmp, format, num);
  Serial.print(tmp);
}

uint8_t getFingerPrintUser() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Sidik Jari Terdeteksi");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("Sidik Jari Tidak Terdeteksi");
      return p;
    default:
      Serial.println("Error Tidak Diketahui");
      return p;
  }

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Sidik Jari Terkonversi");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Gambar Terlalu Buram");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Tidak Menemukan Fitur");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Tidak Menemukan Fitur");
      return p;
    default:
      Serial.println("Error Tidak Diketahui");
      return p;
  }

  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Sidik Jari Cocok!");
    Serial.print("ID = ");
    Serial.println(finger.fingerID);
    Serial.print("Nilai Confidence = ");
    Serial.println(finger.confidence);
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Sidik Jari Tidak Cocok");
    return p;
  } else {
    Serial.println("Error Tidak Diketahui");
    return p;
  }

  Serial.print("Sedang Mengunduh Template dari ID = "); Serial.println(finger.fingerID);
  p = finger.loadModel(finger.fingerID);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Template Terunduh");
      break;
    default:
      Serial.print("Error Tidak Diketahui"); Serial.println(p);
      return p;
  }

  Serial.print("Mengambil Model dari ID = "); Serial.println(finger.fingerID);
  p = finger.getModel();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.print("Template "); Serial.print(finger.fingerID); Serial.println(" Sedang Ditransfer:");
      break;
    default:
      Serial.print("Unknown error "); Serial.println(finger.fingerID);
      return p;
  }

  uint8_t bytesReceived[534];
  memset(bytesReceived, 0xff, 534);

  uint32_t starttime = millis();
  int i = 0;
  while (i < 534 && (millis() - starttime) < 20000) {
    if (serialPort.available()) {
      bytesReceived[i++] = serialPort.read();
    }
  }
  Serial.print(i); Serial.println(" bytes read.");
  Serial.println("Decoding packet...");

  uint8_t fingerTemplate[512];
  memset(fingerTemplate, 0xff, 512);

  int uindx = 9, index = 0;
  memcpy(fingerTemplate + index, bytesReceived + uindx, 256);
  uindx += 256;
  uindx += 2;
  uindx += 9;
  index += 256;
  memcpy(fingerTemplate + index, bytesReceived + uindx, 256);

  // for (int i = 0; i < 512; ++i) {
  //   printHex(fingerTemplate[i], 2);
  // }
  // Serial.println("\ndone.");

  String fingerprintHex = "";
  for (int i = 0; i < 512; ++i) {
    printHex(fingerTemplate[i], 2);
    char hex[3];
    sprintf(hex, "%02X", fingerTemplate[i]);
    fingerprintHex += hex;
  }
  Serial.println("\ndone.");

  return p;
}
