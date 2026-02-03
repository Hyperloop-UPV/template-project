#pragma once
#include "ST-LIB.hpp"
#include "memory"
enum class GeneralStateMachineStatus: uint8_t {
    CONNECTING,
    OPERATIONAL,
    FAULT
};

enum class OperationalStateMachineStatus: uint8_t {
    HV_OPEN,
    PRECHARGE,
    HV_CLOSED,
    CHARGING
};

enum class ImdStatus: uint8_t{
    SHORTCIRCUIT,
    NORMAL,
    UNDERVOLTAGE,
    FAST_EVAL,
    EQUIPMENT_FAULT,
    GROUNDING_FAULT
};

enum class SdcStatus:uint8_t {
    ENGAGED,
    DISENGAGED
};

enum class BmsStatus:uint8_t {
    OK,
    FAULT
};
namespace var {

inline float battery1_SOC = 77.3f;
inline float battery1_cell1 = 3.7f;
inline float battery1_cell2 = 3.8f;
inline float battery1_cell3 = 3.75f;
inline float battery1_cell4 = 3.72f;
inline float battery1_cell5 = 3.78f;
inline float battery1_cell6 = 3.76f;
inline float battery1_temperature1 = 32.1f;
inline float battery1_temperature2 = 31.9f;
inline float battery1_total_voltage = 22.5f;
inline float battery1_conv_rate = 0.95f;

inline float battery2_SOC = 79.1f;
inline float battery2_cell1 = 3.71f;
inline float battery2_cell2 = 3.82f;
inline float battery2_cell3 = 3.76f;
inline float battery2_cell4 = 3.73f;
inline float battery2_cell5 = 3.77f;
inline float battery2_cell6 = 3.75f;
inline float battery2_temperature1 = 32.3f;
inline float battery2_temperature2 = 31.8f;
inline float battery2_total_voltage = 22.6f;
inline float battery2_conv_rate = 0.96f;

inline float battery3_SOC = 78.0f;
inline float battery3_cell1 = 3.69f;
inline float battery3_cell2 = 3.81f;
inline float battery3_cell3 = 3.74f;
inline float battery3_cell4 = 3.72f;
inline float battery3_cell5 = 3.76f;
inline float battery3_cell6 = 3.77f;
inline float battery3_temperature1 = 32.0f;
inline float battery3_temperature2 = 31.7f;
inline float battery3_total_voltage = 22.4f;
inline float battery3_conv_rate = 0.94f;

inline float battery4_SOC = 76.5f;
inline float battery4_cell1 = 3.68f;
inline float battery4_cell2 = 3.79f;
inline float battery4_cell3 = 3.73f;
inline float battery4_cell4 = 3.71f;
inline float battery4_cell5 = 3.75f;
inline float battery4_cell6 = 3.74f;
inline float battery4_temperature1 = 31.9f;
inline float battery4_temperature2 = 31.6f;
inline float battery4_total_voltage = 22.3f;
inline float battery4_conv_rate = 0.93f;

inline float battery5_SOC = 80.2f;
inline float battery5_cell1 = 3.72f;
inline float battery5_cell2 = 3.83f;
inline float battery5_cell3 = 3.77f;
inline float battery5_cell4 = 3.74f;
inline float battery5_cell5 = 3.78f;
inline float battery5_cell6 = 3.76f;
inline float battery5_temperature1 = 32.4f;
inline float battery5_temperature2 = 32.0f;
inline float battery5_total_voltage = 22.7f;
inline float battery5_conv_rate = 0.97f;

inline float battery6_SOC = 75.8f;
inline float battery6_cell1 = 3.67f;
inline float battery6_cell2 = 3.78f;
inline float battery6_cell3 = 3.72f;
inline float battery6_cell4 = 3.70f;
inline float battery6_cell5 = 3.74f;
inline float battery6_cell6 = 3.73f;
inline float battery6_temperature1 = 31.8f;
inline float battery6_temperature2 = 31.5f;
inline float battery6_total_voltage = 22.2f;
inline float battery6_conv_rate = 0.92f;

inline float battery7_SOC = 79.5f;
inline float battery7_cell1 = 3.71f;
inline float battery7_cell2 = 3.82f;
inline float battery7_cell3 = 3.76f;
inline float battery7_cell4 = 3.73f;
inline float battery7_cell5 = 3.77f;
inline float battery7_cell6 = 3.75f;
inline float battery7_temperature1 = 32.2f;
inline float battery7_temperature2 = 31.9f;
inline float battery7_total_voltage = 22.6f;
inline float battery7_conv_rate = 0.95f;

inline float battery8_SOC = 78.8f;
inline float battery8_cell1 = 3.70f;
inline float battery8_cell2 = 3.81f;
inline float battery8_cell3 = 3.75f;
inline float battery8_cell4 = 3.72f;
inline float battery8_cell5 = 3.76f;
inline float battery8_cell6 = 3.74f;
inline float battery8_temperature1 = 32.1f;
inline float battery8_temperature2 = 31.8f;
inline float battery8_total_voltage = 22.5f;
inline float battery8_conv_rate = 0.94f;

inline float battery9_SOC = 77.9f;
inline float battery9_cell1 = 3.69f;
inline float battery9_cell2 = 3.80f;
inline float battery9_cell3 = 3.74f;
inline float battery9_cell4 = 3.71f;
inline float battery9_cell5 = 3.75f;
inline float battery9_cell6 = 3.73f;
inline float battery9_temperature1 = 32.0f;
inline float battery9_temperature2 = 31.7f;
inline float battery9_total_voltage = 22.4f;
inline float battery9_conv_rate = 0.93f;

inline float battery10_SOC = 80.0f;
inline float battery10_cell1 = 3.72f;
inline float battery10_cell2 = 3.83f;
inline float battery10_cell3 = 3.77f;
inline float battery10_cell4 = 3.74f;
inline float battery10_cell5 = 3.78f;
inline float battery10_cell6 = 3.76f;
inline float battery10_temperature1 = 32.4f;
inline float battery10_temperature2 = 32.0f;
inline float battery10_total_voltage = 22.7f;
inline float battery10_conv_rate = 0.97f;

inline float battery11_SOC = 76.8f;
inline float battery11_cell1 = 3.68f;
inline float battery11_cell2 = 3.79f;
inline float battery11_cell3 = 3.73f;
inline float battery11_cell4 = 3.71f;
inline float battery11_cell5 = 3.75f;
inline float battery11_cell6 = 3.74f;
inline float battery11_temperature1 = 31.9f;
inline float battery11_temperature2 = 31.6f;
inline float battery11_total_voltage = 22.3f;
inline float battery11_conv_rate = 0.93f;

inline float battery12_SOC = 79.2f;
inline float battery12_cell1 = 3.71f;
inline float battery12_cell2 = 3.82f;
inline float battery12_cell3 = 3.76f;
inline float battery12_cell4 = 3.73f;
inline float battery12_cell5 = 3.77f;
inline float battery12_cell6 = 3.75f;
inline float battery12_temperature1 = 32.3f;
inline float battery12_temperature2 = 31.9f;
inline float battery12_total_voltage = 22.6f;
inline float battery12_conv_rate = 0.96f;

inline float battery13_SOC = 78.1f;
inline float battery13_cell1 = 3.69f;
inline float battery13_cell2 = 3.81f;
inline float battery13_cell3 = 3.74f;
inline float battery13_cell4 = 3.72f;
inline float battery13_cell5 = 3.76f;
inline float battery13_cell6 = 3.77f;
inline float battery13_temperature1 = 32.0f;
inline float battery13_temperature2 = 31.7f;
inline float battery13_total_voltage = 22.4f;
inline float battery13_conv_rate = 0.94f;

inline float battery14_SOC = 77.5f;
inline float battery14_cell1 = 3.70f;
inline float battery14_cell2 = 3.82f;
inline float battery14_cell3 = 3.75f;
inline float battery14_cell4 = 3.73f;
inline float battery14_cell5 = 3.76f;
inline float battery14_cell6 = 3.74f;
inline float battery14_temperature1 = 32.1f;
inline float battery14_temperature2 = 31.8f;
inline float battery14_total_voltage = 22.5f;
inline float battery14_conv_rate = 0.95f;

inline float battery15_SOC = 79.0f;
inline float battery15_cell1 = 3.71f;
inline float battery15_cell2 = 3.82f;
inline float battery15_cell3 = 3.76f;
inline float battery15_cell4 = 3.73f;
inline float battery15_cell5 = 3.77f;
inline float battery15_cell6 = 3.75f;
inline float battery15_temperature1 = 32.2f;
inline float battery15_temperature2 = 31.9f;
inline float battery15_total_voltage = 22.6f;
inline float battery15_conv_rate = 0.95f;

inline float battery16_SOC = 78.5f;
inline float battery16_cell1 = 3.70f;
inline float battery16_cell2 = 3.81f;
inline float battery16_cell3 = 3.75f;
inline float battery16_cell4 = 3.73f;
inline float battery16_cell5 = 3.76f;
inline float battery16_cell6 = 3.74f;
inline float battery16_temperature1 = 32.1f;
inline float battery16_temperature2 = 31.8f;
inline float battery16_total_voltage = 22.5f;
inline float battery16_conv_rate = 0.94f;

inline float battery17_SOC = 77.9f;
inline float battery17_cell1 = 3.69f;
inline float battery17_cell2 = 3.80f;
inline float battery17_cell3 = 3.74f;
inline float battery17_cell4 = 3.71f;
inline float battery17_cell5 = 3.75f;
inline float battery17_cell6 = 3.73f;
inline float battery17_temperature1 = 32.0f;
inline float battery17_temperature2 = 31.7f;
inline float battery17_total_voltage = 22.4f;
inline float battery17_conv_rate = 0.93f;

inline float battery18_SOC = 80.0f;
inline float battery18_cell1 = 3.72f;
inline float battery18_cell2 = 3.83f;
inline float battery18_cell3 = 3.77f;
inline float battery18_cell4 = 3.74f;
inline float battery18_cell5 = 3.78f;
inline float battery18_cell6 = 3.76f;
inline float battery18_temperature1 = 32.4f;
inline float battery18_temperature2 = 32.0f;
inline float battery18_total_voltage = 22.7f;
inline float battery18_conv_rate = 0.97f;

inline float batteries_voltage_reading = 400.0f;
inline float voltage_reading = 12.3f;
inline float current_reading = 5.6f;

inline GeneralStateMachineStatus general_state_machine_status = GeneralStateMachineStatus::CONNECTING;
inline OperationalStateMachineStatus operational_state_machine_status = OperationalStateMachineStatus::HV_OPEN;

inline int32_t driver_reading_period = 100;

inline ImdStatus imd_status = ImdStatus::NORMAL;
inline float imd_resistance = 120.0f;
inline bool imd_is_ok = true;

inline SdcStatus sdc_status = SdcStatus::ENGAGED;

inline float minimum_soc = 20.0f;

inline BmsStatus bms_status = BmsStatus::OK;

inline float voltage_min = 3.5f;
inline float voltage_max = 4.2f;
inline float temp_min = 25.0f;
inline float temp_max = 35.0f;

} // namespace var
namespace Communication_Data
{
    constexpr int battery_1 = 910;
    constexpr int battery_2 = 911;
    constexpr int battery_3 = 912;
    constexpr int battery_4 = 913;
    constexpr int battery_5 = 914;
    constexpr int battery_6 = 915;
    constexpr int battery_7 = 916;
    constexpr int battery_8 = 917;
    constexpr int battery_9 = 918;
    constexpr int battery_10 = 919;
    constexpr int battery_11 = 920;
    constexpr int battery_12 = 921;
    constexpr int battery_13 = 922;
    constexpr int battery_14 = 923;
    constexpr int battery_15 = 924;
    constexpr int battery_16 = 925;
    constexpr int battery_17 = 926;
    constexpr int battery_18 = 927;
    constexpr int batteries_voltage = 928;
    constexpr int voltage_sensor = 930;
    constexpr int current_sensor = 931;
    constexpr int general_state_machine = 940;
    constexpr int operational_state_machine = 941;
    constexpr int driver_diagnosis = 942;
    constexpr int imd = 943;
    constexpr int sdc = 944;
    constexpr int minimum_soc = 945;
    constexpr int bms = 946;
    constexpr int batteries_data = 947;
}
namespace Packets {
    // Declaración de punteros globales
    inline HeapPacket* battery_1;
    inline HeapPacket* battery_2;
    inline HeapPacket* battery_3;
    inline HeapPacket* battery_4;
    inline HeapPacket* battery_5;
    inline HeapPacket* battery_6;
    inline HeapPacket* battery_7;
    inline HeapPacket* battery_8;
    inline HeapPacket* battery_9;
    inline HeapPacket* battery_10;
    inline HeapPacket* battery_11;
    inline HeapPacket* battery_12;
    inline HeapPacket* battery_13;
    inline HeapPacket* battery_14;
    inline HeapPacket* battery_15;
    inline HeapPacket* battery_16;
    inline HeapPacket* battery_17;
    inline HeapPacket* battery_18;
    inline HeapPacket* batteries_voltage;
    inline HeapPacket* voltage_sensor;
    inline HeapPacket* current_sensor;
    inline HeapPacket* general_state_machine;
    inline HeapPacket* operational_state_machine;
    inline HeapPacket* driver_diagnosis;
    inline HeapPacket* imd;
    inline HeapPacket* sdc;
    inline HeapPacket* minimum_soc;
    inline HeapPacket* bms;
    inline HeapPacket* batteries_data;

