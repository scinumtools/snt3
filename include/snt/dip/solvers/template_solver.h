#ifndef DIP_SOLVER_TEMPLATE_H
#define DIP_SOLVER_TEMPLATE_H

#include <snt/dip/environment.h>
#include <string>

namespace snt::dip {

    class TemplateSolver {
        Environment* environment; ///< Environment for the references
        Path current;             ///< Current path for in the hierarchy

      public:
        /**
         * Template solver constructor
         *
         * @param env Environment class object
         * @param current Current path in the node hierarchy
         */
        TemplateSolver(Environment& env, Path current = Path(""));

        /**
         * Solve DIP string template
         *
         * @param expression Template string
         * @returns Data of a value node
         */
        ValueNodeData eval(std::string expression);
    };

} // namespace snt::dip

#endif // DIP_SOLVER_TEMPLATE_H
