#pragma once
#include "Brakes.hpp"
#include "Communications/Packets/DataPackets.hpp"
#include "Communications/Packets/OrderPackets.hpp"
#include "Pinout.hpp"
#include "ST-LIB.hpp"

// -----------------END OF COMPILE TIME STUFF-----------------

enum class GeneralStates : uint8_t { CONNECTING, OPERATIONAL, FAULT };

enum class OperationalStates : uint8_t { BRAKED, UNBRAKED };

class VCU {
   private:
    //--------------------STATES--------------------
    static constexpr auto connecting_state =
        make_state(DataPackets::general_state::CONNECTING,
                   Transition<DataPackets::general_state>{
                       DataPackets::general_state::OPERATIONAL,
                       []() { return OrderPackets::control_station_tcp->is_connected(); }});

    static constexpr auto operational_state =
        make_state(DataPackets::general_state::OPERATIONAL,
                   Transition<DataPackets::general_state>{
                       DataPackets::general_state::FAULT,
                       []() { return !OrderPackets::control_station_tcp->is_connected(); }});

    static constexpr auto fault_state = make_state(DataPackets::general_state::FAULT);

    //--------------------------------------------------

    static constexpr auto braked_state = make_state(
        DataPackets::operational_state::BRAKED,
        Transition<DataPackets::operational_state>{DataPackets::operational_state::UNBRAKED,
                                                   []() { return !Brakes::is_POD_braked(); }});

    static constexpr auto unbraked_state = make_state(
        DataPackets::operational_state::UNBRAKED,
        Transition<DataPackets::operational_state>{DataPackets::operational_state::BRAKED,
                                                   []() { return Brakes::is_POD_braked(); }});

    //--------------------STATE MACHINE--------------------

    static inline constinit auto OperationalStateMachine = []() consteval {
        auto sm = make_state_machine(DataPackets::operational_state::BRAKED, braked_state,
                                     unbraked_state);

        return sm;
    }();

    static inline constinit auto GeneralStateMachine = []() consteval {
        auto nested =
            StateMachineHelper::add_nesting(operational_state, OperationalStateMachine);

        auto sm = make_state_machine(DataPackets::general_state::CONNECTING,
                                     StateMachineHelper::add_nested_machines(nested),
                                     connecting_state,
                                     operational_state,
                                     fault_state);

        sm.add_enter_action(
            []() {
                ProtectionManager::fault_and_propagate();
                Brakes::brake();
            },
            fault_state);

        return sm;
    }();

    static inline DataPackets::general_state current_state{};
    static inline DataPackets::operational_state current_nested_state{};

   public:
    static void init(void) {
        init_board();
        Brakes::init(&get_actuator(), &get_nucleo_led(), &get_tape_enable(), &get_regulator_out());

        // Comms
        DataPackets::Current_State_init(current_state, current_nested_state);
        OrderPackets::Brake_init();
        OrderPackets::Unbrake_init();

        DataPackets::start();
        OrderPackets::start();

        ProtectionManager::link_state_machine(GeneralStateMachine,
                                              static_cast<uint8_t>(GeneralStates::FAULT));
        ProtectionManager::add_standard_protections();
        ProtectionManager::initialize();

        Scheduler::register_task(
            16670, +[]() {
                current_state = GeneralStateMachine.get_current_state();
                current_nested_state = OperationalStateMachine.get_current_state();
                GeneralStateMachine.check_transitions();
            });
        Scheduler::register_task(16670, +[]() { Brakes::update(); });
    };

    static void update(void) {
        get_ethernet().update();
        ProtectionManager::check_protections();
        Scheduler::update();
    };
};