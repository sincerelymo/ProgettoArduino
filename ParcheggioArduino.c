#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// Inizializzazione del display LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Sensori di ingresso e uscita
const int trigIngresso = 11;
const int echoIngresso = 10;

const int trigUscita = 4;
const int echoUscita = 3;

// Pin per il servo motore e i LED
const int pinServo = 9;
const int pinLedVerde = 2;
const int pinLedRosso = 7;

const int postiMax = 5; // Numero massimo di posti
int postiDisponibili = postiMax;

Servo cancelloServo;

// Pin dei segmenti a, b, c, d, e, f, g del display 7 segmenti
const int pinSegmenti[7] = {A0, A1, A2, A3, 5, 6, 8};

// Mappa cifre 0-9 per display a 7 segmenti (catodo comune)
const byte cifre[10][7] = {
  {1,1,1,1,1,1,0}, // 0
  {0,1,1,0,0,0,0}, // 1
  {1,1,0,1,1,0,1}, // 2
  {1,1,1,1,0,0,1}, // 3
  {0,1,1,0,0,1,1}, // 4
  {1,0,1,1,0,1,1}, // 5
  {1,0,1,1,1,1,1}, // 6
  {1,1,1,0,0,0,0}, // 7
  {1,1,1,1,1,1,1}, // 8
  {1,1,1,1,0,1,1}  // 9
};

bool autoRilevataIngresso = false;
bool autoRilevataUscita = false;
unsigned long ultimoRilevamentoIngresso = 0;
unsigned long ultimoRilevamentoUscita = 0;
const unsigned long tempoDebounce = 3000;

void setup() {
  pinMode(trigIngresso, OUTPUT);
  pinMode(echoIngresso, INPUT);
  pinMode(trigUscita, OUTPUT);
  pinMode(echoUscita, INPUT);
  pinMode(pinLedVerde, OUTPUT);
  pinMode(pinLedRosso, OUTPUT);

  for (int i = 0; i < 7; i++) {
    pinMode(pinSegmenti[i], OUTPUT);
  }

  cancelloServo.attach(pinServo);
  cancelloServo.write(0);

  lcd.init();
  lcd.backlight();

  lcd.clear();
  lcd.write(67);  // C
  lcd.write(104); // h
  lcd.write(105); // i
  lcd.write(117); // u
  lcd.write(115); // s
  lcd.write(111); // o

  lcd.setCursor(0, 1);
  lcd.write(80);  // P
  lcd.write(111); // o
  lcd.write(115); // s
  lcd.write(116); // t
  lcd.write(105); // i
  lcd.write(32);  // spazio
  lcd.write(68);  // D
  lcd.write(105); // i
  lcd.write(115); // s
  lcd.write(112); // p
  lcd.write(58);  // :
  lcd.write(32);  // spazio
  lcd.write(45);  // -
  lcd.write(62);  // >

  visualizzaCifra(postiDisponibili); // Mostra il numero iniziale
  digitalWrite(pinLedRosso, HIGH);
  digitalWrite(pinLedVerde, LOW);

  Serial.begin(9600);
}

