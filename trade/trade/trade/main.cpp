#include <memory>
#include <vector>
#include <map>
#include <cassert>

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
using Price = NominalCurrency;

class Currency : private NominalCurrency {
public:
  using NominalCurrency::Type;

  // Currency cannot be copied. It must change hands.
  Currency(Currency&& from) {}

  Type getType() const { return type_;  }

  Price getPrice() const {
    return Price{type_, amount_};
  }

  Currency add(Currency n) {
    assert(n.getType() == type_);
    amount_ += n.amount_;
  }

  // Deduct part of the currency out
  Currency deduct(Price p) {
    assert(p.type_ == type_);
    assert(amount_ >= p.amount_);
    
    amount_ -= p.amount_;
    return Currency(p.type_, p.amount_);
  }

private:
  friend class ForeignExchange;

  Currency(Type type, double amount) {
    type_ = type;
    amount_ = amount;
  }
};

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
  Currency exchange(Currency in, Currency::Type toType) const {
    auto target = exchange(in.getPrice(), toType);
    return Currency(toType, target.amount_);
  }

  Price exchange(Price in, Price::Type toType) const {
    auto target = exchangeRateMap_.at(std::make_pair(in.type_, toType));
    return Price{ toType, target };
  }

private:
  ForeignExchange() {
    exchangeRateMap_ = {
      { { Price::USD, Price::RMB }, 6.87 },
    };

    // Build the inverse map
    auto m = exchangeRateMap_;
    for (auto& kv : m) {
      auto p = kv.first;
      m[std::make_pair(p.second, p.first)] = 1 / kv.second;
    }
  }

  using CurrencyPair = std::pair<Currency::Type, Currency::Type>;
  std::map<CurrencyPair, double> exchangeRateMap_;
};

// Just a wrapper around ForeignExchange and fast return 
// if the toType is already the same as the type of the `in' currency.
Currency convertCurrency(Currency in, Currency::Type toType) {
  return toType == in.getType() ? std::move(in) : ForeignExchange::get().exchange(std::move(in), toType);
}

Price convertPrice(Price in, Price::Type toType) {
  return toType == in.type_ ? in : ForeignExchange::get().exchange(in, toType);
}

class Goods {
public:
  Goods() {}

  // Goods cannot be copied. The ownership must change hands.
  Goods(Goods&& from) {

  }
};

class Assets {
public:
  void add(Goods goods) {
    inventory_.push_back(std::move(goods));
  }
  void add(Currency capital) {
    capital_.add(std::move(capital));
  }
  Currency deduct(Price p) {
    return capital_.deduct(p);
  }

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
    assets_.add(convertCurrency(std::move(payments), assets_.getCurrencyType()));
  }

  // Sell transaction agrees on what currency to use and denote the price in that currency.
  Currency sell(Goods goods, Price price) {
    // Assume I have enough cash in my assets. 

    // Convert the price to asset currency type
    auto cash = assets_.deduct(convertPrice(price, assets_.getCurrencyType()));

    // Convert into the transaction currency
    auto converted = convertCurrency(std::move(cash), price.type_);
    assets_.add(std::move(goods));

    return converted;
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