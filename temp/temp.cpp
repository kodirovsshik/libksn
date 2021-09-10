

constexpr int f()
{
	ksn::long_int512_t x(-3.141592);
	return (int)(float)x;
}


int main()
{

	constexpr int x = f();
	f();

	return 0;
}
