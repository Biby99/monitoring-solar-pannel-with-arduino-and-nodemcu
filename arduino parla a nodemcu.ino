#include <Wire.h>
#include <Adafruit_INA219.h>
#include <SdFat.h>

Adafruit_INA219 ina219;

void intToBytes(int x);          //converte da int a bytes prima di mandare

byte sending[2];                   //accumula bytes che devono essere mandati
int b=true;

// microSD 
#define CHIPSELECT 10
#define ENABLE_DEDICATED_SPI 1
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)
#define SPI_DRIVER_SELECT 0
uint8_t cycles = 0;
SdFat32 sd;
File32 measurFile;
//ina
float shuntvoltage = 0.0;
float busvoltage = 0.0;
float current_mA = 0.0;
float loadvoltage = 0.0; 
//variabili da mandare al nodemcu
float voltsend = 0.0;
float cursend = 0.0;
void setup() {
 Wire.begin(8);                /* apertura i2c bus con indirizzo 8 */
 ina219.begin(); 
 Wire.onReceive(receiveEvent);
 Wire.onRequest(requestEvent); /* evento richiesta  */

 // SD
  sd.begin(CHIPSELECT);
  measurFile.open("SOLAR.csv", O_WRITE | O_CREAT | O_TRUNC);
  measurFile.print("Volt,Corrente\n");
  measurFile.sync();
 /* serial per  debug */  
 //Serial.begin(9600);          
}

void loop() {

shuntvoltage = ina219.getShuntVoltage_mV();
busvoltage = ina219.getBusVoltage_V();
current_mA = ina219.getCurrent_mA();
loadvoltage = busvoltage+(shuntvoltage/1000);


if(!b){ voltsend = loadvoltage; cursend = current_mA;// per ottenere i risulati sincronizzati e non sfasati
  intToBytes(cursend*100); }
else{intToBytes(voltsend*100); }


writeFile();
delay(100);

}

void receiveEvent(int howMany) {
 while (0 <Wire.available()) {
    int c = Wire.read();      /* riceve bit a bit */
    b=c;
    }
    
}

// funzione eseguita quando i dati sono richiesti dal master(nodemcu)
void requestEvent() {
 Wire.write(sending[0]);               //Invio dati su richiesta
 Wire.write(sending[1]);
}

void intToBytes(int x)
{
  sending[0]= (x >>8);                  //Operazione bit a bit
  sending[1]= x & 0xFF;
}

void writeFile() {
    char buf[32], voltbuf[16]={0}, curbuf[16]={0};

    // buffer con the volt e corrente in stringhe
    dtostrf(loadvoltage, 10, 3, voltbuf);
    dtostrf(current_mA, 10, 3, curbuf);
    
    //linea csv : voltage,current\n
    sprintf(buf, "%s,%s\n", voltbuf, curbuf);

    //scrittura 
    measurFile.write(buf);

    //dopo 9 cicli (1 sec.), da buffer a file in SD
    if(cycles >=9)
      measurFile.sync();

    //incrementa contatore cicli e reset a 0 dop 10 cicli
    cycles++;
    cycles %= 10;
}