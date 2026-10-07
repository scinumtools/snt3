#include "pch_tests.h"

#include <snt/puq/exceptions.h>
#include <snt/puq/math.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>
#include <snt/puq/result.h>
#include <snt/puq/systems/unit_system.h>
#include <snt/puq/to_string.h>

using namespace snt;

// test from https://www.quora.com/How-does-one-calculate-uncertainty-in-an-exponent
// and checked using http://www.julianibus.de/ online calculator

TEST(Math, Power) {

    {
        // Base Units
        puq::BaseUnits bu0;
        puq::BaseUnits bu1("kg*m2/s2");
        puq::Exponent exp(1, 2);
        bu0 = puq::math::pow(bu1, exp);
        EXPECT_EQ(bu0.to_string(), "kg1:2*m*s-1");
    }
    {
        // Result
        puq::Result res0;
        puq::Result res1(4.36, 0.16);
        puq::Result res2(2.35, 0.04);
        res0 = puq::math::pow(res1, 2.35);         // with a float exponent
        EXPECT_EQ(res0.to_string(), "3.18(27)e1"); // est 31.826820135086383 unc 2.74469829832924
        res0 = puq::math::pow(res1, res2);         // with an result expoenent
        EXPECT_EQ(res0.to_string(), "3.18(33)e1"); // est 31.826820135086383 unc 3.323756901862083
    }
    {
        // Measurement
        puq::Measurement msr0;
        puq::Measurement msr1(2.35, 0.04, "km2");
        puq::Measurement msr2(3.23, 0.07);
        puq::ExponentVariant exp(puq::Exponent(1, 2));
        msr0 = puq::math::pow(msr1, 2);
        EXPECT_EQ(msr0.to_string(), "5.52(19)*km4"); // est 5.522500000000001 unc 0.18800000347596324
        msr0 = puq::math::pow(msr1, exp);
        EXPECT_EQ(msr0.to_string(), "1.533(13)*km"); // est 1.5329709716755893 unc 0.013046561253560185
        msr0 = puq::math::pow(msr1, msr2);
        EXPECT_EQ(msr0.to_string(), "1.58(13)e1*km2"); // est 15.79607529212455 unc 1.2832583987008226
        try {
            puq::math::pow(msr1, msr1);
            FAIL() << "Expected puq::UnitException";
        } catch (const puq::UnitException& e) {
            EXPECT_EQ(e.info().message, "Dimension mismatch");
            EXPECT_EQ(e.info().details, "The exponent in the power function must be dimensionless.");
            EXPECT_EQ(e.info().suggestion, "Provide a dimensionless quantity as the exponent.");
        } catch (...) {
            FAIL() << "Expected puq::SyntaxException";
        }
    }
    {
        // Quantity
        puq::Quantity quant0;
        puq::Quantity quant1(2.34e4, 56, "m2");
        puq::Quantity quant2(2.1, 0.1);
        quant0 = puq::math::pow(quant1, 2);
        EXPECT_EQ(quant0.to_string(), "5.476(26)e8*m4"); // est 547560000 unc 2620820.999145508
        quant0 = puq::math::pow(quant1, quant2);
        EXPECT_EQ(quant0.to_string(), "1.5(15)e9*m2"); // est 1497453345.9230108 unc 1506531188.1869695
    }
}

