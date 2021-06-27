// #define ESP32
#define ESP8266

#ifdef ESP32
  #include "esp_task_wdt.h"
  #define WDT_TIMEOUT 9
#endif

#define LED_BUILTIN 2

void setup() 
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  delay(3000);

  Serial.println("***************************************************");
  Serial.println("*              Arduino Benchmark                  *");
  Serial.println("***************************************************");
  Serial.println("");

  #ifdef ESP32
    esp_task_wdt_init(WDT_TIMEOUT, 0);
  #endif // ESP32
}

void benchmark_digitalWrite(uint32_t count)
{
  uint32_t n;
  for(n=0;n<count/8;n++)
  {
    // unroled loop to minimize for loop influence
    digitalWrite(LED_BUILTIN,HIGH);
    digitalWrite(LED_BUILTIN,LOW);
    digitalWrite(LED_BUILTIN,HIGH);
    digitalWrite(LED_BUILTIN,LOW);
    digitalWrite(LED_BUILTIN,HIGH);
    digitalWrite(LED_BUILTIN,LOW);
    digitalWrite(LED_BUILTIN,HIGH);
    digitalWrite(LED_BUILTIN,LOW);

  }
}

void benchmark_analogRead(uint32_t count)
{
  uint32_t n;
  for(n=0;n<count;n++)
  {
    analogRead(0);
  }
}

void loop() 
{
  uint16_t n;

  uint32_t startTime;
  uint32_t stopTime;
  uint32_t numberOfTriesD=65536 * 16;
  uint32_t numberOfTriesA=2048;
  uint32_t timeNeeded;
  
  Serial.println("digitalWrite speed benchmark");
  Serial.println("=============================");
    
  startTime=millis();
  benchmark_digitalWrite(numberOfTriesD);
  stopTime=millis();
  timeNeeded = stopTime-startTime;
  
  Serial.print("number of tries: ");Serial.print(numberOfTriesD);Serial.print("  ");
  Serial.print("duration [ms]: "); Serial.println(timeNeeded);Serial.print("  ==> ");
  Serial.print("digitalWrite speed [megaSamples/second] : "); Serial.println((float)numberOfTriesD / timeNeeded / 1000); Serial.print("  "); 
  Serial.println("");   
  
  Serial.println("analogRead speed benchmark");
  Serial.println("=============================");
    
  startTime=millis();
  benchmark_analogRead(numberOfTriesA);
  stopTime=millis();
  timeNeeded = stopTime-startTime;
  
  Serial.print("number of tries: ");Serial.print(numberOfTriesA);Serial.print("  ");
  Serial.print("duration [ms]: "); Serial.println(timeNeeded);Serial.print("  ==> ");
  Serial.print("analogRead speed [kiloSamples/second] : "); Serial.println((float)numberOfTriesA / timeNeeded); Serial.print("  ");
  Serial.println("");
  
  delay(100);
  Serial.println("");
  Serial.println("");
  Serial.println("");
  delay(100);
}
