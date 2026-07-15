#include "pch_tests.h"

#include <snt/dip/dip.h>

using namespace snt;

TEST(SchemaList, Declaration) {

    dip::DIP d;
    d.add_string(
        "$schema human\n"
        "  height float cm\n"
        "  weight float kg\n"
        "  name str\n"
        "jerk int = 1\n"
    );
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 1);

    dip::EnvSchema schema = env.schemas.at("human");
    EXPECT_EQ(schema.nodes.size(), 3);
}

TEST(SchemaList, Assignment) {

    // TODO: needs to be debugged

    dip::DIP d;
    d.add_string(
        "$schema car\n"
        "  speed float kph\n"
        "  weight int kg\n"
        //"  jerk[bob]\n"
        //"    fup str\n"
        "jaguar : car\n"
        "  speed = 230.0\n"
        "  weight = 1450\n"
        //        "ford[focus] : car\n"
        //"  speed = 230.0\n"
        //"  weight = 1450\n"
    );
    dip::Environment env = d.parse();
    // EXPECT_EQ(env.nodes.size(), 2);

    // dip::EnvSchema schema = env.schemas.at("human");
    // EXPECT_EQ(schema.nodes.size(), 3);
}
