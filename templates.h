//
// Created by dumpling on 23.04.19.
//

#ifndef JIT_TEMPLATES_H
#define JIT_TEMPLATES_H

#include <vector>
#include <cstdint>
#include <sstream>
#include <algorithm>

using byte = unsigned char;

const static int cmp_size = 30;

template<typename T>
std::vector<byte> get_hex(T n);

std::vector<byte> get_code(const std::string &s);

#endif //JIT_TEMPLATES_H
