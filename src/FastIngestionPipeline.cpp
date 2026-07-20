#include "../include/FastIngestionPipeline.h"

#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#endif

using namespace std;

static inline void skipDelimiter(const char *&ptr)
{
    while (*ptr == ',' || *ptr == '\n' || *ptr == '\r')
        ptr++;
}

static inline uint64_t parseUInt64(const char *&ptr)
{
    uint64_t x = 0;

    while (*ptr >= '0' && *ptr <= '9')
    {
        x = x * 10 + (*ptr - '0');
        ptr++;
    }

    skipDelimiter(ptr);
    return x;
}

static inline int parseInt(const char *&ptr)
{
    bool neg = false;

    if (*ptr == '-')
    {
        neg = true;
        ptr++;
    }

    int x = 0;

    while (*ptr >= '0' && *ptr <= '9')
    {
        x = x * 10 + (*ptr - '0');
        ptr++;
    }

    skipDelimiter(ptr);

    return neg ? -x : x;
}

static inline char parseChar(const char *&ptr)
{
    char c = *ptr;

    while (*ptr != ',' &&
           *ptr != '\n' &&
           *ptr != '\r' &&
           *ptr != '\0')
        ptr++;

    skipDelimiter(ptr);

    return c;
}

FastIngestionPipeline::FastIngestionPipeline(OrderBook &book)
    : engine(book)
{
#ifdef MEASURE_LATENCY
    latencies.reserve(1000000);
#endif
}

void FastIngestionPipeline::processQueue()
{
    OrderMessage msg;

    while (!finished || !ringBuffer.empty())
    {
        if(!ringBuffer.pop(msg))
        {
            std::this_thread::yield();
            continue;
        }
        Order order;

        order.id = -1;
        order.timestamp = msg.timestamp;
        order.isBuy = msg.isBuy;
        order.price = msg.price;
        order.qty = msg.qty;

        if (msg.msgType == 'A')
        {
            order.type = OrderType::LIMIT;

            int internalID;

            if (order.isBuy)
                internalID = engine.addBuy(order);
            else
                internalID = engine.addSell(order);

            if (internalID != -1)
                idMap.insert(msg.externalID, internalID);
        }
        else if (msg.msgType == 'C')
        {
            int* internalID = idMap.find(msg.externalID);

            if (internalID != nullptr)
            {
                engine.cancel(*internalID);
                idMap.erase(msg.externalID);
            }
        }
        else if (msg.msgType == 'M')
        {
            if (msg.price != -1)
                continue;

            order.type = OrderType::MARKET;

            if (order.isBuy)
                engine.addMarketBuy(order);
            else
                engine.addMarketSell(order);
        }
        #ifdef MEASURE_LATENCY
        auto end = std::chrono::high_resolution_clock::now();

        latencies.push_back(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                end - msg.startTime
            ).count()
        );
        #endif
    }

}


bool FastIngestionPipeline::ingestCSV(const string &filename)
{
    
    finished = false;
    processedOrders = 0;
    #ifdef MEASURE_LATENCY
    latencies.clear();
    #endif

    std::thread consumer(
        &FastIngestionPipeline::processQueue,
        this
    );
    
#ifdef _WIN32

    HANDLE hFile = CreateFileA(
        filename.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        finished = true;
        consumer.join();
        return false;
    }

    LARGE_INTEGER fileSize;
    GetFileSizeEx(hFile, &fileSize);

    HANDLE hMap = CreateFileMapping(
        hFile,
        NULL,
        PAGE_READONLY,
        0,
        0,
        NULL);

    if (!hMap)
    {
        CloseHandle(hFile);
        finished = true;
        consumer.join();
        return false;
    }

    const char *data =
        (const char *)MapViewOfFile(
            hMap,
            FILE_MAP_READ,
            0,
            0,
            0);

    if (!data)
    {
        CloseHandle(hMap);
        CloseHandle(hFile);
        finished = true;
        consumer.join();
        return false;
    }

    const char *ptr = data;
    const char *end = data + fileSize.QuadPart;

#else

    int fd = open(filename.c_str(), O_RDONLY);

    if (fd == -1)
    {
        finished = true;
        consumer.join();
        return false;
    }

    struct stat sb;

    if (fstat(fd, &sb) == -1)
    {
        close(fd);
        finished = true;
        consumer.join();
        return false;
    }

    size_t fileSize = sb.st_size;

    const char *data =
        (const char *)mmap(
            nullptr,
            fileSize,
            PROT_READ,
            MAP_PRIVATE,
            fd,
            0);

    if (data == MAP_FAILED)
    {
        close(fd);
        finished = true;
        consumer.join();
        return false;
    }

    const char *ptr = data;
    const char *end = data + fileSize;

#endif

    while (ptr < end && *ptr != '\n')
        ptr++;

    while (ptr < end && (*ptr == '\n' || *ptr == '\r'))
        ptr++;

    while (ptr < end)
    {
        uint64_t timestamp = parseUInt64(ptr);
        uint64_t externalID = parseUInt64(ptr);

        char msgType = parseChar(ptr);
        char side = parseChar(ptr);

        int price = parseInt(ptr);
        int qty = parseInt(ptr);


        OrderMessage msg;

        msg.timestamp = timestamp;
        msg.externalID = externalID;
        msg.msgType = msgType;
        msg.isBuy = (side == 'B');
        msg.price = price;
        msg.qty = qty;
        #ifdef MEASURE_LATENCY
        msg.startTime = std::chrono::high_resolution_clock::now();
        #endif
        while (!ringBuffer.push(msg))
        {
            std::this_thread::yield();
        }


        processedOrders++;

        while (ptr < end &&
               (*ptr == '\n' || *ptr == '\r'))
            ptr++;
    }

#ifdef _WIN32

    UnmapViewOfFile(data);
    CloseHandle(hMap);
    CloseHandle(hFile);

#else

    munmap((void *)data, fileSize);
    close(fd);

#endif
    finished = true;

    consumer.join();

    return true;
}