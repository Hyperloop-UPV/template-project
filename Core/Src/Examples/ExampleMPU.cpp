#ifdef EXAMPLE_MPU

#include "main.h"
#include "ST-LIB.hpp"

using namespace ST_LIB;

inline void mpu_assert(bool condition) {
    if (!condition) {
        __BKPT(0);
        while (1) {
        }
    }
}

template <typename T> bool in_range(const T& var, const char& start, const char& end) {
    auto addr = reinterpret_cast<uintptr_t>(&var);
    return addr >= reinterpret_cast<uintptr_t>(&start) && addr < reinterpret_cast<uintptr_t>(&end);
}

template <typename T>
bool in_range_cached(const T& var, const char& nc_end, const char& base, const char& size) {
    auto addr = reinterpret_cast<uintptr_t>(&var);
    return addr >= reinterpret_cast<uintptr_t>(&nc_end) &&
           addr < reinterpret_cast<uintptr_t>(&base) + reinterpret_cast<uintptr_t>(&size);
}

template <typename T> bool in_range_from(const T& var, const char& base, const char& size) {
    auto addr = reinterpret_cast<uintptr_t>(&var);
    return addr >= reinterpret_cast<uintptr_t>(&base) &&
           addr < reinterpret_cast<uintptr_t>(&base) + reinterpret_cast<uintptr_t>(&size);
}

inline bool ptr_in_range(const volatile void* ptr, const char& start, const char& end) {
    auto addr = reinterpret_cast<uintptr_t>(ptr);
    return addr >= reinterpret_cast<uintptr_t>(&start) && addr < reinterpret_cast<uintptr_t>(&end);
}

inline bool ptr_in_range_cached(
    const volatile void* ptr,
    const char& nc_end,
    const char& base,
    const char& size
) {
    auto addr = reinterpret_cast<uintptr_t>(ptr);
    return addr >= reinterpret_cast<uintptr_t>(&nc_end) &&
           addr < reinterpret_cast<uintptr_t>(&base) + reinterpret_cast<uintptr_t>(&size);
}

inline bool ptr_in_range_from(const volatile void* ptr, const char& base, const char& size) {
    auto addr = reinterpret_cast<uintptr_t>(ptr);
    return addr >= reinterpret_cast<uintptr_t>(&base) &&
           addr < reinterpret_cast<uintptr_t>(&base) + reinterpret_cast<uintptr_t>(&size);
}

#ifdef TEST_0
// No Buffers requested
using myBoard = ST_LIB::Board<ST_LIB::DefaultFaultPolicy>;
extern "C" void BoardInit() { myBoard::init(); }
int main(void) {
    while (1) {
    }
}
#endif

#ifdef TEST_1
// Basic test with buffers in all domains
D1_NC_BSS uint32_t my_d1_nc_bss_1;
D1_NC_BSS uint8_t my_d1_nc_bss_2;
D1_C_BSS uint32_t my_d1_c_bss_1;
D1_C_BSS uint8_t my_d1_c_bss_2;
D2_NC_BSS uint32_t my_d2_nc_bss_1;
D2_NC_BSS uint8_t my_d2_nc_bss_2;
D2_C_BSS uint32_t my_d2_c_bss_1;
D2_C_BSS uint8_t my_d2_c_bss_2;
D3_NC_BSS uint32_t my_d3_nc_bss_1;
D3_NC_BSS uint8_t my_d3_nc_bss_2;
D3_C_BSS uint32_t my_d3_c_bss_1;
D3_C_BSS uint8_t my_d3_c_bss_2;

D1_NC_DATA uint32_t my_d1_nc_data_1{40};
D1_NC_DATA uint8_t my_d1_nc_data_2{41};
D1_C_DATA uint32_t my_d1_c_data_1{42};
D1_C_DATA uint8_t my_d1_c_data_2{43};
D2_NC_DATA uint32_t my_d2_nc_data_1{44};
D2_NC_DATA uint8_t my_d2_nc_data_2{45};
D2_C_DATA uint32_t my_d2_c_data_1{46};
D2_C_DATA uint8_t my_d2_c_data_2{47};
D3_NC_DATA uint32_t my_d3_nc_data_1{48};
D3_NC_DATA uint8_t my_d3_nc_data_2{49};
D3_C_DATA uint32_t my_d3_c_data_1{50};
D3_C_DATA uint8_t my_d3_c_data_2{51};

