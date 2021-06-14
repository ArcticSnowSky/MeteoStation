#ifndef __IMPULSE_HELPER_H__
#define __IMPULSE_HELPER_H__

#include <Arduino.h>
//#include "utils.h"

class ImpulseHelper {

public:
    ImpulseHelper() { clear(); }

    void inc() { ctr++; step(); }
    void inc(long cnt) { ctr += cnt; step(); }

    void dec() { ctr--; step(); }
    void dec(long cnt) { ctr -= cnt; step(); }

    void clear() {
        ctr = 0;
        last_time = start_time =
        slowest_diff = fastest_diff = 0;
    }

    long getSum() { return ctr; }

    unsigned long getTimeDiffNow() { return millis() - start_time; }
    unsigned long getTimeDiff() { return last_time - start_time; }

    unsigned long getDiffFastest() { return fastest_diff; }
    unsigned long getDiffSlowestInc() { return slowest_diff; }
    unsigned long getDiffSlowest() {
        // Das langsamste Intervall kann auch nach einem Klick kommen
        if (ctr > 0) {
            unsigned long diff = millis() - last_time;
            if (diff > slowest_diff)
                return diff;
        }
        return slowest_diff;
    }

    unsigned long getStartTime() { return start_time; }
    unsigned long getLastTime() { return last_time; }

protected:
    long ctr;
    unsigned long start_time;
    unsigned long last_time;
    unsigned long slowest_diff, fastest_diff;

    void step() {
        unsigned long now = millis();
        unsigned long diff = now - last_time;
        last_time = now;
        if (start_time == 0)
            start_time = now;
        else {
            if (diff > slowest_diff)
                slowest_diff = diff;
            if (diff < fastest_diff || fastest_diff == 0)
                fastest_diff = diff;
        }
    }

};

#endif
