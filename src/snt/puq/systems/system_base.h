#ifndef PUQ_SYSTEM_BASE_H
#define PUQ_SYSTEM_BASE_H

template <typename T>
std::vector<T> operator+(const std::vector<T>& v1, const std::vector<T>& v2) {
  std::vector<T> vr(std::begin(v1), std::end(v1));
  vr.insert(std::end(vr), std::begin(v2), std::end(v2));
  return vr;
}

const std::vector<std::string> BaseUnitOrder = {"m", "g", "s", "K", "A", "cd", "mol", "rad"};

const UnitListType _BASE_UNITS = {
    {"m", {UT_LIN_BAS, "m", "meter", true, {}}},
    {"g", {UT_LIN_BAS, "g", "gram", true, {}}},
    {"s", {UT_LIN_BAS, "s", "second", true, {}}},
    {"K", {UT_LIN_BAS_TMP, "K", "Kelvin", true, {}}},
    {"A", {UT_LIN_BAS, "A", "Ampere", true, {}}},
    {"cd", {UT_LIN_BAS, "cd", "candela", true, {}}},
    {"mol", {UT_LIN_BAS, "mol", "mole", true, {}}},
    {"rad", {UT_LIN_BAS, "rad", "radian", true, {"m"}}},
};

const DimensionMapType _BASE_MAP = {
    {"m", {1.0, 0.0, {1, 0, 0, 0, 0, 0, 0, 0}}},
    {"g", {1.0, 0.0, {0, 1, 0, 0, 0, 0, 0, 0}}},
    {"s", {1.0, 0.0, {0, 0, 1, 0, 0, 0, 0, 0}}},
    {"K", {1.0, 0.0, {0, 0, 0, 1, 0, 0, 0, 0}}},
    {"A", {1.0, 0.0, {0, 0, 0, 0, 1, 0, 0, 0}}},
    {"cd", {1.0, 0.0, {0, 0, 0, 0, 0, 1, 0, 0}}},
    {"mol", {1.0, 0.0, {0, 0, 0, 0, 0, 0, 1, 0}}},
    {"rad", {1.0, 0.0, {0, 0, 0, 0, 0, 0, 0, 1}}},
};

const UnitListType _BASE_CONSTANTS = {
    // dimensionless constants
    {"[#alpha]", {UT_LIN_CSB, "7.2973525643(11)e-3", "fine str. const.", false, {}}},
    {"[#euler]", {UT_LIN_CSB, "2.718281828459", "Euler's num.", false, {}}},
    {"[#N_0]", {UT_LIN_CSB, "6.02214076e23", "Avogadro's num.", false, {}}},
    {"[#pi]", {UT_LIN_CSB, "3.1415926535", "pi num.", false, {}}},

    // natural constants
    {"[#a_0]", {UT_LIN_CSB, "5.29177210544(82)e-11*m", "Bohr radius", false, {}}},
    {"[#c]", {UT_LIN_CSB, "299792458*m/s", "speed of light", false, {}}},     // exact SI
    {"[#e]", {UT_LIN_CSB, "1.602176634e-19*A*s", "elem. charge", false, {}}}, // exact SI
    {"[#eps_0]", {UT_LIN_CSB, "8.8541878188(14)e-12*m-3*kg-1*s4*A2", "permit. of vac.", false, {}}},
    {"[#G]", {UT_LIN_CSB, "6.67430(15)e-11*m3*kg-1*s-2", "grav. const.", false, {}}},
    {"[#g]", {UT_LIN_CSB, "9.80665*m/s2", "grav. accel.", false, {}}},                     // convention
    {"[#h]", {UT_LIN_CSB, "6.62607015e-34*m2*kg*s-1", "Planck const.", false, {}}},        // exact SI
    {"[#hbar]", {UT_LIN_CSB, "1.054571817e-34*m2*kg*s-1", "Reduced Pl. con.", false, {}}}, // exact SI
    {"[#k_B]", {UT_LIN_CSB, "1.380649e-23*m2*kg*s-2*K-1", "Boltzmann const.", false, {}}}, // exact SI
    {"[#mu_0]", {UT_LIN_CSB, "1.25663706127(20)e-6*m*kg*s-2*A-2", "permeab. of vac.", false, {}}},
    {"[#mu_B]", {UT_LIN_CSB, "9.2740100657(29)e-24*m2*A", "Bohr magneton", false, {}}},
    {"[#m_e]", {UT_LIN_CSB, "9.1093837139(28)e-31*kg", "electron mass", false, {}}},
    {"[#m_p]", {UT_LIN_CSB, "1.67262192595(52)e-27*kg", "proton mass", false, {}}},
    {"[#m_n]", {UT_LIN_CSB, "1.67492749804(95)e-27*kg", "neutron mass", false, {}}},
    {"[#R_inf]", {UT_LIN_CSB, "10973731.568157(12)*m-1", "Rydberg constant", false, {}}}, // CODATA
    {"[#sigma]", {UT_LIN_CSB, "5.670374419e-8*kg*s-3*K-4", "Stef.Bolt. const.", false, {}}},
    {"[#N_A]", {UT_LIN_CSB, "6.02214076e23/mol", "Avogadro's const.", false, {}}},
    {"[#kap_0]", {UT_LIN_CSB, "1.11265005620e-13*m-3*g-1*s4*A2", "permittivity", false, {}}},

    {"[#k_e]", {UT_LIN_CSB, "8.9875517923(14)e9*m3*kg*s-4*A-2", "Coulomb const.", false, {}}}, // CODATA 2018
    {"[#H_0]", {UT_LIN_CSB, "67.36(54)*3.24076e-20*s-1", "Hubble const.", false, {}}},         // Planck 2018
    {"[#L_sol]", {UT_LIN_CSB, "3.828e26*m2*kg*s-3", "Solar luminosity", false, {}}},           // IAU definition
    {"[#M_sol]", {UT_LIN_CSB, "1.988475(92)e30*kg", "Solar mass", false, {}}},                 // Prsa, et al. 2016
    {"[#R_sol]", {UT_LIN_CSB, "6.9566(14)e8*m", "Solar radius", false, {}}},                   // Schmutz&Kosovichev 2008
};

#endif // PUQ_SYSTEM_BASE_H