D1_NC_RODATA uint32_t my_d1_nc_rodata_1{100};
D1_NC_RODATA uint8_t my_d1_nc_rodata_2{101};
D1_C_RODATA uint32_t my_d1_c_rodata_1{102};
D1_C_RODATA uint8_t my_d1_c_rodata_2{103};
D2_NC_RODATA uint32_t my_d2_nc_rodata_1{104};
D2_NC_RODATA uint8_t my_d2_nc_rodata_2{105};
D2_C_RODATA uint32_t my_d2_c_rodata_1{106};
D2_C_RODATA uint8_t my_d2_c_rodata_2{107};
D3_NC_RODATA uint32_t my_d3_nc_rodata_1{108};
D3_NC_RODATA uint8_t my_d3_nc_rodata_2{109};
D3_C_RODATA uint32_t my_d3_c_rodata_1{110};
D3_C_RODATA uint8_t my_d3_c_rodata_2{111};

DTCM_RODATA uint32_t my_dtcm_rodata_1{112};
DTCM_RODATA uint8_t my_dtcm_rodata_2{113};

// INLINE variants
D1_NC_DATA_INLINE(my_inline_d1_nc_data_1) uint32_t my_inline_d1_nc_data_1{200};
D1_NC_BSS_INLINE(my_inline_d1_nc_bss_1) uint32_t my_inline_d1_nc_bss_1;
D1_NC_RODATA_INLINE(my_inline_d1_nc_rodata_1) uint32_t my_inline_d1_nc_rodata_1{201};
D1_C_DATA_INLINE(my_inline_d1_c_data_1) uint32_t my_inline_d1_c_data_1{202};
D1_C_BSS_INLINE(my_inline_d1_c_bss_1) uint32_t my_inline_d1_c_bss_1;
D1_C_RODATA_INLINE(my_inline_d1_c_rodata_1) uint32_t my_inline_d1_c_rodata_1{203};
D2_NC_DATA_INLINE(my_inline_d2_nc_data_1) uint32_t my_inline_d2_nc_data_1{204};
D2_NC_BSS_INLINE(my_inline_d2_nc_bss_1) uint32_t my_inline_d2_nc_bss_1;
D2_NC_RODATA_INLINE(my_inline_d2_nc_rodata_1) uint32_t my_inline_d2_nc_rodata_1{205};
D2_C_DATA_INLINE(my_inline_d2_c_data_1) uint32_t my_inline_d2_c_data_1{206};
D2_C_BSS_INLINE(my_inline_d2_c_bss_1) uint32_t my_inline_d2_c_bss_1;
D2_C_RODATA_INLINE(my_inline_d2_c_rodata_1) uint32_t my_inline_d2_c_rodata_1{207};
D3_NC_DATA_INLINE(my_inline_d3_nc_data_1) uint32_t my_inline_d3_nc_data_1{208};
D3_NC_BSS_INLINE(my_inline_d3_nc_bss_1) uint32_t my_inline_d3_nc_bss_1;
D3_NC_RODATA_INLINE(my_inline_d3_nc_rodata_1) uint32_t my_inline_d3_nc_rodata_1{209};
D3_C_DATA_INLINE(my_inline_d3_c_data_1) uint32_t my_inline_d3_c_data_1{210};
D3_C_BSS_INLINE(my_inline_d3_c_bss_1) uint32_t my_inline_d3_c_bss_1;
D3_C_RODATA_INLINE(my_inline_d3_c_rodata_1) uint32_t my_inline_d3_c_rodata_1{211};
DTCM_RODATA_INLINE(my_inline_dtcm_rodata_1) uint32_t my_inline_dtcm_rodata_1{212};

RAM_CODE void ram_code_func() { __NOP(); }
RAM_CODE_INLINE(my_inline_ram_code_func) void my_inline_ram_code_func() { __NOP(); }

