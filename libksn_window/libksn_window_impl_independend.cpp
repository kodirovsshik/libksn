
#include <ksn/window.hpp>

#include <chrono>
#include <thread>

#pragma warning(disable : 4996)



_KSN_BEGIN



class window_t::_window_independend_impl
{

	friend class window_t;

	uint64_t m_last_tick = 0;
	uint32_t m_framerate = 0;
	uint32_t m_vsync_type = 0;
	static FILE* m_log;


public:


	_window_independend_impl() noexcept {}
	~_window_independend_impl() noexcept {}

	void tick() noexcept
	{
		using namespace std::chrono;
		using clock = steady_clock;

		uint64_t current_tick = (uint64_t)duration_cast<nanoseconds>(clock::now().time_since_epoch()).count();

		if (this->m_framerate != 0)
		{
			uint64_t desired_dt = uint64_t(1e9f / this->m_framerate);

			std::this_thread::sleep_until(time_point<clock>() + nanoseconds(m_last_tick + desired_dt));
			this->m_last_tick = (uint64_t)duration_cast<nanoseconds>(clock::now().time_since_epoch()).count();
		}
		else
			this->m_last_tick = current_tick;

	}
};

FILE* window_t::_window_independend_impl::m_log = fopen("log.txt", "w");



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
