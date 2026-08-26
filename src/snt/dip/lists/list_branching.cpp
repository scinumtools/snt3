#include <regex>
#include <snt/dip/exceptions.h>
#include <snt/dip/lists/list_branching.h>
#include <snt/dip/nodes/node_case.h>

namespace snt::dip {

    // Get ID of a current branch
    size_t BranchingList::get_branch_id() {
        if (state.size() > 0)
            return state.back();
        throw dip::EnvironmentException(
            "Missing conditional branch",
            "An active conditional branch must exist.",
            "The conditional branch list is empty.",
            "Enter an `if` or `elif` branch before requesting its ID.",
            __FILE__,
            __LINE__
        );
    }

    // Get ID of a current case
    size_t BranchingList::get_case_id(size_t branch_id) {
        if (branch_id == 0)
            branch_id = get_branch_id();
        Branch& branch = branches.at(branch_id);
        if (branch.cases.empty()) {
            std::string branch_name = std::string(1, SIGN_CONDITION) + "B" + std::to_string(num_branches);
            throw dip::EnvironmentException(
                "Missing conditional case",
                "The conditional branch must contain at least one case.",
                "The branch `" + branch_name + "` does not contain any cases.",
                "Add an `if` or `elif` case before requesting its ID.",
                __FILE__,
                __LINE__
            );
        } else {
            return branch.cases.back();
        }
    }

    // Start a new branch
    int BranchingList::open_branch(const size_t case_id) {
        size_t branch_id = ++num_branches;
        state.push_back(branch_id);
        branches[branch_id] = Branch{{case_id}, {CaseType::IF}};
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
            throw dip::EnvironmentException(
                "No active conditional branch",
                "An active conditional branch must exist before it can be closed.",
                "The conditional branch list is empty.",
                "Make sure that each branch is closed only after entering an `if` or `elif` branch.",
                __FILE__,
                __LINE__
            );
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
        return (num_true != 1 || !cs.value);
    }

    // Manage condition nodes
    void BranchingList::solve_case(const BaseNode::PointerType& node) {
        std::ostringstream oss;
        oss << "(.*[" << SIGN_CONDITION << "])C([0-9]+)";
        std::regex pattern(oss.str());
        std::smatch matchResult;
        if (std::regex_search(node->path.name, matchResult, pattern)) {
            std::shared_ptr<CaseNode> cnode = std::dynamic_pointer_cast<CaseNode>(node);
            if (cnode == nullptr)
                throw dip::SyntaxException(
                    "Invalid case node",
                    "The node must be a case node.",
                    "The node at path `" + node->path.name + "` is not a case node.",
                    "Check the node type and make sure that the case path is assigned to a case node.",
                    __FILE__,
                    __LINE__,
                    node->line
                );
            std::string path_new = matchResult[1].str();
            std::string path_old;
            if (!state.empty()) {
                size_t case_id = get_case_id();
                Case& cs = cases.at(case_id);
                path_old = cs.path;
            }
            // validate correct condition type and process end of the case
            if (cnode->case_type == CaseType::IF) {
                // continue
            } else if (cnode->case_type == CaseType::ELIF && !cases.empty()) {
                // continue
            } else if (cnode->case_type == CaseType::ELSE && !cases.empty()) {
                // continue
            } else if (cnode->case_type == CaseType::END && !cases.empty() && path_old.size() >= path_new.size()) {
                close_branch();
                return;
            } else {
                // std::cout << cases.size() << " " << path_old << " " << path_new << '\n';
                throw dip::SyntaxException(
                    "Invalid conditional case",
                    "The conditional case must be valid for the current branching state.",
                    "The case type is not valid in the current branching context.",
                    "Check the order of `if`, `elif`, `else`, and `end` cases and make sure they form a valid "
                    "conditional structure.",
                    __FILE__,
                    __LINE__,
                    node->line
                );
            }
            // determine branch part and ID
            size_t branch_part;
            size_t case_id = std::stoull(matchResult[2].str());
            if (path_new == path_old) {
                branch_part = switch_case(case_id, cnode->case_type);
            } else if (path_new.size() < path_old.size()) {
                // close openned branches until the same branch is reached
                while (path_new != path_old && path_old.size() > 0) {
                    close_branch();
                    if (state.size() > 0) {
                        size_t case_id_old = get_case_id();
                        Case& cs = cases.at(case_id_old);
                        path_old = cs.path;
                    } else {
                        path_old = "";
                    }
                }
                if (state.empty() && cnode->case_type == CaseType::IF)
                    branch_part = open_branch(case_id);
                else if (cnode->case_type == CaseType::ELIF || cnode->case_type == CaseType::ELSE)
                    branch_part = switch_case(case_id, cnode->case_type);
                else
                    throw dip::SyntaxException(
                        "Invalid branch opening",
                        "A new conditional branch can only be opened by an `if` case.",
                        "The current case cannot open a new conditional branch.",
                        "Use `if` to open a new branch, or use `elif` or `else` only within an existing branch.",
                        __FILE__,
                        __LINE__,
                        node->line
                    );
            } else if (cnode->case_type == CaseType::IF) {
                branch_part = open_branch(case_id);
            } else {
                throw dip::SyntaxException(
                    "Invalid branch opening",
                    "A new conditional branch can only be opened by an `if` case.",
                    "The current case cannot open a new conditional branch.",
                    "Use `if` to open a new branch, or use `elif` or `else` only within an existing branch.",
                    __FILE__,
                    __LINE__,
                    node->line
                );
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
            cases[case_id] =
                Case{path_new, cnode->line.code, expr, case_value, branch_id, branch_part, case_id, cnode->case_type};
        } else {
            throw dip::SyntaxException(
                "Invalid conditional path",
                "The case node path must follow the expected conditional path format.",
                "The node path does not match the expected conditional case format.",
                "Check the conditional path and make sure it identifies a case using the expected format.",
                __FILE__,
                __LINE__,
                node->line
            );
        }
    }

    // Manage parameter nodes in a condition
    void BranchingList::prepare_node(const BaseNode::PointerType& node) {
        if (state.empty())
            return;
        size_t case_id = get_case_id();
        Case& cs = cases.at(case_id);
        // ending case at less indent
        if (!(node->path.name.rfind(cs.path, 0) == 0)) {
            close_branch();
        }
        // register node to a branch
        if (!state.empty()) {
            size_t branch_id = get_branch_id();
            node->branch_id = branch_id;
            node->case_id = get_case_id();
            std::string node_name = clean_name(node->path.name);
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

} // namespace snt::dip