TEST(Math, PowerAcrossSystems) {
    if constexpr (!puq::Config::use_system_cgs) {
        GTEST_SKIP() << "CGS unit system is disabled";
        return;
    }

    const puq::Quantity si(2.0, 0.1, "J", puq::SystemType::SI);
    const puq::Quantity esu(2e7, 1e6, "erg", puq::SystemType::ESU);
    const auto si_squared = puq::math::pow(si, 2);
    const auto esu_squared = puq::math::pow(esu, 2);
    EXPECT_EQ(si_squared.stype, puq::SystemType::SI);
    EXPECT_EQ(esu_squared.stype, puq::SystemType::ESU);
    EXPECT_EQ(si_squared.measurement.baseunits.to_string(), "J2");
    EXPECT_EQ(esu_squared.measurement.baseunits.to_string(), "erg2");
    ASSERT_TRUE(si_squared.measurement.result.uncertainty);
    ASSERT_TRUE(esu_squared.measurement.result.uncertainty);
    EXPECT_NEAR(val::ArrayValueFloat64(si_squared.measurement.result.estimate.get()).get_value(0), 4.0, 1e-12);
    EXPECT_NEAR(val::ArrayValueFloat64(si_squared.measurement.result.uncertainty.get()).get_value(0), 0.4, 1e-12);
    EXPECT_NEAR(val::ArrayValueFloat64(esu_squared.measurement.result.estimate.get()).get_value(0), 4e14, 1e2);
    EXPECT_NEAR(val::ArrayValueFloat64(esu_squared.measurement.result.uncertainty.get()).get_value(0), 4e13, 1e1);

    for (const auto& squared : {si_squared, esu_squared}) {
        const auto in_joules_squared = squared.convert("J2", puq::SystemType::SI);
        EXPECT_NEAR(val::ArrayValueFloat64(in_joules_squared.measurement.result.estimate.get()).get_value(0), 4.0,
                    1e-12);
        ASSERT_TRUE(in_joules_squared.measurement.result.uncertainty);
        EXPECT_NEAR(val::ArrayValueFloat64(in_joules_squared.measurement.result.uncertainty.get()).get_value(0), 0.4,
                    1e-12);
    }

    const auto converted_then_squared = puq::math::pow(esu.convert("J", puq::SystemType::SI), 2);
    EXPECT_EQ(converted_then_squared.stype, puq::SystemType::SI);
    EXPECT_EQ(converted_then_squared.measurement.baseunits.to_string(), "J2");
    EXPECT_NEAR(val::ArrayValueFloat64(converted_then_squared.measurement.result.estimate.get()).get_value(0), 4.0,
                1e-12);
    ASSERT_TRUE(converted_then_squared.measurement.result.uncertainty);
    EXPECT_NEAR(val::ArrayValueFloat64(converted_then_squared.measurement.result.uncertainty.get()).get_value(0), 0.4,
                1e-12);
}

TEST(Math, Exponent) {

    {
        // Result
        puq::Result res0;
        puq::Result res1(2.35, 0.04);
        res0 = puq::math::exp(res1);
        EXPECT_EQ(res0.to_string(), "1.049(42)e1"); // est 10.485569724727576 unc 0.41942280901707824
    }
    {
        // Measurement
        puq::Measurement msr0;
        puq::Measurement msr1(7.23, 0.07);
        puq::Measurement msr2(2.35, 0.04, "km3");
        msr0 = puq::math::exp(msr1);
        EXPECT_EQ(msr0.to_string(), "1.380(97)e3");
        try {
            puq::math::exp(msr2);
            FAIL() << "Expected puq::UnitException";
        } catch (const puq::UnitException& e) {
            EXPECT_EQ(e.info().message, "Dimension mismatch");
            EXPECT_EQ(e.info().details, "The exponential function accepts only dimensionless quantities.");
            EXPECT_EQ(
                e.info().suggestion, "Provide a dimensionless quantity as the argument of the exponential function."
            );
        } catch (...) {
            FAIL() << "Expected puq::SyntaxException";
        }
    }
    {
        // Quantity    TODO: implement tests with different systems
        puq::Quantity quant0;
        puq::Quantity quant1(2.35, 0.04);
        quant0 = puq::math::exp(quant1);
        EXPECT_EQ(quant0.to_string(), "1.049(42)e1");
    }
}

TEST(Math, LogarithmNatural) {

    {
        // Result
        puq::Result res0;
        puq::Result res1(4.36, 0.16);
        res0 = puq::math::log(res1);
        EXPECT_EQ(res0.to_string(), "1.472(37)"); // est 1.472472057360943 unc 0.03669724719657097
    }
    {
        // Measurement
        puq::Measurement msr0;
        puq::Measurement msr1(7.23, 0.07);
        puq::Measurement msr2(2.35, 0.04, "km3");
        msr0 = puq::math::log(msr1);
        EXPECT_EQ(msr0.to_string(), "1.9782(97)"); // est 1.9782390361706734 unc 0.009681881008027917
        try {
            puq::math::log(msr2);
            FAIL() << "Expected puq::UnitException";
        } catch (const puq::UnitException& e) {
            EXPECT_EQ(e.info().message, "Dimension mismatch");
            EXPECT_EQ(e.info().details, "The natural logarithm cannot be applied to a dimensional quantity.");
            EXPECT_EQ(
                e.info().suggestion, "Provide a dimensionless quantity as the argument of the natural logarithm."
            );
        } catch (...) {
            FAIL() << "Expected puq::SyntaxException";
        }
    }
    {
        // Quantity    TODO: implement tests with different systems
        puq::Quantity quant0;
        puq::Quantity quant1(2.35, 0.04);
        quant0 = puq::math::log(quant1);
        EXPECT_EQ(quant0.to_string(), "8.54(17)e-1"); // est 0.8544153281560676 unc 0.01702127621072691
    }
}

