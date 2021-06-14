#include <Arduino.h>
#include <driver/adc.h>
#include "WindDir.h"
#include "WindSpd.h"
#include "ImpulseHelper.h"
#include "MeteoProt.h"
#include <LoRa.h>

#define STATION_ID  0
#define SENDING_TIME_INTERVAL_WIND 150E3
#define SENDING_TIME_INTERVAL_ALL 300E3
#define LOOPING_TIME_INTERVAL 10E3
#define LOOPING_TIME_INTERVAL_WIND 10E3

#define GPIO_WINDDIR ADC2_CHANNEL_0_GPIO_NUM  // 4
#define GPIO_WINDSPD 5
#define GPIO_RAIN 18

#define SPI_SCK   SCK   // 14 Serial Clock
#define SPI_MISO  MISO  // 12 Master In, Slave Out - SDI
#define SPI_MOSI  MOSI  // 13 Master Out, Slave In - SDO
#define SPI_SS    SS    // 16 SlaveSelect, CableSelect, ChipSelect, ChipEnable

#define LORA_CS     SPI_SS
#define LORA_RESET  -1      // 5
#define LORA_IRQ    26

#define DEBOUNCE_TIME 80  // ms

ImpulseHelper windspd;
ImpulseHelper rain;

MeteoProt meteoProt(STATION_ID);

static_assert(sizeof(meteoProt.getWind()) == sizeof(MeteoData_Wind), "Compiler doesn't use return type!");

void blink(int time_ms = 100, int times = 1) {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  for (int i = 0; i < times * 2 -1; i++) {
    delay(time_ms);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}


void IRAM_ATTR rainInterrupt() {
  static long allowNextInterrupt;
  if (allowNextInterrupt < millis()) {
    rain.inc();
    allowNextInterrupt = millis() + DEBOUNCE_TIME;
    Serial.println("RainImp");
    //blink();
  }
};

void IRAM_ATTR windspdInterrupt() {
  static long allowNextInterrupt;
  if (allowNextInterrupt < millis()) {
    windspd.inc();
    allowNextInterrupt = millis() + DEBOUNCE_TIME;
    Serial.println("WindImp");
    //blink();
  }
};

void sendLoRa(byte* data, size_t size) {
  LoRa.beginPacket();
  LoRa.write(data, size);
  LoRa.endPacket();
}

static byte windDirList[(int)(SENDING_TIME_INTERVAL_WIND / LOOPING_TIME_INTERVAL_WIND) + 5]; // +5 BufferOverflow safety
static int windDirCnt;

void setWind() {
  meteoProt.setSpeed(WindSpd::calcWindSpd_kmh(windspd.getSum(), windspd.getTimeDiff()));
  meteoProt.setSpeed_slow(WindSpd::calcWindSpd_kmh(1, windspd.getDiffSlowest()));
  meteoProt.setSpeed_fast(WindSpd::calcWindSpd_kmh(1, windspd.getDiffFastest()));

  byte winddirPossibilities[16] = {}; // Anzahl jedes möglichen Werts
  byte windDirMax = -1; // max gefundene Anzahl jedes gefundenen Werts
  byte avgWindDirIdx = 0;
  for (int i = 0; i < windDirCnt; i++) {
    if (windDirMax < ++winddirPossibilities[windDirList[i]]) {
      windDirMax = winddirPossibilities[windDirList[i]];
      avgWindDirIdx = windDirList[i];
    }
  }
  meteoProt.setWindDirIdx(avgWindDirIdx);

  windDirCnt = 0;
  windspd.clear();

  WindDir windDir = meteoProt.getWindDir();
  Serial.printf("Spd %4.1f (%4.1f | %4.1f) | Dir %3s  %5.1f  %2d\n",
    meteoProt.getSpeed(), meteoProt.getSpeed_slow(), meteoProt.getSpeed_fast(),
    windDir.getText().c_str(), windDir.getDirection(), meteoProt.getDirectionIdx());
}

void setGeneric() {
  meteoProt.setRain(rain.getSum());

  rain.clear();

  Serial.printf("Rain %3d\n", meteoProt.getRain());
}

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing...");

  Serial.print("Initializing GPIO... ");
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(GPIO_WINDDIR, INPUT);
  pinMode(GPIO_WINDSPD, INPUT_PULLUP);
  pinMode(GPIO_RAIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(GPIO_RAIN), rainInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(GPIO_WINDSPD), windspdInterrupt, FALLING);
  Serial.println("initialized.");

  Serial.print("Initializing LoRa... ");
  LoRa.setPins(LORA_CS, LORA_RESET, LORA_IRQ);// set CS, reset, IRQ pin
  while (!LoRa.begin(METEOPROT_LORA_BAND)) {
    Serial.println("LoRa init failed. Check your connections.");
    sleep(1);
  }
  LoRa.setSignalBandwidth(METEOPROT_LORA_BANDWIDTH);
  LoRa.setSpreadingFactor(METEOPROT_LORA_SPREADING_FACTOR);
  LoRa.setSyncWord(METEOPROT_LORA_SYNC_WORD);
  Serial.println("initialized.");

  Serial.println("Setup done");
}