using myBoard = ST_LIB::Board<ST_LIB::DefaultFaultPolicy>;
extern "C" void BoardInit() { myBoard::init(); }

const uint32_t my_rodata{301};

int main(void) {
    mpu_assert(in_range(my_d1_nc_bss_1, _ram_d1_nc_start, _ram_d1_nc_end));
    mpu_assert(in_range(my_d1_nc_bss_2, _ram_d1_nc_start, _ram_d1_nc_end));
    mpu_assert(in_range_cached(my_d1_c_bss_1, _ram_d1_nc_end, _ram_d1_base, _ram_d1_size));
    mpu_assert(in_range_cached(my_d1_c_bss_2, _ram_d1_nc_end, _ram_d1_base, _ram_d1_size));
    mpu_assert(in_range(my_d2_nc_bss_1, _ram_d2_nc_start, _ram_d2_nc_end));
    mpu_assert(in_range(my_d2_nc_bss_2, _ram_d2_nc_start, _ram_d2_nc_end));
    mpu_assert(in_range_cached(my_d2_c_bss_1, _ram_d2_nc_end, _ram_d2_base, _ram_d2_size));
    mpu_assert(in_range_cached(my_d2_c_bss_2, _ram_d2_nc_end, _ram_d2_base, _ram_d2_size));
    mpu_assert(in_range(my_d3_nc_bss_1, _ram_d3_nc_start, _ram_d3_nc_end));
    mpu_assert(in_range(my_d3_nc_bss_2, _ram_d3_nc_start, _ram_d3_nc_end));
    mpu_assert(in_range_cached(my_d3_c_bss_1, _ram_d3_nc_end, _ram_d3_base, _ram_d3_size));
    mpu_assert(in_range_cached(my_d3_c_bss_2, _ram_d3_nc_end, _ram_d3_base, _ram_d3_size));

    mpu_assert(in_range(my_d1_nc_data_1, _ram_d1_nc_start, _ram_d1_nc_end));
    mpu_assert(in_range(my_d1_nc_data_2, _ram_d1_nc_start, _ram_d1_nc_end));
    mpu_assert(in_range_cached(my_d1_c_data_1, _ram_d1_nc_end, _ram_d1_base, _ram_d1_size));
    mpu_assert(in_range_cached(my_d1_c_data_2, _ram_d1_nc_end, _ram_d1_base, _ram_d1_size));
    mpu_assert(in_range(my_d2_nc_data_1, _ram_d2_nc_start, _ram_d2_nc_end));
    mpu_assert(in_range(my_d2_nc_data_2, _ram_d2_nc_start, _ram_d2_nc_end));
    mpu_assert(in_range_cached(my_d2_c_data_1, _ram_d2_nc_end, _ram_d2_base, _ram_d2_size));
    mpu_assert(in_range_cached(my_d2_c_data_2, _ram_d2_nc_end, _ram_d2_base, _ram_d2_size));
    mpu_assert(in_range(my_d3_nc_data_1, _ram_d3_nc_start, _ram_d3_nc_end));
    mpu_assert(in_range(my_d3_nc_data_2, _ram_d3_nc_start, _ram_d3_nc_end));
    mpu_assert(in_range_cached(my_d3_c_data_1, _ram_d3_nc_end, _ram_d3_base, _ram_d3_size));
    mpu_assert(in_range_cached(my_d3_c_data_2, _ram_d3_nc_end, _ram_d3_base, _ram_d3_size));

    mpu_assert(in_range(my_d1_nc_rodata_1, _ram_d1_nc_start, _ram_d1_nc_end));
    mpu_assert(in_range(my_d1_nc_rodata_2, _ram_d1_nc_start, _ram_d1_nc_end));
    mpu_assert(in_range_cached(my_d1_c_rodata_1, _ram_d1_nc_end, _ram_d1_base, _ram_d1_size));
    mpu_assert(in_range_cached(my_d1_c_rodata_2, _ram_d1_nc_end, _ram_d1_base, _ram_d1_size));
    mpu_assert(in_range(my_d2_nc_rodata_1, _ram_d2_nc_start, _ram_d2_nc_end));
    mpu_assert(in_range(my_d2_nc_rodata_2, _ram_d2_nc_start, _ram_d2_nc_end));
    mpu_assert(in_range_cached(my_d2_c_rodata_1, _ram_d2_nc_end, _ram_d2_base, _ram_d2_size));
    mpu_assert(in_range_cached(my_d2_c_rodata_2, _ram_d2_nc_end, _ram_d2_base, _ram_d2_size));
    mpu_assert(in_range(my_d3_nc_rodata_1, _ram_d3_nc_start, _ram_d3_nc_end));
    mpu_assert(in_range(my_d3_nc_rodata_2, _ram_d3_nc_start, _ram_d3_nc_end));
    mpu_assert(in_range_cached(my_d3_c_rodata_1, _ram_d3_nc_end, _ram_d3_base, _ram_d3_size));
    mpu_assert(in_range_cached(my_d3_c_rodata_2, _ram_d3_nc_end, _ram_d3_base, _ram_d3_size));

    mpu_assert(in_range_from(my_dtcm_rodata_1, _dtcm_base, _dtcm_size));
    mpu_assert(in_range_from(my_dtcm_rodata_2, _dtcm_base, _dtcm_size));

    // INLINE variants
    mpu_assert(in_range(my_inline_d1_nc_data_1, _ram_d1_nc_start, _ram_d1_nc_end));
    mpu_assert(in_range(my_inline_d1_nc_bss_1, _ram_d1_nc_start, _ram_d1_nc_end));
    mpu_assert(in_range(my_inline_d1_nc_rodata_1, _ram_d1_nc_start, _ram_d1_nc_end));
    mpu_assert(in_range_cached(my_inline_d1_c_data_1, _ram_d1_nc_end, _ram_d1_base, _ram_d1_size));
    mpu_assert(in_range_cached(my_inline_d1_c_bss_1, _ram_d1_nc_end, _ram_d1_base, _ram_d1_size));
    mpu_assert(in_range_cached(my_inline_d1_c_rodata_1, _ram_d1_nc_end, _ram_d1_base, _ram_d1_size)
    );
    mpu_assert(in_range(my_inline_d2_nc_data_1, _ram_d2_nc_start, _ram_d2_nc_end));
    mpu_assert(in_range(my_inline_d2_nc_bss_1, _ram_d2_nc_start, _ram_d2_nc_end));
    mpu_assert(in_range(my_inline_d2_nc_rodata_1, _ram_d2_nc_start, _ram_d2_nc_end));
    mpu_assert(in_range_cached(my_inline_d2_c_data_1, _ram_d2_nc_end, _ram_d2_base, _ram_d2_size));
    mpu_assert(in_range_cached(my_inline_d2_c_bss_1, _ram_d2_nc_end, _ram_d2_base, _ram_d2_size));
    mpu_assert(in_range_cached(my_inline_d2_c_rodata_1, _ram_d2_nc_end, _ram_d2_base, _ram_d2_size)
    );
    mpu_assert(in_range(my_inline_d3_nc_data_1, _ram_d3_nc_start, _ram_d3_nc_end));
    mpu_assert(in_range(my_inline_d3_nc_bss_1, _ram_d3_nc_start, _ram_d3_nc_end));
    mpu_assert(in_range(my_inline_d3_nc_rodata_1, _ram_d3_nc_start, _ram_d3_nc_end));
    mpu_assert(in_range_cached(my_inline_d3_c_data_1, _ram_d3_nc_end, _ram_d3_base, _ram_d3_size));
    mpu_assert(in_range_cached(my_inline_d3_c_bss_1, _ram_d3_nc_end, _ram_d3_base, _ram_d3_size));
    mpu_assert(in_range_cached(my_inline_d3_c_rodata_1, _ram_d3_nc_end, _ram_d3_base, _ram_d3_size)
    );
    mpu_assert(in_range_from(my_inline_dtcm_rodata_1, _dtcm_base, _dtcm_size));

    // RAM_CODE functions in ITCM
    mpu_assert(
        reinterpret_cast<uintptr_t>(&ram_code_func) >= reinterpret_cast<uintptr_t>(&_itcm_base) &&
        reinterpret_cast<uintptr_t>(&ram_code_func) <
            reinterpret_cast<uintptr_t>(&_itcm_base) + reinterpret_cast<uintptr_t>(&_itcm_size)
    );
    mpu_assert(
        reinterpret_cast<uintptr_t>(&my_inline_ram_code_func) >=
            reinterpret_cast<uintptr_t>(&_itcm_base) &&
        reinterpret_cast<uintptr_t>(&my_inline_ram_code_func) <
            reinterpret_cast<uintptr_t>(&_itcm_base) + reinterpret_cast<uintptr_t>(&_itcm_size)
    );

    // .data in DTCM
    uint32_t my_data{300};
    mpu_assert(in_range_from(my_data, _dtcm_base, _dtcm_size));

    // .bss in DTCM
    static uint32_t my_bss;
    mpu_assert(in_range_from(my_bss, _dtcm_base, _dtcm_size));

    // stack in DTCM
    uint32_t my_stack;
    mpu_assert(in_range_from(my_stack, _dtcm_base, _dtcm_size));

    // .rodata in FLASH
    mpu_assert(in_range_from(my_rodata, _flash_base, _flash_size));

    while (1)
        ;
}
#endif

