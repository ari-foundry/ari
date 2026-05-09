#include "common.hpp"
#include "driver.hpp"

#include <exception>
#include <iostream>

int main(int argc, char** argv) {
    try {
        return ari::run(argc, argv);
    } catch (const std::exception& error) {
        std::cerr << "ari: error: " << error.what() << "\n";
        return 1;
    }
}
