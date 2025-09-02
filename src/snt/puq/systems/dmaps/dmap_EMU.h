/*
 * Do not modify this file!
 * This file can be updated using 'dmap' executable.
 *
 * Unit system:  Electromagnetic CGS units (EMU)
 * Last update:  Mon Oct  7 14:17:44 2024
 * Code version: 1.2.8
 *
 * Symbol legend:
 * ..    units
 * [..]  constants
 * [#..] collapsed constants
 * <..>  quantities
 * |..|  quantity SI conversion factors
 */
{"rad", {1, 0, {0, 0, 0, 0, 0, 0, 0, 1}}},                                           // radian
    {"cd", {1, 0, {0, 0, 0, 0, 0, 1, 0, 0}}},                                        // candela
    {"K", {1, 0, {0, 0, 0, 1, 0, 0, 0, 0}}},                                         // Kelvin
    {"A", {1, 0, {0, 0, 0, 0, 1, 0, 0, 0}}},                                         // Ampere
    {"s", {1, 0, {0, 0, 1, 0, 0, 0, 0, 0}}},                                         // second
    {"g", {1, 0, {0, 1, 0, 0, 0, 0, 0, 0}}},                                         // gram
    {"mol", {1, 0, {0, 0, 0, 0, 0, 0, 1, 0}}},                                       // mole
    {"m", {1, 0, {1, 0, 0, 0, 0, 0, 0, 0}}},                                         // meter
    {"abH", {0.01, 0, {1, 0, 0, 0, 0, 0, 0, 0}}},                                    // Abhenry
    {"abV", {0.001, 0, {(FRC){3, 2}, (FRC){1, 2}, -2, 0, 0, 0, 0, 0}}},              // Abvolt
    {"abF", {100, 0, {-1, 0, 2, 0, 0, 0, 0, 0}}},                                    // Abfarad
    {"abOhm", {0.01, 0, {1, 0, 1, 0, 0, 0, 0, 0}}},                                  // Abohm
    {"Rayl", {10000, 0, {-2, 1, -1, 0, 0, 0, 0, 0}}},                                // Rayl
    {"Gal", {0.01, 0, {1, 0, -2, 0, 0, 0, 0, 0}}},                                   // Gal
    {"Oe", {10, 0, {(FRC){-1, 2}, (FRC){1, 2}, -1, 0, 0, 0, 0, 0}}},                 // Oersted
    {"St", {0.0001, 0, {2, 0, -1, 0, 0, 0, 0, 0}}},                                  // Stokes
    {"Gb", {0.1, 0, {(FRC){1, 2}, (FRC){1, 2}, -1, 0, 0, 0, 0, 0}}},                 // Gilbert
    {"[a_0]", {5.291772105e-11, 0, {1, 0, 0, 0, 0, 0, 0, 0}}},                       // Bohr radius
    {"[m_e]", {9.10938371e-28, 0, {0, 1, 0, 0, 0, 0, 0, 0}}},                        // electron mass
    {"Pma", {0.0001, 0, {2, 0, 0, 0, 0, 0, 0, 0}}},                                  // permeability
    {"dyn", {0.01, 0, {1, 1, -2, 0, 0, 0, 0, 0}}},                                   // dyne
    {"[m_u]", {1.660539069e-24, 0, {0, 1, 0, 0, 0, 0, 0, 0}}},                       // atom. mass const.
    {"P", {100, 0, {-1, 1, -1, 0, 0, 0, 0, 0}}},                                     // Poise
    {"G", {10, 0, {(FRC){-1, 2}, (FRC){1, 2}, -1, 0, 0, 0, 0, 0}}},                  // Gauss
    {"[pi]", {3.1415926, 0, {0, 0, 0, 0, 0, 0, 0, 0}}},                              // pi num.
    {"[alpha]", {0.007297352564, 0, {0, 0, 0, 0, 0, 0, 0, 0}}},                      // fine-str. const.
    {"Ka", {100, 0, {-1, 0, 0, 0, 0, 0, 0, 0}}},                                     // Kayser
    {"[c]", {299792458, 0, {1, 0, -1, 0, 0, 0, 0, 0}}},                              // speed of light
    {"Bi", {0.1, 0, {(FRC){1, 2}, (FRC){1, 2}, -1, 0, 0, 0, 0, 0}}},                 // biot
    {"abA", {0.1, 0, {(FRC){1, 2}, (FRC){1, 2}, -1, 0, 0, 0, 0, 0}}},                // Abampere
    {"abC", {0.1, 0, {(FRC){1, 2}, (FRC){1, 2}, 0, 0, 0, 0, 0, 0}}},                 // Abcoulomb
    {"aC", {0.1, 0, {(FRC){1, 2}, (FRC){1, 2}, 0, 0, 0, 0, 0, 0}}},                  // Abcoulomb
    {"Ba", {100, 0, {-1, 1, -2, 0, 0, 0, 0, 0}}},                                    // Barye
    {"Mx", {1e-05, 0, {(FRC){5, 2}, (FRC){1, 2}, -1, 0, 0, 0, 0, 0}}},               // Maxwell
    {"[G]", {6.6743e-14, 0, {3, -1, -2, 0, 0, 0, 0, 0}}},                            // gravit. const.
    {"erg", {0.0001, 0, {2, 1, -2, 0, 0, 0, 0, 0}}},                                 // erg
    {"[hbar]", {1.054571817e-31, 0, {2, 1, -1, 0, 0, 0, 0, 0}}},                     // reduced Pl. con.
    {"[k]", {1.380649e-20, 0, {2, 1, -2, -1, 0, 0, 0, 0}}},                          // Boltzmann const.
    {"[e]", {1.602176634e-21, 0, {(FRC){1, 2}, (FRC){1, 2}, 0, 0, 0, 0, 0, 0}}},     // elementary charge
    {"b", {100, 0, {-1, 1, -2, 0, 0, 0, 0, 0}}},                                     // Barye
    {"[mu_B]", {9.274010066e-26, 0, {(FRC){5, 2}, (FRC){1, 2}, -1, 0, 0, 0, 0, 0}}}, // Bohr magneton
    {"radd", {0.01, 0, {2, 0, -2, 0, 0, 0, 0, 0}}},                                  // radiation dose
    {"[h]", {6.62607015e-31, 0, {2, 1, -1, 0, 0, 0, 0, 0}}},                         // Planck const.
    {"<phi_e>", {0.001, 0, {(FRC){3, 2}, (FRC){1, 2}, -2, 0, 0, 0, 0, 0}}},          // electric potential
    {"|phi_e|", {1e-08, 0, {0, 0, 0, 0, 0, 0, 0, 0}}},                               // electric potential SI factor
    {"<C>", {100, 0, {-1, 0, 2, 0, 0, 0, 0, 0}}},                                    // capacitance
    {"|C|", {1000000000, 0, {0, 0, 0, 0, 0, 0, 0, 0}}},                              // capacitance SI factor
    {"<eps>", {10000, 0, {-2, 0, 2, 0, 0, 0, 0, 0}}},                                // permittivity
    {"|eps|", {7957747290.3393, 0, {0, 0, 0, 0, 0, 0, 0, 0}}},                       // permittivity SI factor
    {"<I>", {0.1, 0, {(FRC){1, 2}, (FRC){1, 2}, -1, 0, 0, 0, 0, 0}}},                // current
    {"|I|", {10, 0, {0, 0, 0, 0, 0, 0, 0, 0}}},                                      // current SI factor
    {"<k_n>", {100, 0, {-1, 0, 0, 0, 0, 0, 0, 0}}},                                  // wave number
    {"<the>", {1, 0, {0, 0, 0, 0, 0, 0, 0, 1}}},                                     // angular displacement
    {"<nu_v>", {0.0001, 0, {2, 0, -1, 0, 0, 0, 0, 0}}},                              // kinematic viscosity
    {"<P>", {0.0001, 0, {2, 1, -3, 0, 0, 0, 0, 0}}},                                 // power
    {"<D_e>", {1000, 0, {(FRC){-3, 2}, (FRC){1, 2}, 0, 0, 0, 0, 0, 0}}},             // electric displacement field
    {"|D_e|", {7957.7472903393, 0, {0, 0, 0, 0, 0, 0, 0, 0}}},                       // electric displacement field SI factor
    {"<Phi_E>", {0.1, 0, {(FRC){1, 2}, (FRC){1, 2}, 0, 0, 0, 0, 0, 0}}},             // electric flux
    {"|Phi_E|", {0.79577472903393, 0, {0, 0, 0, 0, 0, 0, 0, 0}}},                    // electric flux SI factor
    {"<Iv>", {1, 0, {0, 0, 0, 0, 0, 1, 0, 0}}},                                      // luminous intensity
    {"<p_e>", {0.001, 0, {(FRC){3, 2}, (FRC){1, 2}, 0, 0, 0, 0, 0, 0}}},             // electric dipole moment
    {"|p_e|", {0.1, 0, {0, 0, 0, 0, 0, 0, 0, 0}}},                                   // electric dipole moment SI factor
    {"<mu>", {1, 0, {0, 0, 0, 0, 0, 0, 0, 0}}},                                      // permeability
    {"|mu|", {1.25663704e-06, 0, {0, 0, 0, 0, 0, 0, 0, 0}}},                         // permeability SI factor
    {"<a>", {0.01, 0, {1, 0, -2, 0, 0, 0, 0, 0}}},                                   // acceleration
    {"<rho_e>", {0.0001, 0, {2, 0, 1, 0, 0, 0, 0, 0}}},                              // electrical resistivity
    {"|rho_e|", {1e-11, 0, {0, 0, 0, 0, 0, 0, 0, 0}}},                               // electrical resistivity SI factor
    {"<B>", {10, 0, {(FRC){-1, 2}, (FRC){1, 2}, -1, 0, 0, 0, 0, 0}}},                // magnetic flux density
    {"|B|", {0.0001, 0, {0, 0, 0, 0, 0, 0, 0, 0}}},                                  // magnetic flux density SI factor
    {"<v>", {0.01, 0, {1, 0, -1, 0, 0, 0, 0, 0}}},                                   // velocity
    {"<l>", {0.01, 0, {1, 0, 0, 0, 0, 0, 0, 0}}},                                    // length
    {"<H>", {10, 0, {(FRC){-1, 2}, (FRC){1, 2}, -1, 0, 0, 0, 0, 0}}},                // magnetic field strenght
    {"|H|", {79.577472903393, 0, {0, 0, 0, 0, 0, 0, 0, 0}}},                         // magnetic field strenght SI factor
    {"<E>", {0.0001, 0, {2, 1, -2, 0, 0, 0, 0, 0}}},                                 // energy
    {"<mu_v>", {100, 0, {-1, 1, -1, 0, 0, 0, 0, 0}}},                                // dynamic viscosity
    {"<p>", {100, 0, {-1, 1, -2, 0, 0, 0, 0, 0}}},                                   // pressure
    {"<q>", {0.1, 0, {(FRC){1, 2}, (FRC){1, 2}, 0, 0, 0, 0, 0, 0}}},                 // electric charge
    {"|q|", {10, 0, {0, 0, 0, 0, 0, 0, 0, 0}}},                                      // electric charge SI factor
    {"<E_e>", {0.1, 0, {(FRC){1, 2}, (FRC){1, 2}, -2, 0, 0, 0, 0, 0}}},              // electric field strength
    {"|E_e|", {1e-06, 0, {0, 0, 0, 0, 0, 0, 0, 0}}},                                 // electric field strength SI factor
    {"<t>", {1, 0, {0, 0, 1, 0, 0, 0, 0, 0}}},                                       // time
    {"<n>", {1, 0, {0, 0, 0, 0, 0, 0, 1, 0}}},                                       // amount of substance
    {"<m>", {1, 0, {0, 1, 0, 0, 0, 0, 0, 0}}},                                       // mass
    {"<F>", {0.01, 0, {1, 1, -2, 0, 0, 0, 0, 0}}},                                   // force
    {"<Phi_M>", {1e-05, 0, {(FRC){5, 2}, (FRC){1, 2}, -1, 0, 0, 0, 0, 0}}},          // magnetic flux
    {"|Phi_M|", {1e-08, 0, {0, 0, 0, 0, 0, 0, 0, 0}}},                               // magnetic flux SI factor
    {"<L>", {0.01, 0, {1, 0, 0, 0, 0, 0, 0, 0}}},                                    // inductance
    {"|L|", {1e-09, 0, {0, 0, 0, 0, 0, 0, 0, 0}}},                                   // inductance SI factor
    {"<mm>", {1e-05, 0, {(FRC){5, 2}, (FRC){1, 2}, -1, 0, 0, 0, 0, 0}}},             // magnetic dipole moment
    {"|mm|", {0.001, 0, {0, 0, 0, 0, 0, 0, 0, 0}}},                                  // magnetic dipole moment SI factor
    {"<R>", {0.01, 0, {1, 0, 1, 0, 0, 0, 0, 0}}},                                    // electrical resistance
    {"|R|", {1e-09, 0, {0, 0, 0, 0, 0, 0, 0, 0}}},                                   // electrical resistance SI factor
