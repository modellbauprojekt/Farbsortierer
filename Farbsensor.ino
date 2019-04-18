#include <Wire.h>  // Bibliothek zur Nutzung des I2C-Datenbus am Arduino
#include <LiquidCrystal_I2C.h> // Bibliothek zum Steuern der Datenausgabe auf dem 16x2-Display
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Festlegender I2C Addresse (0x27 or 0x3F)
#include <Servo.h> // Bibliothek zum einfachen Ansteuern von Servos
Servo Servooben; //Erstellt für das Programm ein oberes Servo mit dem Namen "Servooben"
Servo Servounten; // dito für "Servounten"

typedef unsigned int uint;
//Verbindung der Farbsensorkontakte mit dem Arduino festlegen
const int s1 = 9;
const int s0 = 8; 
const int s2 = 12; 
const int s3 = 11;
const int out = 10;
// Variablen initialisieren
int rot = 0; 
int gruen = 0; 
int blau = 0;
int i; 
int loops; 
int OK;
// Counter für Smarties initialisieren

struct Color {
  uint lower_red;
  uint lower_green;
  uint lower_blue;
  uint higher_red;
  uint higher_green;
  uint higher_blue;
  uint matched;
  uint state_matched;
  uint out_value;
  char[] name;
}

Color colors[] = {
 struct Color( 
}
// Variablen für Servowinkel initialisieren
int Schieberwinkel = 90; 
int Rutschenwinkel = 90; 
int speedoben = 2; 
int speedunten = 0;


void setup()
{ 
  // PIN-Funktionen festlegen
  pinMode(s0, OUTPUT); 
  pinMode(s1, OUTPUT); 
  pinMode(s2, OUTPUT); 
  pinMode(s3, OUTPUT); 
  pinMode(out, INPUT); 
  digitalWrite(s0, HIGH); 
  digitalWrite(s1, LOW);
  // Servofunktionen anlegen
  Servooben.attach(7); 
  Servooben.write(Schieberwinkel); // Oberes Servo an Pin 7 und nach ganz links
  Servounten.attach(6); 
  Servounten.write(Rutschenwinkel); // Unteres Servo an Pin 6 und in die Mitte
  // Datenausgabe am PC-Monitor starten
  Serial.begin(9600); // Serielle Kommunikation zur Ausgabe auf das Display starten
  delay(500); // kurz auf Verbindungsaufbau warten...
  lcd.begin(16,2); // Wir haben ein 16x2-Display angeschlossen
  lcd.backlight(); // Die Hintergrundbeleuchtung einschalten
  lcd.clear(); // Display leeren
  lcd.setCursor(0,0);
  lcd.print("Scn#");
  lcd.setCursor(0,1); 
  lcd.print("R--- G--- B---"); 
} // SETUP Ende



void loop()
{ 
  loops++; 
  lcd.setCursor(0,0); 
  lcd.print("Scn#"); 
  lcd.print(loops); 
  lcd.print("   "); // Schleifendurchläufe ausgeben
  OK=0; // Status zurücksetzen. Wird auf 1 gesetzt, wenn eine Smartiefarbe zugeordnet werden konnte!

  // SCHRITT 1: Smartie holen
  Drehschieber(170);

  // SCHRITT 2: Smartie zum Sensor fahren
  Drehschieber(100);
  delay(1500);

  // SCHRITT 3: Farbsensor auslesen mit Hilfe der Funktion color(). Zurückgeliefert werden die Werte rot, gruen, blau
  color(); // Diese Funktion wird am Ende des Codes festgelegt (s.“void color();“)

  // SCHRITT 4: Farbwerte ausgeben
  Serial.print(" Wert Rot: ");
  Serial.print(rot, DEC); 
  Serial.print(" Wert Gruen: ");
  Serial.print(gruen, DEC);
  Serial.print(" Wert Blau: ");
  Serial.print(blau, DEC);
  Serial.print(" - ");
  lcd.setCursor(0,1); 
  lcd.print("R    G    B    ");
  lcd.setCursor(1,1); 
  lcd.print(rot); 
  lcd.setCursor(6,1);
  lcd.print(gruen);
  lcd.setCursor(11,1); 
  lcd.print(blau);

  // SCHRITT 5: Farbe & Servoposition zuordnen per IF-Bedingungen 
  // Das Herausfinden der richtigen Frenzwerte ist echt fummlig - zumal die Werte sich abhängig vom Umgebungslicht ändern können! Saublöd...

  for(uint i = 0; i < colors_length; i++) {
    if(      
    rot >= colors[i].lower_red 
      && rot <= colors[i].higher_red 
      && gruen >= colors[i].lower_green 
      && gruen <= colors[i].higher_green 
      && blau >= colors[i].lower_blue
      && blau <= colors[i].higher_blue
      ) {
      //Matches
      colors[i].matched++;
      lcd.setCursor(0,0);
      lcd.print(colors[i].matched);
      lcd.print(colors[i].name);
      Serial.println(colors[i].name);
      OK = colors[i].state_matched;
      Rutsche(colors[i].out_value);
    }
  } 

  if (OK == 0) { 
    Fehler++; 
    lcd.setCursor(0,0); 
    lcd.print("nicht eindeutig!"); 
    delay(1000); 
    lcd.setCursor(0,0); 
    lcd.print("                "); 
    Serial.println(" - unklar"); 
  } 
  if (OK == 1) {
    Drehschieber(60); 
  }
  delay(1000);



  if (leer == 10)
  { 
    lcd.clear(); 
    lcd.setCursor(0,0); 
    lcd.print("..gehe 30 Sek."); 
    lcd.setCursor(0,1); 
    lcd.print("  schlafen...");
    delay(3000); 
    lcd.noBacklight(); 
    for (i = 30 ; i > -1 ; i--) {
      delay(1000); 
      lcd.setCursor(7,0); 
      lcd.print(i); 
      lcd.print(" "); 
    }
    leer=0; 
    lcd.backlight();
    lcd.clear(); 
    lcd.setCursor(0,1); 
    lcd.print("R--- G--- B---"); 
  }

  // SCHRITT 6: Rutsche positionieren
  // Hier habe ich noch nix...

  // SCHRITT 7: Smartie auf Rutsche auswerfen
  // for (i=70 ; i>=35 ; i--)  { Servooben.write(i); delay(10); } delay(1000);

} // LOOP Ende



void color() // Hier werden die Werte vom Farbsensor ausgelesen und unter den
// entsprechenden Variablen gespeichert
{
  // Sensor für Rot-Messung einstellen
  digitalWrite(s2, LOW); 
  digitalWrite(s3, LOW); 
  delay(100);
  rot = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);

  // Sensor für Blau-Messung einstellen
  digitalWrite(s3, HIGH); 
  delay(100);
  blau = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);

  // Sensor für Grün-Messung einstellen
  digitalWrite(s2, HIGH); 
  delay(100);
  gruen = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
} // Funktion color() ENDE

