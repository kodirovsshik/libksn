
#include <ksn/resource_manager.hpp>

#include <ksn/metapr.hpp>
#include <ksn/stuff.hpp>
#include <ksn/crc.hpp>

#include <stdio.h>

#include <stdexcept>
#include <filesystem>



#pragma warning(disable : 4996)



static_assert(sizeof(char) == 1, "");
static_assert(sizeof(signed char) == 1, "");
static_assert(sizeof(unsigned char) == 1, "");



_KSN_BEGIN

namespace
{

	template<ksn::arithmetic data_t>
	data_t file_bin_read(FILE* f, resource_load_result_t err = resource_load_result::file_corrupted)
	{
		data_t result;
		size_t read = fread(&result, 1, sizeof(result), f);
		if (read != sizeof(result))
			throw std::runtime_error(std::to_string(err));
		return result;
	}

	bool check_constant_presence(FILE* f, const void* data, size_t size, resource_load_result_t err = resource_load_result::file_corrupted)
	{
		const uint8_t* p = (const uint8_t*)data;
		while (size-- > 0)
		{
			if (file_bin_read<uint8_t>(f, err) != *p++)
				return false;
		}
		return true;
	};



#define assert_return(expr, result) if (!(expr)) { return (result); }
#define assert_corrupted(expr) assert_return(expr, resource_load_result::file_corrupted)
#define assert_const(fd, data, len) assert_corrupted(check_constant_presence(fd, data, len))

	using container_t = std::pmr::unordered_map<std::string, std::vector<uint8_t>>;
	resource_load_result_t _load_0001(FILE* f, container_t& cont)
	{
		size_t read;
		std::string name;

		uint32_t n = file_bin_read<uint32_t>(f);
		uint32_t max = file_bin_read<uint32_t>(f);

		ksn::malloc_guard mg;
		assert_return(mg.reserve(1), resource_load_result::out_of_memory);

		char* buffer = (char*)mg.alloc(max);
		assert_return(buffer, resource_load_result::out_of_memory);


		assert_const(f, "\x04", 1);


		while (n --> 0)
		{
			name.clear();


			assert_const(f, "\x01", 1); //Resource begin

			uint32_t name_size = file_bin_read<uint32_t>(f);
			uint32_t data_size = file_bin_read<uint32_t>(f);
			assert_corrupted(max >= name_size && max >= data_size);


			assert_const(f, "\x02", 1); //Name begin

			read = fread(buffer, 1, name_size, f);
			assert_corrupted(read == name_size);
			name = std::string(buffer, name_size);


			assert_const(f, "\x03\x02", 2); //Data begin

			read = fread(buffer, 1, data_size, f);
			assert_corrupted(read == data_size);

			cont.emplace(std::move(name), std::vector<uint8_t>(buffer, buffer + data_size));

			assert_const(f, "\x03\x04", 2); //End of resource
		}

		assert_const(f, "\x04\x00", 2); //End of Transmission
		return resource_load_result::ok;
	}

} //!namespace <anonimous>



resource_manager_t::resource_manager_t() noexcept {}

resource_manager_t::resource_manager_t(const std::string& fname) noexcept
{
	this->load(fname);
}

resource_load_result_t resource_manager_t::load(const std::string& fname) noexcept
{
	try
	{
		return this->_load(fname);
	}
	catch (const std::bad_alloc&)
	{
		return resource_load_result::out_of_memory;
	}
	catch (const std::runtime_error& excp)
	{
		int err;
		if (sscanf(excp.what(), "%i", &err) == 1)
			return err;
		else
			return resource_load_result::unknown;
	}
	catch (...)
	{
		return resource_load_result::unknown;
	}
}

resource_load_result_t resource_manager_t::_load(const std::string& fname)
{
	//Plan:
	//Read resource header, see below
	//Check for valid resource file
	//Load all the resources present in file


	FILE *f = fopen(fname.data(), "rb");
	if (!f) return resource_load_result::file_unavailable;

	if (!check_constant_presence(f, "\x00\x00\x00\x00\xFFRES\xFF\x01", 10, resource_load_result::not_a_resource_file))
		return resource_load_result::not_a_resource_file;

	if (true)
	{
		uint64_t crc_expected = file_bin_read<uint64_t>(f, resource_load_result::file_corrupted);
		long saved_pos = ftell(f);
		uint64_t crc = ksn::crc64_ecma_init();
		size_t read = 0;

		static char buffer[4096];

		do
		{
			read = fread(buffer, 1, ksn::countof(buffer), f);
			crc = ksn::crc64_ecma_update(buffer, read, crc);
		}
		while (read > 0);

		if (crc != crc_expected) return resource_load_result::file_corrupted;

		fseek(f, saved_pos, SEEK_SET);
	}


	uint16_t version = file_bin_read<uint16_t>(f);

	if (version < 0x0001)
		return resource_load_result::resource_file_version_too_old;

	resource_load_result_t result;

	switch (version)
	{
	case 0x0001:
		result = _load_0001(f, this->m_resources);
		break;
	default:
		result = resource_load_result::resource_file_version_too_new;
		break;
	}

	return result;
}

