//
// Created by dumpling on 22.04.19.
//

#include <iostream>
#include <cstring>
#include <algorithm>
#include <vector>

#include <sys/mman.h>
#include <sstream>

#include "templates.h"

using power_func_type = uint64_t (*)();
using substr_type = int (*)(const char *s);

static byte power_code[] = {
        0x55,
        0x48, 0x89, 0xe5,
        0x48, 0xb8, 0x00, 0x00, 0x00, 0x0, 0x00, 0x00, 0x00, 0x00,
        0x48, 0x89, 0x45, 0xf8,
        0xc7, 0x45, 0xec, 0x00, 0x00, 0x00, 0x00,
        0x48, 0xc7, 0x45, 0xf0, 0x01, 0x00, 0x00, 0x00,
        0x48, 0x8b, 0x45, 0xf0,
        0x48, 0x01, 0xc0,
        0x48, 0x39, 0x45, 0xf8,
        0x72, 0x0a,
        0x83, 0x45, 0xec, 0x01,
        0x48, 0xd1, 0x65, 0xf0,
        0xeb, 0xe9,
        0x8b, 0x45, 0xec,
        0x5d,
        0xc3
};

static void print_err(const std::string &message) {
    std::cerr << "\033[31m" << message;
    if (errno) {
        std::cerr << ": " << std::strerror(errno);
    }
    std::cerr << "\033[0m" << std::endl;
}

static void print_help() {
    std::cout << "The program takes n as input and outputs the maximum k such that 2 to the power of k <= n"
              << std::endl
              << "n must be a positive integer number" << std::endl
              << "use: ./power <number>" << std::endl
              << "or ./power help - for help" << std::endl;
}

uint64_t get_ull(std::string number) {

    if (number[0] == '-') {
        throw std::invalid_argument(number + " should be not negative");
    }

    size_t st = 0;
    auto real_number = std::stoull(number, &st);

    if (st != number.size()) {
        throw std::invalid_argument(number + " is not a positive integer number");
    }

    return real_number;
}

void fix_code(uint64_t n) {
    auto hex = get_hex(n);
    for (int i = 0; i < hex.size(); ++i) {
        power_code[6 + i] = hex[i];
    }
}

void *place_code(unsigned char *code_to_place, size_t length) {
    auto *data = mmap(nullptr, length, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (data == MAP_FAILED) {
        print_err("Can't allocate memory");
        return MAP_FAILED;
    }

    memcpy(data, code_to_place, length);

    if (mprotect(data, length, PROT_READ | PROT_EXEC) == -1) {
        print_err("Can't execute function");
        return MAP_FAILED;
    }

    return data;
}

int clean(void *mem_to_clean, size_t length) {
    if (munmap(mem_to_clean, length) == -1) {
        print_err("Can't free memory");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int power(const std::string &arg) {

    uint64_t n;
    try {
        n = get_ull(arg);
    } catch (...) {
        print_err(arg + " is not a positive integer number");
        return EXIT_FAILURE;
    }

    fix_code(n);
    auto *data = place_code(power_code, sizeof(power_code));
    if (data == MAP_FAILED) {
        return EXIT_FAILURE;
    }

    auto res = (reinterpret_cast<power_func_type>(data))();

    std::cout << res << std::endl;

    return clean(data, sizeof(power_code));
}

int substring(int length, const char *s, const char *substr) {

    auto e = get_code(length, substr);

    auto *data = place_code(e.data(), e.size());
    if (data == MAP_FAILED) {
        return EXIT_FAILURE;
    }

    auto res = (reinterpret_cast<substr_type>(data))(s);

    if (res == -1) {
        std::cout << "No match" << std::endl;
    } else {
        std::cout << res << std::endl;
    }

    return clean(data, e.size());
}


int main(int argc, char **argv) {

    if (argc < 2) {
        print_err("Command expected, use help");
        return EXIT_FAILURE;
    }


    std::string arg(argv[1]);
    if (arg == "help") {
        print_help();
        return EXIT_SUCCESS;
    }

    if (arg == "power") {
        if (argc > 3) {
            print_err("Too many arguments, use help");
            return EXIT_FAILURE;
        }
        return power(std::string(argv[2]));
    }

    if (arg == "substr") {
        if (argc < 4) {
            print_err("Too few arguments, use help");
            return EXIT_FAILURE;
        }

        if (argc > 4) {
            print_err("Too many arguments, use help");
            return EXIT_FAILURE;
        }

        if (strlen(argv[2]) < strlen(argv[3])) {
            std::cout << "No match" << std::endl;
            return EXIT_SUCCESS;
        }

        return substring(strlen(argv[2]), argv[2], argv[3]);
    }

    print_err("Unknown command");
    return EXIT_FAILURE;
}

