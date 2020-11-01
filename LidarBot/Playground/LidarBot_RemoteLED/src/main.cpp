#include <Arduino.h>
#include <M5Stack.h>
#include <esp_now.h>

#include "espnow.h"
#include "keyboard.h"
#include "logo.h"

Espnow espnow;
KeyBoard keyboard;

static uint16_t distance[360], oldDisX[360], oldDisY[360];
uint8_t led[5] = {0x03, 0x03, 0x03, 0x03, 0x03};

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  if (espnow.OnRemotRecv(mac_addr, data, data_len)) {
    return;
  }

  if (data_len == 180) {
    int j = 0;
    for (int i = 0; i < 45; i++) {
      j = data[4 * i] * 256 + data[4 * i + 1];
      distance[j] = data[4 * i + 2] * 256 + data[4 * i + 3];
    }
  }
}

void MapDisplay(void) {
  uint16_t disX = 0, disY = 0;

  for (int showAngle = 0; showAngle < 360; showAngle++) {
    disX = (80 + (distance[showAngle] / 70) *
                     cos(3.14159 * showAngle / 180 + 0.13)) *
           2;
    disY = (100 + (distance[showAngle] / 70) *
                      sin(3.14159 * showAngle / 180 + 0.13)) *
           2;
    M5.Lcd.drawPixel(oldDisX[showAngle], oldDisY[showAngle], BLACK);

    if (distance[showAngle] == 250) {
      M5.Lcd.drawPixel(disX, disY, BLUE);
    } else {
      M5.Lcd.drawPixel(disX, disY, YELLOW);
    }

    oldDisX[showAngle] = disX;
    oldDisY[showAngle] = disY;
  }
}

void setup() {
  M5.begin();

  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.pushImage(0, 0, 320, 240, (uint16_t *)gImage_logo);
  M5.Lcd.setCursor(240, 1, 4);
  M5.Lcd.printf("JEENO");
  delay(2000);
  M5.Lcd.fillScreen(TFT_BLACK);

  M5.Speaker.beep();
  delay(100);
  M5.Speaker.mute();

  keyboard.Init();

  espnow.RemoteInit();
  esp_now_register_recv_cb(OnDataRecv);
}

uint8_t dataTankTurnMode[3] = {0, 0, 0};
uint8_t dataMecanumMode[3] = {0, 0, 1};

void motorControlTankTurnMode(uint8_t speed, uint8_t turn) {
  dataTankTurnMode[0] = turn;
  dataTankTurnMode[1] = speed;

  esp_now_send(espnow.peer_addr, dataTankTurnMode, 3);
}

void motorControlMecanumMode(uint8_t speedDirectionX, uint8_t speedDirectionY) {
  dataTankTurnMode[0] = speedDirectionX;
  dataTankTurnMode[1] = speedDirectionY;

  esp_now_send(espnow.peer_addr, dataMecanumMode, 3);
}

void loop() {
  espnow.RemoteConnectUpdate();
  keyboard.GetValue();
  esp_now_send(espnow.peer_addr, keyboard.keyData, 3);
  // motorControlTankTurnMode(1, 0);
  /*Serial.println("D0: " + String(distance[0]) + "\t\tD45: " +
                 String(distance[44]) + "\t\tD90: " + String(distance[89]) +
                 "\t\tD135: " + String(distance[134]) + "\t\tD180: " +
                 String(distance[179]) + +"\t\tD225: " + String(distance[224]) +
                 "\t\tD270: " + String(distance[269]) +
                 +"\t\tD315: " + String(distance[314]));*/

  String log = String(distance[269]) + "--------";
  M5.Lcd.drawString(log, 20, 100, 6);
  MapDisplay();

  if (digitalRead(37) == LOW) {
    while (digitalRead(37) == LOW)
      ;
    // esp_now_send(espnow.peer_addr, led, 4);
  }
}
