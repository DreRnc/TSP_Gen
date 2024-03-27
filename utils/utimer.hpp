#ifndef UTIMER_HPP
#define UTIMER_HPP

#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>

#define START(timename) auto timename = std::chrono::system_clock::now();
#define STOP(timename, elapsed) auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - timename).count();

class utimer {
private:
    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point stop;
    std::string message;
    using usecs = std::chrono::microseconds;
    using msecs = std::chrono::milliseconds;

private:
    long* us_elapsed;

public:
    utimer(const std::string m) : message(m), us_elapsed(nullptr) {
        start = std::chrono::system_clock::now();
    }

    utimer(const std::string m, long* us) : message(m), us_elapsed(us) {
        start = std::chrono::system_clock::now();
    }

    ~utimer() {
        stop = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed = stop - start;
        auto musec = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

        if (us_elapsed == nullptr)
            std::cout << message << " computed in " << std::setw(15) << musec << " usec "
                      << std::endl;
        if (us_elapsed != nullptr)
            (*us_elapsed) = musec;
    }
};

#endif