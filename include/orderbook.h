#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include "order.h"

const int MAX_ORDERS = 1000000;
const int MAX_PRICE_LEVELS = 10000;
const int MAX_ORDERS_PER_PRICE = 1000000;

struct OrderQueue
{
    int* orderIDs = nullptr;
    int head = 0;
    int tail = 0;
    int size = 0;
};

struct PriceLevel {

    int price;
    int queueID;
};

struct PriceBook {

    PriceLevel levels[MAX_PRICE_LEVELS];
    int size = 0;
};

class OrderBook {

private:

    PriceBook* buyBook;
    
    PriceBook* sellBook;
    
    Order* orderPool;

    OrderQueue* queuePool;
    
    int* freeQueueList;
    int freeQueueTop;

    int* freeList;
    int freeTop;

    int allocateOrder(Order order);
    void freeOrder(int orderID);

    bool queueEmpty(int queueID);
    int queueFront(int queueID);
    void queuePush(int queueID, int orderID);
    void queuePop(int queueID);

    int getBuyPriceLevel(int price);
    int getSellPriceLevel(int price);

    void removeBuyPriceLevel(int idx);
    void removeSellPriceLevel(int idx);


public:

    OrderBook();
    ~OrderBook();

    int addBuy(Order order);
    int addSell(Order order);

    void addMarketBuy(Order order);
    void addMarketSell(Order order);

    void cancel(int orderID);

    void printBook();
};

#endif