TEST(Math, LogarithmDecadic) {

    {
        // Result
        puq::Result res0;
        puq::Result res1(4.36, 0.16);
        res0 = puq::math::log10(res1);
        EXPECT_EQ(res0.to_string(), "6.39(16)e-1"); // est 0.6394864892685861 unc 0.01593741192351672
    }
    {
        // Measurement
        puq::Measurement msr0;
        puq::Measurement msr1(7.23, 0.07);
        puq::Measurement msr2(2.35, 0.04, "km3");
        msr0 = puq::math::log10(msr1);
        EXPECT_EQ(msr0.to_string(), "8.591(42)e-1"); // est 0.8591382972945308 unc 0.00420478755147613
        try {
            puq::math::log10(msr2);
            FAIL() << "Expected puq::UnitException";
        } catch (const puq::UnitException& e) {
            EXPECT_EQ(e.info().message, "Dimension mismatch");
            EXPECT_EQ(e.info().details, "The decadic logarithm accepts only dimensionless quantities.");
            EXPECT_EQ(
                e.info().suggestion, "Provide a dimensionless quantity as the argument of the decadic logarithm."
            );
        } catch (...) {
            FAIL() << "Expected puq::SyntaxException";
        }
    }
    {
        // Quantity    TODO: implement tests with different systems
        puq::Quantity quant0;
        puq::Quantity quant1(2.35, 0.04);
        quant0 = puq::math::log10(quant1);
        EXPECT_EQ(quant0.to_string(), "3.711(74)e-1"); // est 0.37106786227173627 unc 0.0073922463261766325
    }
}

TEST(Math, CubicRoot) {

    {
        // Result
        puq::Result res0;
        puq::Result res1(4.36, 0.16);
        res0 = puq::math::cbrt(res1);
        EXPECT_EQ(res0.to_string(), "1.634(20)"); // est 1.633661834060757 unc 0.019983631105446875
    }
    {
        // Measurement
        puq::Measurement msr0;
        puq::Measurement msr1(2.35, 0.04, "km3");
        msr0 = puq::math::cbrt(msr1);
        EXPECT_EQ(msr0.to_string(), "1.3295(75)*km"); // est 1.3295028952345866 unc 0.0075432788015916685
    }
    {
        // Quantity    TODO: implement tests with different systems
        puq::Quantity quant0;
        puq::Quantity quant1(2.35, 0.04);
        quant0 = puq::math::cbrt(quant1);
        EXPECT_EQ(quant0.to_string(), "1.3295(75)"); // est 1.3295028952345866 unc 0.007543278712773827
    }
}

TEST(Math, SquareRoot) {

    {
        // Result
        puq::Result res0;
        puq::Result res1(4.36, 0.16);
        res0 = puq::math::sqrt(res1);
        EXPECT_EQ(res0.to_string(), "2.088(38)"); // est 2.08806130178211 unc 0.03831305122048434
    }
    {
        // Measurement
        puq::Measurement msr0;
        puq::Measurement msr1(4.36, 0.16, "m2");
        msr0 = puq::math::sqrt(msr1);
        EXPECT_EQ(msr0.to_string(), "2.088(38)*m"); // est 2.08806130178211 unc 0.03831305122048434
    }
    {
        // Quantity    TODO: implement tests with different systems
        puq::Quantity quant0;
        puq::Quantity quant1(2.35, 0.04);
        quant0 = puq::math::sqrt(quant1);
        EXPECT_EQ(quant0.to_string(), "1.533(13)"); // est 1.5329709716755893 unc 0.013046561253560185
    }
}

