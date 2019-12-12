//Introduction des librairies
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Ethernet2.h>

// Initialisation des chaines de caractères
String txt = "";
String data = "";

// Déclaration des constantes (broches des composants)
const int pinBuzzer = 6;
const int pinLedRouge = 3;
const int pinLedVerte = 8;
const int pinGache = 7;
const int pinBouton = 2;

// Déclaration des variables
int dataB = 0;
int dataBtn = 0;
int positiondata = 0;
int debutdata = 0;
int findata = 0;
int dataE = 0;

// Définition des broches nécéssaires pour la communication bluetooth
#define BT_TX 4
#define BT_RX 5

// Initialisation de la variable correspondant à l'adresse MAC du shield Ethernet
byte mac[] = {0x90, 0xA2, 0xDA, 0x10, 0x09, 0x99};

// Initialisation de la variable correspondant à l'adresse IP du serveur
char server[] = "example.com";

// Initialisation du liaison Ethernet
EthernetClient client;

// Initialisation de la liaison série nécéssaire pour la communication bluetooth
SoftwareSerial mySerial(BT_RX, BT_TX);

// Initialisation du système
void setup() {

  // Démarrage de la liaison série
  Serial.begin(9600);

  // Démarrage de la liaison série bluetooth
  mySerial.begin(9600);

  // Déclaration de l'état des composants
  pinMode(pinBuzzer, OUTPUT);
  pinMode(pinLedRouge, OUTPUT);
  pinMode(pinLedVerte, OUTPUT);
  pinMode(pinGache, OUTPUT);
  pinMode(pinBouton, INPUT);

  // Démarrage des tests de fonctionnement
  Serial.println("Demarrage des tests de fonctionnement du systeme");

  // Test de l'adresse MAC
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Adresse MAC non renseignee ou fausse");
    //Signifie une durée infinie
    for (;;);
  }

  //Réinitialiser une seconde fois la carte Ethernet
  delay(1000);
  Serial.println("connexion...");

  // Test de récupération de la donnée dataE provenant du serveur alwaysdata
  if (client.connect(server, 80)) {
    Serial.println("connexion OK");
    //Faire une requête HTTP sur le serveur afin de récupérer la donnée de déverrouillage de la gache
    client.println("GET /data.txt HTTP/1.1");
    client.println("Host: texierelevesinrabelais.alwaysdata.net");
    Serial.println("Connexion avec le serveur etablie, dataE recuperee");
    client.println();
  }

  else {
    Serial.println("Echec de la connexion au serveur, dataE non recue");
  }

  dataBtn = digitalRead(pinBouton);

  //Test de l'envoie de la donnée dataBtn sur le serveur alwaysdata
  if (client.connect(server, 80)) {
    Serial.println("connexion OK");
    //Affectation de la variable correspondant à l'état du capteur de position sur le serveur grâce à une requette HTTP
    client.print("GET /posporte.php?DATA=");
    client.print(dataBtn);
    client.println(" HTTP/1.1");
    client.println("Host: texierelevesinrabelais.alwaysdata.net");
    Serial.print("Connexion avec le serveur etablie, dataBtn envoyee : ");
    Serial.println(dataBtn);
    client.println();
  }

  else {
    Serial.println("Echec de la connexion au serveur, dataBtn non envoyee");
  }

  // Effacer toutes les informations stockées en mémoire sur la carte pour libérer l'espace
  Serial.println("fin des tests de fonctionnement");
  client.stop();

}


