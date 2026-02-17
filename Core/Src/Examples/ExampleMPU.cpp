#ifdef EXAMPLE_MPU

#include "main.h"
#include "ST-LIB.hpp"

#ifdef TEST_0
// No Buffers requested
int main(void) {
    STLIB::start();

    using myBoard = ST_LIB::Board<>;
    myBoard::init();

    while (1) {
        STLIB::update();
    }
}
#endif

#ifdef TEST_1
// Basic test with a buffer in D2
constexpr auto my_uint32_t = MPUDomain::Buffer<uint32_t>();

int main(void) {
    STLIB::start();

    using myBoard = ST_LIB::Board<my_uint32_t>;
    myBoard::init();

    [[maybe_unused]] auto my_buffer =
        myBoard::instance_of<my_uint32_t>().template as<my_uint32_t>();

    while (1) {
        STLIB::update();
    }
}
#endif

#ifdef TEST_2
// Basic test with a buffer in D1
constexpr auto my_uint32_t =
    MPUDomain::Buffer<uint32_t>(MPUDomain::MemoryType::NonCached, MPUDomain::MemoryDomain::D1);

int main(void) {
    STLIB::start();

    using myBoard = ST_LIB::Board<my_uint32_t>;
    myBoard::init();

    [[maybe_unused]] auto my_buffer = MPUDomain::as<myBoard, my_uint32_t>();

    while (1) {
        STLIB::update();
    }
}
#endif

#ifdef TEST_3
// Basic test with a buffer in D3
constexpr auto my_buff =
    MPUDomain::Buffer<uint32_t>(MPUDomain::MemoryType::NonCached, MPUDomain::MemoryDomain::D3);

int main(void) {
    STLIB::start();

    using myBoard = ST_LIB::Board<my_buff>;
    myBoard::init();
    [[maybe_unused]] auto my_buffer = myBoard::instance_of<my_buff>().template as<my_buff>();

    while (1) {
        STLIB::update();
    }
}
#endif

#ifdef TEST_4
// Fail test (too much memory requested)
constexpr auto my_buff = MPUDomain::Buffer<uint32_t[100000]>(
    MPUDomain::MemoryType::NonCached,
    MPUDomain::MemoryDomain::D3
);

int main(void) {
    using myBoard = ST_LIB::Board<my_buff>;
    myBoard::init();

    [[maybe_unused]] auto my_buffer = myBoard::instance_of<my_buff>().template as<my_buff>();

    STLIB::start();

    while (1) {
        STLIB::update();
    }
}
#endif

#ifdef TEST_5
// Cannot request any type of buffer other than the one defined
constexpr auto my_buff = MPUDomain::Buffer<uint32_t>();

int main(void) {
    STLIB::start();

    using myBoard = ST_LIB::Board<my_buff>;
    myBoard::init();

    [[maybe_unused]] auto my_buffer = myBoard::instance_of<my_buff>().template as<uint32_t>();

    while (1) {
        STLIB::update();
    }
}
#endif

#ifdef TEST_6
// Ask for non-cached and cached memory on the same domain
constexpr auto my_buff = MPUDomain::Buffer<uint32_t[100]>();
constexpr auto my_buff2 = MPUDomain::Buffer<uint32_t[200]>(MPUDomain::MemoryType::Cached);

int main(void) {
    STLIB::start();

    using myBoard = ST_LIB::Board<my_buff, my_buff2>;
    myBoard::init();

    [[maybe_unused]] auto my_buffer = myBoard::instance_of<my_buff>().template as<my_buff>();
    [[maybe_unused]] auto my_buffer2 = myBoard::instance_of<my_buff2>().template as<my_buff2>();

    while (1) {
        STLIB::update();
    }
}
#endif

#ifdef TEST_7
// Ask for different alignment buffers
constexpr auto my_buff = MPUDomain::Buffer<uint8_t[100]>();
constexpr auto my_buff2 = MPUDomain::Buffer<uint32_t[200]>();

int main(void) {
    STLIB::start();

    using myBoard = ST_LIB::Board<my_buff, my_buff2>;
    myBoard::init();
    [[maybe_unused]] auto my_buffer = myBoard::instance_of<my_buff>().template as<my_buff>();
    [[maybe_unused]] auto my_buffer2 = myBoard::instance_of<my_buff2>().template as<my_buff2>();

    while (1) {
        STLIB::update();
    }
}
#endif

#ifdef TEST_8
// Request a non-POD type fails
constexpr auto my_buff = MPUDomain::Buffer<std::vector<int>>();

int main(void) {
    STLIB::start();

    using myBoard = ST_LIB::Board<my_buff>;
    myBoard::init();

    [[maybe_unused]] auto my_buffer = myBoard::instance_of<my_buff>().template as<my_buff>();

    while (1) {
        STLIB::update();
    }
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
    STLIB::start();

    using myBoard = ST_LIB::Board<
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

    while (1) {
        STLIB::update();
    }
}
#endif

