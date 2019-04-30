//
// Created by dumpling on 22.04.19.
//

#include <iostream>
#include <algorithm>
#include <vector>
#include <fstream>
#include <cstring>

#include "templates.h"
#include "common.h"
#include "performance.h"

static byte power_code[] = {
        0x55,                                                       //  0: push   rbp
        0x48, 0x89, 0xe5,                                           //  1: mov    rbp, rsp
        0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //  4: movabs rax, 0x0
        0x48, 0x89, 0x45, 0xf8,                                     //  e: mov    QWORD PTR [rbp-0x8], rax
        0xc7, 0x45, 0xec, 0x00, 0x00, 0x00, 0x00,                   // 12: mov    DWORD PTR [rbp-0x14], 0x0
        0x48, 0xc7, 0x45, 0xf0, 0x01, 0x00, 0x00, 0x00,             // 19: mov    QWORD PTR [rbp-0x10], 0x1
        0x48, 0x8b, 0x45, 0xf0,                                     // 21: mov    rax, QWORD PTR [rbp-0x10]
        0x48, 0x01, 0xc0,                                           // 25: add    rax, rax
        0x48, 0x39, 0x45, 0xf8,                                     // 28: cmp    QWORD PTR [rbp-0x8], rax
        0x72, 0x10,                                                 // 2c: jb     0x3e
        0x83, 0x7d, 0xec, 0x3e,                                     // 2e: cmp    DWORD PTR [rbp-0x14], 0x3e
        0x7f, 0x0a,                                                 // 32: jg     0x3e
        0x83, 0x45, 0xec, 0x01,                                     // 34: add    DWORD PTR [rbp-0x14], 0x1
        0x48, 0xd1, 0x65, 0xf0,                                     // 38: shl    QWORD PTR [rbp-0x10], 1
        0xeb, 0xe3,                                                 // 3c: jmp    0x21
        0x8b, 0x45, 0xec,                                           // 3e: mov    eax, DWORD PTR [rbp-0x14]
        0x5d,                                                       // 41: pop    rbp
        0xc3                                                        // 42: ret
};

static const int power_code_size = sizeof(power_code);

static void print_help() {
    std::cout << "Includes 2 utilities:" << std::endl
              << "power <n> — by this number n finds the maximum k such that 2^k <= n" << std::endl
              << "substr <a> <b> — on these strings a and b finds the first occurrence of string b in a or reports that there are no occurrences"
              << std::endl << std::endl
              << "There is also one testing tool:" << std::endl
              << "perf — for check performance of jit substring searcher by comparing with not-jit naive algorithm and prefix-function"
              << std::endl;
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

uint64_t power(const std::string &arg) {

    uint64_t n;
    try {
        n = get_ull(arg);
    } catch (std::exception &e) {
        print_err(e.what());
        exit(EXIT_FAILURE);
    }

    fix_code(n);
    auto *data = place_code(power_code, power_code_size);
    auto res = (reinterpret_cast<power_func_type>(data))();

    clean(data, power_code_size);

    return res;
}

int substring(const char *s, const char *substr) {

    auto length = std::strlen(s);
    auto sub_length = std::strlen(substr);

    if (length < sub_length) {
        return -1;
    }

    auto code = get_code(substr);

    auto *data = place_code(code.data(), code.size());
    auto res = (reinterpret_cast<substr_type>(data))(s, length - sub_length + 1);

    clean(data, code.size());

    return res;
}


int main(int argc, char **argv) {

    if (argc < 2) {
        print_err("Incorrect size of args, use help");
        return EXIT_FAILURE;
    }

    std::string arg(argv[1]);
    if (arg == "help") {
        print_help();
        return EXIT_SUCCESS;
    }

    if (arg == "perf") {
        check_performance();
        return EXIT_SUCCESS;
    }

    if (arg == "power") {
        if (argc > 3) {
            print_err("Too many arguments, use help");
            return EXIT_FAILURE;
        }
        std::cout << power(std::string(argv[2])) << std::endl;
        return EXIT_SUCCESS;
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

        std::string a, b;
        auto res = substring(argv[2], argv[3]);
        if (res == -1) {
            std::cout << "Mo match" << std::endl;
        } else {
            std::cout << res << std::endl;
        }
        return EXIT_SUCCESS;
    }

    print_err("Unknown command, use help");
    return EXIT_FAILURE;
}

