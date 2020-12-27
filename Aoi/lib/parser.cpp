#include <IR.h>
#include <iostream>
#include <tao/pegtl.hpp>

namespace parser {
namespace pegtl = tao::pegtl;

/* Rules */

struct whitespace : pegtl::plus<pegtl::space> {};

struct number : pegtl::plus<pegtl::digit> {};

struct symbol : pegtl::identifier {};

struct binop : pegtl::sor<pegtl::one<'+'>, pegtl::one<'-'>, pegtl::one<'*'>> {};

struct expr;
struct expr2;

struct list_begin : pegtl::seq<pegtl::one<'('>, pegtl::star<pegtl::space>> {};
struct list_end : pegtl::seq<pegtl::star<pegtl::space>, pegtl::one<')'>> {};

struct binding_symbol : pegtl::identifier {};

struct binding : pegtl::seq<list_begin, binding_symbol, whitespace, expr,
                            pegtl::must<list_end>> {};

struct bindings_begin : pegtl::seq<pegtl::one<'('>, pegtl::star<pegtl::space>> {
};
struct bindings_end : pegtl::one<')'> {};

struct bindings : pegtl::seq<bindings_begin,
                             pegtl::list<binding, pegtl::plus<pegtl::space>>,
                             pegtl::must<bindings_end>> {};

struct let : pegtl::seq<list_begin, pegtl::string<'l', 'e', 't'>, whitespace,
                        bindings, whitespace, expr, pegtl::must<list_end>> {};

struct prim : pegtl::seq<list_begin, binop, whitespace, expr, whitespace, expr2,
                         pegtl::must<list_end>> {};

struct expr : pegtl::sor<number, symbol, let, prim> {};

struct expr2 : expr {};

struct anything : pegtl::sor<pegtl::space, expr> {};

struct main : pegtl::until<pegtl::eof, pegtl::must<anything>> {};

/* Actions and States*/

struct parser_state {
  std::vector<std::shared_ptr<IR::Expr>> result;
  std::shared_ptr<IR::Expr> result2;
  std::vector<IR::EPrim::Op> ops;
  std::vector<std::string> vars;
  std::vector<IR::ELet::var_list> binding_lists;
};

template <typename Rule> struct action {};

template <> struct action<number> {
  template <typename ActionInput>
  static void apply(const ActionInput &in, parser_state &state) {
    std::stringstream ss(in.string());
    int64_t num;
    ss >> num;
    state.result.push_back(std::make_shared<IR::ENumber>(num));
  }
};

template <> struct action<symbol> {
  template <typename ActionInput>
  static void apply(const ActionInput &in, parser_state &state) {
    state.result.push_back(std::make_shared<IR::EId>(in.string()));
  }
};

template <> struct action<binop> {
  template <typename ActionInput>
  static void apply(const ActionInput &in, parser_state &state) {
    switch (in.string().front()) {
    case '+':
      state.ops.push_back(IR::EPrim::PLUS);
      break;
    case '-':
      state.ops.push_back(IR::EPrim::MINUS);
      break;
    case '*':
      state.ops.push_back(IR::EPrim::TIMES);
      break;
    }
  }
};

template <> struct action<expr2> {
  static void apply0(parser_state &state) {
    state.result2 = std::move(state.result.back());
    state.result.pop_back();
  }
};

template <> struct action<prim> {
  static void apply0(parser_state &state) {
    auto e1 = std::move(state.result.back());
    state.result.pop_back();
    auto e2 = std::move(state.result2);
    auto op = std::move(state.ops.back());
    state.ops.pop_back();
    state.result.push_back(
        std::make_shared<IR::EPrim>(op, std::move(e1), std::move(e2)));
  }
};

template <> struct action<binding_symbol> {
  template <typename ActionInput>
  static void apply(const ActionInput &in, parser_state &state) {
    state.vars.push_back(in.string());
  }
};

template <> struct action<bindings_begin> {
  static void apply0(parser_state &state) {
    state.binding_lists.push_back(IR::ELet::var_list());
  }
};

template <> struct action<binding> {
  static void apply0(parser_state &state) {
    state.binding_lists.back().push_back(std::make_pair(
        std::move(state.vars.back()), std::move(state.result.back())));
    state.vars.pop_back();
    state.result.pop_back();
  }
};

template <> struct action<let> {
  static void apply0(parser_state &state) {
    auto e = std::move(state.result.back());
    state.result.pop_back();
    auto binding_list = std::move(state.binding_lists.back());
    state.binding_lists.pop_back();
    state.result.push_back(
        std::make_shared<IR::ELet>(std::move(binding_list), std::move(e)));
  }
};

} // namespace parser

int main(int argc, char *argv[]) {
  namespace pegtl = tao::pegtl;

  parser::parser_state state;
  pegtl::argv_input in(argv, 1);
  pegtl::parse<parser::main, parser::action>(in, state);

  std::cout << *state.result.front() << std::endl;
}
