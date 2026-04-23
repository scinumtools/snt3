#include "pch_tests.h"

#include <snt/dip/dip.h>

using namespace snt;

TEST(Expressions, Logical) {

  // define scalar
  dip::DIP d;
  d.add_string("foo bool = (true && false)");
  d.add_string("bar bool = (true || false)");
  d.add_string("jerk bool = true");
  d.add_string("snap int = 2");
  d.add_string("crackle int = 3");
  d.add_string("pop bool = ({?jerk} || {?snap} == {?crackle})");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 6);

  dip::ValueNode::PointerType vnode = env.nodes.at(0);
  EXPECT_EQ(vnode->name, "foo");
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "false");

  vnode = env.nodes.at(1);
  EXPECT_EQ(vnode->name, "bar");
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "true");

  vnode = env.nodes.at(5);
  EXPECT_EQ(vnode->name, "pop");
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "true");
}

TEST(Expressions, Numerical) {

  // define scalar
  dip::DIP d;
  d.add_string("foo int = (3 + 4 / sqrt( 4 ))");
  d.add_string("bar float = (1.23 - log( 0.43 * 3. ))");
  d.add_string("jerk float = 4.234");
  d.add_string("snap int = 2");
  d.add_string("pop float = ({?jerk} + {?snap})");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 5);

  dip::ValueNode::PointerType vnode = env.nodes.at(0);
  EXPECT_EQ(vnode->name, "foo");
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "5");

  vnode = env.nodes.at(1);
  EXPECT_EQ(vnode->name, "bar");
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "0.9754");

  vnode = env.nodes.at(4);
  EXPECT_EQ(vnode->name, "pop");
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "6.234");
}

TEST(Expressions, NumericalUnits) {

  // define scalar
  dip::DIP d;
  d.add_string("foo int = 23 m");
  d.add_string("bar int = 445 cm");
  d.add_string("baz int = ({?foo} + {?bar}) mm");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 3);

  dip::ValueNode::PointerType vnode = env.nodes.at(2);
  EXPECT_EQ(vnode->name, "baz");
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->to_string(), "27450 mm");

  d = dip::DIP();
  d.add_string("foo float = 23.3 cm");
  d.add_string("bar float = 425.6 mm");
  d.add_string("baz float = ({?foo} + {?bar} + 0.1 m) dm");

  env = d.parse();
  EXPECT_EQ(env.nodes.size(), 3);

  vnode = env.nodes.at(2);
  EXPECT_EQ(vnode->name, "baz");
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->to_string(), "7.586 dm");
}

/* TODO: Implement expressions with Quantities/Measurements with units and not just values
TEST(Expressions, LogicalUnits) {

  // define scalar
  dip::DIP d;
  d.add_string("foo int = 23 m");
  d.add_string("bar bool = ({?foo} == 2300 cm)");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 2);

  dip::ValueNode::PointerType vnode = env.nodes.at(2);
  EXPECT_EQ(vnode->name, "bar");
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->to_string(), "true");
}
*/

TEST(Expressions, NestedParentheses) {

  // define scalar
  dip::DIP d;
  d.add_string("foo float = (( 23e6 erg + ( 4 (kg*m2)/s2 ) ) - 6.2415e18 eV ) J");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 1);

  dip::ValueNode::PointerType vnode = env.nodes.at(0);
  EXPECT_EQ(vnode->name, "foo");
  EXPECT_TRUE(vnode);
  // 1e7 erg == 1 J
  // 6.2415e18 eV == 1 J
  // 1 J == 1 kg*m2/s2
  // foo = 2.3 J + 4 J - 1 J = 3.3 J
  EXPECT_EQ(vnode->to_string(), "5.3 J");
}

TEST(Expressions, MultipleLines) {

  // define scalar
  dip::DIP d;
  d.add_string("foo float = (( 1 + 4 )");
  d.add_string("             - 3 )");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 1);

  dip::ValueNode::PointerType vnode = env.nodes.at(0);
  EXPECT_EQ(vnode->name, "foo");
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->to_string(), "2");

  /* TODO: Implement expressions with Quantities/Measurements with units and not just values
  d = dip::DIP();
  d.add_string("a bool = true");
  d.add_string("b float = 23.43 cm");
  d.add_string("c bool = (false || ( {?b} == 23.43 cm || ~{?a} )");
  d.add_string("          && {?a} || ~!{?c})");
  env = d.parse();
  EXPECT_EQ(env.nodes.size(), 3);

  vnode = env.nodes.at(2);
  EXPECT_EQ(vnode->name, "c");
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->to_string(), "true");
  */
}
