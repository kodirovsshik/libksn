
#define CL_TARGET_OPENCL_VERSION 100
#define CL_MINIMUM_OPENCL_VERSION 100

#include <ksn/opencl_selector.hpp>
#include <ksn/try_smol_buffer.hpp>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>

#include <CL/opencl.h>

//GCC 10.2 on my ubuntu machine doesn't support what i need for now
//Commented until i build 11.0 experimental from master branch
//#include <ranges> 
//In replacement
#include <algorithm>



#ifdef _KSN_COMPILER_MSVC
#pragma warning(disable : 4996)
#endif


_KSN_BEGIN


static jmp_buf selector_return_jmp_buffer;

[[noreturn]] void selector_return(int code, const wchar_t* fmt = L"", ...)
{
	va_list ap;
	va_start(ap, fmt);

	vfwprintf(stderr, fmt, ap);

	va_end(ap);

	longjmp(selector_return_jmp_buffer, code);
}

static FILE* print_fd = nullptr;
static void print(const char* str)
{
	fprintf(stdout, "%s", str);
	if (print_fd)
		fprintf(print_fd, "%s", str);
}
static void print(const wchar_t* str)
{
	fwprintf(stdout, L"%s", str);
	if (print_fd)
		fwprintf(print_fd, L"%s", str);
}

static FILE* wide_open(const wchar_t* wpath, const wchar_t* wmode)
{
	if (wpath == nullptr || wmode == nullptr) return nullptr;

	char path[_MAX_PATH + 1], mode[64];
	path[_MAX_PATH] = 0;
	mode[63] = 0;

	if (wcstombs(path, wpath, _MAX_PATH) == size_t(-1)) return nullptr;
	if (wcstombs(mode, wmode, 63) == size_t(-1)) return nullptr;

	return fopen(path, mode);
}



static int selector(opencl_selector_data_t*) noexcept;

int opencl_selector(opencl_selector_data_t* data) noexcept
{
	int code;
	if ((code = setjmp(selector_return_jmp_buffer)) == 0)
	{
		code = selector(data);
	}
	if (code == INT_MAX) code = 0;
	else
	{
		clReleaseProgram(data->program);
		clReleaseCommandQueue(data->q);
		clReleaseContext(data->context);
	}
	return code;
}

