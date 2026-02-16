#import <snt/puq/systems/systems.h>

namespace snt::puq {
  namespace SystemData {

    SystemDataType AU = {
        "AU", "Atomic units",
        _BASE_UNITS + _BASE_CONSTANTS + UnitListType({
                                            {"{hbar}", {UT_LIN_CST, "1", "reduced Pl. con.", false, {}}},
                                            {"{e}", {UT_LIN_CST, "1", "elementary charge", false, {}}},
                                            {"{m_e}", {UT_LIN_CST, "1", "electr. rest mass", false, {}}},
                                            {"{kap_0}", {UT_LIN_CST, "1", "permittivity", false, {}}},
                                            {"{a_0}", {UT_LIN_CST, "{#kap_0}*{#hbar}2/({#m_e}*{#e}2)", "Bohr radius", false, {}}},
                                            {"E_h", {Utype::LIN, "{#hbar}2/({#m_e}*{a_0}2)", "hartree", false, {}}},
                                            {"{alpha}", {UT_LIN_CST, "1/137", "fine-str. const.", false, {}}},
                                            {"{c}", {UT_LIN_CST, "{alpha}-1*{a_0}*E_h/{#hbar}", "speed of light", false, {}}},
                                            {"{r_e}", {UT_LIN_CST, "{alpha}2*{a_0}", "class. e. radius", false, {}}},
                                            {"{lbar}", {UT_LIN_CST, "{alpha}*{a_0}", "red. Com. wavele.", false, {}}},
                                            {"{m_p}", {UT_LIN_CST, "1836*{#m_e}", "proton mass", false, {}}},
                                        }),
        QuantityListType({
            {"l", {"{a_0}"}},
            {"m", {"{#m_e}"}},
            {"t", {"{#hbar}/E_h"}},
            {"I", {"{#e}*E_h/{#hbar}"}},
            {"Iv", {"cd"}},
            {"n", {"mol"}},
            {"the", {"rad"}},

            {"v", {"{a_0}*E_h/{#hbar}"}},
            {"p_mom", {"{#hbar}/{a_0}"}},
            {"rho_q", {"{#e}/{a_0}3"}},
            {"q", {"{#e}"}},
            {"p_e", {"{#e}*{a_0}"}},
            {"p_qe", {"{#e}*{a_0}2"}},
            {"phi_e", {"E_h/{#e}"}},
            {"E_e", {"E_h/{#e}*{a_0}"}},
            {"V_e", {"E_h/{#e}*{a_0}2"}},
            {"eps", {
                        "{#kap_0}",
                    }},
            {"alp_e", {"{#e}2*{a_0}2/E_h"}},
            {"alp_he", {"{#e}3*{a_0}3/E_h2"}},
            {"alp_hhe", {"{#e}4*{a_0}4/E_h3"}},
            {"B", {"{#hbar}/({#e}*{a_0}2)"}},
            {"mm", {"{#hbar}*{#e}/{#m_e}"}},
            {"E", {"E_h"}},
            {"F", {"E_h/{a_0}"}},
            {"S_a", {"{#hbar}"}},
        }),
        DimensionMapType({
#include "dmaps/dmap_AU.h"
        })};

    SystemDataType PU = {
        "PU", "Planck units",
        _BASE_UNITS + _BASE_CONSTANTS + UnitListType({
                                            {"{c}", {UT_LIN_CST, "1", "speed of light", false, {}}},
                                            {"{G}", {UT_LIN_CST, "1", "grav. const.", false, {}}},
                                            {"{hbar}", {UT_LIN_CST, "1", "reduced Pl. con.", false, {}}},
                                            {"{k_B}", {UT_LIN_CST, "1", "Boltzmann const.", false, {}}},
                                        }),
        QuantityListType({
            {"l", {"({#hbar}*{#G}/{#c}3)1:2"}},
            {"m", {"({#hbar}*{#c}/{#G})1:2"}},
            {"t", {"({#hbar}*{#G}/{#c}5)1:2"}},
            {"T", {"({#hbar}*{#c}5*{k_B}-2/{#G})1:2"}},

            {"A", {"{#hbar}*{#G}/{#c}3"}},
            {"V", {"({#hbar}*{#G}*{#c}-3)3:2"}},
            {"p_mom", {"({#hbar}*{#c}3/{#G})1:2"}},
            {"E", {"({#hbar}*{#c}5/{#G})1:2"}},
            {"F", {"{#c}4/{#G}"}},
            {"rho", {"{#c}5/{#hbar}*{#G}-2"}},
            {"a", {"({#c}7/{#hbar}/{#G})1:2"}},
        }),
        DimensionMapType({
#include "dmaps/dmap_PU.h"
        })};

