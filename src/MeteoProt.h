#ifndef __METEO_PROT_H__
#define __METEO_PROT_H__

#include "WindDir.h"

#define METEOPROT_LORA_SYNC_WORD        0x21    // ranges from 0-0xFF, default 0x34, see API docs
#define METEOPROT_LORA_BAND             868E6   // EU 433 868
#define METEOPROT_LORA_SPREADING_FACTOR 7       // 7 fast 12 slow
#define METEOPROT_LORA_BANDWIDTH        250E3   // 125 slow 250 fast, 250 only with SF7

#define METEOPROT_SPEED_MAP_MAX_VAL     80    // 80.0 km/h
#define METEOPROT_SPEED_MAP_MAX_BIT     63    // 6 Bit 63

#define METEOPROT_TEMP_MAP_MAX_VAL      50    // 80.0 km/h
#define METEOPROT_TEMP_MAP_MAX_BIT      63    // 6 Bit 63

#define METEOPROT_PRESSURE_MAP_OFFSET   950
#define METEOPROT_PRESSURE_MAP_MAX      1050
#define METEOPROT_PRESSURE_MAP_MAX_BIT  255   // 8 Bit 255

#define MAX_STATION_ID  3

typedef struct stru_MeteoData_Base {
  byte stationId : 2;
} MeteoData_Base;

// 6 Bit max 63
typedef struct stru_MeteoData_Wind : stru_MeteoData_Base {
  byte speed : 6;
  byte speed_slow : 6;
  byte speed_fast : 6;
  byte directionIdx : 4;  // 0..15
} MeteoData_Wind;

typedef struct stru_MeteoData_Generic : stru_MeteoData_Base {
  byte rain;
  short temperature;
  byte humidity;
  byte pressure;
} MeteoData_Generic;

typedef struct stru_MeteoData_All : stru_MeteoData_Wind {
  byte rain;
  short temperature;
  byte humidity;
  byte pressure;
} MeteoData_All;

enum MeteoData_Type {
  base,
  wind,
  generic
};

float mapFloat(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min;
}
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float map2float(long value, long in_min, long in_max, long out_min, long out_max)
{
  return map(value, in_min, in_max, out_min * 10, out_max * 10) * .1;
}
byte map2bit(float value, long in_min, long in_max, long out_min, long out_max)
{
  return map(value * 10, in_min * 10, in_max * 10, out_min, out_max);
}


byte speedVal2Bit(float speed) { return map2bit(speed, 0, METEOPROT_SPEED_MAP_MAX_VAL, 0, METEOPROT_SPEED_MAP_MAX_BIT); }
float speedBit2Val(byte bit) { return map2float(bit, 0, METEOPROT_SPEED_MAP_MAX_BIT, 0, METEOPROT_SPEED_MAP_MAX_VAL); }

byte temperatureVal2Bit(float temp) { return temp * 10; }
float temperatureBit2Val(byte bit) { return bit * .1; }

byte humidityVal2Bit(float humid) { return humid * 10; }
float humidityBit2Val(byte bit) { return bit * .1; }

byte pressureVal2Bit(float pressure) { return map2bit(pressure, METEOPROT_PRESSURE_MAP_OFFSET, METEOPROT_PRESSURE_MAP_MAX, 0, METEOPROT_PRESSURE_MAP_MAX_BIT); }
float pressureBit2Val(byte bit) { return map2float(bit, 0, METEOPROT_PRESSURE_MAP_MAX_BIT, METEOPROT_PRESSURE_MAP_OFFSET, METEOPROT_PRESSURE_MAP_MAX); }



class MeteoProt {
  public:
    MeteoProt() {}
    MeteoProt(byte stationId) { mda.stationId = stationId; }
    
    void update(MeteoData_Wind data) {
      mda.speed = data.speed;
      mda.speed_slow = data.speed_slow;
      mda.speed_fast = data.speed_fast;
    }
    void update(MeteoData_Generic data) {
      mda.temperature = data.temperature;
      mda.humidity = data.humidity;
      mda.pressure = data.pressure;
    }
    void update(MeteoData_All data) { mda = data; }

    byte getStationId() { return mda.stationId; }

    void setSpeed(float speed) { mda.speed = speedVal2Bit(speed); }
    float getSpeed() { return speedBit2Val(mda.speed); }

