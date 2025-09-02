#include <vector>
#include <cmath>
#include <thread>
#include <mutex>
#include <iostream>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>

std::mutex primes_mutex;
std::vector<int> primes;

// Worker function for threads
void sieve_worker(int start, int end, int id) {
    std::cout << "[C++] thread " << id << " started" << "\n";
    std::vector<bool> is_prime(end - start, true);

    for (int i = 0; i < (int)is_prime.size(); i++) {
        int num = start + i;
        if (num < 2) {
            is_prime[i] = false;
            continue;
        }
        for (int p = 2; p * p <= num; ++p) {
            if (num % p == 0) {
                is_prime[i] = false;
                break;
            }
        }
    }

    // Store results into global primes vector
    std::lock_guard<std::mutex> lock(primes_mutex);
    for (int i = 0; i < (int)is_prime.size(); i++) {
        if (is_prime[i]) {
            primes.push_back(start + i);
        }
    }
    std::cout << "[C++] thread " << id << " finished" << "\n";
}

// Main entry point for JS
void runPrimes(int N, emscripten::val jsCallback) {
    std::cout << "[C++] runPrimes started with N=" << N << "\n";

    primes.clear();
    const int num_threads = 4;
    std::vector<std::thread> threads;

    int chunk = N / num_threads;
    for (int t = 0; t < num_threads; ++t) {
        int start = t * chunk;
        int end = (t == num_threads - 1) ? N : start + chunk;
        threads.emplace_back(sieve_worker, start, end, t);
    }

    for (auto &th : threads) {
        th.join();
    }

    // Sort primes (since threads may push unordered chunks)
    std::sort(primes.begin(), primes.end());

    // Push primes to JS
    emscripten::val jsArray = emscripten::val::array();
    for (size_t i = 0; i < primes.size(); i++) {
        jsArray.set((int)i, primes[i]);
    }

    jsCallback(jsArray);
    std::cout << "[C++] runPrimes finished\n";
}

EMSCRIPTEN_BINDINGS(primes_module) {
    emscripten::function("runPrimes", &runPrimes);
}
