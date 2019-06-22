/* Angeschlossene Hardware deklarieren*/
#include <Wire.h>                      // I2C-Bibliothek
#include <LiquidCrystal_I2C.h>         // Display-Bibliothek
          LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // I2C Addresse (0x27 or 0x3F)
#include <Servo.h>                    // Servo-Bibliothek
          Servo Servooben;            // Erstellt oberes Servo mit dem Namen "Servooben"
          Servo Servounten;           // Erstellt unteres Servo "Servounten"

// Variablen für Servowinkel initialisieren und voreinstellen
int Schieberwinkel = 90; int Rutschenwinkel = 90; int speedoben = 0; int speedunten = 0;

//Verbindung der Farbsensorkontakte mit dem Arduino festlegen
const int s1 = 9; const int s0 = 8; const int s2 = 12; const int s3 = 11; const int out = 10;

// Variablen für Messungen und Berechnungen
float rot = 0;     float gruen = 0;     float blau = 0;      int i;            int helligkeit = 0; int x = 0;
float rotpzt;      float gruenpzt;      float blaupzt;       int farbcode = 0; float hell_kalibrieren;
float Abweichung = 0.10;  // Für RGB%. Zu Beginn sind 10% Sollwertabweichung erlaubt
const float Abweichung_Helligkeit = 0.25; float Abweichung_H = Abweichung_Helligkeit; // Für Helligkeit.
int reporting = 1; // 1 schaltet die Rechenwertausgabe an, 0 schaltet sie aus.

// Tabelle: Mittelwerte zur Smartieerkennung. Diese Werte wurden gemessen.
//    FARBE          R%     G%     B%     Helligkeit-absolut
float GWblau[]    = {33.5 , 39.7 , 26.9 , 830.0 }; // Aktualisiert
float GWgruen[]   = {26.8 , 33.3 , 39.9 , 1168.0}; // Aktualisiert
float GWgelb[]    = {22.6 , 32.1 , 45.2 , 912.0 }; // Aktualisiert 
float GWpink[]    = {21.5 , 47.6 , 31.0 , 1247.0}; // Aktualisiert
float GWlila[]    = {30.0 , 44.0 , 26.5 , 1300.0}; // Aktualisiert
float GWrot[]     = {16.8 , 47.1 , 36.1 , 1671.0}; // Aktualisiert
float GWorange[]  = {17.8 , 42.1 , 40.1 , 1343.0}; // Aktualisiert
float GWbraun[]   = {23.0 , 39.5 , 37.5 , 1630.0}; // Aktualisiert
float GWleer[]    = {31.8 , 37.5 , 30.8 , 500.0 }; // gilt für den leeren Schieber unter dem Sensor!

// Tabelle: Grenzwerte zur Smartieerkennung. Diese Werte werden in loops() abhängig von der Toleranz errechnet
// Aufbau: TW_xxx[1]=unterer Grenzwert ROT, [2]= oberer Grenzwert ROT, ...
float TWblau[8];    float TWgruen[8]; float TWgelb[8]; float TWpink[8]; float TWlila[8]; float TWrot[8];
float TWorange[8];  float TWbraun[8]; float TWleer[8]; 

// Zähler für Smarties
int leer=0;   int Fehler=0; int Gelbe=0;  int Orange = 0; int Pinke=0; int Rote=0; 
int Gruene=0; int Lilane=0; int Braune=0; int Blaue=0;    int loops;   int OK;



