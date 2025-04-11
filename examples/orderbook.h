// MatchingEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <map>
#include <set>
#include <list>
#include <cmath>
#include <ctime>
#include <deque>
#include <queue>
#include <stack>
#include <string>
#include <bitset>
#include <cstdio>
#include <limits>
#include <vector>
#include <climits>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <numeric>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <memory>
#include <variant>
#include <optional>
#include <tuple>
#include <charconv>

enum class OrderType
{
    GoodForDay,
    InsertOrCancel,
};

enum Side
{
    Buy,
    Sell
};

using Price = std::uint32_t;
using Priority = std::uint32_t;
using Quantity = std::uint32_t;
using OrderId = std::string;
using OrderIds = std::vector<OrderId>;

struct LevelInfo
{
    Price price_;
    Quantity quantity_;
};

using LevelInfos = std::vector<LevelInfo>;

class OrderbookInfos
{
public:

    OrderbookInfos(const LevelInfos& buys, const LevelInfos& sells)
        : buys_{ buys }, sells_{ sells } { }

    const LevelInfos& GetBuys() const { return buys_; }
    const LevelInfos& GetSells() const { return sells_; }
private:
    LevelInfos buys_;
    LevelInfos sells_;
};

class Order
{
public:

    Order(OrderType orderType, OrderId orderId, Side side, Price price, Quantity quantity) :
        orderType_{ orderType }, orderId_{ orderId }, price_{ price }, side_{ side }, initialQuantity_{ quantity }, remainingQuantity_{ quantity }
    { }

    OrderId GetId() const { return orderId_; }
    Side GetSide() const { return side_; }
    Quantity GetInitialQuantity() const { return initialQuantity_; }
    Quantity GetRemainingQuantity() const { return remainingQuantity_; }
    void Fill(Quantity quantity)
    {
        if (quantity > remainingQuantity_)
            throw std::logic_error("Cannot fill an order for more than its quantity.");

        remainingQuantity_ -= quantity;
    }
    Price GetPrice() const { return price_; }
    OrderType GetType() const { return orderType_; }
    Priority GetPriority() const { return priority_; }

private:

    OrderType orderType_;
    OrderId orderId_;
    Price price_;
    Side side_;
    Quantity initialQuantity_;
    Quantity remainingQuantity_;
    Priority priority_{ OrderPriority++ }; // Unused

    static Priority OrderPriority;
};

Priority Order::OrderPriority = 0;

using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::vector<OrderPointer>;
using OrderPointerList = std::list<OrderPointer>;

class OrderModify
{
public:
    OrderModify(OrderId orderId, Side side, Price price, Quantity quantity) :
        orderId_{ orderId }, price_{ price }, side_{ side }, quantity_{ quantity } { }

    OrderId GetId() const { return orderId_; }
    Price GetPrice() const { return price_; }
    Side GetSide() const { return side_; }
    Quantity GetQuantity() const { return quantity_; }

    OrderPointer ToOrderPointer(OrderType type) const
    {
        return std::make_shared<Order>(type, GetId(), GetSide(), GetPrice(), GetQuantity());
    }

private:

    OrderId orderId_;
    Price price_;
    Side side_;
    Quantity quantity_;
};

struct TradeInfo
{
    OrderId orderId_;
    Price price_;
    Quantity quantity_;
    Priority priority_;
};

class Trade
{
public:
    Trade(const TradeInfo& tradeInfoOne, const TradeInfo& tradeInfoTwo) : tradeInfoOne_{ tradeInfoOne }, tradeInfoTwo_{ tradeInfoTwo }
    {
        if (tradeInfoOne.priority_ > tradeInfoTwo.priority_)
            throw std::logic_error("Incorrect priority.");
    }

    const TradeInfo& GetFirstTrade() const { return tradeInfoOne_; }
    const TradeInfo& GetSecondTrade() const { return tradeInfoTwo_; }

private:
    TradeInfo tradeInfoOne_;
    TradeInfo tradeInfoTwo_;
};

using Trades = std::vector<Trade>;

class Orderbook
{
public:
    Trades AddOrder(OrderPointer order)
    {
        if (orders_.count(order->GetId()))
            return { };

        if (order->GetType() == OrderType::InsertOrCancel && !CanMatchOrder(order->GetSide(), order->GetPrice()))
            return { };

        OrderPointerList::iterator iterator;

        if (order->GetSide() == Side::Buy)
        {
            auto& orders = buyOrders_[order->GetPrice()];
            orders.push_back(order);
            iterator = std::next(orders.begin(), orders.size() - 1);
        }
        else
        {
            auto& orders = sellOrders_[order->GetPrice()];
            orders.push_back(order);
            iterator = std::next(orders.begin(), orders.size() - 1);
        }

        orders_.insert({ order->GetId(), {order, iterator} });
        return MatchOrders();
    }


    Trades ModifyOrder(OrderModify order)
    {
        if (!orders_.count(order.GetId()))
            return { };

        const auto [currentOrder, _] = orders_.at(order.GetId());

        CancelOrder(order.GetId());
        const auto& trades = AddOrder(order.ToOrderPointer(currentOrder->GetType()));
        return trades;
    }

