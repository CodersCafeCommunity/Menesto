 /*======================================
 Filename        : SmartVest.ino                                                    
 Description     : Arduino sketch file.                                         
 Author          : Coder's Cafe                                                       
 Version         : 1.0.3                                                          
 Initial Release : 01-April-2020                                           
 License         : MIT
 =======================================*/
                                 
#define USE_NODE_MCU_BOARD 
#define SimpleTimer BlynkTimer
#define BLYNK_PRINT Serial
#define APP_DEBUG                    // Comment this out to disable debug prints

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "BlynkProvisioning.h"
#include <DHT.h>
#include <WidgetRTC.h>
#include <OneWire.h> 
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 12
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float humidity;
float temperature;
 
BlynkTimer timer;
WidgetRTC rtc;

#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
int TEC_1 = 4;
int TEC_2 = 5;
int Auto,x,y;

int Button = 16;
int ButtonState = 0; 
unsigned startPressed = 0;  
unsigned endPressed = 0;
unsigned Hold_time;
int flag = 0;
float Lat,Lon;


BLYNK_WRITE(V0)
{
  x = param.asFloat();
}
BLYNK_WRITE(V1)
{
  Auto = param.asInt();
}
BLYNK_WRITE(V2)
{
   GpsParam gps(param);
   Lat = gps.getLat();
   Lon = gps.getLon();
}
BLYNK_CONNECTED() 
{
  rtc.begin();
}


void notification()
{
  Blynk.notify("Drink Water");
}


void HotFlash()
{
   if( Auto == 1)
      {
         float diff = temperature-37;
         x = 25 - diff;
         y = map(x,35,10,255,1024);
         Blynk.virtualWrite(V0,x);
         analogWrite(TEC_1,y);
         analogWrite(TEC_2,y);
         Blynk.setProperty(V0, "color", "#FF6347");
      }
      else
      {
         y = map(x,35,10,255,1024);
         analogWrite(TEC_1,y);
         analogWrite(TEC_2,y);
         Blynk.setProperty(V0, "color", "#03c0f8");
        
      }
}


void setup() 
{
  delay(500);
  Serial.begin(9600);
  pinMode(Button,INPUT);
  BlynkProvisioning.begin();
  dht.begin();
  sensors.begin();
}

void loop() 
{
  BlynkProvisioning.run();  // This handles the network and cloud connection
  
  /*============= Panic button ===============*/
  
  ButtonState = digitalRead(Button); 
  if(ButtonState == HIGH && flag == 0)
  {
     startPressed = millis();
     flag = 1;
  }   
  if(ButtonState == LOW && flag == 1)
  {
        endPressed = millis();
        Hold_time  = endPressed - startPressed;
        if(Hold_time > 3000)
       {
          String bURL = "http://maps.google.com/maps?q="+ String(Lat) +","+ String(Lon);
          Blynk.email("shebinjosejacob2014@gmail.com","Alert",bURL);
          Serial.println("Alert");
       }   
       flag=0;
  }

  /*============= Hot flashes =================*/

  sensors.requestTemperatures();   
  humidity       = dht.readHumidity();
  Serial.println(humidity);
  temperature    = sensors.getTempCByIndex(0);
  Serial.println(temperature);
  
  if(temperature > 20)
  {
     HotFlash();
  }

  /*============ Night sweat =================*/

  String currentTime  = String(hour());
  int Time = currentTime.toInt();
  if(Time > 19 || Time < 6)
  {
    if(humidity > 55)
    {
      HotFlash();
    }
    
  }

   /*========= Hydration assistance ===========*/

   if( Time == 7)
    {
      timer.run();
    }
   if(Time == 22)
    {
      timer.setInterval(2520L,notification);
    }
   if(humidity < 40)
   {
     timer.setInterval(540L,notification);
   }
   else if(humidity > 40 && humidity < 55)
    {
      timer.setInterval(432L,notification);
    } 
   else if(humidity > 55 ) 
    {
      timer.setInterval(360L,notification); 
    }

}

