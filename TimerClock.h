//
// Created by co2ma on 2023/3/26.
//

#ifndef CODE_TIMERCLOCK_H
#define CODE_TIMERCLOCK_H

#include <chrono>

class TimerClock
{
public:
    TimerClock()
    {
        update();
    }

    ~TimerClock()
    {
    }

    void update()
    {
        _start = std::chrono::high_resolution_clock::now();
    }

    double getTimerSecond()
    {
        return getTimerMicroSec() * 0.000001;
    }

    double getTimerMilliSec()
    {
        return getTimerMicroSec() * 0.001;
    }

    long long getTimerMicroSec()
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - _start).count();
    }
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> _start;
};

#endif //CODE_TIMERCLOCK_H