int Drehschieber(int sollwert)
{ // Bei Aufruf soll das Servo auf $sollwert eingestellt werden.
  // $sollwert ist entweder kleiner, größer oder gleich der aktuellen Position in $Schieberwinkel
  if (sollwert < Schieberwinkel )
  { 
    while (sollwert < Schieberwinkel)
    {
      Schieberwinkel=Schieberwinkel-1; 
      delay(speedoben); 
      Servooben.write(Schieberwinkel);
      lcd.setCursor(8,0); 
      lcd.print("    "); 
      lcd.setCursor(9,0); 
      lcd.print(Schieberwinkel);
    } 
  }

  if (sollwert > Schieberwinkel )
  { 
    while (sollwert > Schieberwinkel)
    {
      Schieberwinkel=Schieberwinkel+1; 
      delay(speedoben); 
      Servooben.write(Schieberwinkel);
      lcd.setCursor(8,0); 
      lcd.print("    "); 
      lcd.setCursor(9,0); 
      lcd.print(Schieberwinkel); 
    } 
  }  
}

int Rutsche(int sollwert)
{ // Bei Aufruf soll das Servo auf $sollwert eingestellt werden.
  // $sollwert ist entweder kleiner, größer oder gleich der aktuellen Position in $Schieberwinkel
  if (sollwert < Rutschenwinkel )
  { 
    while (sollwert < Rutschenwinkel)
    {
      Rutschenwinkel=Rutschenwinkel-4; 
      delay(speedunten); 
      Servounten.write(Rutschenwinkel);
      lcd.setCursor(13,0); 
      lcd.print("    "); 
      lcd.setCursor(13,0); 
      lcd.print(Rutschenwinkel);
    } 
  }
  if (sollwert > Rutschenwinkel )
  { 
    while (sollwert > Rutschenwinkel)
    {
      Rutschenwinkel=Rutschenwinkel+4; 
      delay(speedunten); 
      Servounten.write(Rutschenwinkel);
      lcd.setCursor(13,0); 
      lcd.print("    "); 
      lcd.setCursor(13,0); 
      lcd.print(Rutschenwinkel); 
    } 
  }  
}

