#include <regex>

#include "lists.h"

namespace dip {

  // Get ID of a current branch
  size_t BranchingList::get_branch_id() {
    if (state.size() > 0) {
      return state.back();
    } else
      throw std::runtime_error("Cannot get a branch ID because branch list is empty");
  }

  // Get ID of a current case
  size_t BranchingList::get_case_id(size_t branch_id) {
    if (branch_id == 0)
      branch_id = get_branch_id();
    Branch& branch = branches.at(branch_id);
    if (branch.cases.empty()) {
      std::string branch_name =
          std::string(1, SIGN_CONDITION) + "B" + std::to_string(num_branches);
      throw std::runtime_error("Branch with an ID '" + branch_name + "' does not have any cases");
    } else {
      return branch.cases.back();
    }
  }

  // Start a new branch
  int BranchingList::open_branch(const size_t case_id) {
    size_t branch_id = ++num_branches;
    state.push_back(branch_id);
    branches[branch_id] = Branch({case_id}, {CaseType::Case});
    return 0;
  }

  // Go to a new case within a branch
  int BranchingList::switch_case(const size_t case_id, const CaseType case_type) {
    size_t branch_id = get_branch_id();
    Branch& branch = branches.at(branch_id);
    branch.cases.push_back(case_id);
    branch.types.push_back(case_type);
    return static_cast<int>(branch.cases.size() - 1);
  }

  // Close current branch
  void BranchingList::close_branch() {
    if (state.empty())
      throw std::runtime_error("No more branches to be closed");
    else
      state.pop_back();
  }

  // Add a new case
  int BranchingList::register_case() {
    return ++num_cases;
  }

  // Checks if case value is false
  bool BranchingList::false_case() {
    if (state.empty())
      return false;
    size_t branch_id = get_branch_id();
    Branch& branch = branches.at(branch_id);
    int num_true = 0;
    // count number of cases with a true value in the current branch
    for (size_t case_id : branch.cases) {
      Case& cs = cases.at(case_id);
      if (cs.value == true)
        num_true++;
    }
    // get the value of the current case
    size_t case_id = get_case_id();
    Case& cs = cases.at(case_id);
    // the current case is false if this is the first true case in a branch
    return (num_true != 1 or !cs.value);
  }

  // Manage condition nodes
  void BranchingList::solve_case(const BaseNode::PointerType& node) {
    std::ostringstream oss;
    oss << "(.*[" << SIGN_CONDITION << "])C([0-9]+)";
    std::regex pattern(oss.str());
    std::smatch matchResult;
    if (std::regex_search(node->name, matchResult, pattern)) {
      std::shared_ptr<CaseNode> cnode = std::dynamic_pointer_cast<CaseNode>(node);
      if (cnode == nullptr)
        throw std::runtime_error("Given node must be a case node:  " + node->line.code);
      std::string path_new = matchResult[1].str();
      std::string path_old;
      if (!state.empty()) {
        size_t case_id = get_case_id();
        Case& cs = cases.at(case_id);
        path_old = cs.path;
      }
      // validate correct condition type and process end of the case
      if (cnode->case_type == CaseType::Case) {
        // continue
      } else if (cnode->case_type == CaseType::Else and !cases.empty()) {
        // continue
      } else if (cnode->case_type == CaseType::End and !cases.empty() and
                 path_old.size() >= path_new.size()) {
        close_branch();
        return;
      } else {
        std::cout << cases.size() << " " << path_old << " " << path_new << '\n';
        throw std::runtime_error("Invalid condition type:  " + node->line.code);
      }
      // determine branch part and ID
      size_t branch_part;
      size_t case_id = std::stoull(matchResult[2].str());
      if (path_new == path_old) {
        branch_part = switch_case(case_id, cnode->case_type);
      } else if (path_new.size() < path_old.size()) {
        // close openned branches until the same branch is reached
        while (path_new != path_old and path_old.size() > 0) {
          close_branch();
          if (state.size() > 0) {
            size_t case_id_old = get_case_id();
            Case& cs = cases.at(case_id_old);
            path_old = cs.path;
          } else {
            path_old = "";
          }
        }
        if (state.empty())
          branch_part = open_branch(case_id);
        else
          branch_part = switch_case(case_id, cnode->case_type);
      } else {
        branch_part = open_branch(case_id);
      }
      // get current branch id
      size_t branch_id = get_branch_id();
      // take into account values of the parent nodes
      // std::cout << "case " << branch_id << " " << branch_part << " " << case_id << " " <<
      // cnode->case_type << " " << cnode->value;
      bool case_value = cnode->value;
      if (state.size() > 1) {
        size_t parent_branch_id = state[state.size() - 2];
        size_t parent_case_id = get_case_id(parent_branch_id);
        Case& cs = cases.at(parent_case_id);
        case_value &= cs.value;
        // std::cout << " " << case_value << " " << parent_branch_id << parent_case_id;
      }
      // std::cout << '\n';
      // register new case
      std::string expr = (cnode->value_raw.empty()) ? "" : cnode->value_raw.at(0);
      cases[case_id] = Case(path_new, cnode->line.code, expr, case_value, branch_id, branch_part,
                            case_id, cnode->case_type);
    } else {
      throw std::runtime_error("Invalid condition format: " + node->line.code);
    }
  }

  // Manage parameter nodes in a condition
  void BranchingList::prepare_node(const BaseNode::PointerType& node) {
    if (state.empty())
      return;
    size_t case_id = get_case_id();
    Case& cs = cases.at(case_id);
    // ending case at less indent
    if (!(node->name.rfind(cs.path, 0) == 0)) {
      close_branch();
    }
    // register node to a branch
    if (!state.empty()) {
      size_t branch_id = get_branch_id();
      node->branch_id = branch_id;
      node->case_id = get_case_id();
      std::string node_name = clean_name(node->name);
      Branch& branch = branches.at(branch_id);
      auto it = branch.nodes.find(node_name);
      if (it == branch.nodes.end()) {
        branch.nodes[node_name] = 1;
      } else {
        branch.nodes[node_name]++;
      }
    }
  }

  std::string BranchingList::clean_name(const std::string& name) {
    std::ostringstream oss;
    oss << "(" << SIGN_CONDITION << "C[0-9]+" << SIGN_SEPARATOR << ")";
    std::regex pattern(oss.str());
    return std::regex_replace(name, pattern, "");
  }

} // namespace dip
