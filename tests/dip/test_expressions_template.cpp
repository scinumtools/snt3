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
size str = f"W: {{?weight}}, H: {{?height}}"
)");
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 5);

    dip::ValueNode::PointerType vnode = env.nodes.at(3);
    EXPECT_EQ(vnode->path.name, "profile");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "\"\nAge:    23 yr\nWeight: 85.6 kg\nHeight: 184.3 cm\n\"");

    vnode = env.nodes.at(4);
    EXPECT_EQ(vnode->path.name, "size");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "\"W: 85.6 kg, H: 184.3 cm\"");
}