static int selector(opencl_selector_data_t* p) noexcept
{
	if (p->cl_sources == nullptr) selector_return(1, L"data->cl_sources not specified");
	if (p->cl_sources_lengthes == nullptr) selector_return(1, L"data->cl_sources_lengthes not specified");
	if (p->cl_sources_number == 0) selector_return(1, L"data->cl_sources_number not specified");

	union
	{
		char buffer4k[4096];
		int8_t buffer4ki8[4096];
		uint8_t buffer4kui8[4096];
		int32_t buffer4ki32[1024];
		uint32_t buffer4kui32[1024];
	};
	void* buffer4kvoid = (void*)buffer4k;
	memset(buffer4k, 0, sizeof(buffer4k));
	size_t temp = 0;


	static constexpr size_t platforms_static_max = 64;
	cl_platform_id platforms[platforms_static_max];
	uint8_t platforms_situable[platforms_static_max / 8];
	cl_uint platforms_count;
	temp = clGetPlatformIDs(platforms_static_max, platforms, &platforms_count);

	if (platforms_count == 0) selector_return(1, p->msg_no_opencl_platforms);

	memset(platforms_situable, 0, sizeof(platforms_situable));
	for (cl_uint i = 0; i < platforms_count; ++i)
	{
		temp = clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, sizeof(buffer4k), buffer4k, &temp);

		int ver_maj = 0, ver_min = 0;

		auto set_platform_situable = [&]
		() -> void
		{
			if (ver_maj > p->opencl_major || ver_maj == p->opencl_major && ver_min >= p->opencl_minor)
			{
				uint8_t& byte = platforms_situable[i / 8];
				uint8_t value = 1 << (i % 8);
				byte |= value;
			}
		};

		if (
			sscanf(buffer4k, "OpenCL %i.%i", &ver_maj, &ver_min) == 2
			)
		{
			set_platform_situable();
			continue;
		}

		char* p = buffer4k;
		//skip all non-digit in the buffer
		while (*p != '\0' && !isdigit(*p)) ++p;

		if (*p != '\0' && sscanf(buffer4k, "%i.%i", &ver_maj, &ver_min) == 2)
		{
			set_platform_situable();
			continue;
		}
	}


	if (std::find_if_not(platforms_situable, std::end(platforms_situable), [](uint8_t x) { return x == 0; }) == std::end(platforms_situable))
	{
		selector_return(1, p->msg_no_opencl_situable_platforms);
	}


	size_t platform_index;

	if (p->platform == 0)
	{
		setvbuf(stdout, (char*)p->io_buffer, _IOFBF, p->io_buffer_size);
		print(p->msg_platform_list);
		for (cl_uint i = 0; i < platforms_count; ++i)
		{
			if (platforms_situable[i / 8] & (1 << (i % 8)))
			{
				temp = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(buffer4k), buffer4k, nullptr);
				printf("[%i]: ", int(i) + 1);
				print(buffer4k);
				temp = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, sizeof(buffer4k), buffer4k, nullptr);
				print(p->msg_device_by);
				print(buffer4k);
				temp = clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, sizeof(buffer4k), buffer4k, nullptr);
				print(p->msg_device_on);
				print(buffer4k);
				putchar('\n');
			}
		}
		setvbuf(stdout, nullptr, _IONBF, 0);

		rewind(stdin);
		do
		{
			print(p->msg_select);
			int platform_index_signed;
			if (scanf("%i", &platform_index_signed) != 1)
			{
				rewind(stdin);
				continue;
			}

			platform_index = (size_t)platform_index_signed;
			if (platform_index <= 0 || platform_index > (int)platforms_count) continue;
			--platform_index;

			if (platforms_situable[platform_index / 8] & (1 << (platform_index % 8))) break;
		} while (true);

		p->platform = platform_index + 1;
	}
	else
	{
		size_t n = p->platform - 1;
		if (n >= platforms_count || (platforms_situable[n / 8] & (1 << (n % 8))) == 0)
			selector_return(1, L"Wrong preselected platform");
		platform_index = n;
	}

	cl_uint num_devices = 0;
	temp = clGetDeviceIDs(platforms[platform_index], CL_DEVICE_TYPE_ALL, 4096 / sizeof(cl_device_id), (cl_device_id*)buffer4k, &num_devices);

	cl_context_properties context_properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[platform_index], 0 };

	temp = 0;
	p->context = clCreateContext(context_properties, num_devices, (cl_device_id*)buffer4k, nullptr, nullptr, (cl_int*)&temp);
	if (temp != 0 || p->context == nullptr) selector_return(1, p->msgfmt_context_create_error, (int)temp);

	p->program = clCreateProgramWithSource(p->context, (cl_uint)p->cl_sources_number, (const char**)p->cl_sources, p->cl_sources_lengthes, (cl_int*)&temp);
	cl_int tempi = clBuildProgram(p->program, num_devices, (cl_device_id*)buffer4k, p->cl_build_parameters, nullptr, nullptr);
	if (tempi != CL_BUILD_SUCCESS)
	{
		print_fd = wide_open(p->build_log_file_name, p->build_log_file_open_format);
		setvbuf(stdout, (char*)p->io_buffer, _IOFBF, p->io_buffer_size);
		for (cl_uint i = 0; i < num_devices; ++i)
		{
			auto& device = ((cl_device_id*)buffer4k)[i];
			clGetProgramBuildInfo(p->program, device, CL_PROGRAM_BUILD_STATUS, sizeof(temp), &temp, nullptr);
			if (temp != CL_BUILD_SUCCESS)
			{
				char smol_buffer[256];
				size_t length = 0;
				temp = clGetDeviceInfo(device, CL_DEVICE_NAME, 256, smol_buffer, nullptr);
				print(p->msg_device);
				print(smol_buffer);
				print(p->msg_x_reported_build_error);
				print(p->msg_build_log);
				temp = clGetProgramBuildInfo(p->program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &length);
				char* big_buffer = (char*)malloc(length);
				if (big_buffer == nullptr)
					print(p->msg_build_log_not_allocated);
				else
				{
					temp = clGetProgramBuildInfo(p->program, device, CL_PROGRAM_BUILD_LOG, length, big_buffer, nullptr);
					print(big_buffer);
					putchar('\n');
					putchar('\n');
					free(big_buffer);
				}
			}
		}

		setvbuf(stdout, nullptr, _IONBF, 0);
		if (print_fd)
		{
			fclose(print_fd);
			print_fd = nullptr;
			print(p->msg_build_log_saved_to);
			print(p->build_log_file_name);
			putchar('\n');
		}
		selector_return(1);
	}
	else if (tempi != CL_BUILD_SUCCESS)
		selector_return(1, L"OpenCL program build error %i\n", (int)tempi);

	int device_id;
	if (p->device == 0)
	{
		setvbuf(stdout, (char*)p->io_buffer, _IOFBF, p->io_buffer_size);
		print(p->msg_devices_list);
		for (cl_uint i = 0; i < num_devices; ++i)
		{
			cl_device_id device = ((cl_device_id*)buffer4k)[i];
			printf("[%i]: ", i + 1);
			char buffer[4096];
			temp = clGetDeviceInfo(device, CL_DEVICE_NAME, 4096, buffer, &temp);
			print(buffer);
			print(p->msg_device_by);
			temp = clGetDeviceInfo(device, CL_DEVICE_VENDOR, 4096, buffer, &temp);
			print(buffer);
			putchar('\n');
		}
		setvbuf(stdout, nullptr, _IONBF, 0);

		rewind(stdin);
		while (1)
		{
			print(p->msg_device_select);

			if (scanf("%i", &device_id) != 1)
			{
				rewind(stdin);
				continue;
			}
			if (device_id <= 0 || device_id > (int)num_devices) continue;
			break;
		}

		p->device = (size_t)device_id;
	}
	else
	{
		if (p->device > num_devices || p->device == 0)
			selector_return(1);
	}

	p->q = clCreateCommandQueue(p->context, ((cl_device_id*)buffer4k)[p->device - 1], 0, (int*)&temp);
	//p->context = context;
	//p->program = program;


	return p->q == nullptr ? 1 : INT_MAX;
}


_KSN_END
