#ifndef __WIND_SPD_H__
#define __WIND_SPD_H__

#include <Arduino.h>
//#include "utils.h"

class WindSpd {

public:
    static float calcWindSpd_ms(unsigned long impulse, unsigned long time_ms) {
        return time_ms > 0 ? WindSpd::windFaktor_ms * impulse / time_ms : 0;
    }

    static float calcWindSpd_kmh(unsigned long impulse, unsigned long time_ms) {
        return time_ms > 0 ? WindSpd::windFaktor_kmh * impulse / time_ms : 0;
    }
protected:
    static constexpr float windFaktor_ms = 8640;  // Anemometer: 1 imp /sec = 8.64m/s. Faktor =  2.4 * 3.6 * 1000
    static constexpr float windFaktor_kmh= 2400;  // Anemometer: 1 imp /sec = 2.4km/h. Faktor =  2.4 * 1000
};

#endif