void setup()
{ // PIN-Funktionen festlegen
  pinMode(s0, OUTPUT); pinMode(s1, OUTPUT); pinMode(s2, OUTPUT); pinMode(s3, OUTPUT); pinMode(out, INPUT); 
  digitalWrite(s0, HIGH); digitalWrite(s1, LOW); // Empfindlichkeit des Sensors auf 100%
  // Servofunktionen anlegen
  Servooben.attach(7); Servooben.write(Schieberwinkel); // Oberes Servo an Pin 7 und nach ganz links
  Servounten.attach(6); Servounten.write(Rutschenwinkel); // Unteres Servo an Pin 6 und in die Mitte
  // Datenausgabe am PC-Monitor starten
  Serial.begin(115200); // Serielle Kommunikation zur Ausgabe auf das Display starten
  delay(500); // kurz auf Verbindungsaufbau warten...
  lcd.begin(16,2); // Wir haben ein 16x2-Display angeschlossen
  lcd.backlight(); // Die Hintergrundbeleuchtung einschalten
  lcd.clear(); // Display leeren

  // Sensor kalibrieren auf Umgebungslicht
  Serial.println(F("Sensor kalibrieren: Umgebungslichtstärke wird gemessen"));
  lcd.setCursor(0,1); lcd.print(F("Kalibrieren:"));
  Drehschieber(5); Drehschieber(43); // Schieber leeren und unter den Sensor fahren
  for (i=1 ; i<21 ; i++)
  {color(); helligkeit = rot + gruen + blau; Serial.print(F("Messung Nr.")); Serial.print(i);
  Serial.print(F(": ")); Serial.println(helligkeit); hell_kalibrieren += helligkeit; 
  lcd.setCursor(0,0); lcd.print(helligkeit);}
  
  hell_kalibrieren = hell_kalibrieren / 20; delay(300);
  Serial.println(F("---------------"));
  Serial.print(F("   Mittelwert  ")); Serial.println(hell_kalibrieren);
  Serial.print(F("-Tabellenwert  ")); Serial.println(GWleer[3]);
  Serial.println("=====================");
  Serial.print(F("=   Differenz  ")); x = hell_kalibrieren - GWleer[3]; Serial.println(x); Serial.println(); 
  lcd.setCursor(0,1); lcd.print("Abweichung:"); lcd.print(x); delay(1000);
  Serial.println(F("Die Tabellenwerte werden angepasst..."));
  GWleer[3] = GWleer[3] + x;  GWblau[3] = GWblau[3] + x;   GWgruen[3] = GWgruen[3] + x;
  GWgelb[3] = GWgelb[3] + x;  GWpink[3] = GWpink[3] + x;   GWlila[3] = GWlila[3] + x;
  GWrot[3] = GWrot[3] + x;    GWorange[3] = GWbraun[3] + x;
  
} // SETUP Ende

