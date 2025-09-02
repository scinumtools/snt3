#import "systems.h"

namespace snt::puq {
  namespace SystemData {

    SystemDataType SI = {
        "SI", "International System of Units",
        _BASE_UNITS + _BASE_CONSTANTS + UnitListType({
                                            // units of length
                                            {"au", {Utype::LIN, "149597.870691*Mm", "astr. unit", false, {}}},
                                            {"AU", {Utype::LIN, "au", "astr. unit", false, {}}},
                                            {"ly", {Utype::LIN, "[c]*yr_j", "light-year", true, {"k", "M", "G"}}},
                                            {"pc", {Utype::LIN, "3.0857e16*m", "parsec", true, {"k", "M", "G", "T"}}},
                                            {"Ao", {Utype::LIN, "1e-10*m", "Angstrom", true, {"m", "k"}}},

                                            // units of mass
                                            {"u", {Utype::LIN, "g/[N_0]", "atomic mass unit", false, {}}},
                                            {"amu", {Utype::LIN, "u", "atomic mass unit", false, {}}},
                                            {"Da", {Utype::LIN, "u", "Dalton", false, {}}},
                                            {"t", {Utype::LIN, "1e3*kg", "tonne", true, {"k", "m", "G"}}},
                                            {"ct", {Utype::LIN, "200*mg", "carat", false, {}}},

                                            // units of time
                                            {"min", {Utype::LIN, "60*s", "minute", false, {}}},
                                            {"h", {Utype::LIN, "60*min", "hour", false, {}}},
                                            {"day", {Utype::LIN, "24*h", "day", false, {}}},
                                            {"yr_t", {Utype::LIN, "365.24219*day", "tropical year", true, {"k", "m", "G"}}},
                                            {"yr_j", {Utype::LIN, "365.25*day", "Julian year", true, {"k", "m", "G"}}},
                                            {"yr_g", {Utype::LIN, "365.2425*day", "Gregorian year", true, {"k", "m", "G"}}},
                                            {"yr", {Utype::LIN, "yr_j", "year", true, {"k", "m", "G"}}},

                                            // units of temperature
                                            {"degR", {UT_LIN_TMP, "5/9*K", "degree Rankine", false, {}}},
#ifdef UNITS_TEMPERATURES
                                            {"Cel", {Utype::TMP, "K", "degree Celsius", false, {}}},
                                            {"degF", {Utype::TMP, "K", "degree Fahrenheit", false, {}}},
#endif

                                            // angular units
                                            {"deg", {Utype::LIN, "2*[pi]*rad/360", "angle degree", false, {}}},
                                            {"'", {Utype::LIN, "deg/60", "angle minute", false, {}}},
                                            {"''", {Utype::LIN, "'/60", "angle second", false, {}}},

                                            // units of surface
                                            {"ar", {Utype::LIN, "100*m2", "are", true, {"c", "d", "da", "h"}}},
                                            {"b", {Utype::LIN, "1e-28*m2", "barn", true, {"M", "k", "m", "u", "n", "p", "f", "a", "z", "y"}}},

                                            // units of volume
                                            {"l", {Utype::LIN, "dm3", "liter", true, {}}},
                                            {"L", {Utype::LIN, "l", "liter", true, {}}},

                                            // units of energy
                                            {"J", {Utype::LIN, "N*m", "Joule", true, {}}},
                                            {"eV", {Utype::LIN, "[e]*V", "electronvolt", true, {}}},
                                            {"erg", {Utype::LIN, "dyn*cm", "erg", false, {}}},
                                            {"cal", {Utype::LIN, "4.184*J", "calorie", true, {"k", "M"}}},
                                            {"Cal", {Utype::LIN, "kcal", "Calorie", false, {}}},
                                            {"Ha", {Utype::LIN, "4.3597447222071e-18*J", "Hartree", true, {"k", "M"}}},
                                            {"E_h", {Utype::LIN, "Ha", "Hartree", true, {"k", "M"}}},

                                            // units of pressure
                                            {"Pa", {Utype::LIN, "N/m2", "Pascal", true, {}}},
                                            {"atm", {Utype::LIN, "101325*Pa", "atm. pressure", false, {}}},
                                            {"bar", {Utype::LIN, "100*kPa", "bar", true, {"M", "k", "d", "c", "m"}}},
                                            {"Ba", {Utype::LIN, "0.1*Pa", "Barye", false, {}}},

                                            // units of force
                                            {"N", {Utype::LIN, "kg*m/s2", "Newton", true, {}}},
                                            {"dyn", {Utype::LIN, "g*cm/s2", "dyne", true, {}}},

                                            // units of energy absorption
                                            {"Gy", {Utype::LIN, "J/kg", "Gray", false, {}}},
                                            {"Sv", {Utype::LIN, "J/kg", "Sivert", false, {}}},

                                            // units of magnetic flux density
                                            {"T", {Utype::LIN, "Wb/m2", "Tesla", true, {}}},
                                            {"G", {Utype::LIN, "1e-4*T", "Gauss", true, {}}},

                                            // units of frequency
                                            {"Hz", {Utype::LIN, "s-1", "Hertz", true, {}}},
                                            {"Bq", {Utype::LIN, "s-1", "Becquerel", false, {}}},

                                            // units of power
                                            {"W", {Utype::LIN, "J/s", "Watt", true, {}}},
                                            {"hp", {Utype::LIN, "745.7*W", "horse power", false, {}}},

                                            // units of velocity
                                            {"kph", {Utype::LIN, "km/h", "kilometer p. hour", false, {}}},
                                            {"kn", {Utype::LIN, "0.514444*m/s", "knot", false, {}}},

                                            // units of magnetic flux
                                            {"Mx", {Utype::LIN, "1e-8*Wb", "Maxwell", false, {}}},
                                            {"Wb", {Utype::LIN, "V*s", "Weber", false, {}}},

                                            // other CGS units
                                            {"P", {Utype::LIN, "g/(cm*s)", "Poise", true, {"c"}}},
                                            {"St", {Utype::LIN, "cm2/s", "Stokes", true, {"c"}}},
                                            {"Ka", {Utype::LIN, "cm-1", "Kayser", false, {}}},
                                            {"D", {Utype::LIN, "3.33564e-30*C*m", "Debye", true, {}}},
                                            {"Oe", {Utype::LIN, "1e3*A/(4*[pi]*m)", "Oersted", false, {}}},
                                            {"Gal", {Utype::LIN, "cm/s2", "Gal", false, {}}},
                                            {"Bi", {Utype::LIN, "10*A", "Biot", false, {}}},
                                            {"Rad", {Utype::LIN, "0.01*Gy", "radiation dose", false, {}}},

                                            // other derived units
                                            {"sr", {Utype::LIN, "rad2", "steradian", false, {}}},
                                            {"lm", {Utype::LIN, "cd*sr", "lumen", false, {}}},
                                            {"C", {Utype::LIN, "A*s", "coulomb", true, {}}},
                                            {"H", {Utype::LIN, "Wb/A", "Henry", true, {}}},
                                            {"Ohm", {Utype::LIN, "V/A", "Ohm", true, {}}},
                                            {"V", {Utype::LIN, "J/C", "Volt", true, {}}},
                                            {"lx", {Utype::LIN, "lm/m2", "lux", false, {}}},
                                            {"F", {Utype::LIN, "C/V", "Farad", true, {}}},
                                            {"S", {Utype::LIN, "Ohm-1", "Siemens", true, {}}},
                                            {"kat", {Utype::LIN, "mol/s", "katal", true, {}}},
                                            {"dpt", {Utype::LIN, "1/m", "dioptre", true, {}}},
                                            {"Mo", {Utype::LIN, "N*m", "seismic moment", true, {}}},

                                            // percentages
                                            {"%", {Utype::LIN, "1e-2", "percent", false, {}}},
                                            {"ppth", {Utype::LIN, "1e-3", "promile", false, {}}},

#ifdef UNITS_LOGARITHMIC
                                            // logarithmic units and ratios
                                            {"PR", {UT_LIN_LOG, "1", "Power ratio", false, {}}},
                                            {"AR", {UT_LIN_LOG, "1", "Amplitude ratio", false, {}}},
                                            {"Np", {Utype::LOG, "1", "Nepers", true, {"c", "d"}}},
                                            {"B", {Utype::LOG, "1", "Bel", true, {"d"}}},
                                            {"Bm", {Utype::LOG, "mW", "bel-milliwatt", true, {"d"}}},
                                            {"BmW", {Utype::LOG, "mW", "bel-milliwatt", true, {"d"}}},
                                            {"BW", {Utype::LOG, "W", "bel-watt", true, {"d"}}},
                                            {"BV", {Utype::LOG, "V", "bel-volt", true, {"d"}}},
                                            {"BuV", {Utype::LOG, "uV", "bel-microvolt", true, {"d"}}},
                                            {"BA", {Utype::LOG, "A", "bel-amps", true, {"d"}}},
                                            {"BuA", {Utype::LOG, "uA", "bel-microamps", true, {"d"}}},
                                            {"BOhm", {Utype::LOG, "Ohm", "bel-ohms", true, {"d"}}},
                                            {"BSPL", {Utype::LOG, "Pa", "bel-SPL (Pa)", true, {"d"}}},
                                            {"BSIL", {Utype::LOG, "W/m2", "bel-SIL (W/m2)", true, {"d"}}},
                                            {"BSWL", {Utype::LOG, "W", "bel-SWL (W)", true, {"d"}}},
                                            {"Mw", {Utype::LOG, "Mo", "mom. magnitude", false, {}}},
#endif

                                            // dimensionless constants
                                            {"[alpha]", {UT_LIN_CST, "[#alpha]", "fine str. const.", false, {}}},
                                            {"[euler]", {UT_LIN_CST, "[#euler]", "Euler's num.", false, {}}},
                                            {"[N_0]", {UT_LIN_CST, "[#N_0]", "Avogadro's num.", false, {}}},
                                            {"[pi]", {UT_LIN_CST, "[#pi]", "pi num.", false, {}}},

                                            // natural constants
                                            {"[a_0]", {UT_LIN_CST, "[#a_0]", "Bohr radius", false, {}}},
                                            {"[c]", {UT_LIN_CST, "[#c]", "speed of light", false, {}}},
                                            {"[e]", {UT_LIN_CST, "[#e]", "elem. charge", false, {}}},
                                            {"[eps_0]", {UT_LIN_CST, "[#eps_0]", "permit. of vac.", false, {}}},
                                            {"[G]", {UT_LIN_CST, "[#G]", "grav. const.", false, {}}},
                                            {"[g]", {UT_LIN_CST, "[#g]", "grav. accel.", false, {}}},
                                            {"[h]", {UT_LIN_CST, "[#h]", "Planck const.", false, {}}},
                                            {"[hbar]", {UT_LIN_CST, "[#hbar]", "Reduced Pl. con.", false, {}}},
                                            {"[H_0]", {UT_LIN_CST, "[#H_0]", "Hubble const.", false, {}}},
                                            {"[k_B]", {UT_LIN_CST, "[#k_B]", "Boltzmann const.", false, {}}},
                                            {"[k_e]", {UT_LIN_CST, "[#k_e]", "Coulomb const.", false, {}}},
                                            {"[L_sol]", {UT_LIN_CST, "[#L_sol]", "Solar luminosity", false, {}}},
                                            {"[M_sol]", {UT_LIN_CST, "[#M_sol]", "Solar mass", false, {}}},
                                            {"[mu_0]", {UT_LIN_CST, "[#mu_0]", "permeab. of vac.", false, {}}},
                                            {"[mu_B]", {UT_LIN_CST, "[#mu_B]", "Bohr magneton", false, {}}},
                                            {"[m_e]", {UT_LIN_CST, "[#m_e]", "electron mass", false, {}}},
                                            {"[m_p]", {UT_LIN_CST, "[#m_p]", "proton mass", false, {}}},
                                            {"[m_n]", {UT_LIN_CST, "[#m_n]", "neutron mass", false, {}}},
                                            {"[R_inf]", {UT_LIN_CST, "[#R_inf]", "Rydberg constant", false, {}}},
                                            {"[R_sol]", {UT_LIN_CST, "[#R_sol]", "Solar radius", false, {}}},
                                            {"[sigma]", {UT_LIN_CST, "[#sigma]", "Stef.Bolt. const.", false, {}}},
                                            {"[N_A]", {UT_LIN_CST, "[#N_A]", "Avogadro's const.", false, {}}},
                                        }),
        QuantityListType({
            {"l", {"m"}},
            {"m", {"g"}},
            {"t", {"s"}},
            {"T", {"K"}},
            {"I", {"A"}},
            {"Iv", {"cd"}},
            {"n", {"mol"}},
            {"the", {"rad"}},
            {"v", {"m/s"}},
            {"a", {"m/s2"}},
            {"jerk", {"m/s3"}},
            {"snap", {"m/s4"}},
            {"crac", {"m/s5"}},
            {"pop", {"m/s6"}},
            {"rho", {"kg/m3"}},
            {"rho_A", {"kg/m2"}},
            {"rho_l", {"kg/m1"}},
            {"rho_E", {"J/m3"}},
            {"A", {"m2"}},
            {"V", {"m3"}},
            {"p_mom", {"kg*m/s"}},
            {"I_mom", {"kg*m2"}},
            {"ome", {"rad/s"}},
            {"a_ome", {"rad/s2"}},
            {"L_ome", {"kg*m2/s"}},
            {"F", {"N"}},
            {"freq", {"Hz"}},
            {"J_i", {"N*s"}},
            {"Abs", {"m*s"}},
            {"mu_v", {"Pa*s"}},
            {"nu_v", {"m2/s"}},
            {"eps_s", {"1"}},
            {"sig_s", {"Pa"}},
            {"E_Y", {"Pa"}},
            {"k_s", {"N/m"}},
            {"gam", {"J/m2"}},
            {"tau", {"N*m"}},
            {"E", {"J"}},
            {"S", {"J/K"}},
            {"p", {"Pa"}},
            {"mu_c", {"J/mol"}},
            {"W", {"J"}},
            {"P", {"W"}},
            {"E_sp", {"J/kg"}},
            {"C_sp", {"J/(K*kg)"}},
            {"V_sp", {"m3/kg"}},
            {"Q", {"J"}},
            {"C_p", {"J/K"}},
            {"phi_Q", {"W/m2"}},
            {"C_mol", {"mol/m3"}},
            {"E_mol", {"J/mol"}},
            {"c_mol", {"J/(K*mol)"}},
            {"S_mol", {"J/(K*mol)"}},
            {"rr", {"mol/(m3*s)"}},
            {"kap", {"W/K"}},
            {"lam_t", {"W/(m*K)"}},
            {"R_t", {"K/W"}},
            {"R_lam", {"K*m/W"}},
            {"Q_vol", {"m3/s"}},
            {"q", {"C"}},
            {"rho_q", {"C/m3"}},
            {"p_e", {"C*m"}},
            {"p_qe", {"C*m2"}},
            {"D_e", {"C/m2"}},
            {"E_e", {"V/m"}},
            {"V_e", {"V/m2"}},
            {"alp_e", {"C2*m2*J-1"}},
            {"alp_he", {"C3*m3*J-2"}},
            {"alp_hhe", {"C4*m4*J-3"}},
            {"G_e", {"S"}},
            {"sig_e", {"S/m"}},
            {"phi_e", {"V"}},
            {"Phi_E", {"V*m"}},
            {"R", {"Ohm"}},
            {"rho_e", {"Ohm*m"}},
            {"Mag", {"A/m"}},
            {"H", {"A/m"}},
            {"Phi_M", {"Wb"}},
            {"B", {"T"}},
            {"mm", {"N*m/T"}},
            {"mu", {"H/m"}},
            {"eps", {"F/m"}},
            {"C", {"F"}},
            {"J_c", {"A/m2"}},
            {"Z", {"Ohm"}},
            {"L", {"H"}},
            {"Rel", {"1/H"}},
            {"P_o", {"dpt"}},
            {"lam_w", {"m"}},
            {"k_n", {"1/m"}},
            {"k_v", {"1/m"}},
            {"E_il", {"lx"}},
            {"E_ir", {"W/m2"}},
            {"F_l", {"lm"}},
            {"In", {"W/m2"}},
            {"L_r", {"W/(m2*sr)"}},
            {"I_r", {"W/sr"}},
            {"D_a", {"Gy/s"}},
            {"H_de", {"Sv"}},
            {"D_r", {"Gy"}},
            {"A_r", {"Bq"}},
            {"S_a", {"J/Hz"}},
            {"spin", {"kg*m2*s-1"}},

        }),
        DimensionMapType({
#include "dmaps/dmap_SI.h"
        })

    };

  } // namespace SystemData
} // namespace snt::puq
