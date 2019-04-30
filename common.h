//
// Created by dumpling on 27.04.19.
//

#ifndef JIT_COMMON_H
#define JIT_COMMON_H

#include <iostream>

using power_func_type = int (*)();
using substr_type = int (*)(const char *s, int size);

void print_err(const std::string &message);
void *place_code(unsigned char *code_to_place, size_t length);
void clean(void *mem_to_clean, size_t length);

#endif //JIT_COMMON_H
