#include "pch_tests.h"

#include <sstream>

TEST(Quantity, Initialization) {

  puq::Quantity q; // default initialization
  EXPECT_EQ(q.to_string(), "1");

  q = puq::Quantity(1.23); // magnitude
  EXPECT_EQ(q.to_string(), "1.23");

  q = puq::Quantity(1.2, {{"k", "m", 3}}); // magnitude and dimensions
  EXPECT_EQ(q.to_string(), "1.2*km3");

  puq::UnitValue uv(1.2, {{"k", "m", 3}});
  q = puq::Quantity(uv); // UnitValue
  EXPECT_EQ(q.to_string(), "1.2*km3");

  q = puq::Quantity("23.34*g/cm3"); // from a unit expression
  EXPECT_EQ(q.to_string(), "23.34*g*cm-3");

  q = puq::Quantity(1.23, "g/cm3"); // magnitude and a unit expression
  EXPECT_EQ(q.to_string(), "1.23*g*cm-3");

  q = puq::Quantity(1.23, "2*km3"); // magnitude and a unit expression with a number
  EXPECT_EQ(q.to_string(), "2.46*km3");

#if defined(MAGNITUDE_ARRAYS)

  puq::Array a({2, 3, 4});
  q = puq::Quantity(a, "km"); // magnitude as an array
  EXPECT_EQ(q.to_string(), "{2, 3, ...}*km");

  puq::ArrayValue av = {2, 3, 4};
  q = puq::Quantity(av, "km"); // magnitude as an array value
  EXPECT_EQ(q.to_string(), "{2, 3, ...}*km");

  std::vector<double> v = {2, 3, 4, 5}; // magnitude as a vector
  q = puq::Quantity(v, "km");
  EXPECT_EQ(q.to_string(), "{2, 3, ...}*km");

#elif defined(MAGNITUDE_VALUES)

  val::BaseValue::PointerType val = val::ArrayValue<double>::pointer_from_vector({2, 3, 4, 5});
  q = puq::Quantity(std::move(val), "km");
  EXPECT_EQ(q.to_string(), "{2, 3, ...}*km");

#endif

  q = puq::Quantity("34*J");
  std::stringstream ss;
  ss << q; // cast quantity as a string into a stream
  EXPECT_EQ(ss.str(), "34*J");
}

#ifdef MAGNITUDE_ERRORS
TEST(Quantity, InitializationErrors) {

  puq::Quantity q(1.23, 0.01); // magnitudes with errors
  EXPECT_EQ(q.to_string(), "1.230(10)");

  q = puq::Quantity(1.23, 0.01, "km3"); // magnitude, errors and dimensions
  EXPECT_EQ(q.to_string(), "1.230(10)*km3");

  q = puq::Quantity(1.23, 0.01, "2*km3"); // magnitude, errors and dimensions with a number
  EXPECT_EQ(q.to_string(), "2.460(20)*km3");

  q = puq::Quantity("3.40(10)*km3"); // unit expression
  EXPECT_EQ(q.to_string(), "3.40(10)*km3");

#ifdef PREPROCESS_SYMBOLS
  q = puq::Quantity("6.23537(39)×10−12*C4⋅m4⋅J−3");
  EXPECT_EQ(q.to_string(), "6.23537(39)e-12*C4*m4*J-3");

  q = puq::Quantity("8.8541878188(14)×10−12*F⋅m−1");
  EXPECT_EQ(q.to_string(), "8.8541878188(14)e-12*F*m-1");
#endif

#if defined(MAGNITUDE_ARRAYS)

  puq::Array am({2, 3, 4, 5}); // from Array
  puq::Array ae({0.2, 0.3, 0.4, 0.5});
  q = puq::Quantity(am, ae, "km");
  EXPECT_EQ(q.to_string(), "{2.00(20), 3.00(30), ...}*km");

  puq::ArrayValue avm = {2, 3, 4, 5}; // from ArrayValue
  puq::ArrayValue ave = {0.2, 0.3, 0.4, 0.5};
  q = puq::Quantity(avm, ave, "km");
  EXPECT_EQ(q.to_string(), "{2.00(20), 3.00(30), ...}*km");

  std::vector<double> vm = {2, 3, 4, 5}; // from a vector
  std::vector<double> ve = {0.2, 0.3, 0.4, 0.5};
  q = puq::Quantity(vm, ve, "km");
  EXPECT_EQ(q.to_string(), "{2.00(20), 3.00(30), ...}*km");

#elif defined(MAGNITUDE_VALUES)

  val::BaseValue::PointerType vm = val::ArrayValue<double>::pointer_from_vector({2, 3, 4, 5});
  val::BaseValue::PointerType ve = val::ArrayValue<double>::pointer_from_vector({0.2, 0.3, 0.4, 0.5});
  q = puq::Quantity(std::move(vm), std::move(ve), "km");
  EXPECT_EQ(q.to_string(), "{2.00(20), 3.00(30), ...}*km");

#endif
}
#endif

