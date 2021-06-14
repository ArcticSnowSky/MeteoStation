#ifndef __WIND_DIR_H__
#define __WIND_DIR_H__

#include <Arduino.h>
//#include "utils.h"

class WindDir {

public:
    WindDir(int analogValue) {
        this->setRaw(analogValue);
    }
    WindDir(byte index, int analogValue) {
        this->index = index;
        this->raw = analogValue;
    }

    void setRaw(int analogValue) {
        this->raw = analogValue;

        // Loop through the different bit Value maximum to find actual direction
        for (this->index = 0; this->index < WindDir::bitMax_len; this->index++) {
            if (this->raw <= WindDir::bitMax(this->index))
                break;  // We're at the right spot
        }
        
        this->index = WindDir::indices(this->index);
    }

    int getRaw() { return raw; }
    byte getIndex() { return index; }
    float getDirection() { return WindDir::directions(this->index); }
    String getText() { return WindDir::texts(this->index); }

protected:
    static const int bitMax_len = 15;
    // Sorted bitMax Array, use same index on "indices" to get a normalized human-readable index
    static const short bitMax(int index) { return ((short[bitMax_len]){ 299, 353, 438, 622, 858, 1063, 1386, 1734, 2121, 2459, 2665, 2976, 3226, 3429, 3664 })[index]; }
    // Use same index on as on bitMax to get a normalized human-readable index that can be used directly or in combination with directions or texts
    static const byte indices(int index) { return ((byte[]){ 5, 3, 4, 7, 6, 9, 8, 1, 2, 11, 10, 15, 0, 13, 14, 12 })[index]; }

    static const float directions(int index) { return ((float[]) { 0, 22.5, 45, 67.5, 90, 112.5, 135, 157.5, 180, 202.5, 225, 247.5, 270, 292.5, 315, 337.5 })[index]; }
    static const String texts(int index) { return ((String[]) { "N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW" })[index]; }

    //static const short bitMax[] = { 299, 353, 438, 622, 858, 1063, 1386, 1734, 2121, 2459, 2665, 2976, 3226, 3429, 3664 };
    //static constexpr byte indices[] = { 5, 3, 4, 7, 6, 9, 8, 1, 2, 11, 10, 15, 0, 13, 14, 12 };
    //static const int bitMax_len = COUNT(bitMax);
    
    //static constexpr float directions[] = { 0, 22.5, 45, 67.5, 90, 112.5, 135, 157.5, 180, 202.5, 225, 247.5, 270, 292.5, 315, 337.5 };
    //static constexpr char * const texts[] = { "N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW" };

    int raw;
    byte index;
};

#endif
