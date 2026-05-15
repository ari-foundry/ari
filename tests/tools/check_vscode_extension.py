#!/usr/bin/env python3
import json
from pathlib import Path


root = Path(__file__).resolve().parents[2]
package = json.loads((root / "editors/vscode/package.json").read_text())
for source in ("extension.js", "commands.js", "tasks.js", "lsp.js", "paths.js", "config.js"):
    if not (root / "editors/vscode" / source).exists():
        raise SystemExit(f"missing VS Code source file: {source}")

commands = {item["command"] for item in package["contributes"]["commands"]}
required_commands = {
    "ari.checkCurrentFile",
    "ari.lintCurrentFile",
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
for setting in ("ari.compilerPath", "ari.lspPath", "ari.lintPath", "ari.modulePaths", "ari.lintRules"):
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