void loop() {
  static unsigned long nextLoopInterval = millis() + LOOPING_TIME_INTERVAL,
    nextLoopIntervalWindDir = millis() + LOOPING_TIME_INTERVAL_WIND,
    nextSendingIntervalWind = millis() + SENDING_TIME_INTERVAL_WIND,
    nextSendingIntervalAll = millis() + SENDING_TIME_INTERVAL_ALL;

  unsigned long now = millis();

  /*
  md.dir = analogRead(GPIO_WINDDIR);
  md.spd = digitalRead(GPIO_WINDSPD);
  md.rain = digitalRead(GPIO_RAIN);
  WindDir winddir (winddir_raw);

  Serial.printf("Spd %d | Rain %d | Dir %3s  %5.1f  %2d  %4d ", windspd, rain, winddir.getText().c_str(), winddir.getDirection(), winddir.getIndex(), winddir.getRaw());
  Serial.println();
  */

  /*
  WindDir winddir(analogRead(GPIO_WINDDIR));
  meteoProt.setSpeed(WindSpd::calcWindSpd_kmh(windspd.getSum(), windspd.getTimeDiff()));
  meteoProt.setSpeed_slow(WindSpd::calcWindSpd_kmh(1, windspd.getDiffSlowest()));
  meteoProt.setSpeed_fast(WindSpd::calcWindSpd_kmh(1, windspd.getDiffFastest()));
  meteoProt.setWind(winddir);

  //Serial.printf("Slowest: %6lu   | Fastest: %6lu\n", windspd.getDiffSlowest(), windspd.getDiffFastest());

  Serial.printf("Spd %4.1f (%4.1f | %4.1f) | Dir %3s  %5.1f  %2d  %4d | Rain %2ld\n",
    meteoProt.getSpeed(), meteoProt.getSpeed_slow(), meteoProt.getSpeed_fast(),
    meteoProt.getWindDir().getText().c_str(), winddir.getDirection(), meteoProt.getDirectionIdx(), winddir.getRaw(),
    rain.getSum());
  */

  if (now > nextLoopIntervalWindDir) {
    WindDir windDir(analogRead(GPIO_WINDDIR));
    windDirList[windDirCnt++] = windDir.getIndex();
    nextLoopIntervalWindDir = now + LOOPING_TIME_INTERVAL_WIND;
    Serial.printf("Dir %3s  %5.1f  %2d  %4d\n",
      windDir.getText().c_str(), windDir.getDirection(), windDir.getIndex(), windDir.getRaw());
  }

  if (now >= nextSendingIntervalAll)
  {
    setWind();
    setGeneric();

    Serial.print("Sending all... ");
    sendLoRa((byte*)&meteoProt.mda, sizeof(meteoProt.mda));
    Serial.println("all sent.");

    blink(100, 3);

    nextSendingIntervalAll = now + SENDING_TIME_INTERVAL_ALL;
    nextSendingIntervalWind = now + SENDING_TIME_INTERVAL_WIND;
  }
  else if (now >= nextSendingIntervalWind)
  {
    setWind();

    Serial.print("Sending wind... ");
    sendLoRa((byte*)&meteoProt.mda, sizeof(MeteoData_Wind));
    Serial.println("wind sent.");

    blink(100, 2);

    nextSendingIntervalWind = now + SENDING_TIME_INTERVAL_WIND;
  }


  blink();

  int delay_ms = nextLoopInterval - now;
  if (delay_ms > 0)
    delay(delay_ms);
  nextLoopInterval = now + LOOPING_TIME_INTERVAL;
}
