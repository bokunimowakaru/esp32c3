
// https://gist.github.com/nikolaypavlov/f4832cba720917b16905
//
// Fourth order bandpass IIR filter coefficients (100 Hz - 500 Hz, sampling rate 22050 Hz)
//
// a[0]*y[n] + a[1]*y[n-1] + â€¦ + a[n]*y[0] = b[0]*x[n] + b[1]*x[m-1] + â€¦ + b[m]*x[0]
//

#include "esp_task_wdt.h"
#define WDT_TIMEOUT 9

const float a[5] = {1, -3.91745514489661, 5.75755576410991, -3.76266550602334, 0.922565876650813};
const float b[5] = {0.000780326282260527, 0, -0.00156065256452105, 0, 0.000780326282260527};
const int bufferSize = 150;
float x[bufferSize];
float y[bufferSize];

void filt()
{
  // Filter first 9 samples
  y[0] = b[0] * x[0];
  y[1] = b[0] * x[1] + b[1] * x[0] - a[1] * y[0];
  y[2] = b[0] * x[2] + b[1] * x[1] + b[2] * x[0] - a[1] * y[1] - a[2] * y[0];
  y[3] = b[0] * x[3] + b[1] * x[2] + b[2] * x[1] + b[3] * x[0] - a[1] * y[2] - a[2] * y[1] - a[3] * y[0];

  for (int i = 4; i < bufferSize; i++) {
    y[i] = b[0] * x[i] + b[1] * x[i - 1] + b[2] * x[i - 2] + b[3] * x[i - 3] + b[4] * x[i - 4] - a[1] * y[i - 1] - a[2] * y[i - 2] - a[3] * y[i - 3] - a[4] * y[i - 4];
  }

  for (int i = 0; i < bufferSize; i++) {
    x[i] = y[i];
  }
}

void setup()
{
  Serial.begin(115200);
  esp_task_wdt_init(WDT_TIMEOUT, 0);
}

void loop()
{
  uint16_t n;

  uint32_t startTime;
  uint32_t stopTime;
  uint32_t numberOfTries = 4096;
  uint32_t timeNeeded;

  Serial.println("4th order float IIR speed benchmark");
  Serial.println("===================================");

  startTime = millis();
  for (n = 0; n < numberOfTries; n++)
  {
    x[10] = 1000; // input for impulse response
    filt(); // to it n-times to improve time measurement accuracy
  }
  stopTime = millis();
  timeNeeded = stopTime - startTime;

  Serial.print("total number of samples: "); Serial.print(numberOfTries*bufferSize); Serial.print("  ");
  Serial.print("duration [ms]: "); Serial.println(timeNeeded); Serial.print("  ==> ");
  Serial.print("speed [MegaSamples/second] : ");
  Serial.println((float)numberOfTries * bufferSize / timeNeeded / 1000,3);
  Serial.print("  ");
  Serial.println("");
  delay(1);

}

