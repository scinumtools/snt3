#ifndef MAT_MATERIAL_H
#define MAT_MATERIAL_H

#include <snt/mat/matter.h>
#include <snt/mat/set.h>
#include <snt/mat/solvers/material_solver.h>
#include <snt/mat/substance.h>

namespace snt::mat {

    /** A material represented as a set of substances and their composition.
     *
     * The set stores substance amounts as `double` values; the interpretation of
     * those amounts (mass, fraction, or another quantity) follows the material
     * construction and solver context.
     */
    class Material : public Set<Substance, double>, public Matter {
      public:
    };

} // namespace snt::mat

#endif // MAT_MATERIAL_H
