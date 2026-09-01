#include "pch_tests.h"

#include <snt/puq/converter.h>
#include <snt/puq/exceptions.h>

using namespace snt;

TEST(Exceptions, BaseException) {

    try {
        throw puq::Exception("message", "details", "suggestion", "source", 111);
        FAIL() << "Expected puq::Exception";
    } catch (const puq::Exception& e) {
        EXPECT_EQ(e.info().message, "message");
        EXPECT_EQ(e.info().details, "details");
        EXPECT_EQ(e.info().suggestion, "suggestion");
        EXPECT_EQ(e.info().origin->source, "source");
        EXPECT_EQ(e.info().origin->line, 111);
        EXPECT_STREQ(
            e.what(),
            "[SNT-PUQ] message\n"
            "  details:    details\n"
            "  suggestion: suggestion\n"
            "  source:     source:111"
        );
    }

    try {
        throw puq::Exception(
            core::ExceptionInfo{"message", "details", "suggestion", core::SourceLocation{"source", 111}}
        );
        FAIL() << "Expected puq::Exception";
    } catch (const puq::Exception& e) {
        EXPECT_EQ(e.info().message, "message");
        EXPECT_EQ(e.info().details, "details");
        EXPECT_EQ(e.info().suggestion, "suggestion");
        EXPECT_EQ(e.info().origin->source, "source");
        EXPECT_EQ(e.info().origin->line, 111);
        EXPECT_STREQ(
            e.what(),
            "[SNT-PUQ] message\n"
            "  details:    details\n"
            "  suggestion: suggestion\n"
            "  source:     source:111"
        );
    }
}

TEST(Exceptions, ConverterException) {

    try {
        puq::BaseUnits bu1("cm2/kg2");
        puq::BaseUnits bu2("kg2/s2");
        throw puq::ConverterException(bu1, bu2, __FILE__, __LINE__);
        FAIL() << "Expected puq::ConverterException";
    } catch (const puq::ConverterException& e) {
        EXPECT_EQ(e.info().message, "Incompatible dimensions");
        EXPECT_EQ(
            e.info().details,
            "Diemensions are different:\n"
            "         System     Unit                       Dimensions                 \n"
            "         ---------- -------------------------- -------------------------- \n"
            "From     SI         cm2*kg-2                   m2*g-2                     \n"
            "To       SI         kg2*s-2                    g2*s-2                     \n"
        );
        EXPECT_EQ(
            e.info().suggestion,
            "Possible conversions:\n"
            "System     Units                      Name                       Context    \n"
            "---------- -------------------------- -------------------------- ---------- \n"
            "BASE       m2*g-2                     MGS base units             \n"
            "BASE       m2*kg-2                    MKS base units             \n"
            "BASE       cm2*g-2                    CGS base units             \n"
        );
        EXPECT_STREQ(
            e.what(),
            "[SNT-PUQ] Incompatible dimensions\n"
            "  details:    Diemensions are different:\n"
            "         System     Unit                       Dimensions                 \n"
            "         ---------- -------------------------- -------------------------- \n"
            "From     SI         cm2*kg-2                   m2*g-2                     \n"
            "To       SI         kg2*s-2                    g2*s-2                     \n\n"
            "  suggestion: Possible conversions:\n"
            "System     Units                      Name                       Context    \n"
            "---------- -------------------------- -------------------------- ---------- \n"
            "BASE       m2*g-2                     MGS base units             \n"
            "BASE       m2*kg-2                    MKS base units             \n"
            "BASE       cm2*g-2                    CGS base units             \n\n"
            "  source:     scinumtools3/tests/puq/test_exceptions.cpp:54"
        );
    }
}
