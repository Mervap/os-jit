//
// Created by dumpling on 22.04.19.
//

#include <iostream>
#include <cstring>
#include <algorithm>
#include <vector>

#include <sys/mman.h>
#include <sstream>

using func_type = uint64_t (*)();

unsigned char code[] = {
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

// Сode received from here
int power_of_two() {
    uint64_t n = 16;
    int ans = 0;
    uint64_t cur = 1;

    while (cur * 2 <= n) {
        ++ans;
        cur *= 2;
    }

    return ans;
}

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
    std::stringstream stream;
    stream << std::hex << n;
    std::string result(stream.str());

    if (result.length() % 2 == 1) {
        result = "0" + result;
    }

    std::string subs[result.length() / 2];
    for (int i = 0; i < result.length(); i += 2) {
        subs[i / 2] += result.substr(i, 2);
    }

    std::reverse(subs, subs + result.length() / 2);
    for (int i = 0; i < result.length() / 2; ++i) {
        code[6 + i] = (unsigned char) strtol(subs[i].c_str(), nullptr, 16);
    }
}

int main(int argc, char **argv) {

    if (argc < 2) {
        print_err("Number expected, use help");
        return EXIT_FAILURE;
    }

    if (argc > 2) {
        print_err("Too many arguments, use help");
        return EXIT_FAILURE;
    }

    std::string arg(argv[1]);
    if (arg == "help") {
        print_help();
        return EXIT_SUCCESS;
    }

    uint64_t n;
    try {
        n = get_ull(arg);
    } catch (...) {
        print_err(arg + " is not a positive integer number");
        return EXIT_FAILURE;
    }

    auto *data = mmap(nullptr, sizeof(code), PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (data == MAP_FAILED) {
        print_err("Can't allocate memory");
        return EXIT_FAILURE;
    }

    fix_code(n);
    memcpy(data, code, sizeof(code));

    if (mprotect(data, sizeof(code), PROT_READ | PROT_EXEC) == -1) {
        print_err("Can't execute function");
        return EXIT_FAILURE;
    }

    auto res = (reinterpret_cast<func_type>(data))();
    std::cout << res << std::endl;

}

