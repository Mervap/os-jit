//
// Created by dumpling on 27.04.19.
//
#include <random>
#include <chrono>
#include <assert.h>
#include <functional>

#include "common.h"
#include "templates.h"

static const int NANOSEC_IN_MILLISEC = 1'000'000;

struct times {
    uint64_t jit, native, pref;

    times &operator+=(const times &other) {
        jit += other.jit;
        native += other.native;
        pref += other.pref;

        return *this;
    }

    void div(int divisor) {
        jit /= divisor;
        native /= divisor;
        pref /= divisor;
    }

};

std::string get_str(int size) {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<char> dis('a', 'z');

    std::string res;
    for (int i = 0; i < size; ++i) {
        res += dis(gen);
    }

    return res;
}

int prefix(const std::string &a, const std::string &b) {
    std::string s = b + "#" + a;
    int tmp[s.length()];

    tmp[0] = 0;
    for (int i = 1; i < s.length(); ++i) {
        int cur = tmp[i - 1];
        while (cur > 0 && s[cur] != s[i]) {
            cur = tmp[cur - 1];
        }

        tmp[i] = cur + (s[cur] == s[i]);
        if (tmp[i] == b.size()) {
            return i - 2 * b.size();
        }
    }

    return -1;
}

int naive(const std::string &a, const std::string &b) {
    for (int i = 0; i < a.size() - b.size() + 1; ++i) {
        for (int j = 0; j < b.size(); ++j) {
            if (a[i + j] != b[j]) {
                break;
            }

            if (j == b.size() - 1) {
                return i;
            }
        }
    }

    return -1;
}

long get_nanosec(const std::chrono::steady_clock::duration &dur) {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
}


times run(int a_size, int b_size, int cycle_cnt) {

    std::string b = get_str(b_size);

    auto start = std::chrono::steady_clock::now();
    auto code = get_code(b);
    auto *data = place_code(code.data(), code.size());
    auto end = std::chrono::steady_clock::now();

    uint64_t time = get_nanosec(end - start), time1 = 0, time2 = 0;

    for (int i = 0; i < cycle_cnt; ++i) {
        std::string a = get_str(a_size);

        start = std::chrono::steady_clock::now();
        int r1 = (reinterpret_cast<substr_type>(data))(a.c_str(), a.size());
        end = std::chrono::steady_clock::now();

        time += get_nanosec(end - start);

        start = std::chrono::steady_clock::now();
        int r2 = naive(a, b);
        end = std::chrono::steady_clock::now();

        time1 += get_nanosec(end - start);

        start = std::chrono::steady_clock::now();
        int r3 = prefix(a, b);
        end = std::chrono::steady_clock::now();

        time2 += get_nanosec(end - start);
        assert(r1 == r2 && r2 == r3);
    }

    start = std::chrono::steady_clock::now();
    clean(data, code.size());
    end = std::chrono::steady_clock::now();

    time += get_nanosec(end - start);

    return {time, time1, time2};
}

void check_performance() {

    std::cout << "It can takes about a minute" << std::endl << std::endl;

    auto start = std::chrono::steady_clock::now();
    std::vector<int> sizes = {10, 100, 1000, 10000, 100000};
    std::vector<int> cycles_cnt = {100, 100, 100, 10};

    for (int i = 1; i < sizes.size(); ++i) {
        for (int j = 0; j < i; ++j) {
            times time = {0, 0, 0};
            for (int k = 0; k < 10; ++k) {
                time += run(sizes[i], sizes[j], cycles_cnt[j]);
            }
            time.div(10);

            std::cout << "string size = " << sizes[i] << ", substr size = " << sizes[j] << ", cycles = "
                      << cycles_cnt[j] << std::endl
                      << "jit   : " << time.jit / NANOSEC_IN_MILLISEC << "ms" << std::endl
                      << "naive : " << time.native / NANOSEC_IN_MILLISEC << "ms" << std::endl
                      << "prefix: " << time.pref / NANOSEC_IN_MILLISEC << "ms" << std::endl << std::endl;
        }
    }

    auto end = std::chrono::steady_clock::now();

    std::cout << "Finish: " << get_nanosec(end - start) / NANOSEC_IN_MILLISEC << "ms" << std::endl;
}