#ifndef PUQ_QUANTITIES_H
#define PUQ_QUANTITIES_H

struct QuantityStruct {
  std::string definition;
  std::string sifactor;
};
typedef std::unordered_map<std::string, QuantityStruct> QuantityListType;

template <typename T>
std::unordered_map<std::string, T> operator+(const std::unordered_map<std::string, T>& map1, const std::unordered_map<std::string, T>& map2) {
  std::unordered_map<std::string, T> mapr(map1.begin(), map1.end());
  mapr.insert(map2.begin(), map2.end());
  return mapr;
}

typedef std::unordered_map<std::string, std::string> QuantityNamesType;
const QuantityNamesType QuantityNames = {
    // base quantities
    {"l", "length"},
    {"m", "mass"},
    {"t", "time"},
    {"T", "temperature"},
    {"I", "current"},
    {"Iv", "luminous intensity"},
    {"n", "amount of substance"},
    {"the", "angular displacement"},

    // mechanics
    {"v", "velocity"},
    {"a", "acceleration"},
    {"jerk", "jerk"},
    {"snap", "snap"},
    {"crac", "crackle"},
    {"pop", "pop"},

    {"rho", "density"},
    {"rho_A", "area density"},
    {"rho_l", "linear density"},

    {"rho_E", "energy density"},

    {"A", "area"},
    {"V", "volume"},

    {"p_mom", "momentum"},
    {"I_mom", "moment of inertia"},

    {"ome", "angular velocity"},
    {"a_ome", "angular acceleration"},
    {"L_ome", "angular momentum"},

    {"F", "force"},
    {"freq", "frequecy"},

    {"J_i", "impulse"},
    {"Abs", "absement"},

    // material properties
    {"mu_v", "dynamic viscosity"},
    {"nu_v", "kinematic viscosity"},
    {"eps_s", "strain"},
    {"sig_s", "stress"},
    {"E_Y", "Young's modulus"},
    {"k_s", "spring constant"},
    {"gam", "surface tension"},
    {"tau", "torque"},

    // thermodynamics
    {"E", "energy"},
    {"S", "entropy"},
    {"p", "pressure"},
    {"mu_c", "chemical potential"},
    {"W", "work"},
    {"P", "power"},

    {"E_sp", "specific energy"},
    {"C_sp", "specific heat capacity"},
    {"V_sp", "specific volume"},

    {"C_mol", "molar concentration"},
    {"E_mol", "molar energy"},
    {"c_mol", "molar heat capacity"},
    {"S_mol", "molar entropy"},

    {"rr", "reaction rate"},

    {"Q", "heat"},
    {"C_p", "head capacity"},
    {"phi_Q", "heat flux density"},

    {"kap", "thermal conductance"},
    {"lam_t", "thermal conductivity"},
    {"R_t", "thermal resistance"},
    {"R_lam", "thermal resistivity"},

    {"Q_vol", "volumetric flow rate"},

    // electromagetism
    {"q", "electric charge"},
    {"rho_q", "electric charge density"},
    {"p_e", "electric dipole moment"},
    {"p_qe", "electric quadrupole moment"},
    {"D_e", "electric displacement field"},
    {"E_e", "electric field strength"},
    {"V_e", "electric field gradient"},
    {"alp_e", "electric polarizability"},
    {"alp_he", "1st hyperpolarizability"},
    {"alp_hhe", "2nd hyperpolarizability"},
    {"G_e", "electrical conductance"},
    {"sig_e", "electrical conductivity"},
    {"Phi_E", "electric flux"},
    {"phi_e", "electric potential"},
    {"R", "electrical resistance"},
    {"rho_e", "electrical resistivity"},

    {"Mag", "magnetization"},
    {"H", "magnetic field strenght"},
    {"Phi_M", "magnetic flux"},
    {"B", "magnetic flux density"},
    {"mm", "magnetic dipole moment"},

    {"mu", "permeability"},
    {"eps", "permittivity"},

    {"C", "capacitance"},
    {"J_c", "current density"},
    {"Z", "impedance"},
    {"L", "inductance"},
    {"Rel", "reluctance"},

    // wave and optics
    {"P_o", "optical power"},
    {"lam_w", "wave length"},
    {"k_n", "wave number"},
    {"k_v", "wave vector"},

    // radiation
    {"E_il", "illuminance"},
    {"E_ir", "irradiance"},
    {"F_l", "luinous flux"},
    {"In", "intensity"},
    {"L_r", "radiance"},
    {"I_r", "radiant intensity"},

    // radioactivity
    {"D_a", "absorbed dose rate"},
    {"H_de", "dose equivalent"},
    {"D_r", "radiation dose"},
    {"A_r", "radioactivity"},

    // quantum mechanics
    {"S_a", "action"},
    {"spin", "spin"},

};

#endif // PUQ_QUANTITIES_H
