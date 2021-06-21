
#include <memory>


template<class impl_t>
class impl_wrapper
{
    std::unique_ptr< impl_t> m_impl;
public:
    impl_wrapper();
    ~impl_wrapper();
};

class wgl
{
    class wgl_impl;
public:

    impl_wrapper<wgl_impl> m_impl;
};

int main()
{

    wgl obj;

}