#include "../include/orderbook.h"
#include "../include/FastIngestionPipeline.h"

#include <chrono>
#include <iomanip>
#include <algorithm>
#include <iostream>

using namespace std;
using namespace std::chrono;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <orders.csv>\n";
        return 1;
    }

    OrderBook engine;
    FastIngestionPipeline parser(engine);

    auto start = high_resolution_clock::now();

    bool ok = parser.ingestCSV(argv[1]);

    auto end = high_resolution_clock::now();

    if (!ok)
    {
        cerr << "Failed to process CSV.\n";
        return 1;
    }

    long long totalTime =
        duration_cast<nanoseconds>(end - start).count();

    size_t orders = parser.getProcessedOrders();

    auto latencies = parser.getLatencies();

    cout << "Processed Orders = " << orders << '\n';
cout << "Latency Samples  = " << latencies.size() << '\n';
    sort(latencies.begin(), latencies.end());

    auto percentile = [&](double p)
    {
        size_t idx = static_cast<size_t>(p * latencies.size());

        if (idx >= latencies.size())
            idx = latencies.size() - 1;

        return latencies[idx];
    };

    double seconds = totalTime / 1e9;

    cout << fixed << setprecision(3);

    cout << "\n=========================================\n";
    cout << " NanoMatch Benchmark\n";
    cout << "=========================================\n";

    cout << "Orders Processed : " << orders << '\n';
    cout << "Total Time       : " << totalTime << " ns\n";
    cout << "                 : " << totalTime / 1000.0 << " us\n";
    cout << "                 : " << totalTime / 1e6 << " ms\n";
    cout << "                 : " << seconds << " sec\n";

    if (orders)
    {
        cout << "Throughput       : "
             << orders / seconds
             << " orders/sec\n";
        uint64_t sum = 0;

        for (uint64_t x : latencies)
            sum += x;

        double avgLatency =
            (double)sum / latencies.size();
        cout << "Average Latency  : "
             << avgLatency
             << " ns/order\n";
    }

    cout << "p50 Latency      : "
        << percentile(0.50)
        << " ns\n";

    cout << "p90 Latency      : "
        << percentile(0.90)
        << " ns\n";

    cout << "p99 Latency      : "
        << percentile(0.99)
        << " ns\n";

    cout << "=========================================\n";

    return 0;
}