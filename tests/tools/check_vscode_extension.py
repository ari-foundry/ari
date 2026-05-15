#!/usr/bin/env python3
import json
from pathlib import Path


root = Path(__file__).resolve().parents[2]
package = json.loads((root / "editors/vscode/package.json").read_text())

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
for setting in ("ari.compilerPath", "ari.lspPath", "ari.lintPath", "ari.modulePaths"):
    if setting not in properties:
        raise SystemExit(f"missing VS Code setting: {setting}")