#ifdef TEST_2
// MPUDomain buffers in D1 and D3 non-cached
constexpr auto my_d1 = MPUDomain::Buffer<volatile uint32_t>(
    MPUDomain::MemoryType::NonCached,
    MPUDomain::MemoryDomain::D1
);
constexpr auto my_d3 = MPUDomain::Buffer<volatile uint32_t>(
    MPUDomain::MemoryType::NonCached,
    MPUDomain::MemoryDomain::D3
);

int main(void) {

    using myBoard = ST_LIB::Board<ST_LIB::DefaultFaultPolicy, my_d1, my_d3>;
    myBoard::init();

    auto* d1 = MPUDomain::as<myBoard, my_d1>();
    auto* d3 = MPUDomain::as<myBoard, my_d3>();

    mpu_assert(ptr_in_range(d1, _ram_d1_nc_start, _ram_d1_nc_end));
    mpu_assert(ptr_in_range(d3, _ram_d3_nc_start, _ram_d3_nc_end));

    while (1)
        ;
}
#endif

#ifdef TEST_3
// POD struct type buffer
struct MPUStruct {
    uint8_t a;
    float b;
    char c[10];
};
constexpr auto my_struct = MPUDomain::Buffer<volatile MPUStruct>();

int main(void) {

    using myBoard = ST_LIB::Board<ST_LIB::DefaultFaultPolicy, my_struct>;
    myBoard::init();

    auto* s = MPUDomain::as<myBoard, my_struct>();
    mpu_assert(ptr_in_range(s, _ram_d1_nc_start, _ram_d1_nc_end));

    while (1)
        ;
}
#endif

