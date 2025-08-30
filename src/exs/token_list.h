#ifndef EXS_TOKEN_LIST_H
#define EXS_TOKEN_LIST_H

#include "settings.h"
#include "token_list.h"
#include "operator_list.h"

#include <algorithm>
#include <deque>
#include <stdexcept>
#include <vector>

namespace exs {

  template <class A, typename S = EmptySettings>
  class TokenList : public TokenListBase<A> {
  public:
    std::deque<Token<A>> left;
    std::deque<Token<A>> right;
    OperatorList<A, S>* operators;
    S* settings;
    AtomList<A, S> atoms;
    TokenList(OperatorList<A, S>* o, S* set = nullptr) : operators(o), settings(set) {};
    void append(TokenType t) {
      right.push_back(Token<A>(t));
    };
    void append(TokenType t, int o) {
      right.push_back(Token<A>(t, o));
    };
    void append(TokenType t, std::string s) {
      A* a = atoms.append(s, settings);
      right.push_back(Token<A>(t, a));
    };
    void append(TokenType t, A at) {
      A* a = atoms.append(at);
      right.push_back(Token<A>(t, a));
    };
    Token<A> get_left() {
      if (left.empty()) {
        return Token<A>(EMPTY_TOKEN);
      } else {
        Token<A> t = left.back();
        left.pop_back();
        return t;
      }
    };
    Token<A> get_right() {
      if (right.empty()) {
        return Token<A>(EMPTY_TOKEN);
      } else {
        Token<A> t = right.front();
        right.pop_front();
        return t;
      }
    };
    void put_left(Token<A> t) {
      if (t.type != EMPTY_TOKEN) {
        left.push_back(t);
      }
    };
    void put_right(Token<A> t) {
      if (t.type != EMPTY_TOKEN) {
        right.push_front(t);
      }
    };
    void operate(const std::vector<int>& ops, OperationType oitype) {
      /*
      std::cout << "token_list::operate oitype=" << oitype << " operators=[ ";
      for (auto i: ops) std::cout << i << " ";
      std::cout << "]" << std::endl;
      print(true);
      */
      // perform operations on the individual tokens
      while (!right.empty()) {
        Token<A> token = get_right();
        // token.print();
        if (std::find(ops.begin(), ops.end(), token.optype) != ops.end()) {
          OperatorBase<A, S>* op = operators->select(token.optype);
          // token is an operator
          if (oitype == UNARY_OPERATION) {
            op->operate_unary(this, settings);
          } else if (oitype == BINARY_OPERATION) {
            op->operate_binary(this, settings);
          } else if (oitype == TERNARY_OPERATION) {
            op->operate_ternary(this, settings);
          } else if (oitype == GROUP_OPERATION) {
            op->operate_group(this, settings);
          } else {
            throw std::invalid_argument("Invalid operation type: " + std::to_string(oitype));
          }
        } else {
          // token is something else
          // std::cout << "putting left" << std::endl;
          put_left(token);
        }
        // print(true);
      }
      // move all tokens from left to right
      left.swap(right);
      // print(true);
      // std::cout << std::endl;
    };
    void print(bool details = false) {
      std::cout << to_string(details) << "\n";
    };
    std::string to_string(bool details = false) {
      std::stringstream str;
      if (details) {
        str << "TokenList( ";
        for (auto i = left.begin(); i != left.end(); ++i) {
          str << print_details(i->type, i->optype, i->atom);
        }
        str << "| ";
        for (auto i = right.begin(); i != right.end(); ++i) {
          str << print_details(i->type, i->optype, i->atom);
        }
        str << ")\n";
      } else {
        str << "TokenList(" << left.size() << " | " << right.size() << ")\n";
      }
      return str.str();
    }

  private:
    std::string print_details(TokenType type, int optype, A* atom) {
      std::stringstream str;
      switch (type) {
      case EMPTY_TOKEN:
        str << "E ";
        break;
      case ATOM_TOKEN:
        str << "A{" << atom->to_string() << "} ";
        break;
      case OPERATOR_TOKEN:
        OperatorBase<A, S>* op = operators->select(optype);
        str << op->name << " ";
        break;
      }
      return str.str();
    };
  };

} // namespace exs

#endif // EXS_TOKEN_LIST_H