#if defined(MAGNITUDE_ARRAYS) || defined(MAGNITUDE_VALUES)
TEST(Quantity, InitializationArrays) {

  puq::Quantity q("{2,3.4,5e6}*km/s"); // unit expression
  EXPECT_EQ(q.to_string(), "{2, 3.4, ...}*km*s-1");

  q = puq::Quantity("{2, 3.4, 5e6}*km/s"); // unit expression with an empty space
  EXPECT_EQ(q.to_string(), "{2, 3.4, ...}*km*s-1");

#if defined(MAGNITUDE_ARRAYS)

  q = puq::Quantity(puq::Array({2, 3.4, 5e6}), "km2"); // magnitudes and units
  EXPECT_EQ(q.to_string(), "{2, 3.4, ...}*km2");

#elif defined(MAGNITUDE_VALUES)

  val::BaseValue::PointerType val = val::ArrayValue<double>::pointer_from_vector({2, 3.4, 5e6});
  q = puq::Quantity(std::move(val), "km2"); // magnitudes and units
  EXPECT_EQ(q.to_string(), "{2, 3.4, ...}*km2");

#endif

#ifdef PREPROCESS_SYMBOLS
  q = puq::Quantity("{2,3.4,5×106}*C4⋅m4⋅J−3");
  EXPECT_EQ(q.to_string(), "{2, 3.4, ...}*C4*m4*J-3");
#endif
}
#endif

#if defined(MAGNITUDE_ARRAYS)

TEST(Quantity, Size) {

  puq::Quantity q(puq::Array({2, 3, 4, 5}));
  EXPECT_EQ(q.size(), 4);
}

TEST(Quantity, Shape) {

  puq::ArrayValue v = {2, 3, 4, 5, 6, 7};
  puq::ArrayShape s = {2, 3};
  puq::Quantity q(puq::Array(v, s));
  EXPECT_EQ(q.shape(), s);
}

#elif defined(MAGNITUDE_VALUES)

TEST(Quantity, Size) {

  val::BaseValue::PointerType val = val::ArrayValue<double>::pointer_from_vector({2, 3, 4, 5});
  puq::Quantity q(std::move(val));
  EXPECT_EQ(q.size(), 4);
}

TEST(Quantity, Shape) {

  val::Array::ShapeType shape({2, 3});
  std::vector<double> arr = {2, 3, 4, 5};
  val::BaseValue::PointerType val = std::make_unique<val::ArrayValue<double>>(arr, shape);
  puq::Quantity q(std::move(val));
  EXPECT_EQ(q.shape(), shape);
}

#endif

TEST(Quantity, UnitConversion) {

  puq::Quantity q;

  q = puq::Quantity(6, "cm").convert("km");
  EXPECT_EQ(q.to_string(), "6e-5*km");

  q = puq::Quantity(1, "au").convert("km");
  EXPECT_EQ(q.to_string(), "1.49598e8*km");
}

TEST(Quantity, ArithmeticsAdd) {

  puq::Quantity q1, q2, q3;

  q1 = puq::Quantity(6, "cm"); // same units
  q2 = puq::Quantity(3, "cm");
  q3 = q1 + q2;
  EXPECT_EQ(q3.to_string(), "9*cm");
  q1 += q2;
  EXPECT_EQ(q1.to_string(), "9*cm");

  q3 = puq::Quantity(3, "cm2"); // different units
  EXPECT_THROW(q1 + q3, puq::ConvDimExcept);

  q1 = +puq::Quantity(6, "cm"); // unary addition
  EXPECT_EQ(q1.to_string(), "6*cm");

  q1 = puq::Quantity(4); // adding scalars
  EXPECT_EQ((2 + q1).to_string(), "6");
  EXPECT_EQ((q1 + 3).to_string(), "7");

#if defined(MAGNITUDE_ARRAYS)

  q1 = puq::Quantity(4); // adding arrays
  puq::Array a({2, 3, 4});
  EXPECT_EQ((a + q1).to_string(), "{6, 7, ...}");
  EXPECT_EQ((q1 + a).to_string(), "{6, 7, ...}");

#elif defined(MAGNITUDE_VALUES)

  q1 = puq::Quantity(4); // adding arrays
  val::BaseValue::PointerType val = val::ArrayValue<double>::pointer_from_vector({2, 3, 4});
  EXPECT_EQ((val->clone() + q1).to_string(), "{6, 7, ...}");
  EXPECT_EQ((q1 + val->clone()).to_string(), "{6, 7, ...}");

#endif
}

