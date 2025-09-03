#include <vector>
//#include <cmath>
#include <thread>
#include <iostream>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <emscripten/threading.h>

struct AsyncTask {
    int start;
    int end;
    int threadId;
    int totalThreads;
};

// Worker function that finds primes in a range and adds them to JS array
void primes_worker(AsyncTask* task) {
    std::vector<int> localPrimes;
    
    // Find primes in this thread's range
    for (int num = task->start; num < task->end; ++num) {
        if (num < 2) continue;
        
        bool isPrime = true;
        for (int p = 2; p * p <= num; ++p) {
            if (num % p == 0) {
                isPrime = false;
                break;
            }
        }
        
        if (isPrime) {
            localPrimes.push_back(num);
        }
    }
    
    // Add found primes to the JS module's prime collection
    emscripten_async_run_in_main_runtime_thread(
        EM_FUNC_SIG_VIII, // void(int*, int, int)
        (void*)+[](int* primesPtr, int primesCount, int threadId) {
            EM_ASM_({
                if (!Module.primeNumbers) Module.primeNumbers = [];
                
                for (let i = 0; i < $1; i++) {
                    let prime = getValue($0 + i * 4, 'i32');
                    Module.primeNumbers.push(prime);
                }
                
                if (Module.onThreadFinished) {
                    Module.onThreadFinished($2, $1);
                }
            }, primesPtr, primesCount, threadId);
        },
        localPrimes.data(), localPrimes.size(), task->threadId
    );
    
    delete task;
}

void runPrimesAsyncNonBlocking(int N, int numThreads, emscripten::val jsCallback) {
    // Just reset the counters - don't store the callback since we use onThreadFinished
    EM_ASM_({
        Module.primeNumbers = [];
        Module.threadsFinished = 0;
    });

    numThreads = std::max(numThreads, 1);

    std::cout << "[C++] Number threads: " << numThreads << std::endl;

    // Launch worker threads
    int chunk = N / numThreads;
    
    for (int t = 0; t < numThreads; ++t) {
        AsyncTask* task = new AsyncTask();
        task->start = t * chunk;
        task->end = (t == numThreads - 1) ? N : task->start + chunk;
        task->threadId = t;
        task->totalThreads = numThreads;
        
        std::thread(primes_worker, task).detach();
    }
}

EMSCRIPTEN_BINDINGS(primes_async_nonblocking) {
    emscripten::function("runPrimesAsyncNonBlocking", &runPrimesAsyncNonBlocking);
}