void loop()
{ loops++; lcd.setCursor(0,0); lcd.print(F("Scn#")); lcd.print(loops); lcd.print(F("   ")); // Durchläufe ausgeben
  OK=0; // Status zurücksetzen. Wird auf 1 gesetzt, wenn eine Smartiefarbe zugeordnet werden konnte!

// Bevor ein Smartie (nochmal) gemessen wird: Es werden die Grenzwerte für die aktuell eingestellte Toleranz berechnet
// Serial.print(F("Aktuell gültige Fehlertoleranz:")); Serial.print(Abweichung*100); Serial.println(F("%."));
TWblau[0] = (GWblau[0]*(1.0-Abweichung));    TWblau[1] = (GWblau[0]*(1.0+Abweichung)); // Grenzwerte für R-Wert
TWblau[2] = (GWblau[1]*(1.0-Abweichung));    TWblau[3] = (GWblau[1]*(1.0+Abweichung)); // Grenzwerte für G-Wert
TWblau[4] = (GWblau[2]*(1.0-Abweichung));    TWblau[5] = (GWblau[2]*(1.0+Abweichung)); // Grenzwerte für B-Wert
TWblau[6] = (GWblau[3]*(1.0-Abweichung_H));    TWblau[7] = (GWblau[3]*(1.0+Abweichung_H)); // Grenzwerte für Helligkeit

TWgruen[0] = (GWgruen[0]*(1.0-Abweichung));  TWgruen[1] = (GWgruen[0]*(1.0+Abweichung)); // Grenzwerte für R-Wert
TWgruen[2] = (GWgruen[1]*(1.0-Abweichung));  TWgruen[3] = (GWgruen[1]*(1.0+Abweichung)); // Grenzwerte für G-Wert
TWgruen[4] = (GWgruen[2]*(1.0-Abweichung));  TWgruen[5] = (GWgruen[2]*(1.0+Abweichung)); // Grenzwerte für B-Wert
TWgruen[6] = (GWgruen[3]*(1.0-Abweichung_H));  TWgruen[7] = (GWgruen[3]*(1.0+Abweichung_H)); // Grenzwerte für Helligkeit

TWgelb[0] = (GWgelb[0]*(1.0-Abweichung));    TWgelb[1] = (GWgelb[0]*(1.0+Abweichung)); // Grenzwerte für R-Wert
TWgelb[2] = (GWgelb[1]*(1.0-Abweichung));    TWgelb[3] = (GWgelb[1]*(1.0+Abweichung)); // Grenzwerte für G-Wert
TWgelb[4] = (GWgelb[2]*(1.0-Abweichung));    TWgelb[5] = (GWgelb[2]*(1.0+Abweichung)); // Grenzwerte für B-Wert
TWgelb[6] = (GWgelb[3]*(1.0-Abweichung_H));    TWgelb[7] = (GWgelb[3]*(1.0+Abweichung_H)); // Grenzwerte für Helligkeit

TWpink[0] = (GWpink[0]*(1.0-Abweichung));    TWpink[1] = (GWpink[0]*(1.0+Abweichung)); // Grenzwerte für R-Wert
TWpink[2] = (GWpink[1]*(1.0-Abweichung));    TWpink[3] = (GWpink[1]*(1.0+Abweichung)); // Grenzwerte für G-Wert
TWpink[4] = (GWpink[2]*(1.0-Abweichung));    TWpink[5] = (GWpink[2]*(1.0+Abweichung)); // Grenzwerte für B-Wert
TWpink[6] = (GWpink[3]*(1.0-Abweichung_H));    TWpink[7] = (GWpink[3]*(1.0+Abweichung_H)); // Grenzwerte für Helligkeit

TWlila[0] = (GWlila[0]*(1.0-Abweichung));    TWlila[1] = (GWlila[0]*(1.0+Abweichung)); // Grenzwerte für R-Wert
TWlila[2] = (GWlila[1]*(1.0-Abweichung));    TWlila[3] = (GWlila[1]*(1.0+Abweichung)); // Grenzwerte für G-Wert
TWlila[4] = (GWlila[2]*(1.0-Abweichung));    TWlila[5] = (GWlila[2]*(1.0+Abweichung)); // Grenzwerte für B-Wert
TWlila[6] = (GWlila[3]*(1.0-Abweichung_H));    TWlila[7] = (GWlila[3]*(1.0+Abweichung_H)); // Grenzwerte für Helligkeit

TWrot[0] = (GWrot[0]*(1.0-Abweichung));      TWrot[1] = (GWrot[0]*(1.0+Abweichung)); // Grenzwerte für R-Wert
TWrot[2] = (GWrot[1]*(1.0-Abweichung));      TWrot[3] = (GWrot[1]*(1.0+Abweichung)); // Grenzwerte für G-Wert
TWrot[4] = (GWrot[2]*(1.0-Abweichung));      TWrot[5] = (GWrot[2]*(1.0+Abweichung)); // Grenzwerte für B-Wert
TWrot[6] = (GWrot[3]*(1.0-Abweichung_H));      TWrot[7] = (GWrot[3]*(1.0+Abweichung_H)); // Grenzwerte für Helligkeit

TWorange[0] = (GWorange[0]*(1.0-Abweichung)); TWorange[1] = (GWorange[0]*(1.0+Abweichung)); // Grenzwerte für R-Wert
TWorange[2] = (GWorange[1]*(1.0-Abweichung)); TWorange[3] = (GWorange[1]*(1.0+Abweichung)); // Grenzwerte für G-Wert
TWorange[4] = (GWorange[2]*(1.0-Abweichung)); TWorange[5] = (GWorange[2]*(1.0+Abweichung)); // Grenzwerte für B-Wert
TWorange[6] = (GWorange[3]*(1.0-Abweichung_H)); TWorange[7] = (GWorange[3]*(1.0+Abweichung_H)); // Grenzwerte für Helligkeit

TWbraun[0] = (GWbraun[0]*(1.0-Abweichung));   TWbraun[1] = (GWbraun[0]*(1.0+Abweichung)); // Grenzwerte für R-Wert
TWbraun[2] = (GWbraun[1]*(1.0-Abweichung));   TWbraun[3] = (GWbraun[1]*(1.0+Abweichung)); // Grenzwerte für G-Wert
TWbraun[4] = (GWbraun[2]*(1.0-Abweichung));   TWbraun[5] = (GWbraun[2]*(1.0+Abweichung)); // Grenzwerte für B-Wert
TWbraun[6] = (GWbraun[3]*(1.0-Abweichung_H));   TWbraun[7] = (GWbraun[3]*(1.0+Abweichung_H)); // Grenzwerte für Helligkeit

TWleer[0] = (GWleer[0]*(1.0-Abweichung));      TWleer[1] = (GWleer[0]*(1.0+Abweichung)); // Grenzwerte für R-Wert
TWleer[2] = (GWleer[1]*(1.0-Abweichung));      TWleer[3] = (GWleer[1]*(1.0+Abweichung)); // Grenzwerte für G-Wert
TWleer[4] = (GWleer[2]*(1.0-Abweichung));      TWleer[5] = (GWleer[2]*(1.0+Abweichung)); // Grenzwerte für B-Wert
TWleer[6] = (GWleer[3]*(1.0-Abweichung_H));      TWleer[7] = (GWleer[3]*(1.0+Abweichung_H)); // Grenzwerte für Helligkeit

if (reporting==1)
   { Serial.println(); Serial.println(F("TW-Werte für BLAU:")); for (int i=0 ; i<8 ; i++){
            Serial.print(TWblau[i]); Serial.print("<->"); i++; Serial.print(TWblau[i]); Serial.print("  ");}
     Serial.println(); Serial.println(F("TW-Werte für GRÜN:")); for (int i=0 ; i<8 ; i++) {
            Serial.print(TWgruen[i]); Serial.print("<->"); i++; Serial.print(TWgruen[i]); Serial.print("  ");}
     Serial.println(); Serial.println(F("TW-Werte für GELB:")); for (int i=0 ; i<8 ; i++) {
            Serial.print(TWgelb[i]); Serial.print("<->"); i++; Serial.print(TWgelb[i]); Serial.print("  ");}       
     Serial.println(); Serial.println(F("TW-Werte für PINK:")); for (int i=0 ; i<8 ; i++) {
            Serial.print(TWpink[i]); Serial.print("<->"); i++; Serial.print(TWpink[i]); Serial.print("  ");}       
     Serial.println(); Serial.println(F("TW-Werte für LILA:")); for (int i=0 ; i<8 ; i++) {
            Serial.print(TWlila[i]); Serial.print("<->"); i++; Serial.print(TWlila[i]); Serial.print("  ");}       
     Serial.println(); Serial.println(F("TW-Werte für ROT:")); for (int i=0 ; i<8 ; i++) {
            Serial.print(TWrot[i]); Serial.print("<->"); i++; Serial.print(TWrot[i]); Serial.print("  ");}
     Serial.println(); Serial.println(F("TW-Werte für BLAU:")); for (int i=0 ; i<8 ; i++) {
            Serial.print(TWblau[i]); Serial.print("<->"); i++; Serial.print(TWblau[i]); Serial.print("  ");}
     Serial.println(); Serial.println(F("TW-Werte für ORANGE:")); for (int i=0 ; i<8 ; i++) {
            Serial.print(TWorange[i]); Serial.print("<->"); i++; Serial.print(TWorange[i]); Serial.print("  ");}
     Serial.println(); Serial.println(F("TW-Werte für BRAUN:")); for (int i=0 ; i<8 ; i++) {
            Serial.print(TWbraun[i]); Serial.print("<->"); i++; Serial.print(TWbraun[i]); Serial.print("  ");}       
     Serial.println(); Serial.println(F("TW-Werte für LEER:")); for (int i=0 ; i<8 ; i++) {
            Serial.print(TWleer[i]); Serial.print("<->"); i++; Serial.print(TWleer[i]); Serial.print("  ");}
            }
     Serial.println(); Serial.println(); reporting=0; // Ausgabe soll nur 1x passieren        

// SCHRITT 1: Smartie holen
if (farbcode==0) {Drehschieber(113); delay(100);} // Nur neuen Smartie holen, wenn kein Problemsmartie im Schacht ist

// SCHRITT 2: Smartie zum Sensor fahren
// Serial.println(); Serial.println();
// Serial.println("Smartie wird unter Sensor platziert...");
Drehschieber(43); delay(100);
lcd.setCursor(0,0); lcd.print(F("Scn#"));
lcd.setCursor(0,1); lcd.print(F("R---  G---  B---")); 

// SCHRITT 3: Farbsensor auslesen mit Hilfe der Funktion color(). Zurückgeliefert werden die Werte rot, gruen, blau
// Serial.println("Messung läuft...");
color(); // Diese Funktion wird am Ende des Codes festgelegt (s.“void color();“)

// SCHRITT 4: Prozentwerte berechnen
Serial.print(F("Scan#")); Serial.print(loops); Serial.println(F(" - Messwerte und errechneten Lichtanteile in %:"));
helligkeit = rot + gruen + blau;     Serial.print(F("Helligkeit=")); Serial.print(helligkeit);
rotpzt = rot / helligkeit *100;      Serial.print(F(" R=")); Serial.print(rotpzt); Serial.print(F("%"));
gruenpzt = gruen / helligkeit * 100; Serial.print(F(" G=")); Serial.print(gruenpzt); Serial.print(F("%"));
blaupzt = blau / helligkeit * 100;   Serial.print(F(" B=")); Serial.print(blaupzt); Serial.print(F(" %")); 
                                     Serial.print(F(" €%:")); Serial.println(blaupzt+rotpzt+gruenpzt);
// Messdaten aus LCD ausgeben
lcd.setCursor(0,1); lcd.print(F("R     G     B    "));
lcd.setCursor(1,1); lcd.print(round(rotpzt), DEC); lcd.print("%");
lcd.setCursor(7,1); lcd.print(round(gruenpzt), DEC); lcd.print("%");
lcd.setCursor(13,1); lcd.print(round(blaupzt), DEC); lcd.print("%");

// SCHRITT 5: Smartiefarbe anhand der aktuellen Toleranzgrenzwerte zuordnen
farbcode=99; // Gibt an, dass keine Smartiefarbe definiert ist.

/* PRÜFUNG AUF LEER */
if (    rotpzt > TWleer[0]  &&      rotpzt < TWleer[1] &&
      gruenpzt > TWleer[2]  &&    gruenpzt < TWleer[3] &&
       blaupzt > TWleer[4]  &&    blaupzt  < TWleer[5] &&
    helligkeit > TWleer[6]  &&  helligkeit < TWleer[7])
    {leer++; Serial.print(F("Schacht leer!")); Serial.println();farbcode=0;
     lcd.setCursor(0,0); lcd.print(leer); lcd.print(F("xLeer    "));}

/* PRÜFUNG AUF BLAU */
if (    rotpzt > TWblau[0]  &&      rotpzt < TWblau[1] &&
      gruenpzt > TWblau[2]  &&    gruenpzt < TWblau[3] &&
       blaupzt > TWblau[4]  &&    blaupzt  < TWblau[5] &&
    helligkeit > TWblau[6]  &&  helligkeit < TWblau[7])
    {Blaue++; Serial.println(F("Blauer Smartie erkannt!")); farbcode=1;
     lcd.setCursor(0,0); lcd.print(Blaue); lcd.print(F("xBlau    "));}

/* PRÜFUNG AUF GRÜN */
if (     rotpzt > TWgruen[0]  &&      rotpzt   < TWgruen[1] &&
       gruenpzt > TWgruen[2]  &&    gruenpzt   < TWgruen[3] &&
       blaupzt > TWgruen[4]  &&    blaupzt   < TWgruen[5] &&
     helligkeit > TWgruen[6]  &&  helligkeit   < TWgruen[7])
    {Gruene++; Serial.println(F("Grüner Smartie erkannt!")); farbcode=2;
     lcd.setCursor(0,0); lcd.print(Gruene); lcd.print(F("xGruen   "));}

/* PRÜFUNG AUF GELB */
if (    rotpzt > TWgelb[0]  &&      rotpzt < TWgelb[1] &&
      gruenpzt > TWgelb[2]  &&    gruenpzt < TWgelb[3] &&
       blaupzt > TWgelb[4]  &&    blaupzt  < TWgelb[5] &&
    helligkeit > TWgelb[6]  &&  helligkeit < TWgelb[7])
    {Gelbe++; Serial.println(F("Gelber Smartie erkannt!")); farbcode=3;
     lcd.setCursor(0,0); lcd.print(Gelbe); lcd.print(F("xGelb  "));}
    
/* PRÜFUNG AUF PINK */
if (    rotpzt > TWpink[0]  &&      rotpzt < TWpink[1] &&
      gruenpzt > TWpink[2]  &&    gruenpzt < TWpink[3] &&
       blaupzt > TWpink[4]  &&    blaupzt  < TWpink[5] &&
    helligkeit > TWpink[6]  &&  helligkeit < TWpink[7])
    {Pinke++; Serial.println(F("Pinker Smartie erkannt!")); farbcode=4;
     lcd.setCursor(0,0); lcd.print(Pinke); lcd.print(F("xPink "));}

/* PRÜFUNG AUF LILA */
if (    rotpzt > TWlila[0]  &&      rotpzt < TWlila[1] &&
      gruenpzt > TWlila[2]  &&    gruenpzt < TWlila[3] &&
       blaupzt > TWlila[4]  &&    blaupzt  < TWlila[5] &&
    helligkeit > TWlila[6]  &&  helligkeit < TWlila[7])
    {Lilane++; Serial.println(F("Lila Smartie erkannt!")); farbcode=5;
     lcd.setCursor(0,0); lcd.print(Lilane); lcd.print(F("xLila  "));}

/* PRÜFUNG AUF ROT */
if (    rotpzt > TWrot[0]  &&      rotpzt < TWrot[1] &&
      gruenpzt > TWrot[2]  &&    gruenpzt < TWrot[3] &&
       blaupzt > TWrot[4]  &&    blaupzt  < TWrot[5] &&
    helligkeit > TWrot[6]  &&  helligkeit < TWrot[7])
    {Rote++; Serial.println(F("Roter Smartie erkannt!")); farbcode=6;
     lcd.setCursor(0,0); lcd.print(Rote); lcd.print(F("xRot   "));}

/* PRÜFUNG AUF ORANGE */
if (    rotpzt > TWorange[0]  &&      rotpzt < TWorange[1] &&
      gruenpzt > TWorange[2]  &&    gruenpzt < TWorange[3] &&
       blaupzt > TWorange[4]  &&    blaupzt  < TWorange[5] &&
    helligkeit > TWorange[6]  &&  helligkeit < TWorange[7])
    {Orange++; Serial.println(F("Oranger Smartie erkannt!")); farbcode=7;
    lcd.setCursor(0,0); lcd.print(Orange); lcd.print(F("xOrange "));}      

/* PRÜFUNG AUF BRAUN */
if (    rotpzt > TWbraun[0]  &&      rotpzt < TWbraun[1] &&
      gruenpzt > TWbraun[2]  &&    gruenpzt < TWbraun[3] &&
       blaupzt > TWbraun[4]  &&    blaupzt  < TWbraun[5] &&
    helligkeit > TWbraun[6]  &&  helligkeit < TWbraun[7])
    {Braune++; Serial.println(F("Brauner Smartie erkannt!")); farbcode=8;
     lcd.setCursor(0,0); lcd.print(Braune); lcd.print(F("xBraun  "));} 

if (farbcode==99) {Fehler++; lcd.setCursor(0,0); lcd.print(Fehler); lcd.print(F("xERR! T="));
                   Abweichung_H += 0.10; // Toleranz temporär erhöhen;
                   lcd.print(Abweichung_H*100); lcd.print(F("%  "));
                   Serial.println(F("Zuordnung unmöglich - Toleranzschelle wird temporär um 10% erhöht.")); reporting=1;
                   Serial.print(F("Toleranz für Helligkeit jetzt ")); Serial.println(Abweichung_H);
                   Serial.print(F("Toleranztabelle wird neu berechnet & Smartie neu positioniert..."));
                   delay(300); lcd.setCursor(0,0);lcd.print(F("                ")); Drehschieber(60);}
                  
if (farbcode>1) {delay(250);}

if (leer==5) {lcd.setCursor(0,0); lcd.print(F("..going 2 sleep 1 Min"));
              Serial.println(); Serial.println(F("Zusammenfassung:"));
              Serial.print(F("Rot:")); Serial.println(Rote);
              Serial.print(F("Gelb:")); Serial.println(Gelbe);
              Serial.print(F("Grün:")); Serial.println(Gruene);
              Serial.print(F("Blau:")); Serial.println(Blaue);
              Serial.print(F("Braun:")); Serial.println(Braune);
              Serial.print(F("Pink:")); Serial.println(Pinke);
              Serial.print(F("Lila:")); Serial.println(Lilane);
              Serial.print(F("Orange:")); Serial.println(Orange);
              Serial.print(F("Insgesamt:"));
              Serial.println(Rote+Gelbe+Gruene+Blaue+Braune+Pinke+Lilane+Orange);
              lcd.noBacklight(); delay(60000); leer=0; lcd.backlight();
              }
                                       
// SCHRITT 7: Smartie auswerfen
if (farbcode > 0 && farbcode <99 )
   {Rutsche(45 + (farbcode*14)); Drehschieber(5); Abweichung_H = Abweichung_Helligkeit; farbcode=0; Fehler=0; leer=0;}       

} // LOOP Ende



