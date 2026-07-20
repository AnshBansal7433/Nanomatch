#include "../include/orderbook.h"
#include <iostream>

using namespace std;

OrderBook::OrderBook() {

    orderPool = new Order[MAX_ORDERS];

    queuePool = new OrderQueue[MAX_PRICE_LEVELS];

    for (int i = 0; i < MAX_PRICE_LEVELS; i++)
    {
        queuePool[i].orderIDs = new int[MAX_ORDERS_PER_PRICE];
        queuePool[i].head = 0;
        queuePool[i].tail = 0;
        queuePool[i].size = 0;
    }

    buyBook = new PriceBook();
    sellBook = new PriceBook();

    freeQueueList = new int[MAX_PRICE_LEVELS];
    freeList = new int[MAX_ORDERS];

    for(int i = 0; i < MAX_ORDERS; i++)
        freeList[i] = MAX_ORDERS - 1 - i;

    freeTop = MAX_ORDERS;

    buyBook->size = 0;
    sellBook->size = 0;

    for(int i = 0; i < MAX_PRICE_LEVELS; i++)
        freeQueueList[i] = MAX_PRICE_LEVELS - 1 - i;

    freeQueueTop = MAX_PRICE_LEVELS;
}

int OrderBook::allocateOrder(Order order) {

    if(freeTop == 0) {
        cout << "ERROR : Order Pool Full\n";
        return -1;
    }

    int id = freeList[--freeTop];

    order.id = id;

    orderPool[id] = order;

    return id;
}

void OrderBook::freeOrder(int orderID) {

    orderPool[orderID].qty = 0;
    orderPool[orderID].type = OrderType::CANCEL;

    freeList[freeTop++] = orderID;
}

bool OrderBook::queueEmpty(int queueID) {

    return queuePool[queueID].size == 0;
}

int OrderBook::queueFront(int queueID) {
    if (queuePool[queueID].size == 0)
        return -1;
    return queuePool[queueID].orderIDs[queuePool[queueID].head];
}

void OrderBook::queuePush(int queueID, int orderID) {

    OrderQueue &q = queuePool[queueID];

    if (q.size == MAX_ORDERS_PER_PRICE)
    {
        std::cout << "Queue Full\n";
        return;
    }
    q.orderIDs[q.tail] = orderID;

    q.tail++;

    if(q.tail == MAX_ORDERS_PER_PRICE)
        q.tail = 0;

    q.size++;
}

void OrderBook::queuePop(int queueID) {

    OrderQueue &q = queuePool[queueID];
    if (q.size == MAX_ORDERS_PER_PRICE)
    {
        std::cout << "Queue Full\n";
        return;
    }

    q.head++;

    if(q.head == 0)
        q.head = 0;

    q.size--;
}

int OrderBook::getBuyPriceLevel(int price) {

    int pos = 0;

    while(pos < buyBook->size &&
          buyBook->levels[pos].price > price)
        pos++;

    if(pos < buyBook->size &&
       buyBook->levels[pos].price == price)
        return pos;

    for(int i = buyBook->size; i > pos; i--)
        buyBook->levels[i] = buyBook->levels[i - 1];

    buyBook->levels[pos].price = price;

    if(freeQueueTop == 0) {
        cout << "ERROR : Queue Pool Full\n";
        return -1;
    }

    int queueID = freeQueueList[--freeQueueTop];

    buyBook->levels[pos].queueID = queueID;

    buyBook->size++;

    return pos;
}

int OrderBook::getSellPriceLevel(int price) {

    int pos = 0;

    while(pos < sellBook->size &&
          sellBook->levels[pos].price < price)
        pos++;

    if(pos < sellBook->size &&
       sellBook->levels[pos].price == price)
        return pos;

    for(int i = sellBook->size; i > pos; i--)
        sellBook->levels[i] = sellBook->levels[i - 1];

    sellBook->levels[pos].price = price;
    if(freeQueueTop == 0) {
        cout << "ERROR : Queue Pool Full\n";
        return -1;
    }

    int queueID = freeQueueList[--freeQueueTop];

    sellBook->levels[pos].queueID = queueID;

    sellBook->size++;

    return pos;
}

