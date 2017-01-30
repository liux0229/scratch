#include <memory>
#include <vector>

// Nominal currency only denotes in abstract an amount denoted in a particular currency.
// It does not represent physical objects.
struct NominalCurrency {
  enum Type {
    USD,
    RMB,
    Euro,
  };

  Type type_;
  double amount_;
};

class Currency : private NominalCurrency {
public:
  using NominalCurrency::Type;

  Currency() {}

  // Currency cannot be copied. It must change hands.
  Currency(Currency&& from) {}

  Type getType() const { return type_;  }
};

using Price = NominalCurrency;

class ForeignExchange {
 public:
   static ForeignExchange& get() {
     static ForeignExchange exchange;
     return exchange;
   }
   
   // Foreign exchange works by actually running exchange algorithms.
   // Thus the exchange rate may not be fixed.
   // We should allow the caller to specify the type of orders they would like to place.
   // For now, we can assume that the caller always gets the target currency back with some exchange rate.
   Currency exchange(Currency in, Currency::Type to) {
     return in;
   }
};

class Goods {
public:
  Goods() {}

  // Goods cannot be copied. The ownership must change hands.
  Goods(Goods&& from) {

  }
};

class Assets {
public:
  void add(Goods goods) {}
  void add(Currency capital) {}

  Currency::Type getCurrencyType() const {
    return capital_.getType();
  }

private:

  std::vector<Goods> inventory_;

  // For now, assume the business holds its capitcal 
  // in a single currency which is the currency used 
  // by the country the business operates in.
  Currency capital_;
};

class Business {
public:
  // For now, model entity to have a behavior they control internally
  void run() {
    // Produce
    auto goods = produce();

    // Sell
    // Assume the transaction is done using USD
    auto payments = consumer_->sell(std::move(goods), Price{Price::USD, 10});

    // Enter payments into assets; they must be converted to my asset currency first.
    assets_.add(ForeignExchange::get().exchange(std::move(payments), assets_.getCurrencyType()));
  }

  // Sell transaction agrees on what currency to use and denote the price in that currency.
  Currency sell(Goods goods, Price price) {
    return Currency{};
  }

private:
  Goods produce() {
    // Buy from other business

    // Use goods from its inventory
    return Goods{};
  }

  Business* consumer_;
  Assets assets_;
};

int main() {

}