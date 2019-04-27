//
// Created by dumpling on 27.04.19.
//
#include <cstring>
#include <sstream>

#include <sys/mman.h>

#include "common.h"

void print_err(const std::string &message) {
    std::cerr << "\033[31m" << message;
    if (errno) {
        std::cerr << ": " << std::strerror(errno);
    }
    std::cerr << "\033[0m" << std::endl;
}

void *place_code(unsigned char *code_to_place, size_t length) {
    auto *data = mmap(nullptr, length, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (data == MAP_FAILED) {
        print_err("Can't allocate memory");
        exit(EXIT_FAILURE);
    }

    memcpy(data, code_to_place, length);

    if (mprotect(data, length, PROT_READ | PROT_EXEC) == -1) {
        print_err("Can't execute function");
        exit(EXIT_FAILURE);
    }

    return data;
}

void clean(void *mem_to_clean, size_t length) {
    if (munmap(mem_to_clean, length) == -1) {
        print_err("Can't free memory");
        exit(EXIT_FAILURE);
    }
}
