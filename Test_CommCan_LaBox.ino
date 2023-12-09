/*

  Test de communication CXAN avec LaBox Locoduino

  Christophe Bobille - Locoduino

  v 0.3 - 08/12/23
  v 0.4 - 09/12/23 - Optimisation de la fonction 0xFE
  v 0.5 - 09/12/23 - Ajout de la reception de messages en provenance de la centrale LaBox.
                     Pour ce test, c'est la mesure de courant qui a été choisie
*/

#ifndef ARDUINO_ARCH_ESP32
#error "Select an ESP32 board"
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

const uint8_t myID = 15;   // Identifiant de cet expéditeur au hasard
const uint8_t hash = 127;  // hash
byte prioMsg = 0;

struct Loco {
  uint16_t address;
  uint8_t speed;
  uint8_t direction;
  byte fn0;
  byte fn1;
  byte fn2;
};
struct Loco loco[3];

void Task0(void *pvParameters);

#include <ACAN_ESP32.h>

/* ----- CAN ----------------------*/
#define CAN_RX GPIO_NUM_22  // Choisir les broches RX et TX en fonction de votre montage
#define CAN_TX GPIO_NUM_23
static const uint32_t DESIRED_BIT_RATE = 1000UL * 1000UL;  // 1 Mb/s

class CanMsg {
public:
  CanMsg() = delete;
  static void setup();
  static void loop();
  static void sendMsg(CANMessage &);
  static void sendMsg(byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte, byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte, byte, byte, byte, byte, byte, byte);
};

void CanMsg::setup() {
  //debug.printf("[CanConfig %d] : Configure ESP32 CAN\n", __LINE__);
  ACAN_ESP32_Settings settings(DESIRED_BIT_RATE);
  settings.mRxPin = CAN_RX;
  settings.mTxPin = CAN_TX;
  uint32_t errorCode;

  errorCode = ACAN_ESP32::can.begin(settings);
  //debug.printf("[CanConfig %d] : config without filter\n", __LINE__);

  if (errorCode == 0)
    Serial.printf("[CanConfig %d] : configuration OK !\n", __LINE__);
  else {
    Serial.printf("[CanConfig %d] : configuration error 0x%x\n", __LINE__, errorCode);
    return;
  }
}

void CanMsg::sendMsg(CANMessage &frame) {
  if (0 == ACAN_ESP32::can.tryToSend(frame))
    Serial.printf("Echec envoi message CAN\n");
  else
    Serial.printf("Envoi fonction 0x%0X\n", (frame.id & 0x7F8) >> 3);
}

auto parseMsg = [](CANMessage &frame, byte priorite, byte idExp, byte idDes, byte fonct) -> CANMessage {
  frame.id |= priorite << 27;  // Priorite 0, 1 ou 2
  frame.id |= idExp << 19;     // ID expediteur
  frame.id |= idDes << 11;     // Hash
  frame.id |= fonct << 3;      // Fonction appelée
  frame.ext = true;
  return frame;
};

void CanMsg::sendMsg(byte priorite, byte idExp, byte idDes, byte fonct) {
  CANMessage frame;
  frame = parseMsg(frame, priorite, idExp, idDes, fonct);
  frame.len = 0;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte idExp, byte idDes, byte fonct, byte data0) {
  CANMessage frame;
  frame = parseMsg(frame, priorite, idExp, idDes, fonct);
  frame.len = 1;
  frame.data[0] = data0;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte idExp, byte idDes, byte fonct, byte data0, byte data1) {
  CANMessage frame;
  frame = parseMsg(frame, priorite, idExp, idDes, fonct);
  frame.len = 2;
  frame.data[0] = data0;
  frame.data[1] = data1;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte idExp, byte idDes, byte fonct, byte data0, byte data1, byte data2) {
  CANMessage frame;
  frame = parseMsg(frame, priorite, idExp, idDes, fonct);
  frame.len = 3;
  frame.data[0] = data0;
  frame.data[1] = data1;
  frame.data[2] = data2;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte idExp, byte idDes, byte fonct, byte data0, byte data1, byte data2, byte data3) {
  CANMessage frame;
  frame = parseMsg(frame, priorite, idExp, idDes, fonct);
  frame.len = 4;
  frame.data[0] = data0;
  frame.data[1] = data1;
  frame.data[2] = data2;
  frame.data[3] = data3;
  CanMsg::sendMsg(frame);
}