#ifdef TEST_4
// Fail test (too much memory requested)
constexpr auto my_buff = MPUDomain::Buffer<volatile uint32_t[100000]>(
    MPUDomain::MemoryType::NonCached,
    MPUDomain::MemoryDomain::D3
);

int main(void) {
    using myBoard = ST_LIB::Board<ST_LIB::DefaultFaultPolicy, my_buff>;
    myBoard::init();

    [[maybe_unused]] auto my_buffer = myBoard::instance_of<my_buff>().template as<my_buff>();

    while (1)
        ;
}
#endif

#ifdef TEST_5
// Cannot request any type of buffer other than the one defined
constexpr auto my_buff = MPUDomain::Buffer<volatile uint32_t>();

int main(void) {

    using myBoard = ST_LIB::Board<ST_LIB::DefaultFaultPolicy, my_buff>;
    myBoard::init();

    [[maybe_unused]] auto my_buffer = myBoard::instance_of<my_buff>().template as<uint32_t>(
    ); // Wrong type, should be as<my_buff>()

    while (1)
        ;
}
#endif

#ifdef TEST_6
// Non-cached and cached memory on the same domain
// TODO (doesn't work)
constexpr auto my_nc = MPUDomain::Buffer<volatile uint32_t[100]>();
constexpr auto my_c = MPUDomain::Buffer<uint32_t[200]>(MPUDomain::MemoryType::Cached);