    // https://www.seas.upenn.edu/~amyers/NaturalUnits.pdf
    SystemDataType SRU = {
        "SRU", "Units for special relativity",
        _BASE_UNITS + _BASE_CONSTANTS + UnitListType({
                                            {"{c}", {UT_LIN_CST, "1", "speed of light", false, {}}},
                                        }),
        QuantityListType({
            {"l", {"m"}},
            {"m", {"kg"}},
            {"t", {"m", "{#c}-1"}},

            {"E", {"kg", "{#c}2"}},
            {"p_mom", {"kg", "{#c}"}},
            {"v", {"1", "{#c}"}},
            {"L_ome", {"kg*m", "{#c}"}},
            {"F", {"kg/m", "{#c}2"}},
            {"a", {"m-1", "{#c}2"}},
            {"rho_E", {"kg/m3", "{#c}2"}},
        }),
        DimensionMapType({
#include "dmaps/dmap_SRU.h"
        })};

    SystemDataType GRU = {
        "GRU", "Units for general relativity",
        _BASE_UNITS + _BASE_CONSTANTS + UnitListType({
                                            {"{c}", {UT_LIN_CST, "1", "speed of light", false, {}}},
                                            {"{hbar}", {UT_LIN_CST, "1", "reduced Pl. con.", false, {}}},
                                            {"{eps_0}", {UT_LIN_CST, "1", "permit. of vac.", false, {}}},
                                            {"{k_B}", {UT_LIN_CST, "1", "Boltzmann const.", false, {}}},

                                            {"eV", {Utype::LIN, "1.602176634e-19*m2*kg*s-2", "electronvolt", true, {"k", "M", "G", "T"}}},
                                            {"{pi}", {UT_LIN_CST, "{#pi}", "pi num.", false, {}}},
                                            {"{alpha}", {UT_LIN_CST, "{#e}2/(4*{pi}*{#eps_0}*{#hbar}*{#c})", "fine-str. const.", false, {}}},
                                            {"{e}", {UT_LIN_CST, "(4*{pi}*{alpha})1:2", "elem. charge", false, {}}},
                                        }),
        QuantityListType({
            {"l", {"GeV-1", "{#hbar}*{#c}"}},
            {"m", {"GeV", "{#c}-2"}},
            {"t", {"GeV-1", "{#hbar}"}},

            {"E", {"GeV"}},
            {"p_mom", {"GeV", "{#c}-1"}},
            {"v", {"1", "{#c}"}},
            {"L_ome", {"1", "{#hbar}"}},
            {"A", {"GeV-2", "({#hbar}*{#c})2"}},
            {"F", {"GeV2", "({#hbar}*{#c})-1"}},
            {"rho_E", {"GeV4", "({#hbar}*{#c})-3"}},
            {"q", {""
                   "{#e}/{e}"}},
        }),
        DimensionMapType({
#include "dmaps/dmap_GRU.h"
        })};

    SystemDataType GEO = {
        "GEO", "Geometrized units",
        _BASE_UNITS + _BASE_CONSTANTS + UnitListType({
                                            {"{c}", {UT_LIN_CST, "1", "speed of light", false, {}}},
                                            {"{G}", {UT_LIN_CST, "1", "grav. const.", false, {}}},
                                        }),
        QuantityListType({
            {"l", {"m"}},
            {"m", {"m", "{#c}2/{#G}"}},
            {"t", {"m", "{#c}-1"}},

            {"E", {"m", "{#c}4/{#G}"}},
            {"p_mom", {"m", "{#c}3/{#G}"}},
            {"v", {"1", "{#c}"}},
            {"L_ome", {"m2", "{#c}3/{#G}"}},
            {"F", {"1", "{#c}4/{#G}"}},
            {"a", {"m-1", "{#c}2"}},
            {"rho_E", {"m-2", "{#c}4/{#G}"}},
        }),
        DimensionMapType({
#include "dmaps/dmap_GEO.h"
        })};

  } // namespace SystemData
} // namespace snt::puq

