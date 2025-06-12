#include <iostream>
#include <string>
#include <unordered_map> // ?
#include <map>
#include <set>
#include <list>
#include <deque>
#include <queue>
#include <stack>
#include <vector>
#include <tuple>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <memory> // ?
#include <variant> // ?
#include <optional> // ?
#include <numeric> // ?


// using enum class ensures type safty
enum class OrderType{
    GoodTillCancel, 
    FillAndKill // excute immediatly or cancel
};
enum class Side{
    Buy,
    Sell
};

using Price = std::int32_t;
using Quantity = std::uint32_t;
using OrderId = std::uint64_t;

// for L1 and L2 books, an aggregate representation
struct LevelInfo{
    Price price_;
    Quantity quantity_;
};

using LevelInfos = std::vector<LevelInfo>;

class OrderbookLevelInfos{
    public:
        OrderbookLevelInfos(const LevelInfos& bids, const LevelInfos& asks){
            bids_ = bids;
            asks_ = asks;
        }
        const LevelInfos& GetBids() const {return bids_;}
        const LevelInfos& GetAsks() const {return asks_;}
    private:
        LevelInfos bids_;
        LevelInfos asks_;
};

class Order{
    public:
        Order(OrderType orderType, OrderId orderId, Price price, Side side, Quantity quantity){
            orderType_ = orderType;
            orderId_ = orderId;
            side_ = side;
            price_ = price;
            initialQuantity_ = quantity;
            remainingQuantity_ = quantity;
        }
        OrderId GetOrderId() const {return orderId_;}
        Side getSide() const {return side_;}
        Price GetPrice() const {return price_;}
        OrderType GetOrderType() const {return orderType_;}
        Quantity GetInitialQuantity() const {return initialQuantity_;}
        Quantity GetRemainingQuantity() const {return remainingQuantity_;}
        Quantity GetFilledQuantity() const {return GetInitialQuantity() - GetRemainingQuantity();}
        bool isFilled() const {return remainingQuantity_ == 0;}
        void Fill(Quantity quantity){ 
            if (quantity > GetRemainingQuantity())
                throw std::logic_error("Order  cannot be filled for more than its remaining quantity" + std::to_string(GetOrderId()));
            remainingQuantity_ -= quantity;
        }

    private:
        OrderType orderType_;
        OrderId orderId_;
        Side side_;
        Price price_;
        Quantity initialQuantity_;
        Quantity remainingQuantity_;
};
// as we are going to use several data structures later
using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::list<OrderPointer>; // could use vectors

// we can add,modify or canel and order
// add requires the order, cancel requires and OrderId

class OrderModify{
    public:
        OrderModify(OrderId orderId, Side side, Price price, Quantity quantity){
            orderId_ = orderId;
            side_ = side; // its unusual to allow changing sides when modifying an order
            price_ = price;
            quantity_ = quantity;
        }
        OrderId GetOrderId() const {return orderId_;}
        Side GetSide() const {return side_;}
        Price GetPrice() const {return price_;}
        Quantity GetQuantity() const {return quantity_;}
        // supports cancel + replace order
        // returns a shared pointer to te modfied order object
        OrderPointer ToOrderPointer(OrderType type) const{
            return std::make_shared<Order>(type, GetOrderId(), GetSide(), GetPrice(), GetQuantity()); 
        }
    private:
        OrderId orderId_;
        Side side_;
        Price price_;
        Quantity quantity_;
};

// representation of a matched order
// trade object needed - bid, ask
struct TradeInfo{
    OrderId orderId_;
    Price price_;
    Quantity quantity_;
};

class Trade{
    public:
        Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade){
            bidTrade_ = bidTrade;
            askTrade_ = askTrade; 
        }
        const  TradeInfo& GetBidTrade() const {return bidTrade_;}
        const TradeInfo& GetBidTrade() const {return askTrade_;}
    private:
        TradeInfo bidTrade_;
        TradeInfo askTrade_;
};

// since one order can match a lot of orders
using Trades = std::vector<Trade>;

// bid are sorted in descending order
// ask are sorted in ascending order
class OrderBook{
    private:
        struct OrderEntry{
            OrderPointer order_{nullptr};
            OrderPointers::iterator location_;
        };

        std::map<Price, OrderPointers, std::greater<Price>> bids_;
        std::map<Price, OrderPointers, std::less<Price>> asks_;
        std::unordered_map<OrderId, OrderEntry> order_;

        // for FillAndKill
        bool CanMatch(Side side, Price price) const{
            if (side == Side::Buy) {
                if(asks_.empty())
                    return false;
                const auto& [bestAsk, _] = *asks_.begin();
                return price >= bestAsk;
            }
            else{
                if(bids_.empty())
                    return false;
                const auto& [bestBid, _] = *bids_.begin();
                return price <= bestBid;
            }
        }

        Trades MatchOrders(){
            Trades trades;
            trades.reserve(order_.size()); // being pesimestic, if every order matches every other

            while(true){
                if(bids_.empty() || asks_.empty())
                    break;

                auto& [bidPrice, bids] = *bids_.begin();
                auto& [askPrice, asks] = *asks_.begin();

                if(bidPrice < askPrice)
                    break; // no matches

                while (bids.size() & asks.size()){
                    auto& bid = bids.front();
                    auto& ask = asks.front();

                    Quantity quantity = std::min(bid->GetRemainingQuantity(), ask->GetRemainingQuantity());

                    bid->Fill(quantity);
                    ask->Fill(quantity);

                    if(bid->isFilled()){
                        bids.pop_front();
                        order_.erase(bid->GetOrderId());
                    }

                    if(ask->isFilled()){
                        asks.pop_front();
                        order_.erase(ask->GetOrderId());
                    }

                    if(bids.empty()){
                        bids_.erase(bidPrice);
                    }

                    if(asks.empty()){
                        asks_.erase(askPrice);
                    }
// quantity could be part of the constructor of trade object becasue its the same
                    trades.push_back(Trade { 
                        TradeInfo{ bid->GetOrderId(), bid->GetPrice(), quantity },
                        TradeInfo{ ask->GetOrderId(), ask->GetPrice(), quantity}
                        });
                }
            }
            // make sure the FillAndKill order is removed if not full filled
            if(!bids_.empty()){
                auto& [_, bids] = *bids_.begin();
                auto& order = bids.front();
                if(order->GetOrderType() == OrderType::FillAndKill)
                    CancelOrder(order->GetOrderId());
            }

            if(!asks_.empty()){
                auto& [_, asks] = *asks_.begin();
                auto& order = asks.front();
                if(order->GetOrderType() == OrderType::FillAndKill)
                    CancelOrder(order->GetOrderId());
            }

            return trades;
        }
        public:

            Trades AddOrder
};
int main(){

    return 0;
}