int main(void) {
    using myBoard = ST_LIB::Board<ST_LIB::DefaultFaultPolicy, my_nc, my_c>;
    myBoard::init();

    auto* nc = MPUDomain::as<myBoard, my_nc>();
    auto* c = MPUDomain::as<myBoard, my_c>();

    mpu_assert(ptr_in_range(nc, _ram_d1_nc_start, _ram_d1_nc_end));
    mpu_assert(ptr_in_range_cached(c, _ram_d1_nc_end, _ram_d1_base, _ram_d1_size));

    while (1)
        ;
}
#endif

#ifdef TEST_7
// Different alignment buffers
constexpr auto my_8 = MPUDomain::Buffer<volatile uint8_t[100]>();
constexpr auto my_32 = MPUDomain::Buffer<volatile uint32_t[200]>();

int main(void) {
    using myBoard = ST_LIB::Board<ST_LIB::DefaultFaultPolicy, my_8, my_32>;
    myBoard::init();

    auto* a8 = MPUDomain::as<myBoard, my_8>();
    auto* a32 = MPUDomain::as<myBoard, my_32>();

    mpu_assert(ptr_in_range(a8, _ram_d1_nc_start, _ram_d1_nc_end));
    mpu_assert(ptr_in_range(a32, _ram_d1_nc_start, _ram_d1_nc_end));

    while (1)
        ;
}
#endif

#ifdef TEST_8
// Request a non-POD type fails
constexpr auto my_buff = MPUDomain::Buffer<std::vector<int>>();

int main(void) {
    using myBoard = ST_LIB::Board<ST_LIB::DefaultFaultPolicy, my_buff>;
    myBoard::init();

    [[maybe_unused]] auto my_buffer = myBoard::instance_of<my_buff>().template as<my_buff>();

    while (1)
        ;
}
#endif

