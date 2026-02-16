#include "pch_tests.h"
#include <snt/puq/nostd/to_string.h>
#include <snt/puq/converter.h>

using namespace snt;

TEST(Converter, Initialization) {

  puq::BaseUnits bu1("km/s");
  puq::BaseUnits bu2("m/s");
  puq::Converter con(bu1, bu2); // using BaseUnits

  con = puq::Converter("ms", "s"); // using strings
}

TEST(Converter, LinearConv) {

  puq::Converter con;

  EXPECT_EQ(nostd::to_string(puq::Converter("erg", "J").convert(1)), "1e-7");      // different units
  EXPECT_EQ(nostd::to_string(puq::Converter("kg*m2/s2", "J").convert(6, 2)), "3"); // different units with magnitudes
  EXPECT_EQ(nostd::to_string(puq::Converter("mg", "g").convert(1)), "1e-3");       // different prefixes
  EXPECT_EQ(nostd::to_string(puq::Converter("1/s", "kHz").convert(6, 2)), "3e-3"); // different prefixes

  EXPECT_EQ(nostd::to_string(puq::Converter("J/erg", "").convert(2)), "2e7");  // converting to no units
  EXPECT_EQ(nostd::to_string(puq::Converter("J/erg", "1").convert(2)), "2e7"); // converting to number
}

TEST(Converter, LogarithmicConv) {
  if constexpr (!puq::Config::use_units_logarithmic) {
    GTEST_SKIP() << "Logarithmic units are disabled";
    return;
  }

  // Nepers and Bels
  EXPECT_EQ(nostd::to_string(puq::Converter("B", "dB").convert(1)), "10");
  EXPECT_EQ(nostd::to_string(puq::Converter("dB", "B").convert(1)), "0.1");
  EXPECT_EQ(nostd::to_string(puq::Converter("B", "Np").convert(1)), "1.15128");
  EXPECT_EQ(nostd::to_string(puq::Converter("Np", "B").convert(1)), "0.8686");
  EXPECT_EQ(nostd::to_string(puq::Converter("dB", "cNp").convert(1)), "11.5128");

  EXPECT_EQ(nostd::to_string(puq::Converter("AR", "dB").convert(1000)), "60");
  EXPECT_EQ(nostd::to_string(puq::Converter("dB", "AR").convert(30)), "31.6228");
  EXPECT_EQ(nostd::to_string(puq::Converter("PR", "dB").convert(1000)), "30");
  EXPECT_EQ(nostd::to_string(puq::Converter("dB", "PR").convert(6)), "3.98107");

  EXPECT_EQ(nostd::to_string(puq::Converter("AR", "Np").convert(3.16228)), "1.15129");
  EXPECT_EQ(nostd::to_string(puq::Converter("Np", "AR").convert(1)), "2.71828");
  EXPECT_EQ(nostd::to_string(puq::Converter("PR", "Np").convert(7.38)), "0.999387");
  EXPECT_EQ(nostd::to_string(puq::Converter("Np", "PR").convert(0.115)), "1.2586");

  // Decibel-milliwatts (dBm)
  EXPECT_EQ(nostd::to_string(puq::Converter("mW", "W").convert(1)), "1e-3");
  EXPECT_EQ(nostd::to_string(puq::Converter("mW", "dBm").convert(1)), "0");
  EXPECT_EQ(nostd::to_string(puq::Converter("W", "dBm").convert(10)), "40");
  EXPECT_EQ(nostd::to_string(puq::Converter("pW", "dBm").convert(10)), "-80");
  EXPECT_EQ(nostd::to_string(puq::Converter("dBm", "uW").convert(-3)), "501.187");
  EXPECT_EQ(nostd::to_string(puq::Converter("dBm", "mW").convert(22)), "158.489");

  // Decibel-milliwatts (dBmW)
  EXPECT_EQ(nostd::to_string(puq::Converter("W", "dBmW").convert(10)), "40");
  EXPECT_EQ(nostd::to_string(puq::Converter("dBmW", "uW").convert(-3)), "501.187");

  // Decibel-watts (dBW)
  EXPECT_EQ(nostd::to_string(puq::Converter("W", "dBW").convert(1)), "0");
  EXPECT_EQ(nostd::to_string(puq::Converter("dBW", "W").convert(20)), "100");
  EXPECT_EQ(nostd::to_string(puq::Converter("dBW", "kW").convert(30)), "1");

  // Conversions btw. dBm, dBmW and dBW
  EXPECT_EQ(nostd::to_string(puq::Converter("dBW", "dBm").convert(2)), "32");
  EXPECT_EQ(nostd::to_string(puq::Converter("dBm", "dBW").convert(-20)), "-50");
  EXPECT_EQ(nostd::to_string(puq::Converter("dBW", "dBmW").convert(2)), "32");
  EXPECT_EQ(nostd::to_string(puq::Converter("dBmW", "dBW").convert(-20)), "-50");
  EXPECT_EQ(nostd::to_string(puq::Converter("dBm", "dBmW").convert(2)), "2");
  EXPECT_EQ(nostd::to_string(puq::Converter("dBmW", "dBm").convert(-20)), "-20");

  // Decibel-Volt (dBV)
  EXPECT_EQ(nostd::to_string(puq::Converter("mV", "dBV").convert(100)), "-20");
  EXPECT_EQ(nostd::to_string(puq::Converter("V", "dBV").convert(1)), "0");
  EXPECT_EQ(nostd::to_string(puq::Converter("V", "dBV").convert(10)), "20");
  EXPECT_EQ(nostd::to_string(puq::Converter("dBV", "mV").convert(-60)), "1");
  EXPECT_EQ(nostd::to_string(puq::Converter("dBV", "V").convert(0)), "1");
  EXPECT_EQ(nostd::to_string(puq::Converter("dBV", "V").convert(40)), "100");

  // Decibel-microvolt (dBuV)
  EXPECT_EQ(nostd::to_string(puq::Converter("dBuV", "uV").convert(42)), "125.893");
  EXPECT_EQ(nostd::to_string(puq::Converter("uV", "dBuV").convert(1000)), "60");

  // Conversions btw. dBV and dBuV
  EXPECT_EQ(nostd::to_string(puq::Converter("dBV", "dBuV").convert(10)), "130");
  EXPECT_EQ(nostd::to_string(puq::Converter("dBuV", "dBV").convert(-20)), "-140");

  // Decibel-microamps (dBA, dBuA)
  EXPECT_EQ(nostd::to_string(puq::Converter("dBA", "A").convert(10)), "3.16228");
  EXPECT_EQ(nostd::to_string(puq::Converter("A", "dBA").convert(0.1)), "-20");
  EXPECT_EQ(nostd::to_string(puq::Converter("dBuA", "uA").convert(-40)), "0.01");
  EXPECT_EQ(nostd::to_string(puq::Converter("uA", "dBuA").convert(1000)), "60");

  // Decibel-ohm (dBOhm)
  EXPECT_EQ(nostd::to_string(puq::Converter("dBOhm", "Ohm").convert(10)), "3.16228");
  EXPECT_EQ(nostd::to_string(puq::Converter("Ohm", "dBOhm").convert(100)), "40");

  // Sound pressure level (dBSPL)
  EXPECT_EQ(nostd::to_string(puq::Converter("Pa", "dBSPL").convert(110)), "134.807");
  EXPECT_EQ(nostd::to_string(puq::Converter("dBSPL", "Pa").convert(10)), "6.32456e-5");

  // Sound intensity level (dBSIL)
  EXPECT_EQ(nostd::to_string(puq::Converter("W/m2", "dBSIL").convert(100)), "140");
  EXPECT_EQ(nostd::to_string(puq::Converter("dBSIL", "W/m2").convert(100)), "0.01");

  // Sound power level (dBSWL)
  EXPECT_EQ(nostd::to_string(puq::Converter("W", "dBSWL").convert(100)), "140");
  EXPECT_EQ(nostd::to_string(puq::Converter("dBSWL", "W").convert(100)), "0.01");

  // Conversions of a form: dBx/y -> x/y
  EXPECT_EQ(nostd::to_string(puq::Converter("dBmW/Hz", "W/Hz").convert(10)), "0.01");

  // Conversions of moment magnitude to seismic moment
  EXPECT_EQ(nostd::to_string(puq::Converter("Mw", "Mo").convert(0)), "1.12202e9");
  EXPECT_EQ(nostd::to_string(puq::Converter("Mw", "Mo").convert(9)), "3.54813e22");
  EXPECT_EQ(nostd::to_string(puq::Converter("Mo", "Mw").convert(1.12202e+12)), "2");
  EXPECT_EQ(nostd::to_string(puq::Converter("Mo", "Mw").convert(3.54813e+19)), "7");
  EXPECT_EQ(nostd::to_string(puq::Converter("Mw", "dyn*cm").convert(0)), "1.12202e16");
  EXPECT_EQ(nostd::to_string(puq::Converter("dyn*cm", "Mw").convert(1.12202e+19)), "2");
}

