#include <iostream>
#include <chrono>
#include <thread>

#define NOMINMAX
#include <Windows.h>

int main()
{

    bool b = std::numeric_limits<uint64_t>::max() > int(-1);

    using namespace std;
    using namespace chrono;
    using clock_t = high_resolution_clock;

    auto time_now = clock_t::now();
    auto time_last = clock_t::now();

    while (true)
    {


        //work
        Sleep(50);


        time_now = clock_t::now();

        auto work_time = duration_cast<milliseconds>(time_now - time_last).count();

        std::this_thread::sleep_for(milliseconds(200 - work_time));

        time_last = clock_t::now();
        auto sleep_time = duration_cast<milliseconds>(time_last - time_now).count();

        printf("Work: %i, Sleep: %i, Cycle: %i\n", (int)work_time, (int)sleep_time, int(work_time + sleep_time));
    }

}