#include <Arduino.h>
#include "M5Dial.h"
#include <DFRobot_MLX90614.h>
#include <MedianFilterLib.h>

#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_bt.h>
#include "esp_sleep.h"
#include <HTTPClient.h>
#include <ESPSupabase.h>

const uint8_t SampleNumber = 10;

DFRobot_MLX90614_I2C mlx;

MedianFilter<float> ObjMedian(SampleNumber);
MedianFilter<float> AmbMedian(SampleNumber);

M5GFX display;
M5Canvas canvas(&M5Dial.Display);

// Add you Wi-Fi credentials
const char* ssid = "Schindler 1_EXT";
const char* password = "ragu8642";

// Supabase credentials
const char* supabaseUrl = "https://flpujlogtzczxrcjopfj.supabase.co/";
const char* supabaseKey = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImZscHVqbG9ndHpjenhyY2pvcGZqIiwicm9sZSI6InNlcnZpY2Vfcm9sZSIsImlhdCI6MTc4MjI4NzU0NiwiZXhwIjoyMDk3ODYzNTQ2fQ.OAxrQi81OjMxuJwpMTukkLmqTf0Uvmhyj3VyXs99zUY";

Supabase supabase;

void setup() 
{
  auto cfg = M5.config();
  M5Dial.begin(cfg, false, false);
  M5Dial.Display.sleep(); 
  //Wire.setClock(20000);

  while( NO_ERR != mlx.begin() && millis()<3000)
  {
    mlx.enterSleepMode(false);
    delay(200);
  }
  if(millis()>=3000)
    M5Dial.Power.timerSleep(5);
  delay(500);

  btStop(); // Bluetooth deaktivieren

  //setCpuFrequencyMhz(240);
}

void loop() 
{
  for(uint8_t i=0; i<SampleNumber; i++)
  {
    ObjMedian.AddValue(mlx.getObjectTempCelsius());
    delay(50);
    AmbMedian.AddValue(mlx.getAmbientTempCelsius());
    delay(50);
  }
  //mlx.enterSleepMode();

  float TObj = ObjMedian.GetFiltered();
  float TAmb = AmbMedian.GetFiltered();
  if(TAmb<15.0 || TAmb>50.0)
    M5Dial.Power.timerSleep(10);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED && millis()<10000) {
    delay(500);
  }
  // Init Supabase
  supabase.begin(supabaseUrl, supabaseKey);

  String tableName = "temps";

  String body = "{";
  body += "\"id\":\"dial2\",";
  body += "\"obj\":" + String(TObj,1) + ",";
  body += "\"amb\":" + String(TAmb,1);
  body += "}";

  // sending data to supabase
  int response = supabase.insert(tableName, body, false);

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  // schlafen
  M5Dial.Power.timerSleep(30);
}
