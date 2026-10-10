const form = document.querySelector('#puq-converter');

if (form) {
  const result = document.querySelector('#puq-converter-result');
  const systems = document.querySelector('#puq-system-names');
  let puqPromise;

  function loadConverter() {
    if (!puqPromise) {
      puqPromise = import('./web/index.mjs')
        .then((module) => module.loadPUQ())
        .then((puq) => {
          for (const name of puq.systems()) {
            const option = document.createElement('option');
            option.value = name;
            systems.append(option);
          }
          return puq;
        })
        .catch((error) => {
          puqPromise = undefined;
          throw error;
        });
    }
    return puqPromise;
  }

  form.addEventListener('focusin', () => { void loadConverter().catch(() => {}); }, { once: true });
  form.addEventListener('submit', async (event) => {
    event.preventDefault();
    result.classList.remove('puq-converter-error');
    result.textContent = 'Converting…';
    const fields = new FormData(form);
    try {
      const puq = await loadConverter();
      result.textContent = puq.convert(
        String(fields.get('expression')).trim(),
        String(fields.get('outputUnits')).trim(),
        {
          inputSystem: String(fields.get('inputSystem')).trim(),
          outputSystem: String(fields.get('outputSystem')).trim(),
          outputQuantity: String(fields.get('outputQuantity')).trim(),
        },
      );
    } catch (error) {
      result.classList.add('puq-converter-error');
      result.textContent = error instanceof Error
        ? [error.message, error.details, error.suggestion].filter(Boolean).join('\n')
        : String(error);
    }
  });
}
