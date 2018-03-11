#include <dht.h>
#include <SPI.h>
#include <Wire.h>
#include <SFE_BMP180.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LowPower.h>

#define CE_PIN 7
#define CSN_PIN 8
 
const uint64_t pipe = 0xE8E8F0F0E1LL; // Definisce la pipe di trasmissione
RF24 radio(CE_PIN, CSN_PIN); // Crea un oggetto Radio
 
// Crea un'istanza per il sensore BMP180.
SFE_BMP180 pressure;
 
// Dati da trasmettere
// payload[0] = identificativo Stazione
// payload[3] = voltaggio di alimentazione
// payload[1] = temperatura di DHT 22
//payload[2] = umidità di DHT 22
// payload[4] = press. atmosfrica in mBar
// payload[5] = press. atmosferica SLDM in mBar
// payload[6] = temperatura in gradi dal BMP180
float payload[7];
 
 
// --------------------------------------------
// LETTURA TEMP E UMID DAL SENSORE DHT22
// I DUE PARAMETRI T ED H SONO DI USCITA
// E RESTITUISCONO I DUE VALORI
// --------------------------------------------
 int i=0;
int leggi_DHT22(float* t, float* h) {
 
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
 
// --------------------------------------------------------------------
// Routine di lettura del sensore BMP180
// All'interno va cablato il valore dell'altitudine (A) della lettura.
//
// Parametri in uscita:
// altit = altezza in metri della stazione di lettura
// temp = temperatura letta dal sensore in gradi centigradi
// press = pressione atmosferica assoluta in milliBar
// press0 = pressione equivalente sul livello del mare in milliBar
// --------------------------------------------------------------------
void leggi_BMP180(float* altit, float* temp, float* press, float* press0) {
     char status;
     double A,T,P,p0;
 
 
     // Altitudine della stazione di misura in metri. Può essere ricavata da un GPS
     A = 227.0;
 
     // Prima è necessaria una lettura della temperatura.
     // Ritorna il tempo di attesa per la lettura della pressione.
 
     status = pressure.startTemperature();
     // Attende il completamento della lettura
     delay(status);
 
     // Ritorna la lettura di temperatura.
     status = pressure.getTemperature(T);
 
     // Legge la pressione:
     // Va inserito il parametro di oversampling ( da 0 a 3)
     // Più è alto migliore è la precisione, maggiore è il tempo di attesa
     status = pressure.startPressure(3);
 
     // Attende il completamento della misura.
     delay(status);
 
     // Legge la pressione atmosferica. La funzione oltre a ritornare
     // la pressione letta ritorna di nuovo il valore di temperatura.
     status = pressure.getPressure(P,T);
 
     // Ricava la pressione atmosferica SUL LIVELLO del mare.
     // Questa è confrontabile con quella emessa dai bollettini meteo.
     p0 = pressure.sealevel(P,A);
 
     *altit = A;
     *temp = T;
     *press = P;
     *press0 = p0;
}
 
// ------------------------------------------------------
// Legge il voltaggio di riferimento della scheda Arduino
// Il valore ideale è 5.0V
// Ritorna il valore in Volts
// ------------------------------------------------------
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
 
// ----------------
// INIZIALIZZAZIONE
// ----------------
void setup() {
     Serial.begin(9600);
     Wire.begin();
 
     // Inizializza e calibra il sensore BMP180
     if (pressure.begin())
        Serial.println("BMP180 operativo.");
     else
        { 
     Serial.println("BMP180 non collegato correttamente.\n\n");
     while(1); // Loop perpetuo
     }
 
     radio.begin();
     radio.setChannel(0x60);
     radio.openWritingPipe(pipe);
 
     delay(1000);
     Serial.println("\nStazione Meteo 2.0.0 - 22/06/2016\n\n");
}
 
void loop() {

 // Stazione numero 1
 payload[0] = 1;

 //Lettura DHT 22
 leggi_DHT22(&payload[1], &payload[2]);

// Legge il voltaggio di alimentazione
 payload[3] = readVcc();
 
 // Legge il sensore BMP180 
 float pressione_mbar;
 float pressione_slm;
 float altitudine;
 float temp_gradi;
 leggi_BMP180(&altitudine, &temp_gradi, &pressione_mbar, &pressione_slm);
 payload[4] = pressione_mbar;
 payload[5] = pressione_slm;
 payload[6] = temp_gradi;
 Serial.println("a.ttt\n\n");  
 //Trasmissione dati
 radio.write( payload, sizeof(payload) );
  Serial.println("a.\n\n");
 
 // Stampa per il debug dei dati
 Serial.println("STAZIONE = " + String(payload[0]));
 Serial.println("DHT 22 T = " + String(payload[1]));
 Serial.println("DHT 22 H = " + String(payload[2]));
 Serial.println("Vcc REF = " + String(payload[3]));
 Serial.println("BMP180 P = " + String(payload[4]));
 Serial.println("BMP180 P0 = " + String(payload[5]));
 Serial.println("BMP180 T = " + String(payload[6]));
 Serial.println();
for(i=0;i<120;i++) {                               //esegue 3 volte
 LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);    //lo sleep di 8s 
  }


  
}