    // Array para acceder fácilmente
    inline std::array<HeapPacket*, 29> all{};
}



constexpr size_t NUM_ORDERS = 29;
std::array<std::unique_ptr<HeapOrder>, NUM_ORDERS> create_all_orders()
{
    return {
        std::make_unique<HeapOrder>(950,
            &var::battery1_SOC,&var::battery1_cell1,&var::battery1_cell2,&var::battery1_cell3,
            &var::battery1_cell4,&var::battery1_cell5,&var::battery1_cell6,
            &var::battery1_temperature1,&var::battery1_temperature2,
            &var::battery1_total_voltage,&var::battery1_conv_rate),

        std::make_unique<HeapOrder>(951,
            &var::battery2_SOC,&var::battery2_cell1,&var::battery2_cell2,&var::battery2_cell3,
            &var::battery2_cell4,&var::battery2_cell5,&var::battery2_cell6,
            &var::battery2_temperature1,&var::battery2_temperature2,
            &var::battery2_total_voltage,&var::battery2_conv_rate),

        std::make_unique<HeapOrder>(952,
            &var::battery3_SOC,&var::battery3_cell1,&var::battery3_cell2,&var::battery3_cell3,
            &var::battery3_cell4,&var::battery3_cell5,&var::battery3_cell6,
            &var::battery3_temperature1,&var::battery3_temperature2,
            &var::battery3_total_voltage,&var::battery3_conv_rate),

        std::make_unique<HeapOrder>(953, 
            &var::battery4_SOC,&var::battery4_cell1,&var::battery4_cell2,&var::battery4_cell3,
            &var::battery4_cell4,&var::battery4_cell5,&var::battery4_cell6,
            &var::battery4_temperature1,&var::battery4_temperature2,
            &var::battery4_total_voltage,&var::battery4_conv_rate),

        std::make_unique<HeapOrder>(954, 
            &var::battery5_SOC,&var::battery5_cell1,&var::battery5_cell2,&var::battery5_cell3,
            &var::battery5_cell4,&var::battery5_cell5,&var::battery5_cell6,
            &var::battery5_temperature1,&var::battery5_temperature2,
            &var::battery5_total_voltage,&var::battery5_conv_rate),

        std::make_unique<HeapOrder>(955, 
            &var::battery6_SOC,&var::battery6_cell1,&var::battery6_cell2,&var::battery6_cell3,
            &var::battery6_cell4,&var::battery6_cell5,&var::battery6_cell6,
            &var::battery6_temperature1,&var::battery6_temperature2,
            &var::battery6_total_voltage,&var::battery6_conv_rate),

        std::make_unique<HeapOrder>(956, 
            &var::battery7_SOC,&var::battery7_cell1,&var::battery7_cell2,&var::battery7_cell3,
            &var::battery7_cell4,&var::battery7_cell5,&var::battery7_cell6,
            &var::battery7_temperature1,&var::battery7_temperature2,
            &var::battery7_total_voltage,&var::battery7_conv_rate),

        std::make_unique<HeapOrder>(957, 
            &var::battery8_SOC,&var::battery8_cell1,&var::battery8_cell2,&var::battery8_cell3,
            &var::battery8_cell4,&var::battery8_cell5,&var::battery8_cell6,
            &var::battery8_temperature1,&var::battery8_temperature2,
            &var::battery8_total_voltage,&var::battery8_conv_rate),

        std::make_unique<HeapOrder>(958, 
            &var::battery9_SOC,&var::battery9_cell1,&var::battery9_cell2,&var::battery9_cell3,
            &var::battery9_cell4,&var::battery9_cell5,&var::battery9_cell6,
            &var::battery9_temperature1,&var::battery9_temperature2,
            &var::battery9_total_voltage,&var::battery9_conv_rate),

        std::make_unique<HeapOrder>(959,
            &var::battery10_SOC,&var::battery10_cell1,&var::battery10_cell2,&var::battery10_cell3,
            &var::battery10_cell4,&var::battery10_cell5,&var::battery10_cell6,
            &var::battery10_temperature1,&var::battery10_temperature2,
            &var::battery10_total_voltage,&var::battery10_conv_rate),

        std::make_unique<HeapOrder>(960,
            &var::battery11_SOC,&var::battery11_cell1,&var::battery11_cell2,&var::battery11_cell3,
            &var::battery11_cell4,&var::battery11_cell5,&var::battery11_cell6,
            &var::battery11_temperature1,&var::battery11_temperature2,
            &var::battery11_total_voltage,&var::battery11_conv_rate),

        std::make_unique<HeapOrder>(961, 
            &var::battery12_SOC,&var::battery12_cell1,&var::battery12_cell2,&var::battery12_cell3,
            &var::battery12_cell4,&var::battery12_cell5,&var::battery12_cell6,
            &var::battery12_temperature1,&var::battery12_temperature2,
            &var::battery12_total_voltage,&var::battery12_conv_rate),

        std::make_unique<HeapOrder>(962, 
            &var::battery13_SOC,&var::battery13_cell1,&var::battery13_cell2,&var::battery13_cell3,
            &var::battery13_cell4,&var::battery13_cell5,&var::battery13_cell6,
            &var::battery13_temperature1,&var::battery13_temperature2,
            &var::battery13_total_voltage,&var::battery13_conv_rate),

        std::make_unique<HeapOrder>(963,
            &var::battery14_SOC,&var::battery14_cell1,&var::battery14_cell2,&var::battery14_cell3,
            &var::battery14_cell4,&var::battery14_cell5,&var::battery14_cell6,
            &var::battery14_temperature1,&var::battery14_temperature2,
            &var::battery14_total_voltage,&var::battery14_conv_rate),

        std::make_unique<HeapOrder>(964,
            &var::battery15_SOC,&var::battery15_cell1,&var::battery15_cell2,&var::battery15_cell3,
            &var::battery15_cell4,&var::battery15_cell5,&var::battery15_cell6,
            &var::battery15_temperature1,&var::battery15_temperature2,
            &var::battery15_total_voltage,&var::battery15_conv_rate),

        std::make_unique<HeapOrder>(965, 
            &var::battery16_SOC,&var::battery16_cell1,&var::battery16_cell2,&var::battery16_cell3,
            &var::battery16_cell4,&var::battery16_cell5,&var::battery16_cell6,
            &var::battery16_temperature1,&var::battery16_temperature2,
            &var::battery16_total_voltage,&var::battery16_conv_rate),

        std::make_unique<HeapOrder>(966, 
            &var::battery17_SOC,&var::battery17_cell1,&var::battery17_cell2,&var::battery17_cell3,
            &var::battery17_cell4,&var::battery17_cell5,&var::battery17_cell6,
            &var::battery17_temperature1,&var::battery17_temperature2,
            &var::battery17_total_voltage,&var::battery17_conv_rate),

        std::make_unique<HeapOrder>(967, 
            &var::battery18_SOC,&var::battery18_cell1,&var::battery18_cell2,&var::battery18_cell3,
            &var::battery18_cell4,&var::battery18_cell5,&var::battery18_cell6,
            &var::battery18_temperature1,&var::battery18_temperature2,
            &var::battery18_total_voltage,&var::battery18_conv_rate),

        std::make_unique<HeapOrder>(968,
            &var::batteries_voltage_reading),

        std::make_unique<HeapOrder>(969, 
            &var::voltage_reading),

        std::make_unique<HeapOrder>(970,
            &var::current_reading),

        std::make_unique<HeapOrder>(971, 
            &var::general_state_machine_status),

        std::make_unique<HeapOrder>(972,
            &var::operational_state_machine_status),

        std::make_unique<HeapOrder>(973, 
            &var::driver_reading_period),

        std::make_unique<HeapOrder>(974,
            &var::imd_status,&var::imd_resistance,&var::imd_is_ok),

        std::make_unique<HeapOrder>(975, 
            &var::sdc_status),

        std::make_unique<HeapOrder>(976,
            &var::minimum_soc),

        std::make_unique<HeapOrder>(977, 
            &var::bms_status),

        std::make_unique<HeapOrder>(978, 
            &var::voltage_min,&var::voltage_max,&var::temp_min,&var::temp_max)
    };
}