#ifdef TEST_9
// Request too many buffers fails (you can overwrite this value with a define)
constexpr auto my_buff = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff2 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff3 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff4 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff5 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff6 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff7 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff8 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff9 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff10 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff11 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff12 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff13 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff14 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff15 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff16 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff17 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff18 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff19 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff20 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff21 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff22 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff23 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff24 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff25 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff26 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff27 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff28 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff29 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff30 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff31 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff32 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff33 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff34 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff35 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff36 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff37 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff38 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff39 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff40 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff41 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff42 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff43 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff44 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff45 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff46 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff47 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff48 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff49 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff50 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff51 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff52 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff53 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff54 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff55 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff56 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff57 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff58 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff59 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff60 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff61 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff62 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff63 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff64 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff65 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff66 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff67 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff68 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff69 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff70 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff71 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff72 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff73 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff74 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff75 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff76 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff77 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff78 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff79 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff80 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff81 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff82 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff83 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff84 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff85 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff86 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff87 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff88 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff89 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff90 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff91 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff92 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff93 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff94 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff95 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff96 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff97 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff98 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff99 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff100 = MPUDomain::Buffer<uint8_t>();
constexpr auto my_buff101 = MPUDomain::Buffer<uint8_t>();
int main(void) {

    using myBoard = ST_LIB::Board<
        ST_LIB::DefaultFaultPolicy,
        my_buff,
        my_buff2,
        my_buff3,
        my_buff4,
        my_buff5,
        my_buff6,
        my_buff7,
        my_buff8,
        my_buff9,
        my_buff10,
        my_buff11,
        my_buff12,
        my_buff13,
        my_buff14,
        my_buff15,
        my_buff16,
        my_buff17,
        my_buff18,
        my_buff19,
        my_buff20,
        my_buff21,
        my_buff22,
        my_buff23,
        my_buff24,
        my_buff25,
        my_buff26,
        my_buff27,
        my_buff28,
        my_buff29,
        my_buff30,
        my_buff31,
        my_buff32,
        my_buff33,
        my_buff34,
        my_buff35,
        my_buff36,
        my_buff37,
        my_buff38,
        my_buff39,
        my_buff40,
        my_buff41,
        my_buff42,
        my_buff43,
        my_buff44,
        my_buff45,
        my_buff46,
        my_buff47,
        my_buff48,
        my_buff49,
        my_buff50,
        my_buff51,
        my_buff52,
        my_buff53,
        my_buff54,
        my_buff55,
        my_buff56,
        my_buff57,
        my_buff58,
        my_buff59,
        my_buff60,
        my_buff61,
        my_buff62,
        my_buff63,
        my_buff64,
        my_buff65,
        my_buff66,
        my_buff67,
        my_buff68,
        my_buff69,
        my_buff70,
        my_buff71,
        my_buff72,
        my_buff73,
        my_buff74,
        my_buff75,
        my_buff76,
        my_buff77,
        my_buff78,
        my_buff79,
        my_buff80,
        my_buff81,
        my_buff82,
        my_buff83,
        my_buff84,
        my_buff85,
        my_buff86,
        my_buff87,
        my_buff88,
        my_buff89,
        my_buff90,
        my_buff91,
        my_buff92,
        my_buff93,
        my_buff94,
        my_buff95,
        my_buff96,
        my_buff97,
        my_buff98,
        my_buff99,
        my_buff100,
        my_buff101>;
    myBoard::init();

    [[maybe_unused]] auto my_buffer = myBoard::instance_of<my_buff>().template as<my_buff>();

    while (1)
        ;
}
#endif

#ifdef TEST_10
// Mix types of different alignments and sizes (stress test)
constexpr auto my_buff = MPUDomain::Buffer<volatile uint8_t[3]>();
constexpr auto my_buff2 = MPUDomain::Buffer<volatile uint16_t[5]>();
constexpr auto my_buff3 = MPUDomain::Buffer<volatile uint32_t>();
constexpr auto my_buff4 = MPUDomain::Buffer<volatile uint64_t[2]>();
constexpr auto my_buff5 = MPUDomain::Buffer<uint32_t>(MPUDomain::MemoryType::Cached);
constexpr auto my_buff6 = MPUDomain::Buffer<uint8_t[7]>(MPUDomain::MemoryType::Cached);
constexpr auto my_buff7 = MPUDomain::Buffer<uint16_t>(MPUDomain::MemoryType::Cached);
constexpr auto my_buff8 = MPUDomain::Buffer<uint32_t[3]>(
    MPUDomain::MemoryType::Cached,
    MPUDomain::MemoryDomain::D1,
    true
);
D1_NC uint32_t my_global_var;
D2_C uint32_t my_global_var2;
D3_NC uint32_t my_global_var3;
D3_NC uint8_t my_global_array[50];

