//
// Created by dumpling on 27.04.19.
//

#include "templates.h"


template<typename T>
std::vector<byte> get_hex(T n) {
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

void concat(std::vector<byte> &a, const std::vector<byte> &b) {
    a.insert(a.end(), b.begin(), b.end());
}

std::vector<byte> lea(const std::vector<byte> &bytes) {
    std::vector<byte> ans = {0x48, 0x8d, 0x90, 0x00, 0x00, 0x00, 0x00};

    for (int i = 0; i < bytes.size(); ++i) {
        ans[3 + i] = bytes[i];
    }

    return ans;
}

std::vector<byte> j(byte type, const std::vector<byte> &bytes) {
    std::vector<byte> ans = {0x0f, type, 0x00, 0x00, 0x00, 0x00};

    for (int i = 0; i < bytes.size(); ++i) {
        ans[2 + i] = bytes[i];
    }

    return ans;
}


std::vector<byte> jg(const std::vector<byte> &bytes) {
    return j(0x8f, bytes);
}

std::vector<byte> jne(const std::vector<byte> &bytes) {
    return j(0x85, bytes);
}

std::vector<byte> get_cmp(int n, char c, int d) {
    std::vector<byte> result = {0x8b, 0x45, 0xfc,
                             0x48, 0x98};

    concat(result, lea(get_hex(n)));

    concat(result, {0x48, 0x8b, 0x45, 0xe8,
                 0x48, 0x01, 0xd0,
                 0x0f, 0xb6, 0x00,
                 0x3c, get_hex((int) c)[0]});

    concat(result, jne(get_hex(d)));

    return result;
}

std::vector<byte> get_code(const std::string &s) {
    std::vector<byte> result = {0x55,
                                0x48, 0x89, 0xe5,
                                0x48, 0x89, 0x7d, 0xe8,
                                0x89, 0x75, 0xe4,
                                0xc7, 0x45, 0xfc, 0x00, 0x00, 0x00, 0x00,
                                0x8b, 0x45, 0xfc,
                                0x3b, 0x45, 0xe4};

    concat(result, jg(get_hex(5 + s.length() * cmp_size + 4 + 5)));
    for (int i = 0; i < s.length(); ++i) {
        concat(result, get_cmp(i, s[i], 5 + (s.length() - i - 1) * cmp_size));
    }
    concat(result, {0x8b, 0x45, 0xfc,
                    0xeb, 0x0e,
                    0x83, 0x45, 0xfc, 0x01,
                    0xe9, 0x00, 0x00, 0x00, 0x00});

    auto e = get_hex((unsigned int) (-(5 + 4 + 2 + 3 + s.length() * cmp_size + 6 + 6)));

    for (int i = 0; i < e.size(); ++i) {
        result[result.size() - 4 + i] = e[i];
    }

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

int first_entry(const char *s, int size) {
    for (int i = 0; i < size; ++i) {
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