void loop() {

  // Lecture du capteur de position de porte
  dataBtn = digitalRead(pinBouton);

  if (mySerial.available()) {
    dataB = mySerial.read();

    if (dataB == 1) {
      analogWrite(pinBuzzer, 100);
      delay(200);
      analogWrite(pinBuzzer, 0);;

      // Tant qu'une des deux variables dataE ou dataB est égale à 1, laisser la gache déverrouillée
      while (dataB == 1) {
        digitalWrite(pinLedVerte, HIGH);
        digitalWrite(pinLedRouge, LOW);
        digitalWrite(pinGache, HIGH);;
        dataB = 0;
      }

      // Si la gache était déverrouillée et que les deux variables dataE et dataB sont passées à 0, alors attendre 5 secondes avant de reverrouiller la gache
      delay(5000);
      analogWrite(pinBuzzer, 100);
      delay(200);
      analogWrite(pinBuzzer, 0);
      delay(100);
      analogWrite(pinBuzzer, 100);
      delay(200);
      analogWrite(pinBuzzer, 0);
    }

    // Si aucunne des deux variables dataE ou dataB est égale à 1, alors le programme verrouille la gache
    else {
      digitalWrite(pinLedVerte, LOW);
      digitalWrite(pinLedRouge, HIGH);
      digitalWrite(pinGache, LOW);
      analogWrite(pinBuzzer, 0);
    }
  }



  // Appel du sous programme permettant la reception de la donnée sur le serveur
  reception_donnee_http();


  // Entrée dans une boucle infinie
  for (;;) {
    while (client.available()) {
      char c = client.read();
      txt = txt + c;
      positiondata = txt.indexOf("DATA");
      debutdata = positiondata + 5;
      findata = debutdata + 1;
      data = txt.substring(debutdata, findata);
      // Conversion de la chaine de caractères contenant la donnée en entrée numérique
      dataE = data.toInt();
    }

    // Si le serveur est déconnecté, arrêter la communication et récupererla donnée "dataE" à partir des données reçues du serveur
    if (!client.connected()) {
      Serial.println();
      Serial.println("deconnexion.");
      //Localisation du mot "DATA" dans la chaine de carectères récupérés sur le serveur
      Serial.println(dataE);
      client.read();
      client.stop();
      //Réinitialisation des variables et chaines de caractères à leur valeurs initiale
      positiondata = 0;
      debutdata = 0;
      findata = 0;
      data = "";
      txt = "";

      // Sortie de la boucle infinie
      break;
    }
  }

  // Appel du sous programme permettant la reception de la donnée sur le serveur
  envoie_donnee_http();

  // Effacer toutes les informations stockées en mémoire sur la carte pour libérer l'espace
  delay(1000);

  // Si la connexion Bluetooth est disponible, affecter à la variable dataB le donnée reçue par bluetooth

  // Si dataE ou dataB = 1, déverrouiller la gâche
  if (dataE == 1) {
    analogWrite(pinBuzzer, 100);
    delay(200);
    analogWrite(pinBuzzer, 0);;

    digitalWrite(pinLedVerte, HIGH);
    digitalWrite(pinLedRouge, LOW);
    digitalWrite(pinGache, HIGH);

    // Si la gache était déverrouillée et que les deux variables dataE et dataB sont passées à 0, alors attendre 5 secondes avant de reverrouiller la gache
    delay(5000);
    analogWrite(pinBuzzer, 100);
    delay(200);
    analogWrite(pinBuzzer, 0);
    delay(100);
    analogWrite(pinBuzzer, 100);
    delay(200);
    analogWrite(pinBuzzer, 0);
    digitalWrite(pinLedVerte, LOW);
    digitalWrite(pinLedRouge, HIGH);
    digitalWrite(pinGache, LOW);
    analogWrite(pinBuzzer, 0);
  }

  // Si aucune des deux variables dataE ou dataB est égale à 1, alors le programme verrouille la gache
  else {
    digitalWrite(pinLedVerte, LOW);
    digitalWrite(pinLedRouge, HIGH);
    digitalWrite(pinGache, LOW);
    analogWrite(pinBuzzer, 0);
  }
}



// Fonction effectuant une requete HTTP sur le serveur pour récupérer la donnée dataE
void reception_donnee_http() {

  // Réinitialiser une seconde fois la carte Ethernet:
  delay(1000);
  Serial.println("connexion...");

  // Essais de connection au serveur du site internet
  if (client.connect(server, 80)) {
    Serial.println("connexion OK");
    //Faire une requête HTTP sur le serveur afin de récupérer la donnée de déverrouillage de la gache
    client.println("GET /data.txt HTTP/1.1");
    client.println("Host: texierelevesinrabelais.alwaysdata.net");
    Serial.println("Connexion avec le serveur etablie, dataE recuperee");
    client.println();
  }

  else
  {
    Serial.println("Echec de la connexion au serveur, dataE non recue");
  }
}

// Fonction effectuant une requete HTTP sur le serveur pour envoyer la donnée dataBtn
void envoie_donnee_http() {

  dataBtn = digitalRead(pinBouton);

  // Réinitialiser une seconde fois la carte Ethernet:
  delay(1000);
  Serial.println("connexion...");

  // Essais de connection au serveur du site internet
  if (client.connect(server, 80)) {
    Serial.println("connexion OK");
    // Affectation de la variable correspondant à l'état du capteur de position sur le serveur grâce à une requette HTTP
    client.print("GET /posporte.php?DATA=");
    client.print(dataBtn);
    client.println(" HTTP/1.1");
    client.println("Host: exemple.com");
    Serial.print("Connexion avec le serveur etablie, dataBtn envoyee : ");
    Serial.println(dataBtn);
    client.println();
  }

  else
  {
    Serial.println("Echec de la connexion au serveur, dataBtn non envoyee");
  }
  client.stop();
}