    void setSpeed_slow(float speed) { mda.speed_slow = speedVal2Bit(speed); }
    float getSpeed_slow() { return speedBit2Val(mda.speed_slow); }

    void setSpeed_fast(float speed) { mda.speed_fast = speedVal2Bit(speed); }
    float getSpeed_fast() { return speedBit2Val(mda.speed_fast); }

    void setDirectionIdx(byte directionIdx) { mda.directionIdx = directionIdx; }
    byte getDirectionIdx() { return mda.directionIdx; }

    void setWindDir(WindDir windDir) { mda.directionIdx = windDir.getIndex(); }
    WindDir getWindDir() { return WindDir(mda.directionIdx, 0); }

    void setWindDirIdx(byte windDirIdx) { mda.directionIdx = windDirIdx; }
    byte getWindDirIdx() { return mda.directionIdx; }


    void setRain(byte rain) { mda.rain = rain; }
    byte getRain() { return mda.rain; }


    void setTemperature(float temperature) { mda.temperature = temperatureVal2Bit(temperature); }
    float getTemperature() { return temperatureBit2Val(mda.temperature); }

    void setHumidity(float humidity) { mda.humidity = humidityVal2Bit(humidity); }
    float getHumidity() { return humidityBit2Val(mda.humidity); }

    void setPressure(float pressure) { mda.pressure = pressureVal2Bit(pressure); }
    float getPressure() { return pressureBit2Val(mda.pressure); }

    MeteoData_All mda;

    MeteoData_Base getBase() { return mda; }
    MeteoData_Wind getWind() { return mda; }
    MeteoData_Generic getGeneric() {
      MeteoData_Generic ret;
      ret.stationId = mda.stationId;
      ret.temperature = mda.temperature;
      ret.humidity = mda.humidity;
      ret.pressure = mda.pressure;
      return ret;
    }
    MeteoData_All getAll() { return mda; }
};



class MeteoProt_Base {
  public:
    MeteoProt_Base(byte stationId) { base.stationId = stationId; }
    
    MeteoProt_Base(MeteoData_Base data) { base = data; }
    MeteoProt_Base() { }

    byte getStationId() { return base.stationId; }

    MeteoData_Base base;
};



class MeteoProt_Wind : MeteoProt_Base {
  public:
    MeteoProt_Wind(byte stationId) : MeteoProt_Base(stationId) { wind.stationId = stationId; }
    
    MeteoProt_Wind(MeteoData_Wind data) : MeteoProt_Base(data) { wind = data; }
    MeteoProt_Wind() { }

    byte getStationId() { return wind.stationId; }

    void setSpeed(float speed) { wind.speed = speedVal2Bit(speed); }
    float getSpeed() { return speedBit2Val(wind.speed); }

    void setSpeed_slow(float speed) { wind.speed_slow = speedVal2Bit(speed); }
    float getSpeed_slow() { return speedBit2Val(wind.speed_slow); }

    void setSpeed_fast(float speed) { wind.speed_fast = speedVal2Bit(speed); }
    float getSpeed_fast() { return speedBit2Val(wind.speed_fast); }

    void setDirectionIdx(byte directionIdx) { wind.directionIdx = directionIdx; }
    byte getDirectionIdx() { return wind.directionIdx; }

    void setWindDir(WindDir windDir) { wind.directionIdx = windDir.getIndex(); }
    WindDir getWindDir() { return WindDir(wind.directionIdx, 0); }

    MeteoData_Wind wind;
};



class MeteoProt_Generic : MeteoProt_Base {
  public:
    MeteoProt_Generic(byte stationId) : MeteoProt_Base(stationId) { generic.stationId = stationId; }
    
    MeteoProt_Generic(MeteoData_Generic data) : MeteoProt_Base(data) { generic = data; }
    MeteoProt_Generic() { }

    byte getStationId() { return generic.stationId; }

    void setTemperature(float temperature) { generic.temperature = temperatureVal2Bit(temperature); }
    float getTemperature() { return temperatureBit2Val(generic.temperature); }

    void setHumidity(float humidity) { generic.humidity = humidityVal2Bit(humidity); }
    float getHumidity() { return humidityBit2Val(generic.humidity); }

    void setPressure(float pressure) { generic.pressure = pressureVal2Bit(pressure); }
    float getPressure() { return pressureBit2Val(generic.pressure); }

    MeteoData_Generic generic;
};

#endif