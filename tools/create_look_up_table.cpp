#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
// execute this code to create a LUT in a .hpp
//  Settings
/*Modify this variables to change the  LUT*/
constexpr int NUMBER_POINTS = 4096;
constexpr float START_RAD = 0.0f;
constexpr float END_RAD = 2 * M_PI;
const std::string FUNC_NAME = "sin";
std::function<float(float)> func = [](float x) { return std::sin(x); };

int main() {
    std::ofstream out_file("look_up_table.hpp", std::ios::out | std::ios::trunc);
    if (!out_file) {
        std::cerr << "Error al abrir look_up_table.hpp\n";
        return 1;
    }

    out_file << "#pragma once\n";
    out_file << "#define NUMBER_POINTS " << NUMBER_POINTS << "\n\n";

    out_file << "/* " << FUNC_NAME << " Look up table from " << START_RAD << " to " << END_RAD
             << " rad */\n";

    // Nombre de variable dinámico
    out_file << "const float look_up_table_" << FUNC_NAME << "[NUMBER_POINTS] = {\n";

    for (int i = 0; i < NUMBER_POINTS; ++i) {
        float angle = START_RAD + (END_RAD - START_RAD) * i / NUMBER_POINTS;
        float value = func(angle);
        out_file << value;
        if (i + 1 != NUMBER_POINTS)
            out_file << ", ";
        if ((i + 1) % 8 == 0)
            out_file << "\n";
    }
    out_file << "};\n";
    return 0;
}