TEST(Math, Sinus) {

    {
        // Result
        puq::Result res0;
        puq::Result res1(4.36, 0.16);
        res0 = puq::math::sin(res1);
        EXPECT_EQ(res0.to_string(), "-9.39(55)e-1"); // est 0.9385508568851079 unc 0.055222547779010256
    }
    {
        // Measurement
        puq::Measurement msr0;
        puq::Measurement msr1(7.23, 0.07);
        puq::Measurement msr2(2.35, 0.04, "km3");
        msr0 = puq::math::sin(msr1);
        EXPECT_EQ(msr0.to_string(), "8.12(41)e-1"); // est 0.8115585420741488 unc 0.040898975084413536
        try {
            puq::math::sin(msr2);
            FAIL() << "Expected puq::UnitException";
        } catch (const puq::UnitException& e) {
            EXPECT_EQ(e.info().message, "Dimension mismatch");
            EXPECT_EQ(e.info().details, "The sine function accepts only dimensionless quantities or angles.");
            EXPECT_EQ(e.info().suggestion, "Provide a dimensionless quantity or an angle as the argument of the sine function.");
        } catch (...) {
            FAIL() << "Expected puq::SyntaxException";
        }
    }
    {
        // Quantity    TODO: implement tests with different systems
        puq::Quantity quant0;
        puq::Quantity quant1(2.35, 0.04);
        quant0 = puq::math::sin(quant1);
        EXPECT_EQ(quant0.to_string(), "7.11(28)e-1"); // est 0.7114733527908443 unc 0.02810852444135037
    }
}

TEST(Math, Cosinus) {

    {
        // Result
        puq::Result res0;
        puq::Result res1(4.36, 0.16);
        res0 = puq::math::cos(res1);
        EXPECT_EQ(res0.to_string(), "-3.5(15)e-1"); // est -0.3451409698083231 unc 0.15016814032264847
    }
    {
        // Measurement
        puq::Measurement msr0;
        puq::Measurement msr1(7.23, 0.07);
        puq::Measurement msr2(2.35, 0.04, "km3");
        msr0 = puq::math::cos(msr1);
        EXPECT_EQ(msr0.to_string(), "5.84(57)e-1"); // est 0.5842711124011541 unc 0.0568091001573734
        try {
            puq::math::cos(msr2);
            FAIL() << "Expected puq::UnitException";
        } catch (const puq::UnitException& e) {
            EXPECT_EQ(e.info().message, "Dimension mismatch");
            EXPECT_EQ(e.info().details, "The cosine function accepts only dimensionless quantities or angles.");
            EXPECT_EQ(e.info().suggestion, "Provide a dimensionless quantity or an angle as the argument of the cosine function.");
        } catch (...) {
            FAIL() << "Expected puq::SyntaxException";
        }
    }
    {
        // Quantity    TODO: implement tests with different systems
        puq::Quantity quant0;
        puq::Quantity quant1(2.35, 0.04);
        quant0 = puq::math::cos(quant1);
        EXPECT_EQ(quant0.to_string(), "-7.03(28)e-1"); // est -0.702713076773554 unc 0.028458932632702272
    }
}

TEST(Math, Tangens) {

    {
        // Result
        puq::Result res0;
        puq::Result res1(4.36, 0.16);
        res0 = puq::math::tan(res1);
        EXPECT_EQ(res0.to_string(), "2.7(13)"); // est 2.7193261275424354 unc 1.3431579027667342
    }
    {
        // Measurement
        puq::Measurement msr0;
        puq::Measurement msr1(7.23, 0.07);
        puq::Measurement msr2(2.35, 0.04, "km3");
        msr0 = puq::math::tan(msr1);
        EXPECT_EQ(msr0.to_string(), "1.39(21)"); // est 1.3890102126372825 unc 0.20505448494745337
        try {
            puq::math::tan(msr2);
            FAIL() << "Expected puq::UnitException";
        } catch (const puq::UnitException& e) {
            EXPECT_EQ(e.info().message, "Dimension mismatch");
            EXPECT_EQ(e.info().details, "The tangent function accepts only dimensionless quantities or angles.");
            EXPECT_EQ(e.info().suggestion, "Provide a dimensionless quantity or an angle as the argument of the tangent function.");
        } catch (...) {
            FAIL() << "Expected puq::SyntaxException";
        }
    }
    {
        // Quantity    TODO: implement tests with different systems
        puq::Quantity quant0;
        puq::Quantity quant1(2.35, 0.04);
        quant0 = puq::math::tan(quant1);
        EXPECT_EQ(quant0.to_string(), "-1.012(81)"); // est -1.0124663625978223 unc 0.08100351713835607
    }
}