resource_load_result_t resource_manager_t::load_raw(const std::string& filename, const std::string& resource_name) noexcept
{
	if (this->resource_present(resource_name))
		return resource_load_result::resource_overlap;

	FILE* f = fopen(filename.c_str(), "rb");
	if (!f) return resource_load_result::file_unavailable;

	ksn::malloc_guard mg;
	if (!mg.reserve(1))
		return resource_load_result::out_of_memory;

	size_t size = std::filesystem::file_size(filename);	
	uint8_t* data = (uint8_t*)mg.alloc(size);
	if (!data) 
		return resource_load_result::out_of_memory;

	fread(data, sizeof(char), size, f);
	this->m_resources.emplace(
		std::piecewise_construct,
		std::forward_as_tuple(resource_name),
		std::forward_as_tuple(data, data + size)
	);

	return resource_load_result::ok;
}



void resource_manager_t::free() noexcept
{
	this->m_resources.clear();
}
void resource_manager_t::free(const std::string& name) noexcept
{
	this->m_resources.erase(name);
}

bool resource_manager_t::resource_present(const std::string& name) const noexcept
{
	return this->m_resources.count(name) != 0;
}



std::vector<uint8_t>& resource_manager_t::operator[](const std::string& s)
{
	return this->m_resources.at(s);
}
const std::vector<uint8_t>& resource_manager_t::operator[](const std::string& s) const
{
	return this->m_resources.at(s);
}



//Adds a new resource and returns its empty data
std::vector<uint8_t>& resource_manager_t::add(const std::string& name)
{
	this->free(name);
	return this->m_resources[name];
}

//Inserts a new resource
void resource_manager_t::insert(const std::string& name, const std::vector<uint8_t>& data)
{
	this->m_resources.insert({ name, data });
}
void resource_manager_t::insert(const std::string& name, std::vector<uint8_t>&& data)
{
	this->m_resources.emplace(std::make_pair(name, std::move(data)));
}
void resource_manager_t::insert(std::string&& name, const std::vector<uint8_t>& data)
{
	this->m_resources.emplace(std::make_pair(std::move(name), data));
}
void resource_manager_t::insert(std::string&& name, std::vector<uint8_t>&& data)
{
	this->m_resources.emplace(std::make_pair(std::move(name), std::move(data)));
}


//Saves all the resources into a resource file
bool resource_manager_t::save(const std::string& name) const noexcept
{
	FILE* f = fopen(name.data(), "wb");
	assert_return(f, false);



#define output_str(s) { fwrite(s, 1, ksn::countof(s) - 1, f); crc = ksn::crc64_ecma_update(s, ksn::countof(s) - 1, crc); }
#define output_std(s) { fwrite(s.data(), 1, s.size(), f); crc = ksn::crc64_ecma_update(s.data(), s.size(), crc); }
#define output_data(s) { fwrite(&s, 1, sizeof(s), f); crc = ksn::crc64_ecma_update(&s, sizeof(s), crc); }


	//version 0x0001



	fwrite("\x00\x00\x00\x00\xFFRES\xFF\x01\0\0\0\0\0\0\0\0", 1, 18, f);
	//^^^
	//File indentifier
	//CRC placeholder (8 NULLs)



	uint16_t temp16;
	uint32_t temp;
	uint64_t crc = ksn::crc64_ecma_init();



	temp16 = 0x0001;
	//Version
	output_data(temp16);



	temp = (uint32_t)this->m_resources.size();
	//Resource entries count
	output_data(temp);



	//Max size placeholder
	uint32_t max_buffer_size = 0;

	for (const auto& [name, data] : this->m_resources)
	{
		if (name.size() > max_buffer_size) max_buffer_size = (uint32_t)name.size();
		if (data.size() > max_buffer_size) max_buffer_size = (uint32_t)data.size();
	}

	output_data(max_buffer_size);



	//End of header
	output_str("\x04");



	//Write resource entries
	for (const auto& [name, data] : this->m_resources)
	{
		//SOH, designates start of resource
		output_str("\x01");



		//Size of resource name
		temp = (uint32_t)name.size();
		output_data(temp);
		
		//Size of resource data
		temp = (uint32_t)data.size();
		output_data(temp);


		//STX, (Start of Text), designates resource name start
		output_str("\x02");

		//Write resource name
		output_std(name);

		//ETX, STX (End of Text, Start of Text), designates resource data start
		output_str("\x03\x02");

		//Write resource data
		output_std(data);

		//ETX, EOT (End of Text, End of Transmission), designates end of resource entry
		output_str("\x03\x04");
	}


	//EOT, NUL - End of Transmission
	output_str("\x04\x00");


	fseek(f, 10, SEEK_SET);
	fwrite(&crc, 1, 8, f);

	fclose(f);
	return true;
}


_KSN_END

/*

Resource file specification
Verion 0x0001

Resource header:
	ascii[] = "\x00\x00\x00\x00\xFFRES\xFF\x01"
		File indetifier constant


	uint64 CRC64_ECMA_182
		crc64-ecma checksum of all the data after this entry


	uint16 version
		Resource file version


	uint32 N
		Resource entries count


	uint32 MAXSIZE
		Maximum buffer size needed


	ascii = EOT ('\x04', End Of Transmission)
		Header end constant


	resource_entry[N]:

		ascii[] = SOH ('0x01', Shart of Header)
			Start of resource


		uint32 name_size, data_size
			Size of resource name and resource data


		ascii = STX ('\x02', Start of Text)
			resource name start


		uint8[name_size]
			Resource name


		ascii[] = ETX, STX ("\x03\x02", End of Text, Start Of Text)
			resource name end, resource data begin


		uint8[N]
			Resource data


		ascii[] = ETX, EOT ("\x03\x04", End Of Text, End of Transmission)
			resource data end, resource header end


	ascii[] = EOT, NUL ("\x04\x00", End of Transmission, NULL)
		Resource file end
*/
