
#include <ksn/ksn.hpp>
#include <ksn/graphics.hpp>
#include <ksn/ppvector.hpp>

/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////
// to be moved to ppvector.hpp


/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////


_KSN_BEGIN

_KSN_GRAPHICS_BEGIN


render_texture_t::~render_texture_t() noexcept
{
}
render_texture_t::render_texture_t() noexcept
{
}
render_texture_t::render_texture_t(render_texture_t&& r) noexcept
	: m_impl(std::move(r.m_impl))
{

}



struct render_texture_t::_render_texture_impl
{
	//q for queue
	struct q_element
	{
		uint32_t index;
	};

	size_t m_width, m_height;
	uint8_t* m_data; //24 bits per pixel
	//shape_buffer_t* m_sb;
	
	ksn::ppvector<q_element> m_q;//ueue


	_render_texture_impl() noexcept
		: m_data(nullptr), m_width(0), m_height(0)
	{

	}
	_render_texture_impl(const _render_texture_impl&) noexcept = delete;
	_render_texture_impl(_render_texture_impl&&r) noexcept
		: m_data(r.m_data), m_width(r.m_width), m_height(r.m_height)
	{
		r._render_texture_impl::_render_texture_impl();
	}

	void destroy() noexcept
	{
		::free(this->m_data);
		this->_render_texture_impl::_render_texture_impl();
	}
	error_t create(size_t w, size_t h) noexcept
	{
		if (this->m_data) this->destroy();
		return this->_create(w, h);
	}
	error_t _create(size_t w, size_t h) noexcept
	{
		void* data = malloc(w * h * 3);
		if (!data) return error::out_of_memory;

		this->m_data = (uint8_t*)data;
		this->m_width = w;
		this->m_height = h;
		return error::ok;
	}

	error_t draw_surface(uint32_t index) noexcept
	{
		if (!this->m_q.reserve_more(8))
		{
			if (!this->m_q.reserve_more(1)) return error::out_of_memory;
		}

		this->m_q.push_back(q_element{ index });
	}
};


_KSN_GRAPHICS_END

_KSN_END