TEST(Math, TrigonometricAngles) {
    const double radians_per_degree = std::acos(-1.0) / 180.0;

    EXPECT_THROW(puq::math::sin(puq::Measurement(1.0, "sr")), puq::UnitException);

    const auto sine = puq::math::sin(puq::Measurement(30.0, 3.0, "deg"));
    EXPECT_TRUE(sine.baseunits.size() == 0);
    ASSERT_TRUE(sine.result.uncertainty);
    EXPECT_NEAR(val::ArrayValueFloat64(sine.result.estimate.get()).get_value(0), 0.5, 1e-9);
    EXPECT_NEAR(val::ArrayValueFloat64(sine.result.uncertainty.get()).get_value(0),
                std::cos(30 * radians_per_degree) * 3 * radians_per_degree, 1e-9);

    const auto cosine = puq::math::cos(puq::Measurement(60.0, 3.0, "deg"));
    EXPECT_TRUE(cosine.baseunits.size() == 0);
    ASSERT_TRUE(cosine.result.uncertainty);
    EXPECT_NEAR(val::ArrayValueFloat64(cosine.result.estimate.get()).get_value(0), 0.5, 1e-9);
    EXPECT_NEAR(val::ArrayValueFloat64(cosine.result.uncertainty.get()).get_value(0),
                std::sin(60 * radians_per_degree) * 3 * radians_per_degree, 1e-9);

    const auto tangent = puq::math::tan(puq::Measurement(45.0, 1.0, "deg"));
    EXPECT_TRUE(tangent.baseunits.size() == 0);
    ASSERT_TRUE(tangent.result.uncertainty);
    EXPECT_NEAR(val::ArrayValueFloat64(tangent.result.estimate.get()).get_value(0), 1.0, 1e-9);
    EXPECT_NEAR(val::ArrayValueFloat64(tangent.result.uncertainty.get()).get_value(0),
                2 * radians_per_degree, 1e-9);

    const auto radians = puq::math::sin(puq::Measurement(std::acos(-1.0) / 6.0, 0.1, "rad"));
    EXPECT_TRUE(radians.baseunits.size() == 0);
    EXPECT_NEAR(val::ArrayValueFloat64(radians.result.estimate.get()).get_value(0), 0.5, 1e-12);
    ASSERT_TRUE(radians.result.uncertainty);
    EXPECT_NEAR(val::ArrayValueFloat64(radians.result.uncertainty.get()).get_value(0),
                std::cos(std::acos(-1.0) / 6.0) * 0.1, 1e-12);

    if constexpr (puq::Config::use_system_cgs) {
        puq::UnitSystem active_system(puq::SystemType::ESU);
        const auto si_angle = puq::math::sin(puq::Quantity(30.0, 3.0, "deg", puq::SystemType::SI));
        EXPECT_EQ(si_angle.stype, puq::SystemType::SI);
        EXPECT_TRUE(si_angle.measurement.baseunits.size() == 0);
        EXPECT_NEAR(val::ArrayValueFloat64(si_angle.measurement.result.estimate.get()).get_value(0), 0.5, 1e-9);

        const auto esu_angle = puq::math::sin(
            puq::Quantity(std::acos(-1.0) / 6.0, 0.1, "rad", puq::SystemType::ESU)
        );
        EXPECT_EQ(esu_angle.stype, puq::SystemType::ESU);
        EXPECT_TRUE(esu_angle.measurement.baseunits.size() == 0);
        EXPECT_NEAR(val::ArrayValueFloat64(esu_angle.measurement.result.estimate.get()).get_value(0), 0.5, 1e-12);
    }
}

