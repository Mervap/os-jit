//
// Created by dumpling on 27.04.19.
//

#ifndef JIT_COMMON_H
#define JIT_COMMON_H

#include <iostream>
#include <cstring>

#include <sys/mman.h>
#include <sstream>

using power_func_type = uint64_t (*)();
using substr_type = int (*)(const char *s, int size);

static void print_err(const std::string &message) {
    std::cerr << "\033[31m" << message;
    if (errno) {
        std::cerr << ": " << std::strerror(errno);
    }
    std::cerr << "\033[0m" << std::endl;
}

static void *place_code(unsigned char *code_to_place, size_t length) {
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

static int clean(void *mem_to_clean, size_t length) {
    if (munmap(mem_to_clean, length) == -1) {
        print_err("Can't free memory");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#endif //JIT_COMMON_H
