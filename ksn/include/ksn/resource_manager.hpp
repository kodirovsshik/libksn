
#ifndef _LIBKSN_RESOURCE_MANAGER_HPP_
#define _LIBKSN_RESOURCE_MANAGER_HPP_



#include <unordered_map>
#include <string>

#include <ksn/ksn.hpp>



_KSN_BEGIN



using resource_load_result_t = uint8_t;
struct resource_load_result
{
	using result_t = resource_load_result_t;

	//Unknown error, shoud never happen
	static constexpr result_t unknown = -1;

	//Everything is ok
	static constexpr result_t ok = 0;

	//The file passed into resource manager is not a resource file
	static constexpr result_t not_a_resource_file = 1;

	//Resource file version is too new and unsupported by the current version of the resource manager
	static constexpr result_t resource_file_version_too_new = 2;

	//Resource file version is too old and unsupported anymore
	static constexpr result_t resource_file_version_too_old = 3; //Probably never gonna be needed

	//Memory allocation failed during resource loading
	static constexpr result_t out_of_memory = 4;

	//Failed to open/read from file
	static constexpr result_t file_unavailable = 5;

	//Resource file data is corrupted
	static constexpr result_t file_corrupted = 6;

	//Found two resources with the same names
	static constexpr result_t resource_overlap = 7;
};



class resource_manager_t
{
private:

	using container_t = std::pmr::unordered_map<std::string, std::vector<uint8_t>>;

	container_t m_resources;


	resource_load_result_t _load(const std::string& resource_file);



public:
	resource_manager_t() noexcept;
	resource_manager_t(const resource_manager_t&) = delete;
	resource_manager_t(resource_manager_t&&) = delete;

	resource_manager_t(const std::string& resource_file) noexcept;



	//Saves all the resources into a resource file
	bool save(const std::string& name) const noexcept;

	//Loads all the resources from the resource file
	resource_load_result_t load(const std::string& resource_file) noexcept;

	//Load raw file as a resource
	resource_load_result_t load_raw(const std::string& filename, const std::string& resource_name) noexcept;



	//Frees all the loaded resources
	//All references are invalidated
	void free() noexcept;

	//Frees the specified resource
	void free(const std::string& name) noexcept;



	//Checks if the specified resource was found and loaded
	bool resource_present(const std::string& name) const noexcept;

	//Returns a reference to a present resource data
	std::vector<uint8_t>& operator[](const std::string& s);
	
	//Returns a reference to a present resource data
	const std::vector<uint8_t>& operator[](const std::string& s) const;
	


	//Adds a new resource and returns its empty data
	std::vector<uint8_t>& add(const std::string& name);
	
	//Inserts a new resource
	void insert(const std::string& name, const std::vector<uint8_t>& data);
	void insert(const std::string& name, std::vector<uint8_t>&& data);
	void insert(std::string&& name, const std::vector<uint8_t>& data);
	void insert(std::string&& name, std::vector<uint8_t>&& data);
};


_KSN_END

#endif //!_LIBKSN_RESOURCE_MANAGER_HPP_

