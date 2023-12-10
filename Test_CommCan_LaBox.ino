/*

  Test de communication CXAN avec LaBox Locoduino

  Christophe Bobille - Locoduino

  v 0.3 - 08/12/23
  v 0.4 - 09/12/23 - Optimisation de la fonction 0xFE
  v 0.5 - 09/12/23 - Ajout de la reception de messages en provenance de la centrale LaBox.
                     Pour ce test, c'est la mesure de courant qui a été choisie
  v 0.5.2 - 09/12/23
  v 0.5.3 - 10/12/23
  v 0.5.4 - 10/12/23 : Add POWERMODE::OVERLOAD
  v 0.6.0 - 10/12/23 : Presentation en classes et méthodes
*/

#ifndef ARDUINO_ARCH_ESP32
#error "Select an ESP32 board"
#endif

#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "CanMsg.h"
#include "LaBoxCmd.h"

const uint8_t myID = 15;  // Identifiant de cet expéditeur au hasard
const uint8_t hash = 127; // hash
byte prioMsg = 0;

Loco *loco = new Loco;

void Task0(void *pvParameters);


// Lecture des informations en retour de LaBox
void recepCan(void *pvParameter)
{
  CANMessage frameIn;
  while (1)
  {
    while (ACAN_ESP32::can.receive(frameIn))
    {
      switch ((frameIn.id & 0x7F8) >> 3)
      {
      case 0xFD:
        if (frameIn.data[0] == 2) // OVERLOAD
          Serial.printf("Power overload.\n");
        else
        {
          Serial.printf("Power %s\n", frameIn.data[0] ? "on" : "off");
          Serial.printf("Mesure de courant : %d\n", (frameIn.data[1] << 8) + frameIn.data[2]);
        }
        break;
      }
    }
    vTaskDelay(100 / portTICK_RATE_MS);
  }
}

enum : bool
{
  off,
  on
};

LaBoxCmd laBox(myID, hash);


void setup()
{
  Serial.begin(115200);
  CanMsg::setup();

  loco->address = 22,

  xTaskCreatePinnedToCore(&recepCan, "recepCan", 2 * 1024, NULL, 5, NULL, 0);
}

void loop()
{
  /*------------------------------------------------------------------
   Serie de commandes envoyées a LaBox pour tests
  --------------------------------------------------------------------*/

  // Power on
  laBox.setPower(on);
  delay(100);

  // Test des differentes fonctions du decodeur
  for (byte i = 0; i <= 28; i++)
  {
    // Activation
    loco->fn[i] = on;
    laBox.setFunction(loco, i);
    delay(1000);

    // Desactivation
    loco->fn[i] = off;
    laBox.setFunction(loco, i);
    delay(100);
  }

  // Active les feux et le bruit de la locomotive
  laBox.toggleFunction(loco, 0);
  laBox.toggleFunction(loco, 1);
  delay(10);

  // Avant 25
  loco->speed = 25;
  loco->direction = 1;
  laBox.setThrottle(loco);
  delay(15000);

  // Arret de la locomotive
  loco->speed = 0;
  laBox.setThrottle(loco);
  delay(5000);

  // Avant 50
  loco->speed = 50;
  laBox.setThrottle(loco);
  delay(15000);

  // Arriere 50
  laBox.toggleThrottleDir(loco);
  delay(15000);

  // Arriere 75
  loco->speed = 75;
  laBox.setThrottle(loco);
  delay(15000);

  // emergency stop
  laBox.emergency();
  delay(1000);

  // power off
  laBox.setPower(off);
  delay(5000);
}
