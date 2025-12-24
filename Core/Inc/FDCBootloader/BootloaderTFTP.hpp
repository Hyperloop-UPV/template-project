#pragma once

// #include "C++Utilities/CppUtils.hpp"
#include "lwip/apps/tftp_server.h"
#include "ST-LIB.hpp"
#include "Flash.hpp"



#define TFTP_MAX_DATA_SIZE 					 ((uint16_t)512U)

#define TFTP_MAX_BOOTLOADER_CONEXION_RETRIES ((uint8_t)3U)

class BTFTP{
public:

	struct btftp_file_t{
		uint8_t* payload;
		uint32_t pointer;
		uint32_t max_pointer;
	};

	struct BHandle{
		// string name;
		// string mode;
        const char* name;
        const char* mode;
		uint8_t read_write;
		uint32_t address;
		uint32_t remaining = 0xFFFFF;
		btftp_file_t* file;
		uint8_t current_sector;

		static const uint32_t max_addr = 0x080FFFFF;
		static const uint32_t mem_size = 0xFFFFF;
	};

	enum Mode{
		READ = 0,
		WRITE = 1,
		NONE = 2
	};


public:


	static btftp_file_t* file;
	static inline volatile bool end_bootloader{false};

	static void start();

	static void on(BTFTP::Mode);

	static void off();


private:
	static volatile bool ready;

	static bool error_ok;

	static volatile BTFTP::Mode mode;

	static void* open(const char* fname, const char* mode, u8_t write);

	static void close(void* handle);

	static int read(void* handle, void* buf, int bytes);

	static int write(void* handle, struct pbuf* p);

	static void re(void* handle);
};

//#endif