void OrderBook::removeBuyPriceLevel(int idx) {

    int queueID = buyBook->levels[idx].queueID;

    queuePool[queueID].head = 0;
    queuePool[queueID].tail = 0;
    queuePool[queueID].size = 0;

    freeQueueList[freeQueueTop++] = queueID;

    for(int i = idx; i < buyBook->size - 1; i++)
        buyBook->levels[i] = buyBook->levels[i + 1];

    buyBook->size--;
}

void OrderBook::removeSellPriceLevel(int idx) {

    int queueID = sellBook->levels[idx].queueID;

    queuePool[queueID].head = 0;
    queuePool[queueID].tail = 0;
    queuePool[queueID].size = 0;

    freeQueueList[freeQueueTop++] = queueID;

    for(int i = idx; i < sellBook->size - 1; i++)
        sellBook->levels[i] = sellBook->levels[i + 1];

    sellBook->size--;
}

int OrderBook::addBuy(Order order) {

    int orderID = allocateOrder(order);

    if(orderID == -1)
        return -1;

    Order &buyOrder = orderPool[orderID];

    while(buyOrder.qty > 0 && sellBook->size > 0) {

        PriceLevel &bestSell = sellBook->levels[0];
        int queueID = bestSell.queueID;

        while(!queueEmpty(queueID)) {

            int sellID = queueFront(queueID);
            Order &sellOrder = orderPool[sellID];

            if(sellOrder.type == OrderType::CANCEL) {
                queuePop(queueID);
                freeOrder(sellID);
                continue;
            }

            if(sellOrder.price > buyOrder.price)
                break;

            int tradedQty = min(buyOrder.qty, sellOrder.qty);


            buyOrder.qty -= tradedQty;
            sellOrder.qty -= tradedQty;

            if(sellOrder.qty == 0) {
                queuePop(queueID);
                freeOrder(sellID);
            }

            if(buyOrder.qty == 0)
                break;
        }

        if(queueEmpty(queueID))
            removeSellPriceLevel(0);
        else
            break;
    }

    if(buyOrder.qty > 0) {

        int idx = getBuyPriceLevel(buyOrder.price);
        int queueID = buyBook->levels[idx].queueID;

        queuePush(queueID, orderID);
    }
    else {
        freeOrder(orderID);
    }
    return orderID;
}

int OrderBook::addSell(Order order) {

    int orderID = allocateOrder(order);

    if(orderID == -1)
        return -1;

    Order &sellOrder = orderPool[orderID];

    while(sellOrder.qty > 0 && buyBook->size > 0) {

        PriceLevel &bestBuy = buyBook->levels[0];
        int queueID = bestBuy.queueID;

        while(!queueEmpty(queueID)) {

            int buyID = queueFront(queueID);
            Order &buyOrder = orderPool[buyID];

            if(buyOrder.type == OrderType::CANCEL) {
                queuePop(queueID);
                freeOrder(buyID);
                continue;
            }

            if(buyOrder.price < sellOrder.price)
                break;

            int tradedQty = min(sellOrder.qty, buyOrder.qty);

            sellOrder.qty -= tradedQty;
            buyOrder.qty -= tradedQty;

            if(buyOrder.qty == 0) {
                queuePop(queueID);
                freeOrder(buyID);
            }

            if(sellOrder.qty == 0)
                break;
        }

        if(queueEmpty(queueID))
            removeBuyPriceLevel(0);
        else
            break;
    }

    if(sellOrder.qty > 0) {

        int idx = getSellPriceLevel(sellOrder.price);
        int queueID = sellBook->levels[idx].queueID;

        queuePush(queueID, orderID);
    }
    else {
        freeOrder(orderID);
    }
    return orderID;
}


