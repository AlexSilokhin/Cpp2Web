cmake_minimum_required(VERSION 3.13)
project(PrimesWasm CXX)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# You can pick which source file to use:
#   primes_stream.cpp  → Sieve of Eratosthenes (streamed)
#   primes_naive.cpp   → Naïve trial division (streamed)
#set(SRC primes_stream.cpp)
#set(SRC primes_naive.cpp)
#set(SRC primes_emscripten.cpp)
#set(SRC primes_post_message.cpp)
set(SRC primes_mt.cpp)

# Output will be primes.js / primes.wasm
set(TARGET primes)

add_executable(${TARGET} ${SRC})

# Emscripten-specific compile and link flags
set_target_properties(${TARGET} PROPERTIES
    OUTPUT_NAME "primes"
    LINK_FLAGS " \
      -O2 \
      -lembind \
      -s USE_PTHREADS=1 \
      -s PTHREAD_POOL_SIZE=4 \
      -s ALLOW_MEMORY_GROWTH=1 \
      -s MODULARIZE=1 \
      -s EXPORT_NAME=PrimesModule \
    "
)
