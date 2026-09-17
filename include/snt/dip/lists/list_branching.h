#ifndef DIP_BRANCHING_LIST_H
#define DIP_BRANCHING_LIST_H

#include <map>
#include <snt/dip/nodes/node_case.h>
#include <string>

namespace snt::dip {

    /** One conditional case in a DIPL branching structure. */
    struct Case {
        std::string path;   // case path up to last @ sign
        std::string code;   // code line with the case
        std::string expr;   // case logical expression
        bool value = false;          // final value of the case
        size_t branch_id = 0;        // branch ID
        size_t branch_part = 0;      // part on the branch
        size_t case_id = 0;          // case ID
        CaseType case_type = CaseType::IF; // one of the types: case/else/end
    };

    /** Branch containing the cases selected during DIPL evaluation. */
    struct Branch {
        std::vector<size_t> cases;           // list of case IDs
        std::vector<CaseType> types;         // list of case types
        std::map<std::string, size_t> nodes; // number of node definitions
    };

    /** Collection of DIPL branches and their selected cases. */
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

} // namespace snt::dip

#endif // DIP_BRANCHING_LIST_H