    void CancelOrder(OrderId orderId)
    {
        if (!orders_.count(orderId))
            return;

        const auto [order, orderIterator] = orders_.at(orderId);
        orders_.erase(orderId);

        auto IsOrder = [orderId](const OrderPointer& order) { return order->GetId() == orderId; };

        if (order->GetSide() == Side::Sell)
        {
            auto price = order->GetPrice();
            auto& orders = sellOrders_.at(price);
            orders.erase(orderIterator);
            if (orders.empty())
                sellOrders_.erase(price);
        }
        else
        {
            auto price = order->GetPrice();
            auto& orders = buyOrders_.at(price);
            orders.erase(orderIterator);
            if (orders.empty())
                buyOrders_.erase(price);
        }
    }

    OrderbookInfos GetOrderInfos() const
    {
        LevelInfos sellOrderInfos, buyOrderInfos;
        sellOrderInfos.reserve(orders_.size());
        buyOrderInfos.reserve(orders_.size());

        auto CreateLevelInfo = [](Price price, const OrderPointerList& orders)
        {
            return LevelInfo{ price, std::accumulate(orders.begin(), orders.end(), (Quantity)0, [](std::size_t runningSum, const OrderPointer& order) { return runningSum + order->GetRemainingQuantity(); }) };
        };

        for (auto it = sellOrders_.rbegin(); it != sellOrders_.rend(); it++)
        {
            const auto& [price, orders] = *it;
            sellOrderInfos.push_back(CreateLevelInfo(price, orders));
        }

        for (auto it = buyOrders_.begin(); it != buyOrders_.end(); it++)
        {
            const auto& [price, orders] = *it;
            buyOrderInfos.push_back(CreateLevelInfo(price, orders));
        }

        return { sellOrderInfos, buyOrderInfos };
    }


private:

    bool CanMatchOrder(Side side, Price price) const
    {
        if (side == Side::Sell)
        {
            if (!buyOrders_.size())
                return false;

            auto bestBidPrice = (*buyOrders_.begin()).first;
            return price <= bestBidPrice;
        }
        else
        {
            if (!sellOrders_.size())
                return false;

            auto bestAskPrice = (*sellOrders_.begin()).first;
            return price >= bestAskPrice;
        }
    }

    Trades MatchOrders()
    {
        Trades trades;
        trades.reserve(orders_.size());

        while (true)
        {
            if (!buyOrders_.size() || !sellOrders_.size())
                break;

            auto& [buyPrice, buyOrders] = *buyOrders_.begin();
            auto& [sellPrice, sellOrders] = *sellOrders_.begin();

            if (buyPrice < sellPrice)
                break;

            while (buyOrders.size() && sellOrders.size())
            {
                auto& buyOrder = buyOrders.front();
                auto& sellOrder = sellOrders.front();

                Quantity tradeQuantity = std::min(buyOrder->GetRemainingQuantity(), sellOrder->GetRemainingQuantity());

                buyOrder->Fill(tradeQuantity);
                sellOrder->Fill(tradeQuantity);

                if (!buyOrder->GetRemainingQuantity())
                {
                    buyOrders.pop_front();
                    orders_.erase(buyOrder->GetId());
                }
                if (!sellOrder->GetRemainingQuantity())
                {
                    sellOrders.pop_front();
                    orders_.erase(sellOrder->GetId());
                }

                if (buyOrders.empty())
                    buyOrders_.erase(buyPrice);
                if (sellOrders.empty())
                    sellOrders_.erase(sellPrice);

                const OrderPointer& firstOrder = buyOrder->GetPriority() < sellOrder->GetPriority() ? buyOrder : sellOrder;
                const OrderPointer& secondOrder = firstOrder->GetId() == buyOrder->GetId() ? sellOrder : buyOrder;

                trades.emplace_back(
                    TradeInfo{ firstOrder->GetId(), firstOrder->GetPrice(), tradeQuantity, firstOrder->GetPriority() },
                    TradeInfo{ secondOrder->GetId(), secondOrder->GetPrice(), tradeQuantity, secondOrder->GetPriority() });
            }
        }

        if (!buyOrders_.empty())
        {
            auto& [_, buyOrders] = *buyOrders_.begin();
            OrderPointer firstOrder = buyOrders.front();
            if (firstOrder->GetType() == OrderType::InsertOrCancel)
            {
                CancelOrder(firstOrder->GetId());
            }
        }

        if (!sellOrders_.empty())
        {
            auto& [_, sellOrders] = *sellOrders_.begin();
            auto firstOrder = sellOrders.front();
            if (firstOrder->GetType() == OrderType::InsertOrCancel)
            {
                CancelOrder(firstOrder->GetId());
            }
        }

        return trades;
    }

    struct OrderEntry
    {
        OrderPointer order_;
        OrderPointerList::iterator location_;
    };

    std::map<Price, OrderPointerList, std::greater<Price>> buyOrders_;
    std::map<Price, OrderPointerList, std::less<Price>> sellOrders_;
    std::unordered_map<OrderId, OrderEntry> orders_;
};

int main() 
{
    return 0;
}