TEST(Quantity, ArithmeticsSubtract) {

  puq::Quantity q1, q2, q3;

  q1 = puq::Quantity(6, "cm"); // same units
  q2 = puq::Quantity(3, "cm");
  q3 = q1 - q2;
  EXPECT_EQ(q3.to_string(), "3*cm");
  q1 -= q2;
  EXPECT_EQ(q1.to_string(), "3*cm");

  q3 = puq::Quantity(3, "cm2"); // differet units
  EXPECT_THROW(q1 - q3, puq::ConvDimExcept);

  q1 = -puq::Quantity(6, "cm"); // unary subtraction
  EXPECT_EQ(q1.to_string(), "-6*cm");

  q1 = puq::Quantity(4); // subtracting scalars
  EXPECT_EQ((2 - q1).to_string(), "-2");
  EXPECT_EQ((q1 - 3).to_string(), "1");

#if defined(MAGNITUDE_ARRAYS)

  q1 = puq::Quantity(4); // subtracting arrays
  puq::Array a({2, 3, 4});
  EXPECT_EQ((a - q1).to_string(), "{-2, -1, ...}");
  EXPECT_EQ((q1 - a).to_string(), "{2, 1, ...}");

#elif defined(MAGNITUDE_VALUES)

  q1 = puq::Quantity(4); // subtracting arrays
  val::BaseValue::PointerType val = val::ArrayValue<double>::pointer_from_vector({2, 3, 4});
  EXPECT_EQ((val->clone() - q1).to_string(), "{-2, -1, ...}");
  EXPECT_EQ((q1 - val->clone()).to_string(), "{2, 1, ...}");

#endif
}

TEST(Quantity, ArithmeticsMultiply) {

  puq::Quantity q1, q2, q3;

  q1 = puq::Quantity(6, "cm3"); // basic multiplication
  q2 = puq::Quantity(3, "g2");
  q3 = q1 * q2;
  EXPECT_EQ(q3.to_string(), "18*cm3*g2");
  q1 *= q2;
  EXPECT_EQ(q1.to_string(), "18*cm3*g2");

  q1 = puq::Quantity(4, "cm"); // multiplying scalars
  EXPECT_EQ((2 * q1).to_string(), "8*cm");
  EXPECT_EQ((q1 * 3).to_string(), "12*cm");

#if defined(MAGNITUDE_ARRAYS)

  q1 = puq::Quantity(4); // multiplying arrays
  puq::Array a({2, 3, 4});
  EXPECT_EQ((a * q1).to_string(), "{8, 12, ...}");
  EXPECT_EQ((q1 * a).to_string(), "{8, 12, ...}");

#elif defined(MAGNITUDE_VALUES)

  q1 = puq::Quantity(4); // multiplying arrays
  val::BaseValue::PointerType val = val::ArrayValue<double>::pointer_from_vector({2, 3, 4});
  EXPECT_EQ((val->clone() * q1).to_string(), "{8, 12, ...}");
  EXPECT_EQ((q1 * val->clone()).to_string(), "{8, 12, ...}");

#endif
}

TEST(Quantity, ArithmeticsDivide) {

  puq::Quantity q1, q2, q3;

  q1 = puq::Quantity(6, "cm3");
  q2 = puq::Quantity(3, "g2");
  q3 = q1 / q2;
  EXPECT_EQ(q3.to_string(), "2*cm3*g-2");
  q1 /= q2;
  EXPECT_EQ(q1.to_string(), "2*cm3*g-2");

  q1 = puq::Quantity(2, "cm"); // dividing scalars
  EXPECT_EQ((4 / q1).to_string(), "2*cm-1");
  EXPECT_EQ((q1 / 0.5).to_string(), "4*cm");

#if defined(MAGNITUDE_ARRAYS)

  q1 = puq::Quantity(4); // dividing arrays
  puq::Array a({2, 3, 4});
  EXPECT_EQ((a / q1).to_string(), "{0.5, 0.75, ...}");
  EXPECT_EQ((q1 / a).to_string(), "{2, 1.33333, ...}");

#elif defined(MAGNITUDE_VALUES)

  q1 = puq::Quantity(4); // dividing arrays
  val::BaseValue::PointerType val = val::ArrayValue<double>::pointer_from_vector({2, 3, 4});
  EXPECT_EQ((val->clone() / q1).to_string(), "{0.5, 0.75, ...}");
  EXPECT_EQ((q1 / val->clone()).to_string(), "{2, 1.33333, ...}");

#endif
}

TEST(Quantity, RebaseUnits) {

  puq::Quantity q("23*cm*m2*kg*mg");
  q = q.rebase_prefixes();
  EXPECT_EQ(q.to_string(), "0.23*kg2*cm3");

  q = puq::Quantity("kph2*s3/(h*[c]3)");
  q = q.rebase_dimensions();
  EXPECT_EQ(q.to_string(), "2.20967e-31*kph-1*s2");
}
