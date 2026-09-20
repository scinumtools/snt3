# DIPL TextMate package

This directory is a plug-and-play VS Code language extension. It provides
syntax highlighting for DIPL files with `.dip` and `.dipl` extensions.

## Install locally

1. Copy this directory to a folder such as
   `~/.vscode/extensions/dipl-language-support`.
2. Restart or reload VS Code.
3. Open a `.dip` or `.dipl` file.

VS Code reads `package.json`, associates the file extensions with the `dipl`
language, and loads `dipl.tmLanguage.json` for syntax highlighting. The
`language-configuration.json` file supplies comment, bracket, and quote
handling.

The grammar is also the canonical syntax definition for Visual Studio. Place
the package contents in a VSIX `Grammars` directory and register that
directory through the extension's `.pkgdef` file. Visual Studio then uses the
same `source.dipl` grammar for `.dip` and `.dipl` files.

For VS Code, the package can be packaged with `vsce package`.

## Test in VS Code

From the repository root, install this working copy as a local VS Code
extension and reload an open VS Code window:

```bash
mkdir -p ~/.vscode/extensions && ln -sfn "$PWD/docs/dipl/highlight/textmate" ~/.vscode/extensions/scinumtools.dipl-language-support-0.1.0 && code --reload-window
```

Open `docs/dipl/highlight/highlighting-test.dipl` afterwards. The language
indicator in the lower-right corner should show `DIPL` and the fixture should
be highlighted automatically.

The command requires the VS Code `code` shell command. If it is unavailable,
open the Command Palette (`Shift+Command+P`) in VS Code and run `Shell Command:
Install 'code' command in PATH`, then restart the terminal.
