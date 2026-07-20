#ifndef FAST_INGESTION_PIPELINE_H
#define FAST_INGESTION_PIPELINE_H
#define MEASURE_LATENCY

#include <cstdint>
#include <string>
#include <chrono>
#include <thread>
#include <atomic>

#include "orderbook.h"
#include "FlatHashMap.h"
#include "SPSCRingBuffer.h"

#include <vector>
#include <cstdint>


struct OrderMessage
{
    uint64_t timestamp;
    
    #ifdef MEASURE_LATENCY
    std::chrono::high_resolution_clock::time_point startTime;
    #endif

    int externalID;

    char msgType;

    bool isBuy;

    int price;

    int qty;
};

class FastIngestionPipeline
{
private:

    OrderBook& engine;

    FlatHashMap idMap;

    size_t processedOrders = 0;

    SPSCRingBuffer<OrderMessage, 1024> ringBuffer;

    std::vector<uint64_t> latencies;

    std::atomic<bool> finished{false};

public:
    const std::vector<uint64_t>& getLatencies() const
    {
        return latencies;
    }

    void processQueue();

    size_t getProcessedOrders() const
    {
        return processedOrders;
    }

    FastIngestionPipeline(OrderBook& book);

    bool ingestCSV(const std::string& filename);
};

#endif