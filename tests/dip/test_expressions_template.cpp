#include "pch_tests.h"

#include <snt/dip/dip.h>

using namespace snt;

TEST(ExpressionsTemplate, Basic) {

    // define scalar
    dip::DIP d;
    d.add_string("age int = 23 yr");
    d.add_string("weight float = 85.6 kg");
    d.add_string("height float = 184.3 cm");
    d.add_string(R"(
profile str = f"""
Age:    {{?age}}
Weight: {{?weight}}
Height: {{?height}}
"""
)");
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 4);

    dip::ValueNode::PointerType vnode = env.nodes.at(3);
    EXPECT_EQ(vnode->name, "profile");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "\"\nAge:    23 yr\nWeight: 85.6 kg\nHeight: 184.3 cm\"");
}
