#ifndef EXS_TOKEN_LIST_H
#define EXS_TOKEN_LIST_H

#include "atom_list.h"
#include "operator_list.h"
#include "settings.h"
#include "token.h"
#include "token_list.h"

#include <algorithm>
#include <deque>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace snt::exs {

  class TokenList : public TokenListBase {
  public:
    std::deque<Token> left;
    std::deque<Token> right;
    OperatorList* operators;
    BaseSettings* settings;
    AtomList atoms;
    TokenList(OperatorList* o, BaseSettings* set = nullptr) :
      operators(o), settings(set) {
    };
    void append(TokenType t) {
      right.push_back(Token(t));
    };
    void append(TokenType t, int o) {
      right.push_back(Token(t, o));
    };
    //void append(TokenType t, std::string s) {
    //  AtomGrand* a = atoms.append(s, settings);
    //  right.push_back(Token(t, a));
    //};
    void append(TokenType t, std::unique_ptr<AtomGrand> at) {
      AtomGrand* a = atoms.append(std::move(at));
      right.push_back(Token(t, a));
    };
    Token get_left() {
      if (left.empty()) {
        return Token(EMPTY_TOKEN);
      } else {
        Token t = left.back();
        left.pop_back();
        return t;
      }
    };
    Token get_right() {
      if (right.empty()) {
        return Token(EMPTY_TOKEN);
      } else {
        Token t = right.front();
        right.pop_front();
        return t;
      }
    };
    void put_left(Token t) {
      if (t.type != EMPTY_TOKEN) {
        left.push_back(t);
      }
    };
    void put_right(Token t) {
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
        Token token = get_right();
        // token.print();
        if (std::find(ops.begin(), ops.end(), token.optype) != ops.end()) {
          OperatorBase* op = operators->select(token.optype);
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
    std::string print_details(TokenType type, int optype, AtomGrand* atom) {
      std::stringstream str;
      switch (type) {
      case EMPTY_TOKEN:
        str << "E ";
        break;
      case ATOM_TOKEN:
        str << "A{" << atom->to_string() << "} ";
        break;
      case OPERATOR_TOKEN:
        OperatorBase* op = operators->select(optype);
        str << op->name << " ";
        break;
      }
      return str.str();
    };
  };

} // namespace snt::exs

#endif // EXS_TOKEN_LIST_H