TEST(Math, TrigonometricAngleArrays) {
    const val::Array::ShapeType shape = {2, 2};
    puq::Result input(
        std::make_unique<val::ArrayValueFloat64>(std::vector<double>{0.0, 30.0, 60.0, 90.0}, shape),
        std::make_unique<val::ArrayValueFloat64>(std::vector<double>{1.0, 1.0, 1.0, 1.0}, shape)
    );
    const auto output = puq::math::sin(puq::Measurement(input, "deg"));
    EXPECT_TRUE(output.baseunits.size() == 0);
    EXPECT_EQ(output.shape(), shape);
    ASSERT_TRUE(output.result.uncertainty);
    EXPECT_EQ(output.result.uncertainty->get_shape(), shape);
    const val::ArrayValueFloat64 estimates(output.result.estimate.get());
    const val::ArrayValueFloat64 uncertainties(output.result.uncertainty.get());
    for (size_t i = 0; i < 4; ++i) {
        const double angle = i * 30.0 * std::acos(-1.0) / 180.0;
        EXPECT_NEAR(estimates.get_value(i), std::sin(angle), 1e-9);
        EXPECT_NEAR(uncertainties.get_value(i), std::abs(std::cos(angle)) * std::acos(-1.0) / 180.0, 1e-9);
    }
}

TEST(Math, TangensMixedArrayLimit) {
    const double half_pi = std::acos(-1.0) / 2.0;
    const val::Array::ShapeType shape = {2, 2};
    puq::Result input(
        std::make_unique<val::ArrayValueFloat64>(std::vector<double>{0.0, half_pi, half_pi / 2.0, 0.0}, shape),
        std::make_unique<val::ArrayValueFloat64>(std::vector<double>{0.1, 0.1, 0.1, 0.1}, shape)
    );

    puq::Result output = puq::math::tan(input);
    ASSERT_TRUE(output.uncertainty);
    EXPECT_EQ(output.uncertainty->get_shape(), shape);
    const auto* values = dynamic_cast<const val::ArrayValue<double>*>(output.uncertainty.get());
    ASSERT_NE(values, nullptr);
    const auto uncertainty = values->get_values();
    ASSERT_EQ(uncertainty.size(), 4);
    EXPECT_NEAR(uncertainty[0], 0.1, 1e-12);
    EXPECT_TRUE(std::isinf(uncertainty[1]));
    EXPECT_NEAR(uncertainty[2], 0.2, 1e-12);
    EXPECT_NEAR(uncertainty[3], 0.1, 1e-12);
}

TEST(Math, AbsoluteValue) {

    {
        // Result
        puq::Result res0;
        puq::Result res1(-4.36, 0.16);
        res0 = puq::math::abs(res1);
        EXPECT_EQ(res0.to_string(), "4.36(16)");
    }
    {
        // Measurement
        puq::Measurement msr0;
        puq::Measurement msr1(-4.36, 0.16, "m2");
        msr0 = puq::math::abs(msr1);
        EXPECT_EQ(msr0.to_string(), "4.36(16)*m2");
    }
    {
        // Quantity
        puq::Quantity quant0;
        puq::Quantity quant1(-4.36, 0.16, "m");
        quant0 = puq::math::abs(quant1);
        EXPECT_EQ(quant0.to_string(), "4.36(16)*m");
    }
}

TEST(Math, AbsoluteValueAcrossSystems) {
    if constexpr (!puq::Config::use_system_cgs) {
        GTEST_SKIP() << "CGS unit system is disabled";
        return;
    }

    const puq::Quantity si(-2.0, 0.1, "J", puq::SystemType::SI);
    const puq::Quantity esu(-2e7, 1e6, "erg", puq::SystemType::ESU);
    const auto si_absolute = puq::math::abs(si);
    const auto esu_absolute = puq::math::abs(esu);
    EXPECT_EQ(si_absolute.stype, puq::SystemType::SI);
    EXPECT_EQ(esu_absolute.stype, puq::SystemType::ESU);

    for (const auto& absolute : {si_absolute, esu_absolute}) {
        const auto in_joules = absolute.convert("J", puq::SystemType::SI);
        EXPECT_NEAR(val::ArrayValueFloat64(in_joules.measurement.result.estimate.get()).get_value(0), 2.0, 1e-12);
        ASSERT_TRUE(in_joules.measurement.result.uncertainty);
        EXPECT_NEAR(val::ArrayValueFloat64(in_joules.measurement.result.uncertainty.get()).get_value(0), 0.1, 1e-12);
    }
}

