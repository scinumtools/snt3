Conversion and unit systems
===========================

Conversion creates a new ``Quantity`` expressed in target units. The original
quantity remains unchanged, which makes it safe to retain both a canonical
calculation value and a display-specific representation.

Try a conversion
----------------

Enter a PUQ quantity expression and target units. This tool runs the same C++
conversion command as ``snt puq convert`` in your browser. The optional fields
help with units that need an explicit system or physical-quantity context.

.. raw:: html

   <form id="puq-converter" class="puq-converter">
     <div class="puq-converter-fields">
       <label>Quantity expression<input name="expression" type="text" value="35*eV" required spellcheck="false" autocomplete="off"></label>
       <label>Target units<input name="outputUnits" type="text" value="J" required spellcheck="false" autocomplete="off"></label>
     </div>
     <details>
       <summary>Unit systems and quantity context</summary>
       <div class="puq-converter-fields">
         <label>Input system<input name="inputSystem" type="text" list="puq-system-names" placeholder="Default: SI" spellcheck="false" autocomplete="off"></label>
         <label>Output system<input name="outputSystem" type="text" list="puq-system-names" placeholder="Default: input system" spellcheck="false" autocomplete="off"></label>
         <label>Physical quantity<input name="outputQuantity" type="text" placeholder="Optional, for contextual conversions" spellcheck="false" autocomplete="off"></label>
       </div>
       <datalist id="puq-system-names"></datalist>
     </details>
     <button type="submit">Convert</button>
     <output id="puq-converter-result" aria-live="polite">Enter a conversion and select Convert.</output>
   </form>

The conversion runs locally in the browser. No expression is sent to a server.

Converting compatible quantities
--------------------------------

Pass a target unit expression to ``Quantity::convert()``:

.. code-block:: cpp

   #include <snt/puq/quantity.h>

   snt::puq::Quantity distance("1 mile");
   snt::puq::Quantity distance_km = distance.convert("km");

The target must be dimensionally compatible with the source. PUQ reports an
error instead of converting, for example, a length to seconds. The same method
can convert derived units such as speed, energy, or pressure.

Systems and contextual conversions
----------------------------------

PUQ supports SI, US customary, ESU, and other registered unit systems. Most
conversions can infer all required information from the source and target
units. Some cross-system conversions need an explicit physical-quantity
context; the ``convert`` overload accepts a target system and quantity name
for that case.

The command-oriented C++ API supports selectable systems, formatted output,
and unit-definition lists. See :doc:`PUQ systems API
<../../api/cpp/puq/systems>` for the C++ system and conversion classes.