TEST(Converter, TemperatureConv) {
  if constexpr (!puq::Config::use_units_temperature) {
    GTEST_SKIP() << "Temperature units are disabled";
    return;
  }

  // Kelvin conversions
  EXPECT_EQ(nostd::to_string(puq::Converter("K", "Cel").convert(23)), "-250.15");
  EXPECT_EQ(nostd::to_string(puq::Converter("K", "degF").convert(23)), "-418.27");
  EXPECT_EQ(nostd::to_string(puq::Converter("K", "degR").convert(23)), "41.4");

  // Celsius conversions
  EXPECT_EQ(nostd::to_string(puq::Converter("Cel", "K").convert(23)), "296.15");
  EXPECT_EQ(nostd::to_string(puq::Converter("Cel", "degF").convert(23)), "73.4");
  EXPECT_EQ(nostd::to_string(puq::Converter("Cel", "degR").convert(23)), "533.07");

  // Frarenheit conversions
  EXPECT_EQ(nostd::to_string(puq::Converter("degF", "K").convert(23)), "268.15");
  EXPECT_EQ(nostd::to_string(puq::Converter("degF", "Cel").convert(23)), "-5");
  EXPECT_EQ(nostd::to_string(puq::Converter("degF", "degR").convert(23)), "482.67");

  // Rankine conversions
  EXPECT_EQ(nostd::to_string(puq::Converter("degR", "K").convert(23)), "12.7778");
  EXPECT_EQ(nostd::to_string(puq::Converter("degR", "Cel").convert(23)), "-260.372");
  EXPECT_EQ(nostd::to_string(puq::Converter("degR", "degF").convert(23)), "-436.67");

  // Temperature prefixes
  EXPECT_EQ(nostd::to_string(puq::Converter("Cel", "kK").convert(2300)), "2.57315");

  // Conversion of a form: T1/x = T2/y
  EXPECT_EQ(nostd::to_string(puq::Converter("Cel/m", "K/mm").convert(300)), "0.57315");
}
