Custom atoms and operations
===========================

To extend EXS, define an atom for the values your expression language uses,
define an operator implementation, then register its type and add an
evaluation step. The :doc:`CustomSolver example <../../examples/exs>`
implements ``len(...)`` over string atoms and uses the built-in ``<``
operator to compare lengths.

The example allocates a new operator ID after the built-in IDs and derives a
one-argument group operator. This excerpt uses the example's ``CustomAtom``:

.. code-block:: cpp

   enum CustomOperatorType { LENGTH_OPERATOR = snt::exs::NUM_OPERATOR_TYPES };

   class OperatorLength : public snt::exs::OperatorGroup<1> {
   public:
       OperatorLength()
           : OperatorGroup<1>("len", {"len", "(", ")", ","}, LENGTH_OPERATOR) {}

       void operate_group(snt::exs::TokenListBase* tokens) override {
           auto argument = tokens->get_left();
           auto* atom = static_cast<CustomAtom*>(argument.atom);
           atom->custom_length();
           tokens->put_left(argument);
       }
   };

``OperatorGroup<1>`` parses one argument between parentheses. The solver
evaluates that argument first, then ``operate_group`` changes the atom to its
length. The example's ``CustomAtom`` derives from
``snt::exs::AtomBase<CustomAtom, AtomValueType>``; it parses string operands
with ``from_string`` and implements ``custom_length`` and
``comparison_less`` for the example's value types.
For a different operation shape, derive from ``OperatorBase`` and override
``operate_unary``, ``operate_binary``, or ``operate_ternary`` as appropriate.
The chosen step category must match that override.

Register the new operator and place its group operation before the comparison
step:

.. code-block:: cpp

   snt::exs::OperatorList operators;
   operators.append(snt::exs::LESS_OPERATOR,
                    std::make_shared<snt::exs::OperatorLess>());
   operators.append(LENGTH_OPERATOR, std::make_shared<OperatorLength>());

   snt::exs::StepList steps;
   steps.append(snt::exs::GROUP_OPERATION, {LENGTH_OPERATOR});
   steps.append(snt::exs::BINARY_OPERATION, {snt::exs::LESS_OPERATOR});

   snt::exs::Solver<CustomAtom> solver(operators, steps);
   auto result = solver.eval("apple < len(hospital)");

The complete atom and operator implementations are in the
`CustomSolver source <https://github.com/vrtulka23/scinumtools3/tree/main/examples/exs/CustomSolver>`_.
The :doc:`EXS examples <../../examples/exs>` also show arrays
(``ArraySolver``) and atoms that own ``std::unique_ptr`` values
(``UniquePtrSolver``). See :doc:`Solver settings <settings>` for passing
application data to atom parsing and operators.
