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
int main(){

    return 0;
}