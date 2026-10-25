#include "pch_tests.h"

#include <algorithm>
#include <cmath>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>

using namespace snt;

TEST(Uncertainty, Initialization) {

    puq::Measurement msr;
    EXPECT_EQ(msr.to_string(), "1");

    msr = puq::Measurement(0);
    EXPECT_EQ(msr.to_string(), "0");

    msr = puq::Measurement(2.34);
    EXPECT_EQ(msr.to_string(), "2.34");

    msr = puq::Measurement(2.34);
    EXPECT_EQ(msr.to_string(), "2.34");

    msr = puq::Measurement(20, 10);
    EXPECT_EQ(msr.to_string(), "2.0(10)e1");

    msr = puq::Measurement(2, 1);
    EXPECT_EQ(msr.to_string(), "2.0(10)");

    msr = puq::Measurement(2.34, 0.023);
    EXPECT_EQ(msr.to_string(), "2.340(23)");

    msr = puq::Measurement(2.34e7, 2.3e4);
    EXPECT_EQ(msr.to_string(), "2.3400(23)e7");

    msr = puq::Measurement(2.34e-7, 2.3e-9);
    EXPECT_EQ(msr.to_string(), "2.340(23)e-7");

    msr = puq::Measurement(3.234, 0.002);
    std::stringstream ss;
    ss << msr; // cast measurement as a string into a stream
    EXPECT_EQ(ss.str(), "3.2340(20)");
}

TEST(Uncertainty, ErrorConversion) {

    EXPECT_EQ(puq::Measurement::abs_to_rel(30, 0.3), 1); // 0.3/30*100 = 1%

    EXPECT_EQ(puq::Measurement::rel_to_abs(30, 20), 6); // 30*20% = 30/5 = 6
}

TEST(Uncertainty, Size) {

    std::vector<double> arr({2, 3, 4, 5});
    puq::Measurement msr(std::make_unique<val::ArrayValueFloat64>(arr));
    EXPECT_EQ(msr.size(), 4);
}

TEST(Uncertainty, ErrorConversionArrays) {

    val::BaseValue::PointerType val;

    std::vector<double> arr1({30, 20});
    std::vector<double> arr2({0.3, 0.4});
    val = puq::Measurement::abs_to_rel(
        std::make_unique<val::ArrayValueFloat64>(arr1), std::make_unique<val::ArrayValueFloat64>(arr2)
    );
    EXPECT_EQ(val->to_string(), "[1, 2]");

    arr1 = std::vector<double>({30, 20});
    arr2 = std::vector<double>({20, 10});
    val = puq::Measurement::rel_to_abs(
        std::make_unique<val::ArrayValueFloat64>(arr1), std::make_unique<val::ArrayValueFloat64>(arr2)
    );
    EXPECT_EQ(val->to_string(), "[6, 2]");
}

TEST(Uncertainty, ArithmeticsAdd) {

    puq::Measurement msr1, msr2, msr3;

    msr1 = puq::Measurement(4, 0.01);
    msr2 = puq::Measurement(1, 0.005);
    msr3 = msr1 + msr2;
    EXPECT_EQ(msr3.to_string(), "5.000(11)");
    msr1 += msr2;
    EXPECT_EQ(msr1.to_string(), "5.000(11)");

    msr1 = puq::Measurement(4, 0.01);
    msr2 = puq::Measurement(3);
    msr3 = msr1 + msr2;
    EXPECT_EQ(msr3.to_string(), "7.000(10)");
    msr1 += msr2;
    EXPECT_EQ(msr1.to_string(), "7.000(10)");
}

TEST(Uncertainty, ArithmeticsSubtract) {

    puq::Measurement msr1, msr2, msr3;

    msr1 = puq::Measurement(4, 0.01);
    EXPECT_EQ((-msr1).to_string(), "-4.000(10)");

    msr1 = puq::Measurement(4, 0.01);
    msr2 = puq::Measurement(1, 0.005);
    msr3 = msr1 - msr2;
    EXPECT_EQ(msr3.to_string(), "3.000(11)");
    msr1 -= msr2;
    EXPECT_EQ(msr1.to_string(), "3.000(11)");

    msr1 = puq::Measurement(4, 0.01);
    msr2 = puq::Measurement(3);
    msr3 = msr1 - msr2;
    EXPECT_EQ(msr3.to_string(), "1.000(10)");
    msr1 -= msr2;
    EXPECT_EQ(msr1.to_string(), "1.000(10)");
}

TEST(Uncertainty, ArithmeticsMultiply) {

    puq::Measurement msr1, msr2, msr3;

    msr1 = puq::Measurement(4.0, 0.05);
    msr2 = puq::Measurement(7, 0.1);
    msr3 = msr1 * msr2;
    EXPECT_EQ(msr3.to_string(), "2.800(53)e1");
    msr1 *= msr2;
    EXPECT_EQ(msr1.to_string(), "2.800(53)e1");

    msr1 = puq::Measurement(4.0, 0.05);
    msr2 = puq::Measurement(5, 0);
    msr3 = msr1 * msr2;
    EXPECT_EQ(msr3.to_string(), "2.000(25)e1");
    msr1 *= msr2;
    EXPECT_EQ(msr1.to_string(), "2.000(25)e1");

    msr1 = puq::Measurement(5, 0);
    msr2 = puq::Measurement(4.0, 0.05);
    msr3 = msr1 * msr2;
    EXPECT_EQ(msr3.to_string(), "2.000(25)e1");
    msr1 *= msr2;
    EXPECT_EQ(msr1.to_string(), "2.000(25)e1");
}

