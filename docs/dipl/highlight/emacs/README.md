# DIPL Emacs highlighter

`dip-mode.el` defines a lightweight, standalone Emacs major mode for DIPL
files. It is based on `fundamental-mode` and does not require Python mode.
Copy the
file to a directory on Emacs’ `load-path`, then enable it for `.dip` and
`.dipl` files:

```elisp
(add-to-list 'load-path "~/path/to/dip-mode")
(require 'dip-mode)
(add-to-list 'auto-mode-alist '("\\.dipl?\\'" . dip-mode))
```

The mode highlights DIPL types, `$source`/`$unit`/`$schema` declarations,
properties, `@if`/`@elif`/`@else`/`@end` branching, booleans, and numbers.
