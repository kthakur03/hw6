#ifndef HASH_H
#define HASH_H

#include <iostream>
#include <cmath>
#include <random>
#include <chrono>
#include <cctype>
#include <string>

typedef std::size_t HASH_INDEX_T;

struct MyStringHash {
    HASH_INDEX_T rValues[5] { 983132572, 1468777056, 552714139, 984953261, 261934300 };

    MyStringHash(bool debug = true)
    {
        if(false == debug){
            generateRValues();
        }
    }

    // hash function entry point (i.e. this is h(k))
    HASH_INDEX_T operator()(const std::string& k) const
    {
        unsigned long long w[5] = {0, 0, 0, 0, 0};

        // Work from the right side of the string in groups of at most 6.
        // The rightmost group goes into w[4], the previous group into w[3], etc.
        int stringPos = static_cast<int>(k.size()) - 1;
        for(int group = 4; group >= 0 && stringPos >= 0; --group)
        {
            unsigned long long value = 0;
            unsigned long long place = 1;

            for(int count = 0; count < 6 && stringPos >= 0; ++count, --stringPos)
            {
                value += static_cast<unsigned long long>(
                    letterDigitToNumber(k[stringPos])) * place;
                place *= 36ULL;
            }
            w[group] = value;
        }

        unsigned long long result = 0;
        for(int i = 0; i < 5; ++i)
        {
            result += w[i] * static_cast<unsigned long long>(rValues[i]);
        }
        return static_cast<HASH_INDEX_T>(result);
    }

    // Convert a-z/A-Z and 0-9 to values 0-35.
    HASH_INDEX_T letterDigitToNumber(char letter) const
    {
        unsigned char ch = static_cast<unsigned char>(letter);
        if(std::isalpha(ch))
        {
            return static_cast<HASH_INDEX_T>(std::tolower(ch) - 'a');
        }
        return static_cast<HASH_INDEX_T>(letter - '0' + 26);
    }

    // Code to generate the random R values
    void generateRValues()
    {
        // obtain a seed from the system clock:
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::mt19937 generator(seed);

        // Simply call generator() [it has an operator()] to get another random number
        for(int i = 0; i < 5; ++i)
        {
            rValues[i] = generator();
        }
    }
};

#endif
