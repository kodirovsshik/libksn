
#include <ksn/time.hpp>

#include <chrono>
#include <thread>



_KSN_BEGIN


time::time() noexcept
	: m_nsec(0) {}
time::time(unsigned long long ns) noexcept
	: m_nsec(ns) {}



int64_t time::as_nsec() const noexcept
{
	return this->m_nsec;
}
int64_t time::as_usec() const noexcept
{
	return this->m_nsec / 1000;
}
int64_t time::as_msec() const noexcept
{
	return this->m_nsec / 1000000;
}
int64_t time::as_sec() const noexcept
{
	return this->m_nsec / 1000000000;
}



time time::from_nsec(int64_t ns) noexcept
{
	return time(ns);
}
time time::from_usec(int64_t us) noexcept
{
	return time(us * 1000);
}
time time::from_msec(int64_t ms) noexcept
{
	return time(ms * 1000000);
}
time time::from_sec(int64_t s) noexcept
{
	return time(s * 1000000000);
}



time time::now() noexcept
{
	using namespace std;
	using namespace chrono;
	return time(duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count());
}



time::operator int64_t() const noexcept
{
	return this->m_nsec;
}
time::operator bool() const noexcept
{
	return this->m_nsec != 0;
}



time operator+(time lhs, time rhs) noexcept
{
	return time(lhs.m_nsec + rhs.m_nsec);
}
time operator-(time lhs, time rhs) noexcept
{
	return time(lhs.m_nsec - rhs.m_nsec);
}
bool operator<(time lhs, time rhs) noexcept
{
	return lhs.m_nsec < rhs.m_nsec;
}
bool operator>(time lhs, time rhs) noexcept
{
	return lhs.m_nsec > rhs.m_nsec;
}
bool operator<=(time lhs, time rhs) noexcept
{
	return lhs.m_nsec <= rhs.m_nsec;
}
bool operator>=(time lhs, time rhs) noexcept
{
	return lhs.m_nsec >= rhs.m_nsec;
}
bool operator==(time lhs, time rhs) noexcept
{
	return lhs.m_nsec == rhs.m_nsec;
}
bool operator!=(time lhs, time rhs) noexcept
{
	return lhs.m_nsec != rhs.m_nsec;
}





void stopwatch::start() noexcept
{
	this->m_last_time = time::now();
}
time stopwatch::restart() noexcept
{
	if (this->m_last_time)
	{
		time now = time::now();
		time dt = now - this->m_last_time;
		this->m_last_time = now;
		return dt;
	}
	else
	{
		this->m_last_time = time::now();
		return time();
	}
}
time stopwatch::current() noexcept
{
	if (this->m_last_time)
		return time::now() - this->m_last_time;
	else
		return time();
}
time stopwatch::stop() noexcept
{
	time saved_time = this->m_last_time;
	this->m_last_time = time();
	return saved_time;
}



#ifdef _KSN_COMPILER_MSVC

#define NOMINMAX
#include <Windows.h>

namespace
{
	static HANDLE ksn_sleep_timer;

	void nanosleep(int64_t dt, bool absolute)
	{
		//negative absolute time or duration means no sleep
		if (dt < 0) return;

		//At first try to sleep using multimedia timers from winmm
		bool ok = true;

		LARGE_INTEGER li;
		if (!ksn_sleep_timer) ok = false;
		{
			li.QuadPart = absolute ? dt / 100 : -(dt / 100); //negative time interval means relative to current time
			//And the time measured in 0.1 parts of microseconds
		}
		
		if (ok) ok = SetWaitableTimer(ksn_sleep_timer, &li, 0, NULL, NULL, FALSE);
		if (ok) ok = WaitForSingleObject(ksn_sleep_timer, INFINITE) != WAIT_FAILED;
		if (ok) return;

		//If sometring went wrong, fall back to a simple Sleep function
		if (absolute)
		{ //turn an absolute interval into a relative one
			dt -= time::now().as_nsec();
			if (dt < 0) return;
		}
		dt /= 1000000; //time can only be measured in milliseconds for this

		static constexpr DWORD dword_max = std::numeric_limits<DWORD>::max();
		if ((uint64_t)dt > dword_max)
			Sleep(dword_max); //infinite sleep
		else
			Sleep((DWORD)dt);
	}


	struct _lib_sentry //Library constructor and destructor
	{
		_lib_sentry()
		{
			ksn_sleep_timer = CreateWaitableTimerA(NULL, TRUE, NULL);
		}
		~_lib_sentry()
		{
			if (ksn_sleep_timer) CloseHandle(ksn_sleep_timer);
		}
	} _sentry;
}

void sleep_for(time dt)
{
	nanosleep(dt.as_nsec(), false);
}
void sleep_until(time point)
{
	nanosleep(point.as_nsec(), true);
}


#else

#error Not implemented for this platform

#endif


_KSN_END