int main(void) {

    using myBoard = ST_LIB::Board<
        ST_LIB::DefaultFaultPolicy,
        my_buff,
        my_buff2,
        my_buff3,
        my_buff4,
        my_buff5,
        my_buff6,
        my_buff7,
        my_buff8>;
    myBoard::init();

    auto* my_buffer = MPUDomain::as<myBoard, my_buff>();
    auto* my_buffer2 = MPUDomain::as<myBoard, my_buff2>();
    auto* my_buffer3 = MPUDomain::as<myBoard, my_buff3>();
    auto* my_buffer4 = MPUDomain::as<myBoard, my_buff4>();
    auto* my_buffer5 = MPUDomain::as<myBoard, my_buff5>();
    auto* my_buffer6 = MPUDomain::as<myBoard, my_buff6>();
    auto* my_buffer7 = MPUDomain::as<myBoard, my_buff7>();
    auto* my_buffer8 = MPUDomain::as<myBoard, my_buff8>();

    // Buffer placement checks
    mpu_assert(ptr_in_range(my_buffer, _ram_d1_nc_start, _ram_d1_nc_end));
    mpu_assert(ptr_in_range(my_buffer2, _ram_d1_nc_start, _ram_d1_nc_end));
    mpu_assert(ptr_in_range(my_buffer3, _ram_d1_nc_start, _ram_d1_nc_end));
    mpu_assert(ptr_in_range(my_buffer4, _ram_d1_nc_start, _ram_d1_nc_end));
    mpu_assert(ptr_in_range_cached(my_buffer5, _ram_d1_nc_end, _ram_d1_base, _ram_d1_size));
    mpu_assert(ptr_in_range_cached(my_buffer6, _ram_d1_nc_end, _ram_d1_base, _ram_d1_size));
    mpu_assert(ptr_in_range_cached(my_buffer7, _ram_d1_nc_end, _ram_d1_base, _ram_d1_size));
    mpu_assert(ptr_in_range_cached(my_buffer8, _ram_d1_nc_end, _ram_d1_base, _ram_d1_size));

    // Manual macro variable placement
    mpu_assert(in_range(my_global_var, _ram_d1_nc_start, _ram_d1_nc_end));
    mpu_assert(in_range_cached(my_global_var2, _ram_d2_nc_end, _ram_d2_base, _ram_d2_size));
    mpu_assert(in_range(my_global_var3, _ram_d3_nc_start, _ram_d3_nc_end));
    mpu_assert(in_range(my_global_array, _ram_d3_nc_start, _ram_d3_nc_end));

    while (1)
        ;
}
#endif

#ifdef TEST_11
// Dereference a pointer to a non-accessible memory region (should compile fine, hardfault at
// runtime)
int main(void) {
    using myBoard = ST_LIB::Board<ST_LIB::DefaultFaultPolicy>;
    myBoard::init();

    volatile uint32_t* invalid_ptr =
        reinterpret_cast<uint32_t*>(0x80000000); // Address outside of MPU regions

    [[maybe_unused]] uint32_t value = *invalid_ptr; // Dereference

    while (1)
        ;
}
#endif

#ifdef TEST_12
// Try construct method
struct MyStruct {
    uint8_t a;
    float b;
    char c[10];
    MyStruct(uint8_t aa, float bb) : a(aa), b(bb) {
        for (int i = 0; i < 10; ++i)
            c[i] = 'A' + i;
    }
};
constexpr auto my_buff = MPUDomain::Buffer<volatile MyStruct>();

int main(void) {

    using myBoard = ST_LIB::Board<ST_LIB::DefaultFaultPolicy, my_buff>;
    myBoard::init();

    auto& s = MPUDomain::construct<myBoard, my_buff>(42, 3.14f);
    mpu_assert(ptr_in_range(&s, _ram_d1_nc_start, _ram_d1_nc_end));

    while (1)
        ;
}
#endif

#ifdef TEST_13
// Test legacy MPUManager compatibility
D3_NC uint8_t my_legacy_buffer[256];
int main(void) {
    using myBoard = ST_LIB::Board<ST_LIB::DefaultFaultPolicy>;
    myBoard::init();

    auto* legacy = MPUManager::allocate_non_cached_memory(256);
    mpu_assert(ptr_in_range(legacy, _ram_d3_nc_start, _ram_d3_nc_end));
    mpu_assert(in_range(my_legacy_buffer, _ram_d3_nc_start, _ram_d3_nc_end));

    while (1)
        ;
}
#endif

#ifdef TEST_14
// Dereference a nullptr for read/write (should compile fine, hardfault at runtime)
int main(void) {
    using myBoard = ST_LIB::Board<ST_LIB::DefaultFaultPolicy>;
    myBoard::init();
    volatile uint32_t* invalid_ptr = nullptr; // Null pointer

    [[maybe_unused]] uint32_t value = *invalid_ptr; // Dereference

    while (1)
        ;
}
#endif

#endif
