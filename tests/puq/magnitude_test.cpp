#include "pch_tests.h"

#ifdef MAGNITUDE_ERRORS

using namespace snt;

TEST(Magnitude, Initialization) {

  puq::Magnitude m;
  EXPECT_EQ(m.to_string(), "1");

  m = puq::Magnitude(0);
  EXPECT_EQ(m.to_string(), "0");

  m = puq::Magnitude(2.34);
  EXPECT_EQ(m.to_string(), "2.34");

  m = puq::Magnitude(2.34);
  EXPECT_EQ(m.to_string(), "2.34");

  m = puq::Magnitude(20, 10);
  EXPECT_EQ(m.to_string(), "2.0(10)e1");

  m = puq::Magnitude(2, 1);
  EXPECT_EQ(m.to_string(), "2.0(10)");

  m = puq::Magnitude(2.34, 0.023);
  EXPECT_EQ(m.to_string(), "2.340(23)");

  m = puq::Magnitude(2.34e7, 2.3e4);
  EXPECT_EQ(m.to_string(), "2.3400(23)e7");

  m = puq::Magnitude(2.34e-7, 2.3e-9);
  EXPECT_EQ(m.to_string(), "2.340(23)e-7");

  m = puq::Magnitude(3.234, 0.002);
  std::stringstream ss;
  ss << m; // cast magnitude as a string into a stream
  EXPECT_EQ(ss.str(), "3.2340(20)");
}

TEST(Magnitude, ErrorConversion) {

  EXPECT_EQ(puq::Magnitude::abs_to_rel(30, 0.3), 1); // 0.3/30*100 = 1%

  EXPECT_EQ(puq::Magnitude::rel_to_abs(30, 20), 6); // 30*20% = 30/5 = 6
}

#if defined(MAGNITUDE_VALUES)

TEST(Magnitude, Size) {

  puq::Magnitude m(val::ArrayValue<double>::pointer_from_vector({2, 3, 4, 5}));
  EXPECT_EQ(m.size(), 4);
}

TEST(Magnitude, ErrorConversionArrays) {

  val::BaseValue::PointerType a;

  a = puq::Magnitude::abs_to_rel(val::ArrayValue<double>::pointer_from_vector({30, 20}),
                                 val::ArrayValue<double>::pointer_from_vector({0.3, 0.4}));
  EXPECT_EQ(a->to_string(), "[1, 2]");

  a = puq::Magnitude::rel_to_abs(val::ArrayValue<double>::pointer_from_vector({30, 20}),
                                 val::ArrayValue<double>::pointer_from_vector({20, 10}));
  EXPECT_EQ(a->to_string(), "[6, 2]");
}

#endif

TEST(Magnitude, ArithmeticsAdd) {

  puq::Magnitude m1, m2, m3;

  m1 = puq::Magnitude(4, 0.01);
  m2 = puq::Magnitude(1, 0.005);
  m3 = m1 + m2;
  EXPECT_EQ(m3.to_string(), "5.000(15)");
  m1 += m2;
  EXPECT_EQ(m1.to_string(), "5.000(15)");

  m1 = puq::Magnitude(4, 0.01);
  m2 = puq::Magnitude(3);
  m3 = m1 + m2;
  EXPECT_EQ(m3.to_string(), "7.000(10)");
  m1 += m2;
  EXPECT_EQ(m1.to_string(), "7.000(10)");
}

TEST(Magnitude, ArithmeticsSubtract) {

  puq::Magnitude m1, m2, m3;

  m1 = puq::Magnitude(4, 0.01);
  EXPECT_EQ((-m1).to_string(), "-4.000(10)");

  m1 = puq::Magnitude(4, 0.01);
  m2 = puq::Magnitude(1, 0.005);
  m3 = m1 - m2;
  EXPECT_EQ(m3.to_string(), "3.000(15)");
  m1 -= m2;
  EXPECT_EQ(m1.to_string(), "3.000(15)");

  m1 = puq::Magnitude(4, 0.01);
  m2 = puq::Magnitude(3);
  m3 = m1 - m2;
  EXPECT_EQ(m3.to_string(), "1.000(10)");
  m1 -= m2;
  EXPECT_EQ(m1.to_string(), "1.000(10)");
}

TEST(Magnitude, ArithmeticsMultiply) {

  puq::Magnitude m1, m2, m3;

  m1 = puq::Magnitude(4.0, 0.05);
  m2 = puq::Magnitude(7, 0.1);
  m3 = m1 * m2;
  EXPECT_EQ(m3.to_string(), "2.800(75)e1");
  m1 *= m2;
  EXPECT_EQ(m1.to_string(), "2.800(75)e1");

  m1 = puq::Magnitude(4.0, 0.05);
  m2 = puq::Magnitude(5, 0);
  m3 = m1 * m2;
  EXPECT_EQ(m3.to_string(), "2.000(25)e1");
  m1 *= m2;
  EXPECT_EQ(m1.to_string(), "2.000(25)e1");

  m1 = puq::Magnitude(5, 0);
  m2 = puq::Magnitude(4.0, 0.05);
  m3 = m1 * m2;
  EXPECT_EQ(m3.to_string(), "2.000(25)e1");
  m1 *= m2;
  EXPECT_EQ(m1.to_string(), "2.000(25)e1");
}

TEST(Magnitude, ArithmeticsDivide) {

  puq::Magnitude m1, m2, m3;

  m1 = puq::Magnitude(12.0, 0.2);
  m2 = puq::Magnitude(4, 0.1);
  m3 = m1 / m2;
  EXPECT_EQ(m3.to_string(), "3.00(13)");
  m1 /= m2;
  EXPECT_EQ(m1.to_string(), "3.00(13)");

  m1 = puq::Magnitude(12.0, 0.2);
  m2 = puq::Magnitude(6, 0);
  m3 = m1 / m2;
  EXPECT_EQ(m3.to_string(), "2.000(33)");
  m1 /= m2;
  EXPECT_EQ(m1.to_string(), "2.000(33)");

  m1 = puq::Magnitude(6, 0);
  m2 = puq::Magnitude(12.0, 0.2);
  m3 = m1 / m2;
  EXPECT_EQ(m3.to_string(), "5.000(85)e-1");
  m1 /= m2;
  EXPECT_EQ(m1.to_string(), "5.000(85)e-1");
}

TEST(Magnitude, Comparison) {

  puq::Magnitude a, b;

  a = puq::Magnitude(1.234, 0.001);
  b = puq::Magnitude(2.345, 0.002);

  EXPECT_EQ(a == a, true);
  EXPECT_EQ(a != a, false);
  EXPECT_EQ(a == b, false);
  EXPECT_EQ(a != b, true);
}

#if defined(MAGNITUDE_VALUES)

TEST(Magnitude, Arrays) {

  puq::Magnitude m1, m2, m3;

  m1 = puq::Magnitude(val::ArrayValue<double>::pointer_from_vector({12.1, 22.2}),
                      val::ArrayValue<double>::pointer_from_vector({0.1, 0.2}));
  EXPECT_EQ(m1.to_string(), "[1.210(10)e1, 2.220(20)e1]");

  m1 = puq::Magnitude(val::ArrayValue<double>::pointer_from_vector({12.1, 22.2, 32.3}),
                      val::ArrayValue<double>::pointer_from_vector({0.1, 0.2, 0.3}));
  EXPECT_EQ(m1.to_string(), "[1.210(10)e1, 2.220(20)e1, 3.230(30)e1]");
}

#endif

#endif // MAGNITUDE_ERRORS
