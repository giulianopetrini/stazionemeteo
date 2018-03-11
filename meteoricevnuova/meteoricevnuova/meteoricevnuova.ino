                                                                                                
/* Importa eventuali librerie esterne */                                                                                                                                                                                                                                                                                                     
#include <stdlib.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <dht.h>                                                                                                                                                                                                                                                                                                                                                                                                                               
#include <RF24.h>
#include <string.h>
//#include <Ethernet.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>   
 
/* Dichiarazioni di costanti e variabili */
#define CE_PIN 8
#define CSN_PIN 9
 
// Definizione di un pipe di trasmissione
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
const uint64_t pipe = 0xE8E8F0F0E1LL;
 char sendBuffer[256];

/* Dichiarazione dell'oggetto di tipo radio*/
RF24 radio(CE_PIN, CSN_PIN); // Create a Radio
  int counter = 0;
/* Buffer di ricezione. Uguale a quello di trasmissione*/
float payload[7];
  // Alloco le variabili char dove depositerò i valori float del payload
char stazione[6];
char vref_tx[6];
char dht11_t[8];
char dht11_h[8];
char dht22_t[8];
char dht22_h[8];
char bmp180_pres[8];
char bmp180_p0[8];
char bmp180_temp[8];
/* int leggi_DHT11(float* t, float* h) {
 
 dht DHT;
 const int DHT11_PIN = 5;
 int chk = 0;
 
 chk = DHT.read11(DHT11_PIN);
 switch (chk)
 {
 case DHTLIB_OK: 
 // LEGGI I DATI
      *h = DHT.humidity;
      *t = DHT.temperature;
      break;
 case DHTLIB_ERROR_CHECKSUM: 
      *h = 0.0; *t = 0.0;
      break;
 case DHTLIB_ERROR_TIMEOUT: 
      *h = 0.0; *t = 0.0;
      break;
 default: 
      *h = 0.0; *t = 0.0;
 break;
 }
 return(chk);
}*/

 
// -----------------------------------------
// INIZIALIZZAZIONE AMBIENTE
// -----------------------------------------
void setup() 
{
 Serial.begin(9600);
 while (!Serial);
 delay(1000);
 Serial.println("Ricevitore Stazione Meteo v.2.6.16\n\n");
   radio.begin();
 radio.setChannel(0x60);
 radio.openReadingPipe(1,pipe);
 radio.startListening();

   lcd.begin(20, 4);
  lcd.print("Temp.Esterna");
  lcd.setCursor(19,0);
    lcd.print("C");
    lcd.setCursor(0,1);
  lcd.print("Umidita ");
  lcd.setCursor(19,1);
  lcd.print("%");
  lcd.setCursor(0,2);
  lcd.print("Pressione");
 lcd.setCursor(16,2);
  lcd.print("mbar");
 lcd.setCursor(0,3);
  lcd.print("Temp.Interna");
 lcd.setCursor(19,3);
  lcd.print("C");
 
 
}
// ==================================
// LOOP PRINCIPALE
// ==================================
 void loop()
 {
  
 // Controlla se ci sono dati in arrivo
 if (radio.available());
  {
 
 // Riceve i dati dall'NRF24L01
 radio.read(payload, sizeof(payload));

Serial.println(" - RICEZIONE AVVENUTA!");
 
  // Legge i valori del sensore DHT11 locale
 float dht11_temp;
 float dht11_humi;
 leggi_DHT11(&dht11_temp,&dht11_humi);
  // Legge il valore del voltaggio di alimentazione locale
 //float vref_local;
 //vref_local = readVcc();
 
 

 dtostrf(payload[0],4,0,stazione);
 dtostrf(dht11_temp,4,2,dht11_t);
 dtostrf(dht11_humi,4,2,dht11_h);
 dtostrf(payload[1],4,2,dht22_t);
 dtostrf(payload[2],4,2,dht22_h);
 dtostrf(payload[3],4,2,vref_tx);
 //dtostrf(payload[4],4,2,lux);
 dtostrf(payload[4],4,2,bmp180_pres);
 dtostrf(payload[5],4,2,bmp180_p0);
 dtostrf(payload[6],4,2,bmp180_temp);
 sprintf(sendBuffer,"|%s|%s|%s|%s|%s|%s|%s|%s|%s|",stazione,dht11_t,dht11_h,dht22_t,dht22_h,vref_tx,bmp180_pres,bmp180_p0,bmp180_temp);
   if (*vref_tx < 3) {
   lcd.begin(20, 4);
   lcd.print("ATTENZIONE");
   lcd.setCursor(0,1);
   lcd.print("Alimentazione");
   lcd.setCursor(0,2);
   lcd.print("TRASMETTITORE");
   lcd.setCursor(0,3);
   lcd.print(vref_tx); 
   lcd.setCursor(5,3);
   lcd.print("Volt.");
   delay(4000);
  } else {
    
  }
  
 lcd.setCursor(13,0);
   lcd.print(bmp180_temp);
   lcd.setCursor(13,1);
   lcd.print(dht22_h);
   lcd.setCursor(11,2);
   lcd.print(bmp180_pres);
   lcd.setCursor(13,3);
   lcd.print(dht11_t);
   
   
   // Serial.print("\tBuffer = ");
   // Serial.println(sendBuffer);
   

}
 }
// --------------------------------------------
// LETTURA TEMP E UMID DAL SENSORE DHT11
// I DUE PARAMETRI T ED H SONO DI USCITA
// E RESTITUISCONO I DUE VALORI
// --------------------------------------------
int leggi_DHT11(float* t, float* h) {
 
 dht DHT;
 const int DHT11_PIN = 5;
 int chk = 0;
 
 chk = DHT.read11(DHT11_PIN);
 switch (chk)
 {
 case DHTLIB_OK: 
 // LEGGI I DATI
      *h = DHT.humidity;
      *t = DHT.temperature;
      break;
 case DHTLIB_ERROR_CHECKSUM: 
      *h = 0.0; *t = 0.0;
      break;
 case DHTLIB_ERROR_TIMEOUT: 
      *h = 0.0; *t = 0.0;
      break;
 default: 
      *h = 0.0; *t = 0.0;
 break;
 }
 return(chk);
}
 
//
// Legge il voltaggio di alimentazione della scheda Arduino
// Ritorna il valore in volts
//
float readVcc() {
 // Read 1.1V reference against AVcc
 // set the reference to Vcc and the measurement to the internal 1.1V reference
 #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
 ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
 #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
 ADMUX = _BV(MUX5) | _BV(MUX0);
 #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
 ADMUX = _BV(MUX3) | _BV(MUX2);
 #else
 ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
 #endif
 
delay(2); // Wait for Vref to settle
 ADCSRA |= _BV(ADSC); // Start conversion
 while (bit_is_set(ADCSRA,ADSC)); // measuring
 
uint8_t low = ADCL; // must read ADCL first - it then locks ADCH 
 uint8_t high = ADCH; // unlocks both
 
long result = (high<<8) | low;
 
result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
 return result / 1000.0; // Vcc in volts
}
 
// 
// Memory checker (Memoria libera per variabili locali)
// 
int get_free_memory() {
 extern char __bss_end; extern char *__brkval;
 int free_memory;
 
 if ((int)__brkval == 0)
 free_memory = ((int)&free_memory) - ((int)&__bss_end);
 else
 free_memory = ((int)&free_memory) - ((int)__brkval);
 return free_memory;
}
