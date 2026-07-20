#ifndef ORDER_H
#define ORDER_H

#include <cstdint>

enum class OrderType {
    LIMIT,
    MARKET,
    CANCEL
};

struct Order {
    int id;
    bool isBuy;
    OrderType type;
    int price;
    int qty;
    uint64_t timestamp;
};

#endif