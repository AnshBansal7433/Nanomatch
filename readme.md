# NanoMatch - High-Performance Order Matching Engine

## Overview

NanoMatch is a high-performance C++ order matching engine designed to process large volumes of financial orders with low latency and high throughput. The engine implements price-time priority (FIFO) matching while focusing on cache-efficient memory management, high-speed data ingestion, and lock-free communication between components.

The project is built using modern C++ and demonstrates techniques commonly used in real-world electronic trading systems.

---

## Features

- High-throughput CSV ingestion using memory-mapped files
- Lock-free Single Producer Single Consumer (SPSC) ring buffer
- Price-Time Priority (FIFO) order matching
- Support for:
  - Limit Orders
  - Market Orders
  - Order Cancellation
- Cache-optimized custom memory pool
- Queue pool for efficient price level management
- Custom Flat Hash Map for fast Order ID lookup
- Cross-platform support (Windows and Linux)
- Built-in benchmarking framework
- End-to-end latency measurement
- Throughput and percentile latency statistics

---

## System Architecture

```
                 +----------------------+
                 |   Memory Mapped CSV  |
                 +----------+-----------+
                            |
                            v
               +-------------------------+
               | Fast Ingestion Pipeline |
               +-----------+-------------+
                           |
                           v
        +---------------------------------------+
        | Lock-Free SPSC Ring Buffer            |
        +----------------+----------------------+
                         |
                         v
               +----------------------+
               |  Order Matching      |
               |      Engine          |
               +----------+-----------+
                          |
          +---------------+----------------+
          |                                |
          v                                v
  Custom Memory Pool               Flat Hash Map
          |                                |
          +---------------+----------------+
                          |
                          v
                 Price Level Queues
```

---

## Supported Order Types

### Limit Order

Matches against the opposite side of the book while respecting price-time priority. Any remaining quantity is inserted into the order book.

### Market Order

Immediately executes against the best available prices until completely filled or the book becomes empty.

### Cancel Order

Cancels an existing order in O(1) average time using the custom Flat Hash Map.

---

## Performance Optimizations

### Memory-Mapped File Parsing

Instead of traditional file I/O, the engine uses:

- Windows: `CreateFileMapping()`
- Linux: `mmap()`

This minimizes copying and significantly improves parsing throughput.

---

### Lock-Free SPSC Ring Buffer

Orders are transferred from the ingestion pipeline to the matching engine through a lock-free Single Producer Single Consumer ring buffer.

Benefits:

- No mutexes
- No locks
- Constant-time enqueue/dequeue
- Low synchronization overhead

---

### Custom Memory Pool

Orders are allocated from a contiguous memory pool.

Advantages:

- Eliminates repeated heap allocations
- Improves cache locality
- Constant-time allocation and deallocation

---

### Queue Pool

Price levels reuse queue storage to reduce dynamic memory allocation and improve memory efficiency.

---

### Flat Hash Map

A custom Flat Hash Map is used for Order ID lookup, enabling fast insertion, lookup, and deletion while avoiding the overhead of standard hash table implementations.

---

## Matching Algorithm

The engine follows the standard **Price-Time Priority (FIFO)** policy.

Priority rules:

1. Better price executes first.
2. Orders at the same price execute in arrival order.
3. Partial fills preserve FIFO ordering.

---

## Benchmark Metrics

The benchmarking framework reports:

- Orders Processed
- Total Execution Time
- Throughput (orders/sec)
- Average Latency
- 50th Percentile (P50)
- 90th Percentile (P90)
- 99th Percentile (P99)

---

## Complexity Analysis

| Operation | Complexity |
|-----------|------------|
| Add Limit Order | O(log P) |
| Market Order | O(k) |
| Cancel Order | O(1) average |
| Order Lookup | O(1) average |
| Ring Buffer Push | O(1) |
| Ring Buffer Pop | O(1) |

Where:

- **P** = Number of active price levels
- **k** = Number of matched orders

---

## Technologies Used

- C++17
- STL
- Atomics
- Threads
- Memory-Mapped Files
- Lock-Free Programming

---

## Build

### Prerequisites

- C++17 compatible compiler (GCC, Clang, or MSVC)
- CMake 3.16 or later

### Clone the repository

```bash
git clone <repository-url>
cd NanoMatch
```

### Configure

```bash
cmake -S . -B build
```

### Build

```bash
cmake --build build --config Release
```

### Run Benchmark

```bash
./build/NanoBenchmark ../orders.csv
```

On Windows (Visual Studio generator):

```powershell
.\build\Release\NanoBenchmark.exe ..\orders.csv
```

### Run Tests

```bash
./build/NanoMatch.exe ../orders.csv
```

On Windows:

```powershell
.\build\Release\NanoMatch.exe ..\orders.csv
```
### Generating dataset

```bash
./build/Generator.exe ../orders.csv 1000000
```

On Windows:

```powershell
.\build\Release\Generator.exe ..\orders.csv 1000000
```
(the number is for number of lines in CSV, 1000000 to generate csv of 1 million lines)

---

## Project Structure

```
include/
    orderbook.h
    MemoryPool.h
    FlatHashMap.h
    SPSCRingBuffer.h
    FastIngestionPipeline.h

src/
    orderbook.cpp
    MemoryPool.cpp
    FlatHashMap.cpp
    FastIngestionPipeline.cpp

benchmark/
    benchmark.cpp
```

---
## Performance Profiling

The NanoMatch engine was profiled for 1,000,000 orders using Intel VTune Profiler on an Intel Raptor Lake-DT architecture to validate performance claims. Please refer to the /performance_analysis folder to view the full VTune summaries, event counts, and high-resolution Flame Graphs.

### Key Performance Metrics

| Metric | Result | Interpretation |
| :--- | :--- | :--- |
| **CPI Rate** | **0.514** | High Instruction Level Parallelism (~2 instructions/cycle). |
| **Retiring Rate** | **52.1%** | Excellent pipeline efficiency with minimal branch stalls. |
| **L1 Bound** | **10.5%** | Highly cache-efficient; minimal L1 cache misses. |
| **DRAM Bound** | **3.9%** | Optimized memory access; avoided slow system RAM. |

### Architectural Insights

*   **Cache-Friendly Design:** The **L1 Bound (10.5%)** and **L2 Bound (0.0%)** metrics confirm that the contiguous memory pool architecture effectively keeps the working set within the CPU cache hierarchy.
*   **Deterministic Execution:** Flame Graph analysis confirms that the critical matching path is allocation-free (`malloc`/`free` calls are isolated to ingestion/logging), ensuring no non-deterministic latency spikes.
*   **Core Efficiency:** The engine is primarily **Core Bound (20.9%)**, demonstrating that the primary latency driver is the matching logic itself rather than hardware bottlenecks.



---

## Author

**Ansh Bansal**

Department of Mathematics and Computing  
Indian Institute of Technology Guwahati