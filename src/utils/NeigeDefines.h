#pragma once
#include <iostream>
#include <string>

#define NEIGE_VK_CHECK(f) \
	do { \
		int64_t check = f; \
		if (check) { \
			std::cerr << "\033[1m\033[31mVULKAN ERROR :\033[39m\033[0m A Vulkan error has happened." << std::endl << "Error code: " << check << std::endl << "File: " << __FILE__ << std::endl << "Function: " << __FUNCTION__ << std::endl << "Line : " << __LINE__ << std::endl; \
			exit(1); \
		} \
	} while (0)

#define NEIGE_VK_VALIDATION_ERROR(m) \
	do { \
		std::cerr << "\033[1m\033[31mVALIDATION LAYER :\033[39m\033[0m " << m << std::endl; \
	} while(0)

#define NEIGE_WARNING(m) \
	do { \
		std::cerr << "\033[1m\033[33mNEIGE WARNING :\033[39m\033[0m " << m << std::endl; \
	} while(0)

#define NEIGE_ERROR(m) \
	do { \
		std::cerr << "\033[1m\033[31mNEIGE ERROR :\033[39m\033[0m " << m << std::endl; \
		exit(2); \
	} while(0)

#ifndef NDEBUG
#define NEIGE_INFO(m) \
	do { \
		std::cout << "\033[1m\033[94mNEIGE INFO :\033[39m\033[0m " << m << std::endl; \
	} while(0)
#else
#define NEIGE_INFO(m)
#endif

#ifndef NDEBUG
#define NEIGE_ASSERT(c, m) \
	do { \
		if (!(c)) { \
			std::cerr << "\033[1m\033[35mNEIGE ASSERT :\033[39m\033[0m " << m << std::endl; \
			exit(3); \
		} \
	} while(0)
#else
#define NEIGE_ASSERT(c, m)
#endif

#ifndef NDEBUG
#define MEMORY_INFO(m) \
	do { \
		std::cerr << "\033[1m\033[94mMEMORY INFO :\033[39m\033[0m " << m << std::endl; \
	} while(0)
#else
#define MEMORY_INFO(m)
#endif

#define NEIGE_SHADER_ERROR(m) \
	do { \
		std::cerr << "\033[1m\033[31mSHADER ERROR :\033[39m\033[0m " << m << std::endl; \
	} while(0)

#define NEIGE_SCRIPT_ERROR(m) \
	do { \
		std::cerr << "\033[1m\033[31mSCRIPT ERROR :\033[39m\033[0m " << m << std::endl; \
	} while(0)

#define NEIGE_AUDIO_ERROR(m) \
	do { \
		std::cerr << "\033[1m\033[31mAUDIO ERROR :\033[39m\033[0m " << m << std::endl; \
	} while(0)

#ifndef NDEBUG
const bool NEIGE_DEBUG = true;
#else
const bool NEIGE_DEBUG = false;
#endif