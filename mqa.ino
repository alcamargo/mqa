#include <SPI.h>
#include <Ethernet.h>
#include "DHT.h"
#include <Wire.h>
#include <RTClib.h>
//RTC - Real Time Clock
RTC_DS1307 RTC;
//Inicializacao dos modulos 

DHT dht;
static char outstr[15];
static char outstr2[15];
static char outstr3[15];
static char outstr4[15];

int digit;
int sensorValue0 = 0;
int sensorValue1 = 0;


byte mac[] = {  
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

//Change to your server domain
char serverName[] = "192.168.2.1";

// change to your server's port
int serverPort = 9200;

// change to the page on that server

char serverPage[] = "/mqa/sensores/";

EthernetClient client;
int totalCount = 0; 
// insure params is big enough to hold your variables
char params[128];

// set this to the number of milliseconds delay
// this is 10 seconds
#define delayMillis 10000UL

unsigned long thisMillis = 0;
unsigned long lastMillis = 0;

void setup() {
  Serial.begin(9600);

  // disable SD SPI
  pinMode(4,OUTPUT);
  digitalWrite(4,HIGH);

  Serial.print(F("Starting ethernet..."));
  if(!Ethernet.begin(mac)) Serial.println(F("failed"));
  else Serial.println(Ethernet.localIP());

  delay(2000);
  Serial.println(F("Ready"));
  
    Wire.begin();//Inicializacao do protocolo wire
    RTC.begin();//Inicializacao do modulo RTC
  //Verifica se o modulo esta funcionando ou nao
  //if (! RTC.isrunning()) {
    //Serial.println("RTC is NOT running!");
    //Ajusta a data/hora do Clock com a data/hora em que o codigo foi compilado, basta descomentar a linha
    //RTC.adjust(DateTime(__DATE__, __TIME__));
  //}
  
  dht.setup(11);

}

void loop()
{
  // If using a static IP, comment out the next line
  Ethernet.maintain();

  thisMillis = millis();

  if(thisMillis - lastMillis > delayMillis)
  {
    lastMillis = thisMillis;

   DateTime now = RTC.now();
    
  Serial.print("Free SRAM in bytes: ");
  Serial.println(freeRam());

  delay(dht.getMinimumSamplingPeriod());

  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  sensorValue0 = analogRead(A0);
  sensorValue1 = analogRead(A1);
  
  dtostrf(humidity,2, 2, outstr);
  dtostrf(temperature,2, 2, outstr2);
  dtostrf(sensorValue0,2, 0, outstr3);
  dtostrf(sensorValue1,2, 0, outstr4);

    // params must be url encoded.
    sprintf(params,"{\"pm10\":0,\"pm25\":0,\"co2\":%s,\"co\":%s,\"humidity\":%s,\"temperature\":%s,\"date\":\"%i/%i/%i %02i:%02i:%02i\"}",outstr4,outstr3,outstr,outstr2,now.year(),now.month(),now.day(),now.hour(),now.minute(),now.second());
    Serial.println(params);
    if(!postPage(serverName,serverPort,serverPage,params)) Serial.print(F("Fail "));
    else Serial.print(F("Pass "));
    totalCount++;
    Serial.println(totalCount,DEC);

  }    
}


byte postPage(char* domainBuffer,int thisPort,char* page,char* thisData)
{
  int inChar;
  char outBuf[192];

  Serial.print(F("connecting..."));

  if(client.connect(domainBuffer,thisPort) == 1)
  {
    Serial.println(F("connected"));

    // send the header
    sprintf(outBuf,"POST %s HTTP/1.1",page);
    client.println(outBuf);
    sprintf(outBuf,"Host: %s",domainBuffer);
    client.println(outBuf);
    client.println(F("Connection: close\r\nContent-Type: application/x-www-form-urlencoded"));
    sprintf(outBuf,"Content-Length: %u\r\n",strlen(thisData));
    client.println(outBuf);

    // send the body (variables)
    client.print(thisData);
  } 
  else
  {
    Serial.println(F("failed"));
    return 0;
  }

  int connectLoop = 0;

  while(client.connected())
  {
    while(client.available())
    {
      inChar = client.read();
      Serial.write(inChar);
      connectLoop = 0;
    }

    delay(1);
    connectLoop++;
    if(connectLoop > 10000)
    {
      Serial.println();
      Serial.println(F("Timeout"));
      client.stop();
    }
  }

  Serial.println();
  Serial.println(F("disconnecting."));
  client.stop();
  return 1;
}

int freeRam () {
extern int __heap_start, *__brkval;
int v;
return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
