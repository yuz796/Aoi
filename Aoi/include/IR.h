#pragma once

#include <iostream>
#include <list>
#include <string>
#include <utility>

namespace IR {

class Expr {
  Expr() = delete;

public:
  enum Type { ELET, EPRIM, ENUM, EID, EBASE };

  const Type type = EBASE;

  friend std::ostream &operator<<(std::ostream &os, const Expr &eb);

protected:
  Expr(Type type) : type(type) {}

  virtual operator std::string() const = 0;
};

std::ostream &operator<<(std::ostream &os, const Expr &eb) {
  os << (std::string)eb;
  return os;
}

class ELet : public Expr {
public:
  typedef std::list<std::pair<std::string, std::shared_ptr<Expr>>> var_list;

private:
  var_list vars;
  std::shared_ptr<Expr> body;

  ELet() = delete;

public:
  ELet(var_list vars, std::shared_ptr<Expr> body)
      : Expr(ELET), vars(vars), body(body) {}

  std::tuple<var_list, std::shared_ptr<Expr>> unpack() {
    return std::make_tuple(vars, body);
  }

  operator std::string() const override {
    std::stringstream ss;
    ss << "ELet(";
    int count = 0;
    ss << "[";
    for (auto &binding : vars) {
      ss << "(\"" << binding.first << "\"";
      ss << ", ";
      ss << *binding.second << ")";

      if (count != vars.size() - 1) {
        ss << ", ";
      }
      count += 1;
    }
    ss << "]";
    ss << ", ";
    ss << *body;
    ss << ")";

    return ss.str();
  }
};

class EPrim : public Expr {
  EPrim() = delete;

public:
  enum Op { PLUS, MINUS, TIMES };

private:
  Op op;
  std::shared_ptr<Expr> expr1;
  std::shared_ptr<Expr> expr2;

public:
  EPrim(Op op, std::shared_ptr<Expr> expr1, std::shared_ptr<Expr> expr2)
      : Expr(EPRIM), op(op), expr1(expr1), expr2(expr2) {}

  std::tuple<Op, std::shared_ptr<Expr>, std::shared_ptr<Expr>> unpack() {
    return std::make_tuple(op, expr1, expr2);
  }

  operator std::string() const override {
    std::stringstream ss;
    ss << "EPrim(";

    switch (op) {
    case EPrim::PLUS:
      ss << "PLUS";
      break;
    case EPrim::MINUS:
      ss << "MINUS";
      break;
    case EPrim::TIMES:
      ss << "TIMES";
      break;
    }

    ss << ", ";
    ss << *expr1;
    ss << ", ";
    ss << *expr2;
    ss << ")";

    return ss.str();
  }
};

class ENumber : public Expr {
  int64_t val;

  ENumber() = delete;

public:
  ENumber(int64_t val) : Expr(ENUM), val(val) {}

  int64_t unpack() { return val; }

  operator std::string() const override {
    std::stringstream ss;
    ss << "ENumber(" << val << ")";
    return ss.str();
  }
};

class EId : public Expr {
  std::string name;

  EId() = delete;

public:
  EId(std::string name) : Expr(EID), name(name) {}

  std::string unpack() { return name; }

  operator std::string() const override {
    std::stringstream ss;
    ss << "EId(" << name << ")";
    return ss.str();
  }
};

} // namespace IR
