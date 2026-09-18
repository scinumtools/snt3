#include <gtest/gtest.h>

#include <snt/c/puq.h>

#include <array>
#include <string>

TEST(PUQ, EvaluateAndFormat) {
    snt_puq_quantity* quantity = nullptr;
    snt_puq_error error{};

    ASSERT_EQ(snt_puq_quantity_eval("1*cm + 2*dm", &quantity, &error), 0);
    ASSERT_NE(quantity, nullptr);
    EXPECT_EQ(error.code, 0);
    EXPECT_EQ(error.message, nullptr);

    std::array<char, 64> output{};
    EXPECT_EQ(snt_puq_quantity_format(quantity, output.data(), output.size(), &error), 0);
    EXPECT_EQ(std::string(output.data()), "21*cm");

    snt_puq_quantity_free(quantity);
}

TEST(PUQ, Convert) {
    snt_puq_quantity* quantity = nullptr;
    snt_puq_quantity* converted = nullptr;
    snt_puq_error error{};

    ASSERT_EQ(snt_puq_quantity_eval("1*km", &quantity, &error), 0);
    ASSERT_EQ(snt_puq_quantity_convert(quantity, "m", &converted, &error), 0);
    ASSERT_NE(converted, nullptr);

    std::array<char, 64> output{};
    EXPECT_EQ(snt_puq_quantity_format(converted, output.data(), output.size(), &error), 0);
    EXPECT_EQ(std::string(output.data()), "1e3*m");

    snt_puq_quantity_free(converted);
    snt_puq_quantity_free(quantity);
}

TEST(PUQ, ReportsErrors) {
    snt_puq_error error{};

    EXPECT_NE(snt_puq_quantity_eval(nullptr, nullptr, &error), 0);
    EXPECT_NE(error.code, 0);
    ASSERT_NE(error.message, nullptr);
    EXPECT_NE(std::string(error.message).find("required"), std::string::npos);

    snt_puq_quantity_free(nullptr);
}