void color() // Hier werden die Werte vom Farbsensor ausgelesen und unter den
             // entsprechenden Variablen gespeichert
{
  // Sensor für Rot-Messung einstellen
  digitalWrite(s2, LOW); digitalWrite(s3, LOW); delay(50);
  rot = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  rot += pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  rot += pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  rot = (rot/3) + 0.00001;

  // Sensor für Blau-Messung einstellen
  digitalWrite(s3, HIGH); delay(50);
  blau = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  blau += pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  blau += pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  blau = (blau/3) + 0.00001;
  
  // Sensor für Grün-Messung einstellen
  digitalWrite(s2, HIGH); delay(50);
  gruen = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  gruen += pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  gruen += pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  gruen = (gruen/3) + 0.00001;
} // Funktion color() ENDE

int Drehschieber(int sollwert)
{ // Bei Aufruf soll das Servo auf $sollwert eingestellt werden.
  // $sollwert ist entweder kleiner, größer oder gleich der aktuellen Position in $Schieberwinkel
  if (sollwert < Schieberwinkel )
     { while (sollwert < Schieberwinkel)
       {Schieberwinkel=Schieberwinkel-1; Servooben.write(Schieberwinkel);
       lcd.setCursor(8,0); lcd.print(F("     ")); lcd.setCursor(9,0); lcd.print(Schieberwinkel); lcd.print(F(" "));} 
       }
     
  if (sollwert > Schieberwinkel )
     { while (sollwert > Schieberwinkel)
       {Schieberwinkel=Schieberwinkel+1; Servooben.write(Schieberwinkel);
       lcd.setCursor(8,0); lcd.print(F("     ")); lcd.setCursor(9,0); lcd.print(Schieberwinkel); lcd.print(F(" "));} }  
}

int Rutsche(int sollwert)
{ // Bei Aufruf soll das Servo auf $sollwert eingestellt werden.
  // $sollwert ist entweder kleiner, größer oder gleich der aktuellen Position in $Schieberwinkel
  if (sollwert < Rutschenwinkel )
     { while (sollwert < Rutschenwinkel)
       {Rutschenwinkel=Rutschenwinkel-1; delay(speedunten); Servounten.write(Rutschenwinkel);
       lcd.setCursor(13,0); lcd.print("    "); lcd.setCursor(13,0); lcd.print(Rutschenwinkel);} }
  if (sollwert > Rutschenwinkel )
     { while (sollwert > Rutschenwinkel)
       {Rutschenwinkel=Rutschenwinkel+1; delay(speedunten); Servounten.write(Rutschenwinkel);
       lcd.setCursor(13,0); lcd.print("    "); lcd.setCursor(13,0); lcd.print(Rutschenwinkel); } }  
}
