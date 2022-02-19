
#ifndef _KSN_TIME_HPP_
#define _KSN_TIME_HPP_



#include <ksn/ksn.hpp>



_KSN_BEGIN


class time
{
	int64_t m_nsec;


public:

	time() noexcept; //01.01.1980
	time(int64_t nanoseconds_since_epoch) noexcept;


	int64_t as_nsec() const noexcept; //Return current time as nanoseconds since epoch
	int64_t as_usec() const noexcept; //Return current time as microseconds since epoch
	int64_t as_msec() const noexcept; //Return current time as milliseconds since epoch
	int64_t as_sec() const noexcept; //Return current time as seconds since epoch
	float as_float_sec() const noexcept; //Current time as seconds since epoch
	double as_double_sec() const noexcept; //Current time as seconds since epoch


	static time from_nsec(int64_t nanoseconds_since_epoch) noexcept;
	static time from_usec(int64_t microseconds_since_epoch) noexcept;
	static time from_msec(int64_t milliseconds_since_epoch) noexcept;
	static time from_sec(int64_t seconds_since_epoch) noexcept;
	static time from_float_sec(float seconds_since_epoch) noexcept;
	static time from_double_sec(double seconds_since_epoch) noexcept;


	static time now() noexcept; //Current time


	friend time operator+(time lhs, time rhs) noexcept;
	friend time operator-(time lhs, time rhs) noexcept;

	operator int64_t() const noexcept;
	operator bool() const noexcept;

	friend bool operator<(time lhs, time rhs) noexcept;
	friend bool operator>(time lhs, time rhs) noexcept;
	friend bool operator<=(time lhs, time rhs) noexcept;
	friend bool operator>=(time lhs, time rhs) noexcept;
	friend bool operator==(time lhs, time rhs) noexcept;
	friend bool operator!=(time lhs, time rhs) noexcept;
};


class stopwatch
{
private:

	time m_last_time;


public:

	void start() noexcept;
	time restart() noexcept; //returns 0 if not started
	time current() const noexcept; //returns 0 if not started
	time stop() noexcept;

	bool is_started() const noexcept;
};


void sleep_for(time dt);
void sleep_until(time point);

void busy_sleep_for(time dt);
void busy_sleep_until(time point);

void hybrid_sleep_for(time dt);
void hybrid_sleep_until(time point);


ksn::time init_hybrid_sleep_threshold(float max_relative_error = 0.1f) noexcept;
ksn::time get_hybrid_sleep_threshold() noexcept;


_KSN_END

#endif //!_KSN_TIME_HPP_
