#include "pch_tests.h"

#include <vector>

#ifdef MAGNITUDE_ARRAYS

TEST(Array, Initialization) {

  puq::Array a({3, 3.14, 3.14e5});
  EXPECT_EQ(a.to_string(), "{3, 3.14, ...}");

  a = puq::Array(3);
  EXPECT_EQ(a.to_string(), "3");

  a = puq::Array({3, 4});
  EXPECT_EQ(a.to_string(), "{3, 4}");

  a = puq::Array({3, 4, 5});
  EXPECT_EQ(a.to_string(), "{3, 4, ...}");

  a = puq::Array({3e12, 4.34, 5.23e2});
  EXPECT_EQ(a.to_string(), "{3e+12, 4.34, ...}");

  a = puq::Array({3, 4, 5});
  std::stringstream ss;
  ss << a; // cast array as a string into a stream
  EXPECT_EQ(ss.str(), "{3, 4, ...}");

  puq::ArrayShape s = {2, 3};
  a = puq::Array({2, 3, 4, 5, 6, 7}, s);
  EXPECT_EQ(a.shape(), s); // correct shape size
  EXPECT_THROW(puq::Array({2, 3, 4}, s),
               puq::ArraySizeException); // wrong shape size
}

TEST(Array, Arithmetics) {

  puq::Array a({3.1, 4.2, 5.3});
  puq::Array b({0.1, 1.2, 2.3});
  puq::Array c = a + b;
  EXPECT_EQ(c.to_string(), "{3.2, 5.4, ...}");
  a += b;
  EXPECT_EQ(a.to_string(), "{3.2, 5.4, ...}");

  a = puq::Array({3.1, 4.2, 5.3});
  b = puq::Array({0.1, 1.2, 2.3});
  c = a - b;
  EXPECT_EQ(c.to_string(), "{3, 3, ...}");
  a -= b;
  EXPECT_EQ(a.to_string(), "{3, 3, ...}");

  a = puq::Array({3.1, 4.2, 5.3});
  b = puq::Array({0.1, 1.2, 2.3});
  c = a * b;
  EXPECT_EQ(c.to_string(), "{0.31, 5.04, ...}");
  a *= b;
  EXPECT_EQ(a.to_string(), "{0.31, 5.04, ...}");

  a = puq::Array({3.1, 4.2, 5.3});
  b = puq::Array({0.1, 1.2, 2.3});
  c = a / b;
  EXPECT_EQ(c.to_string(), "{31, 3.5, ...}");
  a /= b;
  EXPECT_EQ(a.to_string(), "{31, 3.5, ...}");
}

TEST(Array, ArithmeticsShape) {

  puq::Array a({3.1, 4.2, 5.3, 5.4});
  puq::Array b({0.1, 1.2, 2.3, 3.4}, puq::ArrayShape({2, 2}));
  EXPECT_THROW(a + b, puq::ArraySizeException);
  EXPECT_THROW(a += b, puq::ArraySizeException);
}

TEST(Array, Comparison) {

  puq::Array a({3.1, 4.2, 5.3});
  puq::Array b({0.1, 1.2, 2.3});

  EXPECT_EQ(a == a, true);
  EXPECT_EQ(a != a, false);
  EXPECT_EQ(a == b, false);
  EXPECT_EQ(a != b, true);
}

#endif // MAGNITUDE_ARRAYS
