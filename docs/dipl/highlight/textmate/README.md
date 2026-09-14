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
