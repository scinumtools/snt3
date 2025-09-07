/*
 * Do not modify this file!
 * This file can be updated using 'dmap' executable.
 * 
 * Unit system:  Electrostatic CGS units (ESU)
 * Last update:  Sun Sep  7 23:35:40 2025
 * Code version: v0.1.0
 * 
 * Symbol legend:
 * ..    units
 * {..}  constants
 * {#..} collapsed constants
 * <..>  quantities
 * |..|  quantity SI conversion factors
 */
{"rad",         {1,                       0,                       { 0, 0, 0, 0, 0, 0, 0, 1} } },     // radian
{"cd",          {1,                       0,                       { 0, 0, 0, 0, 0, 1, 0, 0} } },     // candela
{"K",           {1,                       0,                       { 0, 0, 0, 1, 0, 0, 0, 0} } },     // Kelvin
{"A",           {1,                       0,                       { 0, 0, 0, 0, 1, 0, 0, 0} } },     // Ampere
{"s",           {1,                       0,                       { 0, 0, 1, 0, 0, 0, 0, 0} } },     // second
{"g",           {1,                       0,                       { 0, 1, 0, 0, 0, 0, 0, 0} } },     // gram
{"mol",         {1,                       0,                       { 0, 0, 0, 0, 0, 0, 1, 0} } },     // mole
{"m",           {1,                       0,                       { 1, 0, 0, 0, 0, 0, 0, 0} } },     // meter
{"statT",       {1000,                    0,                       {(FRC){-3,2},(FRC){1,2}, 0, 0, 0, 0, 0, 0} } },     // Stattesla
{"statWb",      {0.0001,                  0,                       { 2, 1, 0, 0, 0, 0, 0, 0} } },     // Statweber
{"{pi}",        {3.1415926,               0,                       { 0, 0, 0, 0, 0, 0, 0, 0} } },     // pi num.
{"{m_e}",       {9.10938371e-28,          0,                       { 0, 1, 0, 0, 0, 0, 0, 0} } },     // electron mass
{"Gal",         {0.01,                    0,                       { 1, 0,-2, 0, 0, 0, 0, 0} } },     // Gal
{"statF",       {0.01,                    0,                       { 1, 0, 0, 0, 0, 0, 0, 0} } },     // Statfarad
{"Rayl",        {10000,                   0,                       {-2, 1,-1, 0, 0, 0, 0, 0} } },     // Rayl
{"St",          {0.0001,                  0,                       { 2, 0,-1, 0, 0, 0, 0, 0} } },     // Stokes
{"Ka",          {100,                     0,                       {-1, 0, 0, 0, 0, 0, 0, 0} } },     // Kayser
{"{m_u}",       {1.660539069e-24,         0,                       { 0, 1, 0, 0, 0, 0, 0, 0} } },     // atom. mass const.
{"{a_0}",       {5.291772105e-11,         0,                       { 1, 0, 0, 0, 0, 0, 0, 0} } },     // Bohr radius
{"Pma",         {0.0001,                  0,                       { 2, 0, 0, 0, 0, 0, 0, 0} } },     // permeability
{"{c}",         {299792458,               0,                       { 1, 0,-1, 0, 0, 0, 0, 0} } },     // speed of light
{"dyn",         {0.01,                    0,                       { 1, 1,-2, 0, 0, 0, 0, 0} } },     // dyne
{"{alpha}",     {0.007297352564,          0,                       { 0, 0, 0, 0, 0, 0, 0, 0} } },     // fine-str. const.
{"P",           {100,                     0,                       {-1, 1,-1, 0, 0, 0, 0, 0} } },     // Poise
{"statOhm",     {100,                     0,                       {-1, 0, 1, 0, 0, 0, 0, 0} } },     // Statohm
{"statH",       {100,                     0,                       {-1, 0, 2, 0, 0, 0, 0, 0} } },     // Stathenry
{"Ba",          {100,                     0,                       {-1, 1,-2, 0, 0, 0, 0, 0} } },     // Barye
{"{G}",         {6.6743e-14,              0,                       { 3,-1,-2, 0, 0, 0, 0, 0} } },     // gravit. const.
{"Fr",          {0.001,                   0,                       {(FRC){3,2},(FRC){1,2},-1, 0, 0, 0, 0, 0} } },     // franklin
{"statC",       {0.001,                   0,                       {(FRC){3,2},(FRC){1,2},-1, 0, 0, 0, 0, 0} } },     // Statcoulomb
{"erg",         {0.0001,                  0,                       { 2, 1,-2, 0, 0, 0, 0, 0} } },     // erg
{"{e}",         {4.80320471e-13,          0,                       {(FRC){3,2},(FRC){1,2},-1, 0, 0, 0, 0, 0} } },     // elementary charge
{"statA",       {0.001,                   0,                       {(FRC){3,2},(FRC){1,2},-2, 0, 0, 0, 0, 0} } },     // Statampere
{"b",           {100,                     0,                       {-1, 1,-2, 0, 0, 0, 0, 0} } },     // Barye
{"{h}",         {6.62607015e-31,          0,                       { 2, 1,-1, 0, 0, 0, 0, 0} } },     // Planck const.
{"esu",         {0.001,                   0,                       {(FRC){3,2},(FRC){1,2},-1, 0, 0, 0, 0, 0} } },     // elstat. u.o. charge
{"statV",       {0.1,                     0,                       {(FRC){1,2},(FRC){1,2},-1, 0, 0, 0, 0, 0} } },     // Statvolt
{"radd",        {0.01,                    0,                       { 2, 0,-2, 0, 0, 0, 0, 0} } },     // radiation dose
{"{mu_B}",      {2.780278273e-17,         0,                       {(FRC){7,2},(FRC){1,2},-2, 0, 0, 0, 0, 0} } },     // Bohr magneton
{"{k}",         {1.380649e-20,            0,                       { 2, 1,-2,-1, 0, 0, 0, 0} } },     // Boltzmann const.
{"{hbar}",      {1.054571817e-31,         0,                       { 2, 1,-1, 0, 0, 0, 0, 0} } },     // reduced Pl. con.
{"<Phi_M>",     {0.0001,                  0,                       { 2, 1, 0, 0, 0, 0, 0, 0} } },     // magnetic flux
{"|Phi_M|",     {299.792458,              0,                       { 0, 0, 0, 0, 0, 0, 0, 0} } },     // magnetic flux SI factor
{"<k_n>",       {100,                     0,                       {-1, 0, 0, 0, 0, 0, 0, 0} } },     // wave number
{"<I>",         {0.001,                   0,                       {(FRC){3,2},(FRC){1,2},-2, 0, 0, 0, 0, 0} } },     // current
{"|I|",         {3.33564095198152e-10,    0,                       { 0, 0, 0, 0, 0, 0, 0, 0} } },     // current SI factor
{"<P>",         {0.0001,                  0,                       { 2, 1,-3, 0, 0, 0, 0, 0} } },     // power
{"<Iv>",        {1,                       0,                       { 0, 0, 0, 0, 0, 1, 0, 0} } },     // luminous intensity
{"<D_e>",       {10,                      0,                       {(FRC){-1,2},(FRC){1,2},-1, 0, 0, 0, 0, 0} } },     // electric displacement field
{"|D_e|",       {2.65441877471758e-07,    0,                       { 0, 0, 0, 0, 0, 0, 0, 0} } },     // electric displacement field SI factor
{"<Phi_E>",     {0.001,                   0,                       {(FRC){3,2},(FRC){1,2},-1, 0, 0, 0, 0, 0} } },     // electric flux
{"|Phi_E|",     {2.65441877471757e-11,    0,                       { 0, 0, 0, 0, 0, 0, 0, 0} } },     // electric flux SI factor
{"<a>",         {0.01,                    0,                       { 1, 0,-2, 0, 0, 0, 0, 0} } },     // acceleration
{"<B>",         {1000,                    0,                       {(FRC){-3,2},(FRC){1,2}, 0, 0, 0, 0, 0, 0} } },     // magnetic flux density
{"|B|",         {2997924.58,              0,                       { 0, 0, 0, 0, 0, 0, 0, 0} } },     // magnetic flux density SI factor
{"<rho_e>",     {1,                       0,                       { 0, 0, 1, 0, 0, 0, 0, 0} } },     // electrical resistivity
{"|rho_e|",     {8987551787.36818,        0,                       { 0, 0, 0, 0, 0, 0, 0, 0} } },     // electrical resistivity SI factor
{"<the>",       {1,                       0,                       { 0, 0, 0, 0, 0, 0, 0, 1} } },     // angular displacement
{"<nu_v>",      {0.0001,                  0,                       { 2, 0,-1, 0, 0, 0, 0, 0} } },     // kinematic viscosity
{"<p>",         {100,                     0,                       {-1, 1,-2, 0, 0, 0, 0, 0} } },     // pressure
{"<l>",         {0.01,                    0,                       { 1, 0, 0, 0, 0, 0, 0, 0} } },     // length
{"<v>",         {0.01,                    0,                       { 1, 0,-1, 0, 0, 0, 0, 0} } },     // velocity
{"<H>",         {0.1,                     0,                       {(FRC){1,2},(FRC){1,2},-2, 0, 0, 0, 0, 0} } },     // magnetic field strenght
{"|H|",         {2.65441877471757e-09,    0,                       { 0, 0, 0, 0, 0, 0, 0, 0} } },     // magnetic field strenght SI factor
{"<E>",         {0.0001,                  0,                       { 2, 1,-2, 0, 0, 0, 0, 0} } },     // energy
{"<mu_v>",      {100,                     0,                       {-1, 1,-1, 0, 0, 0, 0, 0} } },     // dynamic viscosity
{"<n>",         {1,                       0,                       { 0, 0, 0, 0, 0, 0, 1, 0} } },     // amount of substance
{"<E_e>",       {10,                      0,                       {(FRC){-1,2},(FRC){1,2},-1, 0, 0, 0, 0, 0} } },     // electric field strength
{"|E_e|",       {29979.2458,              0,                       { 0, 0, 0, 0, 0, 0, 0, 0} } },     // electric field strength SI factor
{"<t>",         {1,                       0,                       { 0, 0, 1, 0, 0, 0, 0, 0} } },     // time
{"<q>",         {0.001,                   0,                       {(FRC){3,2},(FRC){1,2},-1, 0, 0, 0, 0, 0} } },     // electric charge
{"|q|",         {3.33564095198152e-10,    0,                       { 0, 0, 0, 0, 0, 0, 0, 0} } },     // electric charge SI factor
{"<F>",         {0.01,                    0,                       { 1, 1,-2, 0, 0, 0, 0, 0} } },     // force
{"<m>",         {1,                       0,                       { 0, 1, 0, 0, 0, 0, 0, 0} } },     // mass
{"<R>",         {100,                     0,                       {-1, 0, 1, 0, 0, 0, 0, 0} } },     // electrical resistance
{"|R|",         {898755178736.818,        0,                       { 0, 0, 0, 0, 0, 0, 0, 0} } },     // electrical resistance SI factor
{"<eps>",       {1,                       0,                       { 0, 0, 0, 0, 0, 0, 0, 0} } },     // permittivity
{"|eps|",       {8.85418796865655e-12,    0,                       { 0, 0, 0, 0, 0, 0, 0, 0} } },     // permittivity SI factor
{"<C>",         {0.01,                    0,                       { 1, 0, 0, 0, 0, 0, 0, 0} } },     // capacitance
{"|C|",         {1.11265005605362e-12,    0,                       { 0, 0, 0, 0, 0, 0, 0, 0} } },     // capacitance SI factor
{"<mm>",        {1e-07,                   0,                       {(FRC){7,2},(FRC){1,2},-2, 0, 0, 0, 0, 0} } },     // magnetic dipole moment
{"|mm|",        {3.33564095198152e-14,    0,                       { 0, 0, 0, 0, 0, 0, 0, 0} } },     // magnetic dipole moment SI factor
{"<L>",         {100,                     0,                       {-1, 0, 2, 0, 0, 0, 0, 0} } },     // inductance
{"|L|",         {898755178736.818,        0,                       { 0, 0, 0, 0, 0, 0, 0, 0} } },     // inductance SI factor
{"<mu>",        {10000,                   0,                       {-2, 0, 2, 0, 0, 0, 0, 0} } },     // permeability
{"|mu|",        {1.12940904749251e+15,    0,                       { 0, 0, 0, 0, 0, 0, 0, 0} } },     // permeability SI factor
{"<p_e>",       {1e-05,                   0,                       {(FRC){5,2},(FRC){1,2},-1, 0, 0, 0, 0, 0} } },     // electric dipole moment
{"|p_e|",       {3.33564095198152e-12,    0,                       { 0, 0, 0, 0, 0, 0, 0, 0} } },     // electric dipole moment SI factor
{"<phi_e>",     {0.1,                     0,                       {(FRC){1,2},(FRC){1,2},-1, 0, 0, 0, 0, 0} } },     // electric potential
{"|phi_e|",     {299.792458,              0,                       { 0, 0, 0, 0, 0, 0, 0, 0} } },     // electric potential SI factor
