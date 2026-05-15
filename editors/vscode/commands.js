const cp = require('child_process');
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

function modulePathArgs(modulePaths) {
  const args = [];
  for (const modulePath of modulePaths || []) {
    args.push('-I');
    args.push(resolveWorkspacePath(modulePath));
  }
  return args;
}

function activeAriEditor() {
  const editor = vscode.window.activeTextEditor;
  if (!editor || editor.document.languageId !== 'ari' || editor.document.uri.scheme !== 'file') {
    vscode.window.showWarningMessage('Open an Ari source file first.');
    return undefined;
  }
  return editor;
}

function appendCommand(channel, executable, args) {
  channel.appendLine(`$ ${[executable, ...args].join(' ')}`);
}

function runProcess(channel, executable, args) {
  return new Promise((resolve) => {
    appendCommand(channel, executable, args);
    const child = cp.spawn(executable, args, {
      cwd: workspaceRoot(),
      shell: process.platform === 'win32'
    });
    child.stdout.on('data', (chunk) => channel.append(chunk.toString()));
    child.stderr.on('data', (chunk) => channel.append(chunk.toString()));
    child.on('error', (error) => {
      channel.appendLine(`ari-vscode: failed to start ${executable}: ${error.message}`);
      resolve(1);
    });
    child.on('close', (code) => {
      channel.appendLine(`ari-vscode: exited with code ${code}`);
      resolve(code || 0);
    });
  });
}

async function runCurrentFile(kind, channel) {
  const editor = activeAriEditor();
  if (!editor) return;
  if (editor.document.isDirty) {
    const saved = await editor.document.save();
    if (!saved) {
      vscode.window.showWarningMessage('Save the Ari file before running this command.');
      return;
    }
  }

  const config = vscode.workspace.getConfiguration('ari');
  const compilerPath = resolveWorkspacePath(config.get('compilerPath', 'build/ari'));
  const lintPath = resolveWorkspacePath(config.get('lintPath', 'build/ari-lint'));
  const modulePaths = config.get('modulePaths', []);
  const file = editor.document.uri.fsPath;

  channel.show(true);
  channel.appendLine('');
  if (kind === 'lint') {
    await runProcess(channel, lintPath, ['--ari', compilerPath, ...modulePathArgs(modulePaths), file]);
    return;
  }
  await runProcess(channel, compilerPath, [...modulePathArgs(modulePaths), file, '--check']);
}

function registerAriCommands(context) {
  const channel = vscode.window.createOutputChannel('Ari');
  context.subscriptions.push(channel);
  context.subscriptions.push(vscode.commands.registerCommand('ari.checkCurrentFile', () => {
    runCurrentFile('check', channel);
  }));
  context.subscriptions.push(vscode.commands.registerCommand('ari.lintCurrentFile', () => {
    runCurrentFile('lint', channel);
  }));
}

module.exports = {
  registerAriCommands,
  resolveWorkspacePath
};