// Lecture des informations en retour de LaBox
void recepCan(void *pvParameter) {
  CANMessage frameIn;
  while (1) {
    while (ACAN_ESP32::can.receive(frameIn)) {
      //Serial.printf("Reception de la fonction : 0x%0X\n", (frameIn.id & 0x7F8) >> 3);
      Serial.printf("Mesure de courant : %d\n", (frameIn.data[0] << 8) + frameIn.data[1]);
    }
    vTaskDelay(100 / portTICK_RATE_MS);
  }
}




void setup() {
  Serial.begin(115200);
  CanMsg::setup();

  // Exemple -> renseigner une adresse de locomotive valide
  loco[0].address = 22;

  xTaskCreatePinnedToCore(&recepCan, "recepCan", 2 * 1024, NULL, 5, NULL, 0);

  // Met LaBox power Main sur on
  for (byte i = 0; i < 3; i++) {                    // Repetition du message !!! Pas forcement nécessaire
    CanMsg::sendMsg(prioMsg, myID, hash, 0xFE, 1);  // Message à la centrale DCC++
    delay(1000);
  }
}

void loop() {
  /*------------------------------------------------------------------
   Serie de commandes envoyées a LaBox pour tests
  --------------------------------------------------------------------*/
  // Arret de la locomotive
  loco[0].speed = 0;
  loco[0].direction = 1;
  CanMsg::sendMsg(prioMsg, myID, hash, 0xF0, loco[0].address & 0xFF00, loco[0].address & 0x00FF, loco[0].speed, loco[0].direction);  // Message à la centrale DCC++
  delay(10);

  // Test des differentes fonctions du decodeur
  for (byte i = 0; i <= 28; i++) {
    // Activation
    CanMsg::sendMsg(prioMsg, myID, hash, 0xF1, loco[0].address & 0xFF00, loco[0].address & 0x00FF, i, 1);  // Message à la centrale DCC++
    delay(2000);
    // Desactivation
    CanMsg::sendMsg(prioMsg, myID, hash, 0xF1, loco[0].address & 0xFF00, loco[0].address & 0x00FF, i, 0);  // Message à la centrale DCC++
    delay(10);
  }

  // Active les feux et le bruit de la locomotive
  CanMsg::sendMsg(prioMsg, myID, hash, 0xF1, loco[0].address & 0xFF00, loco[0].address & 0x00FF, 0, 1);  // Message à la centrale DCC++
  delay(10);
  CanMsg::sendMsg(prioMsg, myID, hash, 0xF1, loco[0].address & 0xFF00, loco[0].address & 0x00FF, 1, 1);  // Message à la centrale DCC++
  delay(10);

  // Avant 25
  loco[0].speed = 100;
  loco[0].direction = 1;
  CanMsg::sendMsg(prioMsg, myID, hash, 0xF0, loco[0].address & 0xFF00, loco[0].address & 0x00FF, loco[0].speed, loco[0].direction);  // Message à la centrale DCC++
  delay(15000);

  // Avant 50
  loco[0].speed = 50;
  loco[0].direction = 1;
  CanMsg::sendMsg(prioMsg, myID, hash, 0xF0, loco[0].address & 0xFF00, loco[0].address & 0x00FF, loco[0].speed, loco[0].direction);  // Message à la centrale DCC++
  delay(15000);

  // Arriere 50
  loco[0].speed = 50;
  loco[0].direction = 0;
  CanMsg::sendMsg(prioMsg, myID, hash, 0xF0, loco[0].address & 0xFF00, loco[0].address & 0x00FF, loco[0].speed, loco[0].direction);  // Message à la centrale DCC++
  delay(15000);

  // emergency stop
  CanMsg::sendMsg(prioMsg, myID, hash, 0xFF);  // Message à la centrale DCC++
  delay(1000);
}
