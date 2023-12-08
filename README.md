# Test_CommCan_LaBox

 Le fichier Test_CommCan_LaBox.ino est un programme pour tester l'implantation du CAN et les commandes associées dans le projet LaBox de Locoduino. 

Ce programme ne fonctionne qu'avec une version en cours de développement du programme qui se trouve ici : https://github.com/Locoduino/CommandStation-EX-LaBox/tree/CAN-messaging.

Ce programme fonctionne que sur ESP32.

Configuration préalable :

Assurez-vous que vous avez sélectionné les bonnes broches pour la communication CAN. Par défaut, ce sont les broches 4 et 5 qui sont actives pour respectivement le RX et le TX.

#define CAN_RX GPIO_NUM_22 // Choisir les broches RX et TX en fonction de votre montage
#define CAN_TX GPIO_NUM_23

Sélectionnez le débit qui correspond à votre configuration :
static const uint32_t DESIRED_BIT_RATE = 1000UL * 1000UL;  // 1 Mb/s

Si vous modifiez le réglage ci-dessus, il vous faudra également modifier le programme de LaBox et configurer le débit à la même valeur.

Dans la fonction setup(), il faut renseigner l’adresse DCC de la locomotive que vous utiliserez pour les tests. Ici, nous avons choisi l’adresse 22.

// Exemple -> renseigner une adresse de locomotive valide
  loco[0].address = 22;
  

Dans la fonction setup() également, vous trouverez un premier test qui est la mise sous tension de LaBox :

  for (byte i = 0; i < 3; i++) {  // Repetition du message !!! Pas forcement nécessaire
      CanMsg::sendMsg(prioMsg, myID, hash, 0xFE, 1);  // Message à la centrale DCC++
    delay(100);
  }

Cette commande est répétée 3 fois. Ce n’est probablement pas nécessaire, mais il s’agit d’un programme de test.

Dans la fonction loop(), vous trouverez un certain nombre de tests déjà programmés. Les commentaires associés sont assez explicites pour que vous vous y retrouviez facilement.

  // Test des differentes fonctions du decodeur
  for (byte i = 0; i <= 28; i++) {
    // Activation
    CanMsg::sendMsg(prioMsg, myID, hash, 0xF1, loco[0].address & 0xFF00, loco[0].address & 0x00FF, i, 1);  // Message à la centrale DCC++
    delay(2000);
    // Desactivation
    CanMsg::sendMsg(prioMsg, myID, hash, 0xF1, loco[0].address & 0xFF00, loco[0].address & 0x00FF, i, 0);  // Message à la centrale DCC++
    delay(10);
  }

Cette partie de programme va successivement activer les 28 fonctions (potentielles) du décodeur. 9a a été l’occasion pour moi d’entendre certains bruitages pour la première fois !

  // Active les feux et le bruit de la locomotive
  CanMsg::sendMsg(prioMsg, myID, hash, 0xF1, loco[0].address & 0xFF00, loco[0].address & 0x00FF, 0, 1);  // Message à la centrale DCC++
  delay(10);
  CanMsg::sendMsg(prioMsg, myID, hash, 0xF1, loco[0].address & 0xFF00, loco[0].address & 0x00FF, 1, 1);  // Message à la centrale DCC++
  delay(10);

Cette autre partie de programme va activer les feux et le bruit d’exploitation pour le reste des tests.

// Avant 25
  loco[0].speed = 100;
  loco[0].direction = 1;
  CanMsg::sendMsg(prioMsg, myID, hash, 0xF0, loco[0].address & 0xFF00, loco[0].address & 0x00FF, loco[0].speed, loco[0].direction);  // Message à la centrale DCC++
  delay(15000);

Les tests suivants sont des tests de roulage, ici, marche avant (direction = 1) à la vitesse de 100 (sur 126) et ce pendant 15 secondes.

Le dernier test en fin de programme est le emergency stop, arrêt d’urgence qui s’applique à toutes les locomotives présentent sur le réseau

  // emergency stop
  CanMsg::sendMsg(prioMsg, myID, hash, 0xFF);  // Message à la centrale DCC++





![image](https://github.com/Locoduino/Test_CommCan_LaBox/assets/38000184/efca6410-2e89-4300-857b-e380c1af336d)
