#include <Wire.h>  // Bibliothek zur Nutzung des I2C-Datenbus am Arduino
#include <LiquidCrystal_I2C.h> // Bibliothek zum Steuern der Datenausgabe auf dem 16x2-Display
          LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Festlegender I2C Addresse (0x27 or 0x3F)
#include <Servo.h> // Bibliothek zum einfachen Ansteuern von Servos
Servo Servooben; //Erstellt für das Programm ein oberes Servo mit dem Namen "Servooben"
Servo Servounten; // dito für "Servounten"

//Verbindung der Farbsensorkontakte mit dem Arduino festlegen
const int s1 = 9; const int s0 = 8; const int s2 = 12; const int s3 = 11; const int out = 10;
// Variablen initialisieren
int rot = 0; int gruen = 0; int blau = 0; int i; int loops; int OK;
// Counter für Smarties initialisieren
int leer=0; int Fehler=0; int Rote=0; int Gelbe=0; int Pinke=0; int Orange=0; int Blaue=0; int Gruene=0;
// Variablen für Servowinkel initialisieren
int Schieberwinkel = 90; int Rutschenwinkel = 90; int speedoben = 0; int speedunten = 0;


void setup()
{ // PIN-Funktionen festlegen
  pinMode(s0, OUTPUT); pinMode(s1, OUTPUT); pinMode(s2, OUTPUT); pinMode(s3, OUTPUT); pinMode(out, INPUT); 
  digitalWrite(s0, HIGH); digitalWrite(s1, LOW);
  // Servofunktionen anlegen
  Servooben.attach(7); Servooben.write(Schieberwinkel); // Oberes Servo an Pin 7 und nach ganz links
  Servounten.attach(6); Servounten.write(Rutschenwinkel); // Unteres Servo an Pin 6 und in die Mitte
  // Datenausgabe am PC-Monitor starten
  Serial.begin(115200); // Serielle Kommunikation zur Ausgabe auf das Display starten
  Serial.println("R;G;B;Helligkeit;"); // Kopfzeile ausgeben
  delay(500); // kurz auf Verbindungsaufbau warten...
  lcd.begin(16,2); // Wir haben ein 16x2-Display angeschlossen
  lcd.backlight(); // Die Hintergrundbeleuchtung einschalten
  lcd.clear(); // Display leeren
  lcd.setCursor(0,0); lcd.print("Scn#");
  lcd.setCursor(0,1); lcd.print("R--- G--- B---"); 
} // SETUP Ende



void loop()
{ loops++; lcd.setCursor(0,0); lcd.print("Scn#"); lcd.print(loops); lcd.print("   "); // Schleifendurchläufe ausgeben
  OK=0; // Status zurücksetzen. Wird auf 1 gesetzt, wenn eine Smartiefarbe zugeordnet werden konnte!
  
// SCHRITT 1: Smartie holen
Drehschieber(115); delay(200); // Smartie holen
Drehschieber(43); delay(700); // Smartie unter den Sensor legen & warten bis er ruhig liegt

// Schritt 2: Lichtwerte messen
color(); 

// Schritt 3: Messwerte in Tabelle ausgeben
Serial.print(rot); Serial.print(";");
Serial.print(gruen); Serial.print(";");
Serial.print(blau); Serial.print(";");
Serial.print(rot+gruen+blau); Serial.println(";");

// Schritt 4: Smartie auswerfen
Drehschieber(0); delay(200);


} // LOOP Ende


void color() // Hier werden die Werte vom Farbsensor ausgelesen und unter den
             // entsprechenden Variablen gespeichert
{
  // Sensor für Rot-Messung einstellen
  digitalWrite(s2, LOW); digitalWrite(s3, LOW); delay(50);
  rot = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  rot += pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  rot += pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  rot = rot/3;

  // Sensor für Blau-Messung einstellen
  digitalWrite(s3, HIGH); delay(50);
  blau = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  blau += pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  blau += pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  blau = blau/3;
  
  // Sensor für Grün-Messung einstellen
  digitalWrite(s2, HIGH); delay(50);
  gruen = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  gruen += pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  gruen += pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  gruen = gruen/3;

  lcd.setCursor(0,1); lcd.print("R    G    B    ");
 lcd.setCursor(1,1); lcd.print(rot); lcd.setCursor(6,1); lcd.print(gruen); lcd.setCursor(11,1); lcd.print(blau);
  
} // Funktion color() ENDE

int Drehschieber(int sollwert)
{ // Bei Aufruf soll das Servo auf $sollwert eingestellt werden.
  // $sollwert ist entweder kleiner, größer oder gleich der aktuellen Position in $Schieberwinkel
  if (sollwert < Schieberwinkel )
     { while (sollwert < Schieberwinkel)
       {Schieberwinkel=Schieberwinkel-1; delay(speedoben); Servooben.write(Schieberwinkel);
       lcd.setCursor(8,0); lcd.print("    "); lcd.setCursor(9,0); lcd.print(Schieberwinkel);} }
     
  if (sollwert > Schieberwinkel )
     { while (sollwert > Schieberwinkel)
       {Schieberwinkel=Schieberwinkel+1; delay(speedoben); Servooben.write(Schieberwinkel);
       lcd.setCursor(8,0); lcd.print("    "); lcd.setCursor(9,0); lcd.print(Schieberwinkel); } }  
}

int Rutsche(int sollwert)
{ // Bei Aufruf soll das Servo auf $sollwert eingestellt werden.
  // $sollwert ist entweder kleiner, größer oder gleich der aktuellen Position in $Schieberwinkel
  if (sollwert < Rutschenwinkel )
     { while (sollwert < Rutschenwinkel)
       {Rutschenwinkel=Rutschenwinkel-4; delay(speedunten); Servounten.write(Rutschenwinkel);
       lcd.setCursor(13,0); lcd.print("    "); lcd.setCursor(13,0); lcd.print(Rutschenwinkel);} }
  if (sollwert > Rutschenwinkel )
     { while (sollwert > Rutschenwinkel)
       {Rutschenwinkel=Rutschenwinkel+4; delay(speedunten); Servounten.write(Rutschenwinkel);
       lcd.setCursor(13,0); lcd.print("    "); lcd.setCursor(13,0); lcd.print(Rutschenwinkel); } }  
}
