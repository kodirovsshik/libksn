
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



uint16_t window_t::get_client_width() const noexcept
{
	return this->get_client_size().first;
}
uint16_t window_t::get_client_height() const noexcept
{
	return this->get_client_size().second;
}
void window_t::set_client_width(uint16_t w) const noexcept
{
	this->set_client_size(w, this->get_client_height());
}
void window_t::set_client_height(uint16_t h) const noexcept
{
	this->set_client_size(this->get_client_width(), h);
}
int16_t window_t::get_client_x() const noexcept
{
	return this->get_client_position().first;
}
int16_t window_t::get_client_y() const noexcept
{
	return this->get_client_position().second;
}
void window_t::set_client_x(int16_t x) const noexcept
{
	this->set_client_position(x, this->get_client_y());
}
void window_t::set_client_y(int16_t y) const noexcept
{
	this->set_client_position(this->get_client_x(), y);
}

void window_t::set_client_position(std::pair<int16_t, int16_t> pos) const noexcept
{
	return this->set_client_position(pos.first, pos.second);
}

void window_t::set_client_size(std::pair<uint16_t, uint16_t> size) const noexcept
{
	return this->set_client_size(size.first, size.second);
}

_KSN_END