constexpr uint32_t NUM_PACKETS_UDP = 29;
std::array<HeapPacket*, 29> create_all_packets() {
    size_t i = 0;

    Packets::battery_1 = new HeapPacket(Communication_Data::battery_1,
        &var::battery1_SOC, &var::battery1_cell1, &var::battery1_cell2, &var::battery1_cell3,
        &var::battery1_cell4, &var::battery1_cell5, &var::battery1_cell6,
        &var::battery1_temperature1, &var::battery1_temperature2,
        &var::battery1_total_voltage, &var::battery1_conv_rate);
    Packets::all[i++] = Packets::battery_1;

    Packets::battery_2 = new HeapPacket(Communication_Data::battery_2,
        &var::battery2_SOC, &var::battery2_cell1, &var::battery2_cell2, &var::battery2_cell3,
        &var::battery2_cell4, &var::battery2_cell5, &var::battery2_cell6,
        &var::battery2_temperature1, &var::battery2_temperature2,
        &var::battery2_total_voltage, &var::battery2_conv_rate);
    Packets::all[i++] = Packets::battery_2;

    Packets::battery_3 = new HeapPacket(Communication_Data::battery_3,
        &var::battery3_SOC, &var::battery3_cell1, &var::battery3_cell2, &var::battery3_cell3,
        &var::battery3_cell4, &var::battery3_cell5, &var::battery3_cell6,
        &var::battery3_temperature1, &var::battery3_temperature2,
        &var::battery3_total_voltage, &var::battery3_conv_rate);
    Packets::all[i++] = Packets::battery_3;

    Packets::battery_4 = new HeapPacket(Communication_Data::battery_4,
        &var::battery4_SOC, &var::battery4_cell1, &var::battery4_cell2, &var::battery4_cell3,
        &var::battery4_cell4, &var::battery4_cell5, &var::battery4_cell6,
        &var::battery4_temperature1, &var::battery4_temperature2,
        &var::battery4_total_voltage, &var::battery4_conv_rate);
    Packets::all[i++] = Packets::battery_4;

    Packets::battery_5 = new HeapPacket(Communication_Data::battery_5,
        &var::battery5_SOC, &var::battery5_cell1, &var::battery5_cell2, &var::battery5_cell3,
        &var::battery5_cell4, &var::battery5_cell5, &var::battery5_cell6,
        &var::battery5_temperature1, &var::battery5_temperature2,
        &var::battery5_total_voltage, &var::battery5_conv_rate);
    Packets::all[i++] = Packets::battery_5;

    Packets::battery_6 = new HeapPacket(Communication_Data::battery_6,
        &var::battery6_SOC, &var::battery6_cell1, &var::battery6_cell2, &var::battery6_cell3,
        &var::battery6_cell4, &var::battery6_cell5, &var::battery6_cell6,
        &var::battery6_temperature1, &var::battery6_temperature2,
        &var::battery6_total_voltage, &var::battery6_conv_rate);
    Packets::all[i++] = Packets::battery_6;

    Packets::battery_7 = new HeapPacket(Communication_Data::battery_7,
        &var::battery7_SOC, &var::battery7_cell1, &var::battery7_cell2, &var::battery7_cell3,
        &var::battery7_cell4, &var::battery7_cell5, &var::battery7_cell6,
        &var::battery7_temperature1, &var::battery7_temperature2,
        &var::battery7_total_voltage, &var::battery7_conv_rate);
    Packets::all[i++] = Packets::battery_7;

    Packets::battery_8 = new HeapPacket(Communication_Data::battery_8,
        &var::battery8_SOC, &var::battery8_cell1, &var::battery8_cell2, &var::battery8_cell3,
        &var::battery8_cell4, &var::battery8_cell5, &var::battery8_cell6,
        &var::battery8_temperature1, &var::battery8_temperature2,
        &var::battery8_total_voltage, &var::battery8_conv_rate);
    Packets::all[i++] = Packets::battery_8;

    Packets::battery_9 = new HeapPacket(Communication_Data::battery_9,
        &var::battery9_SOC, &var::battery9_cell1, &var::battery9_cell2, &var::battery9_cell3,
        &var::battery9_cell4, &var::battery9_cell5, &var::battery9_cell6,
        &var::battery9_temperature1, &var::battery9_temperature2,
        &var::battery9_total_voltage, &var::battery9_conv_rate);
    Packets::all[i++] = Packets::battery_9;

    Packets::battery_10 = new HeapPacket(Communication_Data::battery_10,
        &var::battery10_SOC, &var::battery10_cell1, &var::battery10_cell2, &var::battery10_cell3,
        &var::battery10_cell4, &var::battery10_cell5, &var::battery10_cell6,
        &var::battery10_temperature1, &var::battery10_temperature2,
        &var::battery10_total_voltage, &var::battery10_conv_rate);
    Packets::all[i++] = Packets::battery_10;

    Packets::battery_11 = new HeapPacket(Communication_Data::battery_11,
        &var::battery11_SOC, &var::battery11_cell1, &var::battery11_cell2, &var::battery11_cell3,
        &var::battery11_cell4, &var::battery11_cell5, &var::battery11_cell6,
        &var::battery11_temperature1, &var::battery11_temperature2,
        &var::battery11_total_voltage, &var::battery11_conv_rate);
    Packets::all[i++] = Packets::battery_11;

    Packets::battery_12 = new HeapPacket(Communication_Data::battery_12,
        &var::battery12_SOC, &var::battery12_cell1, &var::battery12_cell2, &var::battery12_cell3,
        &var::battery12_cell4, &var::battery12_cell5, &var::battery12_cell6,
        &var::battery12_temperature1, &var::battery12_temperature2,
        &var::battery12_total_voltage, &var::battery12_conv_rate);
    Packets::all[i++] = Packets::battery_12;

    Packets::battery_13 = new HeapPacket(Communication_Data::battery_13,
        &var::battery13_SOC, &var::battery13_cell1, &var::battery13_cell2, &var::battery13_cell3,
        &var::battery13_cell4, &var::battery13_cell5, &var::battery13_cell6,
        &var::battery13_temperature1, &var::battery13_temperature2,
        &var::battery13_total_voltage, &var::battery13_conv_rate);
    Packets::all[i++] = Packets::battery_13;

    Packets::battery_14 = new HeapPacket(Communication_Data::battery_14,
        &var::battery14_SOC, &var::battery14_cell1, &var::battery14_cell2, &var::battery14_cell3,
        &var::battery14_cell4, &var::battery14_cell5, &var::battery14_cell6,
        &var::battery14_temperature1, &var::battery14_temperature2,
        &var::battery14_total_voltage, &var::battery14_conv_rate);
    Packets::all[i++] = Packets::battery_14;

    Packets::battery_15 = new HeapPacket(Communication_Data::battery_15,
        &var::battery15_SOC, &var::battery15_cell1, &var::battery15_cell2, &var::battery15_cell3,
        &var::battery15_cell4, &var::battery15_cell5, &var::battery15_cell6,
        &var::battery15_temperature1, &var::battery15_temperature2,
        &var::battery15_total_voltage, &var::battery15_conv_rate);
    Packets::all[i++] = Packets::battery_15;

    Packets::battery_16 = new HeapPacket(Communication_Data::battery_16,
        &var::battery16_SOC, &var::battery16_cell1, &var::battery16_cell2, &var::battery16_cell3,
        &var::battery16_cell4, &var::battery16_cell5, &var::battery16_cell6,
        &var::battery16_temperature1, &var::battery16_temperature2,
        &var::battery16_total_voltage, &var::battery16_conv_rate);
    Packets::all[i++] = Packets::battery_16;

    Packets::battery_17 = new HeapPacket(Communication_Data::battery_17,
        &var::battery17_SOC, &var::battery17_cell1, &var::battery17_cell2, &var::battery17_cell3,
        &var::battery17_cell4, &var::battery17_cell5, &var::battery17_cell6,
        &var::battery17_temperature1, &var::battery17_temperature2,
        &var::battery17_total_voltage, &var::battery17_conv_rate);
    Packets::all[i++] = Packets::battery_17;

    Packets::battery_18 = new HeapPacket(Communication_Data::battery_18,
        &var::battery18_SOC, &var::battery18_cell1, &var::battery18_cell2, &var::battery18_cell3,
        &var::battery18_cell4, &var::battery18_cell5, &var::battery18_cell6,
        &var::battery18_temperature1, &var::battery18_temperature2,
        &var::battery18_total_voltage, &var::battery18_conv_rate);
    Packets::all[i++] = Packets::battery_18;

    Packets::batteries_voltage = new HeapPacket(Communication_Data::batteries_voltage,
        &var::batteries_voltage_reading);
    Packets::all[i++] = Packets::batteries_voltage;

    Packets::voltage_sensor = new HeapPacket(Communication_Data::voltage_sensor,
        &var::voltage_reading);
    Packets::all[i++] = Packets::voltage_sensor;

    Packets::current_sensor = new HeapPacket(Communication_Data::current_sensor,
        &var::current_reading);
    Packets::all[i++] = Packets::current_sensor;

    Packets::general_state_machine = new HeapPacket(Communication_Data::general_state_machine,
        &var::general_state_machine_status);
    Packets::all[i++] = Packets::general_state_machine;

    Packets::operational_state_machine = new HeapPacket(Communication_Data::operational_state_machine,
        &var::operational_state_machine_status);
    Packets::all[i++] = Packets::operational_state_machine;

    Packets::driver_diagnosis = new HeapPacket(Communication_Data::driver_diagnosis,
        &var::driver_reading_period);
    Packets::all[i++] = Packets::driver_diagnosis;

    Packets::imd = new HeapPacket(Communication_Data::imd,
        &var::imd_status, &var::imd_resistance, &var::imd_is_ok);
    Packets::all[i++] = Packets::imd;

    Packets::sdc = new HeapPacket(Communication_Data::sdc,
        &var::sdc_status);
    Packets::all[i++] = Packets::sdc;

    Packets::minimum_soc = new HeapPacket(Communication_Data::minimum_soc,
        &var::minimum_soc);
    Packets::all[i++] = Packets::minimum_soc;

    Packets::bms = new HeapPacket(Communication_Data::bms,
        &var::bms_status);
    Packets::all[i++] = Packets::bms;

    Packets::batteries_data = new HeapPacket(Communication_Data::batteries_data,
        &var::voltage_min, &var::voltage_max, &var::temp_min, &var::temp_max);
    Packets::all[i++] = Packets::batteries_data;

    return Packets::all;
}

