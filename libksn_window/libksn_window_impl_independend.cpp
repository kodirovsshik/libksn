
#include <ksn/window.hpp>
#include <ksn/time.hpp>

#include <chrono>
#include <thread>

#pragma warning(disable : 4996)



_KSN_BEGIN



class window_t::_window_independend_impl
{

	friend class window_t;

	stopwatch m_sw;
	uint32_t m_framerate = 0;


public:


	_window_independend_impl() noexcept {}
	~_window_independend_impl() noexcept {}

	void tick() noexcept
	{
		if (this->m_framerate != 0)
		{
			if (this->m_sw.is_started())
			{
				time desired_dt = time::from_nsec(1000000000 / this->m_framerate);
				time dt = this->m_sw.stop();
				sleep_for(desired_dt - dt);
			}
			this->m_sw.start();
		}
	}
};



void window_t::set_framerate(uint32_t new_framerate) noexcept
{
	this->m_impl_indep->m_framerate = new_framerate;
}
uint32_t window_t::get_framerate() const noexcept
{
	return this->m_impl_indep->m_framerate;
}
void window_t::tick() noexcept
{
	this->m_impl_indep->tick();
}



_KSN_END