TEST(Math, Maximum) {

    {
        setCheckpoint("one");
        // Result
        puq::Result res0;
        setCheckpoint("1");
        puq::Result res1(4.36, 0.16);
        setCheckpoint("2");
        puq::Result res2(2.35, 0.04);
        setCheckpoint("3");
        res0 = puq::math::max(res1, res2);
        setCheckpoint("4");
        EXPECT_EQ(res0.to_string(), "4.36(16)");
        setCheckpoint("5");
    }
    {
        // Measurement
        puq::Measurement msr0;
        puq::Measurement msr1(2.35, 0.04, "km");
        puq::Measurement msr2(3.45, 0.3, "m");
        msr0 = puq::math::max(msr1, msr2);
        EXPECT_EQ(msr0.to_string(), "2.350(40)*km");
    }
    {
        // Quantity    TODO: implement tests with different systems
        puq::Quantity quant0;
        puq::Quantity quant1(2.35, 0.04, "km");
        puq::Quantity quant2(3.45, 0.3, "m");
        quant0 = puq::math::max(quant1, quant2);
        EXPECT_EQ(quant0.to_string(), "2.350(40)*km");
    }
}

TEST(Math, Minimum) {

    {
        // Result
        puq::Result res0;
        puq::Result res1(4.36, 0.16);
        puq::Result res2(2.35, 0.04);
        res0 = puq::math::min(res1, res2);
        EXPECT_EQ(res0.to_string(), "2.350(40)");
    }
    {
        // Measurement
        puq::Measurement msr0;
        puq::Measurement msr1(2.35, 0.04, "km");
        puq::Measurement msr2(3.45, 0.3, "m");
        msr0 = puq::math::min(msr1, msr2);
        EXPECT_EQ(msr0.to_string(), "3.45(30)e-3*km");
    }
    {
        // Quantity    TODO: implement tests with different systems
        puq::Quantity quant0;
        puq::Quantity quant1(2.35, 0.04, "km");
        puq::Quantity quant2(3.45, 0.3, "m");
        quant0 = puq::math::min(quant1, quant2);
        EXPECT_EQ(quant0.to_string(), "3.45(30)e-3*km");
    }
}

TEST(Math, Floor) {

    {
        // Result
        puq::Result res0;
        puq::Result res1(4.36, 0.16);
        res0 = puq::math::floor(res1);
        EXPECT_EQ(res0.to_string(), "4");
    }
    {
        // Measurement
        puq::Measurement msr0;
        puq::Measurement msr1(4.36, 0.16, "m2");
        msr0 = puq::math::floor(msr1);
        EXPECT_EQ(msr0.to_string(), "4*m2");
    }
    {
        // Quantity    TODO: implement tests with different systems
        puq::Quantity quant0;
        puq::Quantity quant1(2.35, 0.04);
        quant0 = puq::math::floor(quant1);
        EXPECT_EQ(quant0.to_string(), "2");
    }
}

TEST(Math, Ceil) {

    {
        // Result
        puq::Result res0;
        puq::Result res1(4.36, 0.16);
        res0 = puq::math::ceil(res1);
        EXPECT_EQ(res0.to_string(), "5");
    }
    {
        // Measurement
        puq::Measurement msr0;
        puq::Measurement msr1(4.36, 0.16, "m2");
        msr0 = puq::math::ceil(msr1);
        EXPECT_EQ(msr0.to_string(), "5*m2");
    }
    {
        // Quantity    TODO: implement tests with different systems
        puq::Quantity quant0;
        puq::Quantity quant1(2.35, 0.04);
        quant0 = puq::math::ceil(quant1);
        EXPECT_EQ(quant0.to_string(), "3");
    }
}

TEST(Math, Round) {

    {
        // Result
        puq::Result res0;
        puq::Result res1(4.36, 0.16);
        puq::Result res2(4.49, 0.01);
        puq::Result res3(4.49, 0.001);
        res0 = puq::math::round(res1);
        EXPECT_EQ(res0.to_string(), "4.00(50)");
        res0 = puq::math::round(res2);
        EXPECT_EQ(res0.to_string(), "4.00(50)");
        res0 = puq::math::round(res3);
        EXPECT_EQ(res0.to_string(), "4");
    }
    {
        // Measurement
        puq::Measurement msr0;
        puq::Measurement msr1(4.36, 0.16, "m2");
        msr0 = puq::math::round(msr1);
        EXPECT_EQ(msr0.to_string(), "4.00(50)*m2");
    }
    {
        // Quantity    TODO: implement tests with different systems
        puq::Quantity quant0;
        puq::Quantity quant1(2.35, 0.04);
        quant0 = puq::math::round(quant1);
        EXPECT_EQ(quant0.to_string(), "2");
    }
}
