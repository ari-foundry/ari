const path = require('path');
const vscode = require('vscode');

function workspaceRoot() {
  const folder = vscode.workspace.workspaceFolders && vscode.workspace.workspaceFolders[0];
  return folder ? folder.uri.fsPath : undefined;
}

function resolveWorkspacePath(value) {
  if (!value) return value;
  if (path.isAbsolute(value)) return value;
  const root = workspaceRoot();
  if (!root) return value;
  return path.join(root, value);
}

module.exports = {
  workspaceRoot,
  resolveWorkspacePath
};
