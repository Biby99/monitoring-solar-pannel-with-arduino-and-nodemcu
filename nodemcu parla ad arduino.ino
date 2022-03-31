#include <Wire.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#define DEVICE "ESP8266"
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>


// WiFi AP SSID
#define WIFI_SSID "***********"
// WiFi password
#define WIFI_PASSWORD "**************"
// InfluxDB v2 server url, e.g. https://eu-central-1-1.aws.cloud2.influxdata.com (Use: InfluxDB UI -> Load Data -> Client Libraries)
#define INFLUXDB_URL "https://europe-west1-1.gcp.cloud2.influxdata.com"
// InfluxDB v2 server or cloud API token (Use: InfluxDB UI -> Data -> API Tokens -> Generate API Token)
#define INFLUXDB_TOKEN "******************************"
// InfluxDB v2 organization id (Use: InfluxDB UI -> User -> About -> Common Ids )
#define INFLUXDB_ORG "***********"
// InfluxDB v2 bucket name (Use: InfluxDB UI ->  Data -> Buckets)
#define INFLUXDB_BUCKET "bibalosimone's Bucket"

// Set timezone string according to https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
// Examples:
//  Pacific Time: "PST8PDT"
//  Eastern: "EST5EDT"
//  Japanesse: "JST-9"
//  Central Europe: "CET-1CEST,M3.5.0,M10.5.0/3"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

 
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Data point
//Point sensor("wifi_status");
Point sensor("Solar");
//converte  bytes entranti in int
void bytesToInt(byte b0, byte b1);  
//variabili
int a;
bool b;
bool c;
byte coming[2];              //accumula bytes entranti
float volt,curr,watts;
void setup() {
  bool c=true;

  Serial.begin(115200);

  // Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  // Add tags
  sensor.addTag("device", DEVICE);

  // Accurate time is necessary for certificate validation and writing in batches
  // For the fastest time sync find NTP servers in your area: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial.
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }




 /* Serial.begin(9600);   serial per debug */
 Wire.begin(D1, D2); /* apre i2c bus con SDA=D1 e SCL=D2 di NodeMCU */
}



void loop() {
   if(c){

//scambio dati con arduino uno 
Wire.beginTransmission(8); // inizio con device address 8 
if(b){Wire.write(0); }
else{Wire.write(1);}
Wire.endTransmission();    // fine trasmissione 

 Wire.requestFrom(8, 2); // richiesta e lettura dati dal slave (arduino uno)
 while(Wire.available()>0)   
    {
     coming[0]=Wire.read();                 //legge primo byte entrante
     coming[1]=Wire.read();                 //legge secondo byte entrante
     bytesToInt(coming[0], coming[1]);      //converte da bytes a int
     float f_coming= (float) a/100;         //converte da int a float      
     
    if(b){//Serial.print("Voltage "); Serial.println(f_coming);
      volt=f_coming;    
      delay(100);}
      else{//Serial.print("Current ");Serial.println(f_coming);
    curr=f_coming;     
    delay(100);c=!c;}
    
    b=!b;
    }
    watts=volt*(curr/1000);   
   }    
// finescambio dati con arduino uno
else{
  // Pulizia campi. Tags restano
  sensor.clearFields();

  // accumulo valori al point
 // sensor.addField("rssi", WiFi.RSSI());
sensor.addField("voltage", volt);
sensor.addField("current", curr);
sensor.addField("power", watts);
  // Print di cosa scriviamo 
  Serial.print("Writing: ");
  Serial.println(sensor.toLineProtocol());

  // Check WiFi e riconnessione se necessario
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("Wifi connection lost");
  }

  // Scrittura point
  if (!client.writePoint(sensor)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  Serial.println("Wait 10s");
  delay(10000);
  c=!c;
}
}

void bytesToInt(byte b0, byte b1){
  a = (b0 << 8) | (b1); //operazione bit a bit
  }
