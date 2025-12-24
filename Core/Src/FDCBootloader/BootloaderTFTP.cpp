
#include "FDCBootloader/BootloaderTFTP.hpp"

// namespace BLCU {
// 	void finish_write_read_order(bool error_ok) {
//         // Dummy implementation for linker satisfaction
//     }
// }

bool BTFTP::error_ok = true;
volatile bool BTFTP::ready = false;
volatile int BTFTP::debug_state = 0; // 0=ok, 1=not ready/mode mismatch, 2=mode string mismatch

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
		// BLCU::finish_write_read_order(false);
		return nullptr;
	}

	const char* accepted_mode = "octet";
    
    // Manual string comparison to avoid libc dependencies and enable debugging
    bool match = true;
    const char* p1 = mode;
    const char* p2 = accepted_mode;
    
    while (*p1 && *p2) {
        if (*p1++ != *p2++) {
            match = false;
            break;
        }
    }
    // Check if both strings ended
    if (match && (*p1 || *p2)) {
        match = false;
    }

	if (!match) {
		// BLCU::finish_write_read_order(false);
        BTFTP::debug_state = 2;
		return nullptr;
	}


	// if (version != FDCB_CURRENT_VERSION) {
		// ErrorHandler("Mismatch in bootloader version, current version in host: 0x%X in target: 0x%X.", FDCB_CURRENT_VERSION, version);
		// BLCU::finish_write_read_order(false);
	// }

	uint32_t address = FLASH_SECTOR0_START_ADDRESS;
	// BTFTP::BHandle* handle = new BTFTP::BHandle(string(fname), string(mode), write, address);
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

    BTFTP::debug_state = 40; // Success
	return handle;
}

void BTFTP::close(void* handle){
	delete static_cast<BTFTP::BHandle*>(handle);
	// BLCU::finish_write_read_order(error_ok);
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
			// if (not FDCB::read_memory(btftp_handle->current_sector, btftp_handle->file->payload)) {
			// 	error_ok = false;
			// 	return -1;
			// }
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
			// if (not FDCB::write_memory(btftp_handle->current_sector, btftp_handle->file->payload,(btftp_handle->file->pointer))) {
			// 	error_ok = false;
			// 	return -1;
			// }
			btftp_handle->file->pointer = 0;
			btftp_handle->current_sector++;
		}
	}

	memcpy(&btftp_handle->file->payload[btftp_handle->file->pointer], (uint8_t*)p->payload, p->len);

	if (p->len < TFTP_MAX_DATA_SIZE) {
		uint32_t add_amount = 64-(p->len%64);
		btftp_handle->file->pointer += (p->len + add_amount);
		for(uint32_t i = p->len; i < add_amount; i++){
			btftp_handle->file->payload[btftp_handle->file->pointer] = 255;
			btftp_handle->file->pointer++;
		}
		// if (not FDCB::write_memory(btftp_handle->current_sector, btftp_handle->file->payload,btftp_handle->file->pointer)) {
		// 	error_ok = false;
		// 	return -1;
		// }
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


