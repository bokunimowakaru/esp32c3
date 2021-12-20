/*
Simple Deep Sleep with Timer Wake Up
=====================================
ESP32 offers a deep sleep mode for effective power
saving as power is an important factor for IoT
applications. In this mode CPUs, most of the RAM,
and all the digital peripherals which are clocked
from APB_CLK are powered off. The only parts of
the chip which can still be powered on are:
RTC controller, RTC peripherals ,and RTC memories

This code displays the most basic deep sleep with
a timer to wake it up and how to store data in
RTC memory to use it over reboots

This code is under Public Domain License.

Author:
Pranav Cherukupalli <cherukupallip@gmail.com>
*/


#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;

//int TimerWakeUp_time = 0;

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
byte TimerWakeUp_print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  /*
  switch((int)wakeup_reason){
    case 1  : Serial.print("Not a wakeup cause "); break;
    case 2  : Serial.print("Wakeup caused by external signal using RTC_IO "); break;
    case 3  : Serial.print("Wakeup caused by external signal using RTC_CNTL "); break;
    case 4  : Serial.print("Wakeup caused by timer "); break;
    case 5  : Serial.print("Wakeup caused by touchpad "); break;
    case 6  : Serial.print("Wakeup caused by ULP program "); break;
    case 7  : Serial.print("Wakeup caused by GPIO "); break;
    case 8  : Serial.print("Wakeup caused by UART "); break;
    case 9  : Serial.print("Wakeup caused by WIFI "); break;
    case 10 : Serial.print("Wakeup caused by COCPU int "); break;
    case 11 : Serial.print("Wakeup caused by COCPU crash "); break;
    case 12 : Serial.print("Wakeup caused by BT "); break;
    default : Serial.print("Wakeup was not caused by deep sleep "); break;
    */

  switch(wakeup_reason){
    case ESP_SLEEP_WAKEUP_ALL  		: Serial.print("Not a wakeup cause "); break;
    case ESP_SLEEP_WAKEUP_EXT0 		: Serial.print("Wakeup caused by external signal using RTC_IO "); break;
    case ESP_SLEEP_WAKEUP_EXT1 		: Serial.print("Wakeup caused by external signal using RTC_CNTL "); break;
    case ESP_SLEEP_WAKEUP_TIMER		: Serial.print("Wakeup caused by timer "); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD	: Serial.print("Wakeup caused by touchpad "); break;
    case ESP_SLEEP_WAKEUP_ULP		: Serial.print("Wakeup caused by ULP program "); break;
    case ESP_SLEEP_WAKEUP_GPIO		: Serial.print("Wakeup caused by GPIO "); break;
    case ESP_SLEEP_WAKEUP_UART		: Serial.print("Wakeup caused by UART "); break;
    case ESP_SLEEP_WAKEUP_WIFI		: Serial.print("Wakeup caused by WIFI "); break;
    case ESP_SLEEP_WAKEUP_COCPU		: Serial.print("Wakeup caused by COCPU int "); break;
    case ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG: Serial.print("Wakeup caused by COCPU crash "); break;
    case ESP_SLEEP_WAKEUP_BT		: Serial.print("Wakeup caused by BT "); break;
    default : Serial.print("Wakeup was not caused by deep sleep "); break;
    
    // https://github.com/espressif/esp-idf/blob/279c8ae/components/esp_hw_support/include/esp_sleep.h
  }
  Serial.println( "(" + String((int)wakeup_reason) + ")");
  return (int)wakeup_reason;
}

void TimerWakeUp_setSleepTime(int time_sec){
/*if(ESP.getChipRevision() == 0 ){  // for Revision 0
    TimerWakeUp_time = time_sec;
    return;
  }
*/
  esp_sleep_enable_timer_wakeup((uint32_t)time_sec * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(time_sec) +
  " Seconds");
}

void TimerWakeUp_setExternalInput(int pin, int level){
  /* @param mask  bit mask of GPIO numbers which will cause wakeup. Only GPIOs
   *              which are have RTC functionality can be used in this bit map:
   *              0,2,4,12-15,25-27,32-39. <-- for ESP32 // C3は要確認
  */
//if(ESP.getChipRevision() == 0 )return;  // for Revision 0
  esp_deep_sleep_enable_gpio_wakeup(1ULL << pin,(esp_deepsleep_gpio_wake_up_mode_t)level);
  Serial.println("Wakeup ESP32 when IO" + String(pin) + " = " + String(level));
}

int TimerWakeUp_bootCount(){
  return bootCount;
}

byte TimerWakeUp_init(){
  // esp_sleep_config_gpio_isolate();
  // esp_sleep_enable_gpio_switch(false);
  ++bootCount;	//Increment boot number and print it every reboot
  Serial.println();
  Serial.println("Boot number: " + String(bootCount));
  return TimerWakeUp_print_wakeup_reason();  //Print the wakeup reason for ESP32
}

void TimerWakeUp_sleep(){
  Serial.println("Going to sleep now");
  delay(100);
  Serial.flush(); 
  /*
  if(ESP.getChipRevision() == 0 ){  // for Revision 0
    Serial.println("WARN: ESP32 Rev 0 cannot sleep. use Rev 1, or more than");
    int time_sec = TimerWakeUp_time;
    TimerWakeUp_time = 0;
    for(int i=0 ; i < time_sec; i++){
        for(int j=0 ; j<100; j++){
            delay(10);
            if(PIR_EN) if(digitalRead(PIN_PIR)) return;
            if(IR_IN_EN) if(!digitalRead(PIN_IR_IN)) return;
            if(!digitalRead(PIN_SW)) return;
        }
    }
    return;
  }
  */
  // タッチパッド起動を有効にしていなくても動作してしまう不具合対策
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TOUCHPAD);
  Serial.println("Disabled wakeup touchpad, please ignore above \"E () sleep: Incorrect\"");
  esp_deep_sleep_start();
  // while(1) delay(100);
}

void TimerWakeUp_setup(){
  Serial.begin(115200);
  delay(1000); //Take some time to open up the Serial Monitor

  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  //Print the wakeup reason for ESP32
  TimerWakeUp_print_wakeup_reason();

  /*
  First we configure the wake up source
  We set our ESP32 to wake up every 5 seconds
  */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
  " Seconds");

  /*
  Next we decide what all peripherals to shut down/keep on
  By default, ESP32 will automatically power down the peripherals
  not needed by the wakeup source, but if you want to be a poweruser
  this is for you. Read in detail at the API docs
  http://esp-idf.readthedocs.io/en/latest/api-reference/system/deep_sleep.html
  Left the line commented as an example of how to configure peripherals.
  The line below turns off all RTC peripherals in deep sleep.
  */
  //esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  //Serial.println("Configured all RTC Peripherals to be powered down in sleep");

  /*
  Now that we have setup a wake cause and if needed setup the
  peripherals state in deep sleep, we can now start going to
  deep sleep.
  In the case that no wake up sources were provided but deep
  sleep was started, it will sleep forever unless hardware
  reset occurs.
  */
  Serial.println("Going to sleep now");
  Serial.flush(); 
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

void TimerWakeUp_loop(){
  //This is not going to be called
}
