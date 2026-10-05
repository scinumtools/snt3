# DIPL Emacs highlighter

`dip-mode.el` defines a lightweight, standalone Emacs major mode for DIPL
files. It is based on `fundamental-mode`. Copy the
file to a directory on Emacs’ `load-path`, then enable it for `.dip` and
`.dipl` files:

```elisp
(require 'dip-mode)
(add-to-list 'auto-mode-alist '("\\.dip\\'" . dip-mode))
(add-to-list 'auto-mode-alist '("\\.dipl\\'" . dip-mode))
```

The mode highlights DIPL types (including `map` and `list`) and colors only
the brackets in collection items such as `units[]`, `berries[0]`, and
`sources[constants]` gray. Item names and keys keep the default text color.
It also highlights `$source`/`$unit`/`$schema` declarations, properties,
`@if`/`@elif`/`@else`/`@end` branching, booleans, and numbers.
Numbers within `#` line comments retain the comment face.
