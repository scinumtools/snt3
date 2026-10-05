Solver settings
===============

An EXS solver can carry application data alongside its operator registry and
evaluation steps. Derive a settings type from ``snt::exs::BaseSettings`` and
pass it as the second template argument to ``Solver``. The solver stores a
copy of this object and passes a pointer to it when it parses an atom or
applies an operator.

The :doc:`SettingsSolver example <../../examples/exs>` uses settings for a
symbolic value and an indexed list of options:

.. code-block:: cpp

   #include <array>
   #include <string>
   #include <utility>
   #include <snt/exs/solver.h>

   struct Settings : snt::exs::BaseSettings {
       std::string symbol;
       int value;
       std::array<int, 5> options;

       Settings(std::string s, int v, std::array<int, 5> o)
           : symbol(std::move(s)), value(v), options(o) {}
   };

   Settings settings{"?", 5, {1, 2, 3, 4, 5}};
   snt::exs::Solver<CustomAtom, Settings> solver(operators, steps, settings);
   auto result = solver.eval("2 + ? - {3}");  // 3

Here ``CustomAtom``, ``operators``, and ``steps`` are defined by the example.
During tokenization, ``CustomAtom::from_string`` receives the settings
pointer. It turns ``?`` into the configured value ``5`` and parses other
operands as integers:

.. code-block:: cpp

   int CustomAtom::from_string(std::string& text,
                               snt::exs::BaseSettings* base) {
       auto* settings = static_cast<Settings*>(base);
       return text == settings->symbol ? settings->value : std::stoi(text);
   }

The example's ``OperatorSelect`` implements the two-argument
``operate_group`` callback. It reads the same settings object and replaces
``{3}`` with ``options.at(3)``, which is ``4``. The expression therefore
evaluates to ``2 + 5 - 4 = 3``.

.. code-block:: cpp

   void OperatorSelect::operate_group(snt::exs::TokenListBase* tokens,
                                      snt::exs::BaseSettings* base) {
       auto* settings = static_cast<Settings*>(base);
       auto argument = tokens->get_left();
       auto* atom = static_cast<CustomAtom*>(argument.atom);
       atom->value = settings->options.at(atom->value);
       tokens->put_left(argument);
   }

The ``Solver<CustomAtom, Settings>`` template argument ensures that these
callbacks receive the intended settings type. Use ``solver.set_settings(new_settings)``
to replace the stored settings before a later evaluation. For the complete
operator registry, evaluation steps, and atom implementation, see the
`SettingsSolver source <https://github.com/scinumtools/snt3/tree/main/examples/exs/SettingsSolver>`_.
