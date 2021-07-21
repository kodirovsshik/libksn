
#ifndef _KSN_OPENCL_SELECTOR_
#define _KSN_OPENCL_SELECTOR_


#include <ksn/ksn.hpp>
#include <stdio.h>

#include <CL/opencl.h>


_KSN_BEGIN


struct opencl_selector_data_t
{
	//Basic parameters

	size_t cl_sources_number = 0;
	const char* const* cl_sources = nullptr;
	const size_t* cl_sources_lengthes = nullptr;

	int opencl_major = 1;
	int opencl_minor = 2;



	//Set to non-zero to force deivce or platform selection by index

	size_t preselected_platform = 0;
	size_t preselected_device = 0;



	//Auxilary parameters

	const wchar_t* build_log_file_name = nullptr;
	const wchar_t* build_log_file_open_format = L"w";
	size_t io_buffer_size = BUFSIZ;
	void* io_buffer = nullptr;



	//Cosmetic parameters
	
	const wchar_t* msg_no_opencl_platforms = L"No OpenCL platforms found in the system\n";
	const wchar_t* msg_no_opencl_situable_platforms = L"No situable OpenCL platform found in the system\n";
	const wchar_t* msg_platform_list = L"Select an OpenCL platform:\n";
	const wchar_t* msg_select = L"Select: ";
	const wchar_t* msg_device_by = L" by ";
	const wchar_t* msg_device_on = L" on ";
	const wchar_t* msgfmt_context_create_error = L"Failed to create OpenCL context, error %i";
	const char* cl_build_parameters = "";
	const wchar_t* msg_device = L"Device ";
	const wchar_t* msg_x_reported_build_error = L" has reported an OpenCL program build error\n";
	const wchar_t* msg_build_log = L"Build log:\n";
	const wchar_t* msg_build_log_saved_to = L"Build log saved to ";
	const wchar_t* msg_devices_list = L"List of devices on the current platform:\n";
	const wchar_t* msg_device_select = L"Select a device: ";
	const wchar_t* msg_build_log_not_allocated = L"<Failed to allocate memory to store build log>\n";



	//Output

	cl_context context = nullptr;
	cl_program program = nullptr;
	cl_command_queue q = nullptr;
	cl_device_id device = nullptr;
	cl_platform_id platform = nullptr;
};

//Returns OpenCL error value or 1 on selector's internal error (malloc failure, invalid input parameters, etc)
int opencl_selector(opencl_selector_data_t*) noexcept;


_KSN_END



#endif //!_KSN_OPENCL_SELECTOR_