TEST(Uncertainty, ArrayMultiplicationPropagatesBothUncertainties) {
    const puq::Result left(
        std::make_unique<val::ArrayValueFloat64>(std::vector<double>{2.0, -3.0, 1.5}),
        std::make_unique<val::ArrayValueFloat64>(std::vector<double>{0.1, 0.2, 0.05})
    );
    const puq::Result right(
        std::make_unique<val::ArrayValueFloat64>(std::vector<double>{4.0, 2.0, -2.0}),
        std::make_unique<val::ArrayValueFloat64>(std::vector<double>{0.3, 0.1, 0.4})
    );

    const puq::Result product = left * right;
    const auto* values = dynamic_cast<const val::ArrayValue<double>*>(product.estimate.get());
    const auto* errors = dynamic_cast<const val::ArrayValue<double>*>(product.uncertainty.get());
    ASSERT_NE(values, nullptr);
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(values->get_values(), (std::vector<double>{8.0, -6.0, -3.0}));
    const std::vector<double> expected_errors{
        std::hypot(4.0 * 0.1, 2.0 * 0.3),
        std::hypot(2.0 * 0.2, -3.0 * 0.1),
        std::hypot(-2.0 * 0.05, 1.5 * 0.4)
    };
    ASSERT_EQ(errors->get_size(), expected_errors.size());
    for (std::size_t i = 0; i < expected_errors.size(); ++i)
        EXPECT_NEAR(errors->get_value(i), expected_errors[i], 1e-12);
}

TEST(Uncertainty, ArrayMultiplicationPropagatesOneUncertainty) {
    const puq::Result left(
        std::make_unique<val::ArrayValueFloat64>(std::vector<double>{2.0, 3.0}),
        std::make_unique<val::ArrayValueFloat64>(std::vector<double>{0.1, 0.2})
    );
    const puq::Result right(std::make_unique<val::ArrayValueFloat64>(std::vector<double>{4.0, 5.0}));

    const puq::Result product = left * right;
    const auto* errors = dynamic_cast<const val::ArrayValue<double>*>(product.uncertainty.get());
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(errors->get_values(), (std::vector<double>{0.4, 1.0}));
}

TEST(Uncertainty, ArithmeticsDivide) {

    puq::Measurement msr1, msr2, msr3;

    msr1 = puq::Measurement(12.0, 0.2);
    msr2 = puq::Measurement(4, 0.1);
    msr3 = msr1 / msr2;
    EXPECT_EQ(msr3.to_string(), "3.000(90)");
    msr1 /= msr2;
    EXPECT_EQ(msr1.to_string(), "3.000(90)");

    msr1 = puq::Measurement(12.0, 0.2);
    msr2 = puq::Measurement(6, 0);
    msr3 = msr1 / msr2;
    EXPECT_EQ(msr3.to_string(), "2.000(33)");
    msr1 /= msr2;
    EXPECT_EQ(msr1.to_string(), "2.000(33)");

    msr1 = puq::Measurement(6, 0);
    msr2 = puq::Measurement(12.0, 0.2);
    msr3 = msr1 / msr2;
    EXPECT_EQ(msr3.to_string(), "5.000(83)e-1");
    msr1 /= msr2;
    EXPECT_EQ(msr1.to_string(), "5.000(83)e-1");
}

TEST(Uncertainty, Comparison) {

    puq::Measurement msr1, msr2;

    msr1 = puq::Measurement(1.234, 0.001);
    msr2 = puq::Measurement(2.345, 0.002);

    EXPECT_EQ(msr1 == msr1, true);
    EXPECT_EQ(msr1 != msr1, false);
    EXPECT_EQ(msr1 == msr2, false);
    EXPECT_EQ(msr1 != msr2, true);
}

TEST(Uncertainty, Arrays) {

    puq::Measurement msr1, msr2;

    std::vector<double> arr1({12.1, 22.2});
    std::vector<double> arr2({0.1, 0.2});
    msr1 = puq::Measurement(
        std::make_unique<val::ArrayValueFloat64>(arr1), std::make_unique<val::ArrayValueFloat64>(arr2)
    );
    EXPECT_EQ(msr1.to_string(), "[1.210(10)e1, 2.220(20)e1]");

    arr1 = std::vector<double>({12.1, 22.2, 32.3});
    arr2 = std::vector<double>({0.1, 0.2, 0.3});
    msr1 = puq::Measurement(
        std::make_unique<val::ArrayValueFloat64>(arr1), std::make_unique<val::ArrayValueFloat64>(arr2)
    );
    EXPECT_EQ(msr1.to_string(), "[1.210(10)e1, 2.220(20)e1, 3.230(30)e1]");
}

TEST(Uncertainty, DensityConversionPropagatesStandardUncertainty) {
    const puq::Quantity mass(8.0, 0.2, "g");
    const puq::Quantity volume(2.0, 0.05, "cm3");
    const auto density = (mass / volume).convert("kg/m3");

    const auto* estimate = dynamic_cast<const val::ArrayValue<double>*>(density.measurement.result.estimate.get());
    const auto* uncertainty =
        dynamic_cast<const val::ArrayValue<double>*>(density.measurement.result.uncertainty.get());
    ASSERT_NE(estimate, nullptr);
    ASSERT_NE(uncertainty, nullptr);

    // First-order independent propagation combines the relative uncertainties
    // of the numerator and denominator in quadrature.
    const double center = 8.0 / 2.0;
    const double propagated = center * std::hypot(0.2 / 8.0, 0.05 / 2.0);
    EXPECT_NEAR(estimate->get_values().at(0), center * 1000.0, 1e-9);
    EXPECT_NEAR(uncertainty->get_values().at(0), propagated * 1000.0, 1e-9);
    EXPECT_EQ(density.measurement.baseunits.to_string(), "kg*m-3");
}
