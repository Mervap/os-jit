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
static std::vector<byte> get_hex(T n) {
    std::stringstream stream;
    stream << std::hex << n;
    std::string hex(stream.str());

    if (hex.length() % 2 == 1) {
        hex = "0" + hex;
    }

    size_t len = hex.length() / 2;
    std::string subs[len];
    for (int i = 0; i < hex.length(); i += 2) {
        subs[i / 2] += hex.substr(i, 2);
    }

    std::reverse(subs, subs + len);
    std::vector<byte> res(len);
    for (int i = 0; i < len; ++i) {
        res[i] = (byte) strtol(subs[i].c_str(), nullptr, 16);
    }

    return res;
}

static void concat(std::vector<byte> &a, const std::vector<byte> &b) {
    a.insert(a.end(), b.begin(), b.end());
}

static std::vector<byte> lea(const std::vector<byte> &f) {
    std::vector<byte> ans = {0x48, 0x8d, 0x90};
    for (auto e : f) {
        ans.push_back(e);
    }

    for (int i = ans.size(); i < 7; ++i) {
        ans.push_back(0);
    }

    return ans;
}

static std::vector<byte> j(byte f, const std::vector<byte> &d) {
    std::vector<byte> ans = {0x0f, f};
    for (auto e : d) {
        ans.push_back(e);
    }

    for (int i = ans.size(); i < 6; ++i) {
        ans.push_back(0);
    }

    return ans;
}


static std::vector<byte> jg(const std::vector<byte> &d) {
    return j(0x8f, d);
}

static std::vector<byte> jne(const std::vector<byte> &d) {
    return j(0x85, d);
}

static std::vector<byte> get_cmp(int n, char c, int d) {
    std::vector<byte> beg = {0x8b, 0x45, 0xfc,
                             0x48, 0x98};

    concat(beg, lea(get_hex(n)));

    concat(beg, {0x48, 0x8b, 0x45, 0xe8,
                 0x48, 0x01, 0xd0,
                 0x0f, 0xb6, 0x00,
                 0x3c, get_hex((int) c)[0]});

    concat(beg, jne(get_hex(d)));

    return beg;
}

static std::vector<byte> get_code(int length, const std::string &s) {
    std::vector<byte> result = {0x55,
                                0x48, 0x89, 0xe5,
                                0x48, 0x89, 0x7d, 0xe8,
                                0xc7, 0x45, 0xfc, 0x00, 0x00, 0x00, 0x00,
                                0x81, 0x7d, 0xfc};

    auto e = get_hex(length - s.length());
    while (e.size() < 4) {
        e.push_back(0);
    }
    concat(result, e); // cycle length
    concat(result, jg(get_hex(5 + s.length() * cmp_size + 4 + 5)));
    for (int i = 0; i < s.length(); ++i) {
        concat(result, get_cmp(i, s[i], 5 + (s.length() - i - 1) * cmp_size));
    }
    concat(result, {0x8b, 0x45, 0xfc,
                    0xeb, 0x0e,
                    0x83, 0x45, 0xfc, 0x01,
                    0xe9});

    e = get_hex((unsigned int) (-(4 + 4 + 2 + 3 + s.length() * cmp_size + 6 + 4 + 4)));
    while (e.size() < 4) {
        e.push_back(0);
    }
    concat(result, e);
    concat(result, {0xb8, 0xff, 0xff, 0xff, 0xff,
                    0x5d,
                    0xc3});
    return result;
}


// Prototypes for JIT
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

int first_entry(const char s[]) {
    for (int i = 0; i < 7; ++i) {
        if (s[i] == 'a') {
            if (s[i + 1] == 'c') {
                if (s[i + 2] == 'a') {
                    return i;
                }
            }
        }
    }

    return -1;
}

#endif //JIT_TEMPLATES_H