#ifdef TEST_10
// Request a struct type (also works with objects and such, as long as they are POD)
struct MyStruct {
    uint8_t a;
    float b;
    char c[10];
};
constexpr auto my_buff = MPUDomain::Buffer<MyStruct>();

int main(void) {
    STLIB::start();

    using myBoard = ST_LIB::Board<my_buff>;
    myBoard::init();

    [[maybe_unused]] auto my_buffer = myBoard::instance_of<my_buff>().template as<my_buff>();

    while (1) {
        STLIB::update();
    }
}
#endif

#ifdef TEST_11
// Mix types of different alignments and sizes (stress test)
constexpr auto my_buff = MPUDomain::Buffer<uint8_t[3]>();
constexpr auto my_buff2 = MPUDomain::Buffer<uint16_t[5]>();
constexpr auto my_buff3 = MPUDomain::Buffer<uint32_t>();
constexpr auto my_buff4 = MPUDomain::Buffer<uint64_t[2]>();
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
    STLIB::start();

    using myBoard = ST_LIB::
        Board<my_buff, my_buff2, my_buff3, my_buff4, my_buff5, my_buff6, my_buff7, my_buff8>;
    myBoard::init();

    [[maybe_unused]] auto my_buffer = myBoard::instance_of<my_buff>().template as<my_buff>();
    [[maybe_unused]] auto my_buffer2 = myBoard::instance_of<my_buff2>().template as<my_buff2>();
    [[maybe_unused]] auto my_buffer3 = myBoard::instance_of<my_buff3>().template as<my_buff3>();
    [[maybe_unused]] auto my_buffer4 = myBoard::instance_of<my_buff4>().template as<my_buff4>();
    [[maybe_unused]] auto my_buffer5 = myBoard::instance_of<my_buff5>().template as<my_buff5>();
    [[maybe_unused]] auto my_buffer6 = myBoard::instance_of<my_buff6>().template as<my_buff6>();
    [[maybe_unused]] auto my_buffer7 = myBoard::instance_of<my_buff7>().template as<my_buff7>();
    [[maybe_unused]] auto my_buffer8 = myBoard::instance_of<my_buff8>().template as<my_buff8>();
    [[maybe_unused]] auto* global_var1 = &my_global_var;
    [[maybe_unused]] auto* global_var2 = &my_global_var2;
    [[maybe_unused]] auto* global_var3 = &my_global_var3;
    [[maybe_unused]] auto* global_array = &my_global_array;

    while (1) {
        STLIB::update();
    }
}
#endif

#ifdef TEST_12
// Dereference a pointer to a non-accessible memory region (should compile fine, runtime error)
int main(void) {
    STLIB::start();

    using myBoard = ST_LIB::Board<>;
    myBoard::init();

    volatile uint32_t* invalid_ptr =
        reinterpret_cast<uint32_t*>(0x80000000); // Address outside of MPU regions

    [[maybe_unused]] uint32_t value = *invalid_ptr; // Dereference

    while (1) {
        STLIB::update();
    }
}
#endif

#ifdef TEST_13
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
constexpr auto my_buff = MPUDomain::Buffer<MyStruct>();

int main(void) {
    STLIB::start();

    using myBoard = ST_LIB::Board<my_buff>;
    myBoard::init();
    [[maybe_unused]] auto my_buffer =
        myBoard::instance_of<my_buff>().template construct<my_buff>(42, 3.14f);

    while (1) {
        STLIB::update();
    }
}
#endif

#ifdef TEST_14
// Test legacy MPUManager compatibility
D3_NC uint8_t my_legacy_buffer[256];
int main(void) {
    STLIB::start();

    using myBoard = ST_LIB::Board<>;
    myBoard::init();

    [[maybe_unused]] auto my_buff = MPUManager::allocate_non_cached_memory(256);
    [[maybe_unused]] auto legacy_buffer_ptr = my_legacy_buffer;

    while (1) {
        STLIB::update();
    }
}
#endif

#ifdef TEST_15
// Dereference a nullptr for read/write (should compile fine, runtime error)
int main(void) {
    STLIB::start();

    using myBoard = ST_LIB::Board<>;
    myBoard::init();
    volatile uint32_t* invalid_ptr = nullptr; // Null pointer

    [[maybe_unused]] uint32_t value = *invalid_ptr; // Dereference

    while (1) {
        STLIB::update();
    }
}
#endif

void Error_Handler(void) {
    ErrorHandler("HAL error handler triggered");
    while (1) {
    }
}

extern "C" {
void assert_failed(uint8_t* file, uint32_t line) {
    while (1) {
    }
}
}

#endif
