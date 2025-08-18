#ifndef DIP_LISTS_H
#define DIP_LISTS_H

#include "../nodes/nodes.h"
#include "../settings.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace dip {

  // Node List

  class NodeList {
  private:
    BaseNode::NodeListType nodes;

  public:
    NodeList() {};
    NodeList(const BaseNode::NodeListType& nl) : nodes(nl) {};
    size_t size() const;
    void push_front(const BaseNode::PointerType& node);
    void push_back(const BaseNode::PointerType& node);
    BaseNode::PointerType pop_front();
    BaseNode::PointerType pop_back();
    BaseNode::PointerType at(const size_t index);
    BaseNode::PointerType at(const size_t index) const;
  };

  // Source list

  class SourceList; // EnvSource needs a forward declaration

  struct EnvSource {
    std::string name; // source key
    std::string path; // source path
    std::string code; // source code
    Source parent;    // parent source
    NodeList nodes;   // parsed nodes
                      // std::shared_ptr<SourceList> sources;
  };

  class SourceList {
  private:
    std::map<std::string, EnvSource> sources;

  public:
    SourceList();
    void append(const std::string& name, const std::string& path, const std::string& code,
                const Source& parent);
    void append(const std::string& name, const EnvSource& src);
    EnvSource& at(const std::string& name);
    const EnvSource& at(const std::string& name) const;
  };

  // Unit list

  struct EnvUnit {
    std::string name;       // unit key
    std::string definition; // unit definition
  };

  class UnitList {
  private:
    std::map<std::string, EnvUnit> units;

  public:
    UnitList();
    void append(const std::string& name, const std::string& definition);
    void append(const std::string& name, const EnvUnit& src);
    EnvUnit& at(const std::string& name);
    const EnvUnit& at(const std::string& name) const;
  };

  // Hierarchy list

  struct Parent {
    size_t indent;
    std::string name;
  };

  class HierarchyList {
  private:
    std::vector<Parent> parents;

  public:
    void record(const BaseNode::PointerType& node, const std::vector<NodeDtype>& excluded);
  };

  // Branching list

  struct Case {
    std::string path;   // case path up to last @ sign
    std::string code;   // code line with the case
    std::string expr;   // case logical expression
    bool value;         // final value of the case
    size_t branch_id;   // branch ID
    size_t branch_part; // part on the branch
    size_t case_id;     // case ID
    CaseType case_type; // one of the types: case/else/end
  };

  struct Branch {
    std::vector<size_t> cases;           // list of case IDs
    std::vector<CaseType> types;         // list of case types
    std::map<std::string, size_t> nodes; // number of node definitions
  };

  class BranchingList {
  private:
    std::vector<size_t> state;         // list of openned branches
    std::map<size_t, Branch> branches; // all branches
    std::map<size_t, Case> cases;      // all cases
    int num_cases;
    int num_branches;
    size_t get_branch_id();
    size_t get_case_id(size_t branch_id = 0);
    int open_branch(const size_t case_id);
    int switch_case(const size_t case_id, const CaseType case_type);
    void close_branch();

  public:
    BranchingList() : num_cases(0), num_branches(0) {};
    int register_case();
    bool false_case();
    void solve_case(const BaseNode::PointerType& node);
    void prepare_node(const BaseNode::PointerType& node);
    std::string clean_name(const std::string& node);
  };

  class FunctionList {
  public:
    typedef val::BaseValue::PointerType (*ValueFunctionType)(const Environment& env);
    typedef BaseNode::NodeListType (*TableFunctionType)(const Environment& env);

  private:
    std::map<std::string, ValueFunctionType> value_functions;
    std::map<std::string, TableFunctionType> table_functions;

  public:
    void append_value(const std::string& name, ValueFunctionType func);
    void append_table(const std::string& name, TableFunctionType func);
    ValueFunctionType get_value(const std::string& name) const;
    TableFunctionType get_table(const std::string& name) const;
  };

} // namespace dip

#endif // DIP_LISTS_H
