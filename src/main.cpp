#include "../include/orderbook.h"
#include "../include/FastIngestionPipeline.h"

#include <iostream>
#include <chrono>

using namespace std;
using namespace std::chrono;
#include "../include/SPSCRingBuffer.h"


int main(int argc, char* argv[])
{
    SPSCRingBuffer<int, 8> q;
    
    if(argc != 2)
    {
        cout << "Usage: " << argv[0] << " <csv_file>\n";
        return 1;
    }

    
    OrderBook engine;
    
    FastIngestionPipeline parser(engine);
    
    auto start = high_resolution_clock::now();
    bool ok = parser.ingestCSV(argv[1]);

    auto end = high_resolution_clock::now();
    if(!ok)
    {
        cout << "Failed to read file.\n";
        return 1;
    }

    auto duration =
        duration_cast<milliseconds>(end - start);

    cout << "Time : " << duration.count() << " ms\n";
    cout << "\nFinal Order Book:\n";
    engine.printBook();

    return 0;
}