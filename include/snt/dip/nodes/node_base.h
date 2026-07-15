#ifndef DIP_NODE_BASE_H
#define DIP_NODE_BASE_H

#include "node.h"
#include "parser.h"

#include <deque>

namespace snt::dip {

    class Environment;

    class BaseNode : public Node {
      public:
        NodeDtype dtype; // data type of a node; in Python this was 'keyword' variable in Node class
        size_t branch_id;
        size_t case_id;
        using PointerType = std::shared_ptr<BaseNode>;
        using ListType = std::deque<BaseNode::PointerType>;
        BaseNode() : dtype(NodeDtype::None) {};
        BaseNode(const BaseNode& other) = default;
        BaseNode(const NodeDtype dt) : dtype(dt) {};
        BaseNode(const Parser& parser, const NodeDtype dt) : dtype(dt), branch_id(0), case_id(0), Node(parser) {};
        virtual ~BaseNode() = default;
        virtual ListType parse(Environment& env);
        virtual bool set_property(PropertyType property, val::Array::StringType& values, std::string& units);
        virtual std::string to_string(const core::StringFormatType& format = core::StringFormatType()) const;
        virtual BaseNode::PointerType clone(const Path& pth) const;
    };

} // namespace snt::dip

#endif // DIP_NODE_BASE_H
