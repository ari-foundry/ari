#!/usr/bin/env python3
import json
from pathlib import Path


root = Path(__file__).resolve().parents[2]
package = json.loads((root / "editors/vscode/package.json").read_text())
grammar = json.loads((root / "editors/vscode/syntaxes/ari.tmLanguage.json").read_text())
language_config = json.loads((root / "editors/vscode/language-configuration.json").read_text())
for source in ("extension.js", "commands.js", "tasks.js", "lsp.js", "paths.js", "config.js"):
    if not (root / "editors/vscode" / source).exists():
        raise SystemExit(f"missing VS Code source file: {source}")
if not (root / "editors/vscode/scripts/package-vsix.js").exists():
    raise SystemExit("missing VS Code package script")
if not (root / "editors/vscode/.vscode/launch.json").exists():
    raise SystemExit("missing VS Code extension launch configuration")
if not (root / "editors/vscode/README.md").exists():
    raise SystemExit("missing VS Code extension package README")
if not (root / "editors/vscode/.vscodeignore").exists():
    raise SystemExit("missing VS Code extension package ignore file")
for doc in (
    "docs/lint/features.md",
    "docs/lsp/features.md",
    "docs/vscode/features.md",
    "docs/vscode/release.md",
    "docs/vscode/usage.md",
):
    if not (root / doc).exists():
        raise SystemExit(f"missing tool documentation: {doc}")

scripts = package.get("scripts", {})
for script in ("check", "package", "install:local"):
    if script not in scripts:
        raise SystemExit(f"missing VS Code package script: {script}")
if "package-vsix.js" not in scripts["package"]:
    raise SystemExit("VS Code package script must use scripts/package-vsix.js")
if package.get("version") != "0.1.0":
    raise SystemExit("VS Code package version must be 0.1.0")
if "@vscode/vsce" not in package.get("devDependencies", {}):
    raise SystemExit("VS Code package must include @vscode/vsce for local VSIX packaging")
if "vscode-languageclient" not in package.get("dependencies", {}):
    raise SystemExit("VS Code package must depend on vscode-languageclient")

package_script = (root / "editors/vscode/scripts/package-vsix.js").read_text()
for runtime_package in (
    "balanced-match",
    "brace-expansion",
    "minimatch",
    "semver",
    "vscode-jsonrpc",
    "vscode-languageclient",
    "vscode-languageserver-protocol",
    "vscode-languageserver-types",
):
    if runtime_package not in package_script:
        raise SystemExit(f"missing VS Code runtime package in packaging script: {runtime_package}")

commands = {item["command"] for item in package["contributes"]["commands"]}
required_commands = {
    "ari.checkCurrentFile",
    "ari.lintCurrentFile",
    "ari.restartLanguageServer",
}
missing_commands = sorted(required_commands - commands)
if missing_commands:
    raise SystemExit(f"missing VS Code commands: {', '.join(missing_commands)}")

activation_events = set(package["activationEvents"])
for command in required_commands:
    event = f"onCommand:{command}"
    if event not in activation_events:
        raise SystemExit(f"missing VS Code activation event: {event}")

properties = package["contributes"]["configuration"]["properties"]
for setting in (
    "ari.compilerPath",
    "ari.lspPath",
    "ari.lintPath",
    "ari.lintConfigPath",
    "ari.modulePaths",
    "ari.lintRules",
):
    if setting not in properties:
        raise SystemExit(f"missing VS Code setting: {setting}")

task_definitions = package["contributes"].get("taskDefinitions", [])
ari_tasks = [item for item in task_definitions if item.get("type") == "ari"]
if not ari_tasks:
    raise SystemExit("missing Ari VS Code task definition")

targets = set(ari_tasks[0]["properties"]["target"]["enum"])
required_targets = {"build", "check", "tools", "check-tools", "lint", "lsp"}
missing_targets = sorted(required_targets - targets)
if missing_targets:
    raise SystemExit(f"missing Ari VS Code task targets: {', '.join(missing_targets)}")

if "onTaskType:ari" not in activation_events:
    raise SystemExit("missing VS Code activation event: onTaskType:ari")

if grammar.get("scopeName") != "source.ari":
    raise SystemExit("Ari grammar must use source.ari scope")

repository = grammar.get("repository", {})
for section in (
    "attributes",
    "comments",
    "constants",
    "declarations",
    "identifiers",
    "keywords",
    "macros",
    "numbers",
    "operators",
    "punctuation",
    "strings",
    "types",
):
    if section not in repository:
        raise SystemExit(f"missing Ari grammar repository section: {section}")

grammar_text = (root / "editors/vscode/syntaxes/ari.tmLanguage.json").read_text()
for keyword in (
    "fn",
    "const",
    "as",
    "meta",
    "struct",
    "extern",
    "enum",
    "trait",
    "type",
    "dyn",
    "match",
    "mod",
    "pub",
    "use",
    "impl",
    "for",
    "in",
    "let",
    "var",
    "own",
    "ref",
    "mut",
    "ptr",
    "return",
    "if",
    "else",
    "while",
    "init",
    "next",
    "continue",
    "break",
    "drop",
    "null",
    "true",
    "false",
):
    if keyword not in grammar_text:
        raise SystemExit(f"missing Ari grammar keyword: {keyword}")

for operator, spellings in {
    "->": ("->",),
    "=>": ("=>",),
    "::": ("::",),
    "??": ("??", "\\?\\?", "\\\\?\\\\?", "keyword.operator.coalesce.ari"),
    "<<=": ("<<=",),
    ">>=": (">>=",),
    "..=": ("..=", "\\.\\.=", "\\\\.\\\\.=", "keyword.operator.range.ari"),
}.items():
    if not any(spelling in grammar_text for spelling in spellings):
        raise SystemExit(f"missing Ari grammar operator: {operator}")

for type_name in (
    "i8",
    "i16",
    "i32",
    "i64",
    "u8",
    "u16",
    "u32",
    "u64",
    "f32",
    "f64",
    "f128",
    "bool",
    "char",
    "void",
):
    if type_name not in grammar_text:
        raise SystemExit(f"missing Ari grammar type: {type_name}")

comment_config = language_config.get("comments", {})
if comment_config.get("lineComment") != "//":
    raise SystemExit("missing Ari line comment configuration")
if comment_config.get("blockComment") != ["/*", "*/"]:
    raise SystemExit("missing Ari block comment configuration")
