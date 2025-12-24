
#include "FDCBootloader/BootloaderTFTP.hpp"



bool BTFTP::error_ok = true;
volatile bool BTFTP::ready = false;

volatile BTFTP::Mode BTFTP::mode = BTFTP::Mode::NONE;

BTFTP::btftp_file_t* BTFTP::file = nullptr;

//Public:
void BTFTP::on(BTFTP::Mode mode){
	BTFTP::ready = true;
	BTFTP::mode = mode;
}

void BTFTP::off(){
	BTFTP::ready = false;
	BTFTP::mode = BTFTP::Mode::NONE;
}

void BTFTP::start(){
	const tftp_context* context = new tftp_context{&BTFTP::open, &BTFTP::close, &BTFTP::read, &BTFTP::write};
	err_t error = tftp_init(context);

	if (error != ERR_OK) {
		ErrorHandler("Unable to start TFTP server, error code: %lu.", error);
		
		return;
	}
	BTFTP::file = new BTFTP::btftp_file_t();
	BTFTP::file->payload = (uint8_t*)malloc(SECTOR_SIZE_IN_BYTES);
	if (BTFTP::file->payload == nullptr) {
		ErrorHandler("BLCU could not allocate enough memory for tftp file buffer");
		return;
	}
	BTFTP::on(BTFTP::Mode::WRITE);

}

//Private:
void* BTFTP::open(const char* fname, const char* mode, u8_t write){
    
    volatile bool is_ready = BTFTP::ready;
    volatile uint8_t current_mode = (uint8_t)BTFTP::mode;
    volatile uint8_t write_req = write;

	if (not is_ready || write_req != current_mode) {
		return nullptr;
	}

	const char* accepted_mode = "octet";
    
    bool match = true;
    const char* p1 = mode;
    const char* p2 = accepted_mode;
    
    while (*p1 && *p2) {
        if (*p1++ != *p2++) {
            match = false;
            break;
        }
    }
    if (match && (*p1 || *p2)) {
        match = false;
    }

	if (!match) {
		return nullptr;
	}



	uint32_t address = FLASH_SECTOR0_START_ADDRESS;
    BTFTP::BHandle* handle = new BTFTP::BHandle();
    handle->name = fname;
    handle->mode = mode;
    handle->read_write = write;
    handle->address = address;

	handle->file = BTFTP::file;
	handle->current_sector = 0;
	BTFTP::file->max_pointer = SECTOR_SIZE_IN_BYTES - 1;

	if (handle->read_write == 1) {
		handle->file->pointer = 0;
	}else{
		handle->file->pointer = handle->file->max_pointer;
	}

	return handle;
}

void BTFTP::close(void* handle){
	delete static_cast<BTFTP::BHandle*>(handle);
	end_bootloader = true;
}

int BTFTP::read(void* handle, void* buf, int bytes){
	BTFTP::BHandle* btftp_handle = (BTFTP::BHandle*)handle;
	if (btftp_handle->read_write == 1) {
		error_ok = false;
		return -1;
	}

	if (btftp_handle->file->pointer >= btftp_handle->file->max_pointer) {

		if (btftp_handle->current_sector > 5) {
			return 0;
		}else{
			
			btftp_handle->file->pointer = 0;
			btftp_handle->current_sector++;
		}
	}

	memcpy((uint8_t*)buf, &btftp_handle->file->payload[btftp_handle->file->pointer], 512);
	btftp_handle->file->pointer += TFTP_MAX_DATA_SIZE;

	return 512;
}

int BTFTP::write(void* handle, struct pbuf* p){
	BTFTP::BHandle* btftp_handle = (BTFTP::BHandle*)handle;
	if (btftp_handle->read_write == 0) {
		error_ok = false;
		return -1;
	}

	if (btftp_handle->file->pointer >= btftp_handle->file->max_pointer) {
		if (btftp_handle->current_sector > 5) {
			return 1;
		}else{
			uint32_t dest_addr = flash_get_sector_starting_address((sector_t)btftp_handle->current_sector);
			if (flash_write(dest_addr, (uint32_t*)btftp_handle->file->payload, SECTOR_SIZE_IN_32BITS_WORDS) != FLASH_OK) {
				error_ok = false;
				return -1;
			}
			btftp_handle->file->pointer = 0;
			btftp_handle->current_sector++;
		}
	}

	memcpy(&btftp_handle->file->payload[btftp_handle->file->pointer], (uint8_t*)p->payload, p->len);

	if (p->len < TFTP_MAX_DATA_SIZE) {
		btftp_handle->file->pointer += p->len;
		
		uint32_t remainder = btftp_handle->file->pointer % 32;
		if (remainder != 0) {
			uint32_t padding = 32 - remainder;
			memset(&btftp_handle->file->payload[btftp_handle->file->pointer], 0xFF, padding);
			btftp_handle->file->pointer += padding;
		}

		uint32_t dest_addr = flash_get_sector_starting_address((sector_t)btftp_handle->current_sector);
		if (flash_write(dest_addr, (uint32_t*)btftp_handle->file->payload, btftp_handle->file->pointer / 4) != FLASH_OK) {
			error_ok = false;
			return -1;
		}
		return 0;
	}else{
		btftp_handle->file->pointer += TFTP_MAX_DATA_SIZE;
	}

	return 1;
}

void BTFTP::re(void* handle){
	BTFTP::BHandle* btftp_handle = (BTFTP::BHandle*)handle;
	btftp_handle->file->pointer -= TFTP_MAX_DATA_SIZE;
}