void loop() {
  long distanzaIngresso = misuraDistanza(trigIngresso, echoIngresso);
  long distanzaUscita = misuraDistanza(trigUscita, echoUscita);

  // Rilevamento all'ingresso (tra 7 cm e 14 cm)
  if (distanzaIngresso >= 7 && distanzaIngresso <= 14 && !autoRilevataUscita) {
    if (!autoRilevataIngresso && millis() - ultimoRilevamentoIngresso > tempoDebounce) {
      autoRilevataIngresso = true;
      ultimoRilevamentoIngresso = millis();

      if (postiDisponibili > 0) {
        // Aggiungi un delay di 800 ms prima di alzare il servo motore
        delay(800);

        // Apri cancello per l'ingresso
        cancelloServo.write(90);
        digitalWrite(pinLedVerde, HIGH);
        digitalWrite(pinLedRosso, LOW);

        lcd.clear();
        lcd.write(65);  // A
        lcd.write(112); // p
        lcd.write(101); // e
        lcd.write(114); // r
        lcd.write(116); // t
        lcd.write(111); // o

        // Messaggio per "Posti disp: -->"
        lcd.setCursor(0, 1);
        lcd.write(80);  // P
        lcd.write(111); // o
        lcd.write(115); // s
        lcd.write(116); // t
        lcd.write(105); // i
        lcd.write(32);  // spazio
        lcd.write(100); // d
        lcd.write(105); // i
        lcd.write(115); // s
        lcd.write(112); // p
        lcd.write(58);  // :
        lcd.write(32);  // spazio
        lcd.write(45);  // -
        lcd.write(62);  // >

        delay(3000);

        // Chiudi cancello dopo l'ingresso
        cancelloServo.write(0);
        digitalWrite(pinLedVerde, LOW);
        digitalWrite(pinLedRosso, HIGH);

        postiDisponibili--;
        delay(1000);
        visualizzaCifra(postiDisponibili);
        delay(500);
      } else {
        // Messaggio "Pieno"
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.write(80);  // P
        lcd.write(105); // i
        lcd.write(101); // e
        lcd.write(110); // n
        lcd.write(111); // o
        delay(1500);
      }
    }
  } else if (autoRilevataIngresso && distanzaIngresso > 14) {
    autoRilevataIngresso = false;
    // Messaggio "Chiuso"
    lcd.clear();
    lcd.write(67);  // C
    lcd.write(104); // h
    lcd.write(105); // i
    lcd.write(117); // u
    lcd.write(115); // s
    lcd.write(111); // o

    lcd.setCursor(0, 1);
    lcd.write(80);  // P
    lcd.write(111); // o
    lcd.write(115); // s
    lcd.write(116); // t
    lcd.write(105); // i
    lcd.write(32);  // spazio
    lcd.write(68);  // D
    lcd.write(105); // i
    lcd.write(115); // s
    lcd.write(112); // p
    lcd.write(58);  // :
    lcd.write(32);  // spazio
    lcd.write(45);  // -
    lcd.write(62);  // >
  }

  // Rilevamento all'uscita (tra 0 cm e 6 cm)
  if (distanzaUscita >= 0 && distanzaUscita <= 6 && !autoRilevataIngresso) {
    if (!autoRilevataUscita && millis() - ultimoRilevamentoUscita > tempoDebounce) {
      autoRilevataUscita = true;
      ultimoRilevamentoUscita = millis();

      if (postiDisponibili < postiMax) {
        // Aggiungi un delay di 800 ms prima di alzare il servo motore
        delay(800);

        // Apri cancello per l'uscita
        cancelloServo.write(90);
        digitalWrite(pinLedVerde, HIGH);
        digitalWrite(pinLedRosso, LOW);

        lcd.clear();
        lcd.write(65);  // A
        lcd.write(112); // p
        lcd.write(101); // e
        lcd.write(114); // r
        lcd.write(116); // t
        lcd.write(111); // o

        // Messaggio per "Posti disp: -->"
        lcd.setCursor(0, 1);
        lcd.write(80);  // P
        lcd.write(111); // o
        lcd.write(115); // s
        lcd.write(116); // t
        lcd.write(105); // i
        lcd.write(32);  // spazio
        lcd.write(100); // d
        lcd.write(105); // i
        lcd.write(115); // s
        lcd.write(112); // p
        lcd.write(58);  // :
        lcd.write(32);  // spazio

        visualizzaCifra(postiDisponibili);

        delay(3000);

        // Chiudi cancello dopo l'uscita
        cancelloServo.write(0);
        digitalWrite(pinLedVerde, LOW);
        digitalWrite(pinLedRosso, HIGH);

        postiDisponibili++;
        delay(500);
      }
    }
  } else if (autoRilevataUscita && distanzaUscita > 6) {
    autoRilevataUscita = false;
    // Messaggio "Chiuso"
    lcd.clear();
    lcd.write(67);  // C
    lcd.write(104); // h
    lcd.write(105); // i
    lcd.write(117); // u
    lcd.write(115); // s
    lcd.write(111); // o

    lcd.setCursor(0, 1);
    lcd.write(80);  // P
    lcd.write(111); // o
    lcd.write(115); // s
    lcd.write(116); // t
    lcd.write(105); // i
    lcd.write(32);  // spazio
    lcd.write(68);  // D
    lcd.write(105); // i
    lcd.write(115); // s
    lcd.write(112); // p
    lcd.write(58);  // :
    lcd.write(32);  // spazio
    visualizzaCifra(postiDisponibili); // Mostra la disponibilità
  }

  // Trasmetti i dati con il bit di parità
  byte dati = postiDisponibili;
  bool paritaPari = true;
  byte datiConParita = aggiungiParita(dati, paritaPari);

  Serial.print("Dati con parità: ");
  Serial.println(datiConParita, BIN);

  delay(100);
}

// Funzione per misurare la distanza
long misuraDistanza(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long durata = pulseIn(echo, HIGH, 30000);
  long distanza = (durata * 0.034) / 2;
  return distanza;
}

// Funzione per visualizzare una cifra sul display a 7 segmenti
void visualizzaCifra(int cifra) {
  if (cifra < 0 || cifra > 9) return; // Protezione fuori range
  for (int i = 0; i < 7; i++) {
    digitalWrite(pinSegmenti[i], cifre[cifra][i]);
  }
}

// Calcola il bit di parità
int calcolaParita(byte dati, bool paritaPari) {
  int bitCount = 0;
  for (int i = 0; i < 8; i++) {
    if (dati & (1 << i)) {
      bitCount++;
    }
  }
  return paritaPari ? (bitCount % 2 == 0 ? 0 : 1) : (bitCount % 2 == 0 ? 1 : 0);
}

// Aggiungi il bit di parità ai dati
byte aggiungiParita(byte dati, bool paritaPari) {
  int bitParita = calcolaParita(dati, paritaPari);
  byte datiConParita = (dati << 1) | bitParita;
  return datiConParita;
}