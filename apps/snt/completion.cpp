#include "main.h"

#include <iostream>
#include <map>
#include <string>

int module_completion(int argc, char* argv[]) {
    if (argc != 2 || (std::string(argv[1]) != "bash" && std::string(argv[1]) != "zsh")) {
        std::cerr << "Usage: snt completion <bash|zsh>\n";
        return 2;
    }

    // Keep this command metadata in sync with the native CLI dispatcher.
    const std::map<std::string, std::string> candidates{
        {"", "dip puq report format fmt validate lint dmap server view hub mcp completion --help --version"},
        {"dip", "parse compare describe schemas sources override-contract list preview package --help --version --input --project --load --save --record-graph --relative-source-paths --generate --request --print --value --type --path --query --tag-all --tag-any --tag-none --limit --max-value-elements --override --override-file --max-details --format"},
        {"dip parse", "--help --input --project --load --save --record-graph --relative-source-paths --generate --request --print --value --type"},
        {"dip compare", "--help --scope --max-details --max-array-examples"},
        {"dip describe", "--help --project --input --path --format --record-graph --max-value-elements"},
        {"dip schemas", "--help --project --input --format"},
        {"dip sources", "--help --project --input --format"},
        {"dip override-contract", "--help --project --input --path --format"},
        {"dip list", "--help --project --input --query --tag-all --tag-any --tag-none --limit --max-value-elements --format"},
        {"dip preview", "--help --project --input --override --override-file --max-details --format"},
        {"dip package", "--help --project --output --include"},
        {"dip compare --scope", "effective full"},
        {"dip --format", "json"},
        {"report", "--help --version --input --project --load --format --output --intro --title --author --date --report-version --tex-compiler"},
        {"report --format", "tex pdf md rst html typ txt json"},
        {"format", "--help --check --write"},
        {"fmt", "--help --check --write"},
        {"validate", "--help --project --json"},
        {"lint", "--help --project --json"},
        {"puq", "info list eval convert --help --input-system --output-system --output-units --output-quantity -s -S -U -Q"},
        {"puq list", "prefix base deriv log temp const quant sys"},
        {"dmap", "--help"},
        {"server", "--help --host --port"},
        {"view", "--help"},
        {"hub", "--help fetch setup list"},
        {"mcp", "--help --transport --host --port"},
    };

    if (std::string(argv[1]) == "bash") {
        std::cout << R"(_snt() {
  local key="" word cur="${COMP_WORDS[COMP_CWORD]}" candidates=""
  local i
  for ((i=1; i<COMP_CWORD; i++)); do
    word="${COMP_WORDS[i]}"
    key="${key:+$key }$word"
  done
  case "$key" in
)";
        for (const auto& [key, values] : candidates)
            std::cout << "    '" << key << "') candidates='" << values << "' ;;\n";
        std::cout << R"(    *) candidates='' ;;
  esac
  if [[ "$cur" == -* ]]; then
    COMPREPLY=( $(compgen -W "$candidates" -- "$cur") )
  else
    COMPREPLY=( $(compgen -W "$candidates" -- "$cur") )
    if [[ ${#COMPREPLY[@]} -eq 0 ]]; then compopt -o filenames; COMPREPLY=( $(compgen -f -- "$cur") ); fi
  fi
}
complete -F _snt snt
)";
        return 0;
    }

    std::cout << R"(#compdef snt
local key="" word
local -a candidates
typeset -U candidates
for word in "${words[@]:2:$((CURRENT - 2))}"; do
  key="${key:+$key }$word"
done
case "$key" in
)";
    for (const auto& [key, values] : candidates)
        std::cout << "  '" << key << "') candidates=(" << values << ") ;;\n";
    std::cout << R"(  *) candidates=() ;;
esac
if [[ "$words[CURRENT]" == -* ]]; then
  _describe 'option' candidates
elif (( ${#candidates} )); then
  _describe 'command' candidates
else
  _files
fi
)";
    return 0;
}
