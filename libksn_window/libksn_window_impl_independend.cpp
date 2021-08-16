
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

	void* m_arbitrary_data = nullptr;
	size_t m_arbitrary_data_size = 0;

	uint32_t m_framerate = 0;

	bool m_arbitrary_data_owned = false;

public:


	_window_independend_impl() noexcept {}
	~_window_independend_impl() noexcept
	{
		this->cleanup();
	}

	void cleanup() noexcept
	{
		this->deallocate_arbitrary_data();
	}

	void deallocate_arbitrary_data() noexcept
	{
		if (this->m_arbitrary_data && this->m_arbitrary_data_size && this->m_arbitrary_data_owned)
		{
			free(this->m_arbitrary_data);
			this->m_arbitrary_data = nullptr;
			this->m_arbitrary_data_size = 0;
		}
	}

	void tick(void(*sleeper)(ksn::time)) noexcept
	{
		if (this->m_framerate != 0)
		{
			time desired_dt = time::from_nsec(1000000000 / this->m_framerate);
			time dt = this->m_sw.stop();
			sleeper(desired_dt - dt);
			this->m_sw.start();
		}
	}

};



void window_t::set_framerate(uint32_t new_framerate) noexcept
{
	this->m_impl_indep->m_framerate = new_framerate;
	if (new_framerate)
		this->m_impl_indep->m_sw.start();
	else
		this->m_impl_indep->m_sw.stop();
}
uint32_t window_t::get_framerate() const noexcept
{
	return this->m_impl_indep->m_framerate;
}
void window_t::tick() noexcept
{
	this->m_impl_indep->tick(ksn::sleep_for);
}
void window_t::tick_hybrid_sleep() noexcept
{
	if (ksn::get_hybrid_sleep_threshold())
		this->m_impl_indep->tick(ksn::hybrid_sleep_for);
	else
		this->m_impl_indep->tick(ksn::sleep_for);
}



uint16_t window_t::get_client_width() const noexcept
{
	return this->get_client_size().first;
}
uint16_t window_t::get_client_height() const noexcept
{
	return this->get_client_size().second;
}
void window_t::set_client_width(uint16_t w) noexcept
{
	this->set_client_size(w, this->get_client_height());
}
void window_t::set_client_height(uint16_t h) noexcept
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
void window_t::set_client_x(int16_t x) noexcept
{
	this->set_client_position(x, this->get_client_y());
}
void window_t::set_client_y(int16_t y) noexcept
{
	this->set_client_position(this->get_client_x(), y);
}

void window_t::set_client_position(std::pair<int16_t, int16_t> pos) noexcept
{
	return this->set_client_position(pos.first, pos.second);
}

void window_t::set_client_size(std::pair<uint16_t, uint16_t> size) noexcept
{
	return this->set_client_size(size.first, size.second);
}



void window_t::arbitrary_data_set_pointer(void* data, size_t buffer_size) noexcept
{
	auto* const impl = this->m_impl_indep.operator->();

	impl->deallocate_arbitrary_data();
	
	impl->m_arbitrary_data = data;
	impl->m_arbitrary_data_size = buffer_size;
	impl->m_arbitrary_data_owned = false;
}
bool window_t::arbitrary_data_allocate(size_t alloc_size) noexcept
{
	auto* const impl = this->m_impl_indep.operator->();

	bool replace = false;
	if (!impl->m_arbitrary_data_owned)
		replace = true;

	if (impl->m_arbitrary_data_size == 0 || impl->m_arbitrary_data == nullptr)
	{
		if (impl->m_arbitrary_data = malloc(alloc_size))
		{
			impl->m_arbitrary_data_size = alloc_size;
			impl->m_arbitrary_data_owned = true;
			return true;
		}
		return false;
	}
	else
	{
		void* old_data = impl->m_arbitrary_data;
		size_t old_size = impl->m_arbitrary_data_size;

		void* new_data = malloc(alloc_size);
		if (!new_data)
			return false;

		memcpy(new_data, old_data, std::min(old_size, alloc_size));
		if (impl->m_arbitrary_data_owned)
			free(old_data);

		impl->m_arbitrary_data = new_data;
		impl->m_arbitrary_data_size = alloc_size;
		impl->m_arbitrary_data_owned = true;

		return true;
	}
}
void window_t::arbitrary_data_deallocate() noexcept
{
	this->m_impl_indep->deallocate_arbitrary_data();
}

void* window_t::arbitrary_data_get_pointer() noexcept
{
	return this->m_impl_indep->m_arbitrary_data;
}
size_t window_t::arbitrary_data_get_size() noexcept
{
	return this->m_impl_indep->m_arbitrary_data_size;
}





void swap(window_t& a, window_t& b) noexcept
{
	a.swap(b);
}


_KSN_END