void OrderBook::addMarketBuy(Order order) {

    Order &buyOrder = order;

    while(buyOrder.qty > 0 && sellBook->size > 0) {

        PriceLevel &bestSell = sellBook->levels[0];
        int queueID = bestSell.queueID;

        while(!queueEmpty(queueID)) {

            int sellID = queueFront(queueID);
            Order &sellOrder = orderPool[sellID];

            if(sellOrder.type == OrderType::CANCEL) {
                queuePop(queueID);
                freeOrder(sellID);
                continue;
            }

            int tradedQty = min(buyOrder.qty, sellOrder.qty);


            buyOrder.qty -= tradedQty;
            sellOrder.qty -= tradedQty;

            if(sellOrder.qty == 0) {
                queuePop(queueID);
                freeOrder(sellID);
            }

            if(buyOrder.qty == 0)
                break;
        }

        if(queueEmpty(queueID))
            removeSellPriceLevel(0);
        else
            break;
    }

}

void OrderBook::addMarketSell(Order order) {

    Order &sellOrder = order;

    while(sellOrder.qty > 0 && buyBook->size > 0) {

        PriceLevel &bestBuy = buyBook->levels[0];
        int queueID = bestBuy.queueID;

        while(!queueEmpty(queueID)) {

            int buyID = queueFront(queueID);
            Order &buyOrder = orderPool[buyID];

            if(buyOrder.type == OrderType::CANCEL) {
                queuePop(queueID);
                freeOrder(buyID);
                continue;
            }

            int tradedQty = min(sellOrder.qty, buyOrder.qty);


            sellOrder.qty -= tradedQty;
            buyOrder.qty -= tradedQty;

            if(buyOrder.qty == 0) {
                queuePop(queueID);
                freeOrder(buyID);
            }

            if(sellOrder.qty == 0)
                break;
        }

        if(queueEmpty(queueID))
            removeBuyPriceLevel(0);
        else
            break;
    }
}

void OrderBook::cancel(int orderID)
{
    if (orderID < 0 || orderID >= MAX_ORDERS)
        return;

    Order &order = orderPool[orderID];

    if (order.qty == 0)
        return;

    if (order.type == OrderType::CANCEL)
        return;

    order.type = OrderType::CANCEL;
}

void OrderBook::printBook() {
    
    cout << "\n========== BUY BOOK ==========\n";

    for(int i = 0; i < buyBook->size; i++) {

        PriceLevel &level = buyBook->levels[i];
        OrderQueue &q = queuePool[level.queueID];

        cout << "Price " << level.price << " : ";

        int idx = q.head;
        int cnt = q.size;

        while(cnt--) {

            int orderID = q.orderIDs[idx];
            Order &order = orderPool[orderID];

            if(order.type != OrderType::CANCEL && order.qty > 0)
                cout << "[" << order.qty << "] ";

            idx++;
            if(idx == MAX_ORDERS_PER_PRICE)
                idx = 0;
        }

        cout << "\n";
    }

    cout << "\n========== SELL BOOK ==========\n";

    for(int i = 0; i < sellBook->size; i++) {

        PriceLevel &level = sellBook->levels[i];
        OrderQueue &q = queuePool[level.queueID];

        cout << "Price " << level.price << " : ";

        int idx = q.head;
        int cnt = q.size;

        while(cnt--) {

            int orderID = q.orderIDs[idx];
            Order &order = orderPool[orderID];

            if(order.type != OrderType::CANCEL && order.qty > 0)
                cout << "["  << order.qty << "] ";

            idx++;
            if(idx == MAX_ORDERS_PER_PRICE)
                idx = 0;
        }

        cout << "\n";
    }

    cout << "==============================\n";
}

OrderBook::~OrderBook()
{
    for (int i = 0; i < MAX_PRICE_LEVELS; i++)
        delete[] queuePool[i].orderIDs;

    delete[] queuePool;
    delete[] orderPool;

    delete buyBook;
    delete sellBook;

    delete[] freeQueueList;
    delete[] freeList;
}