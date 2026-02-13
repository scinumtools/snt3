#ifndef MAT_MATERIAL_H
#define MAT_MATERIAL_H

#include "substance.h"
#include "set.h"
#include "matter.h"
#include "solvers/material_solver.h"

namespace snt::mat {

  /**
   * @class Material
   * @brief Any substance or mixture of substances that can be used to make objects or structures
   */
  class Material: public Set<Substance,double>,
		  public Matter {
  public:
  };

} // namespace snt::mat

#endif // MAT_MATERIAL_H
