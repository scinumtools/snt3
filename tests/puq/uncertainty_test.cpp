#include "pch_tests.h"

#include <snt/puq/value/measurement.h>

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

  puq::Measurement msr(val::ArrayValue<double>::pointer_from_vector({2, 3, 4, 5}));
  EXPECT_EQ(msr.size(), 4);
}

TEST(Uncertainty, ErrorConversionArrays) {

  val::BaseValue::PointerType val;

  val = puq::Measurement::abs_to_rel(val::ArrayValue<double>::pointer_from_vector({30, 20}),
                                     val::ArrayValue<double>::pointer_from_vector({0.3, 0.4}));
  EXPECT_EQ(val->to_string(), "[1, 2]");

  val = puq::Measurement::rel_to_abs(val::ArrayValue<double>::pointer_from_vector({30, 20}),
                                     val::ArrayValue<double>::pointer_from_vector({20, 10}));
  EXPECT_EQ(val->to_string(), "[6, 2]");
}

TEST(Uncertainty, ArithmeticsAdd) {

  puq::Measurement msr1, msr2, msr3;

  msr1 = puq::Measurement(4, 0.01);
  msr2 = puq::Measurement(1, 0.005);
  msr3 = msr1 + msr2;
  EXPECT_EQ(msr3.to_string(), "5.000(15)");
  msr1 += msr2;
  EXPECT_EQ(msr1.to_string(), "5.000(15)");

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
  EXPECT_EQ(msr3.to_string(), "3.000(15)");
  msr1 -= msr2;
  EXPECT_EQ(msr1.to_string(), "3.000(15)");

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
  EXPECT_EQ(msr3.to_string(), "2.800(75)e1");
  msr1 *= msr2;
  EXPECT_EQ(msr1.to_string(), "2.800(75)e1");

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

TEST(Uncertainty, ArithmeticsDivide) {

  puq::Measurement msr1, msr2, msr3;

  msr1 = puq::Measurement(12.0, 0.2);
  msr2 = puq::Measurement(4, 0.1);
  msr3 = msr1 / msr2;
  EXPECT_EQ(msr3.to_string(), "3.00(13)");
  msr1 /= msr2;
  EXPECT_EQ(msr1.to_string(), "3.00(13)");

  msr1 = puq::Measurement(12.0, 0.2);
  msr2 = puq::Measurement(6, 0);
  msr3 = msr1 / msr2;
  EXPECT_EQ(msr3.to_string(), "2.000(33)");
  msr1 /= msr2;
  EXPECT_EQ(msr1.to_string(), "2.000(33)");

  msr1 = puq::Measurement(6, 0);
  msr2 = puq::Measurement(12.0, 0.2);
  msr3 = msr1 / msr2;
  EXPECT_EQ(msr3.to_string(), "5.000(85)e-1");
  msr1 /= msr2;
  EXPECT_EQ(msr1.to_string(), "5.000(85)e-1");
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

  msr1 = puq::Measurement(val::ArrayValue<double>::pointer_from_vector({12.1, 22.2}),
                          val::ArrayValue<double>::pointer_from_vector({0.1, 0.2}));
  EXPECT_EQ(msr1.to_string(), "[1.210(10)e1, 2.220(20)e1]");

  msr1 = puq::Measurement(val::ArrayValue<double>::pointer_from_vector({12.1, 22.2, 32.3}),
                          val::ArrayValue<double>::pointer_from_vector({0.1, 0.2, 0.3}));
  EXPECT_EQ(msr1.to_string(), "[1.210(10)e1, 2.220(20)e1, 3.230(30)e1]");
}
