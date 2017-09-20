#ifndef PRICE_H
#define PRICE_H
#include <vector>
#include <string>
#include <sstream>

class Price {
private:
  std::string _currencyCode;
  std::string _amount;

public:
  Price() {}
  Price(const std::string& currencyCode, double amount) {
    this->_currencyCode = currencyCode;
    this->_amount = amount;
  }

  std::string debugString() const {
    std::stringstream stream;
    stream << _amount << " " << _currencyCode;
    return stream.str();
  }

  static std::vector<Price> PricesInText(const std::string& text);
  static Price PriceFrom(char* symbol, char* code, char* value);

  std::string amount() const {return _amount;}
  std::string currencyCode() const {return _currencyCode;}
};
#endif
