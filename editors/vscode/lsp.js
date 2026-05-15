const vscode = require('vscode');
const { LanguageClient } = require('vscode-languageclient/node');
const { lintConfigArgs, lintRuleArgs, modulePathArgs } = require('./config');
const { resolveWorkspacePath, workspaceRoot } = require('./paths');

function lspArgs(config) {
  const compilerPath = resolveWorkspacePath(config.get('compilerPath', 'build/ari'));
  return [
    '--ari',
    compilerPath,
    ...modulePathArgs(config.get('modulePaths', [])),
    ...lintConfigArgs(config),
    ...lintRuleArgs(config.get('lintRules', {}))
  ];
}

function createClient() {
  const config = vscode.workspace.getConfiguration('ari');
  return new LanguageClient(
    'ari-lsp',
    'Ari Language Server',
    {
      command: resolveWorkspacePath(config.get('lspPath', 'build/ari-lsp')),
      args: lspArgs(config),
      options: {
        cwd: workspaceRoot()
      }
    },
    {
      documentSelector: [
        { scheme: 'file', language: 'ari' }
      ],
      synchronize: {
        fileEvents: vscode.workspace.createFileSystemWatcher('**/*.{ari,arih}')
      }
    }
  );
}

function shouldRestart(event) {
  return event.affectsConfiguration('ari.compilerPath') ||
    event.affectsConfiguration('ari.lspPath') ||
    event.affectsConfiguration('ari.modulePaths') ||
    event.affectsConfiguration('ari.lintConfigPath') ||
    event.affectsConfiguration('ari.lintRules');
}

class AriLspController {
  constructor() {
    this.client = undefined;
    this.operation = Promise.resolve();
  }

  start() {
    this.client = createClient();
    return this.client.start();
  }

  stop() {
    if (!this.client) return Promise.resolve();
    const client = this.client;
    this.client = undefined;
    return client.stop();
  }

  restart() {
    this.operation = this.operation
      .catch(() => undefined)
      .then(() => this.stop())
      .then(() => this.start());
    return this.operation;
  }

  dispose() {
    return this.stop();
  }
}

async function restartFromCommand(controller) {
  try {
    await controller.restart();
    vscode.window.showInformationMessage('Ari language server restarted.');
  } catch (error) {
    const message = error && error.message ? error.message : String(error);
    vscode.window.showErrorMessage(`Failed to restart Ari language server: ${message}`);
  }
}

function registerAriLsp(context) {
  const controller = new AriLspController();
  controller.start().catch((error) => {
    const message = error && error.message ? error.message : String(error);
    vscode.window.showErrorMessage(`Failed to start Ari language server: ${message}`);
  });
  context.subscriptions.push(vscode.commands.registerCommand(
    'ari.restartLanguageServer',
    () => restartFromCommand(controller)
  ));
  context.subscriptions.push({
    dispose: () => controller.dispose()
  });
  context.subscriptions.push(vscode.workspace.onDidChangeConfiguration((event) => {
    if (shouldRestart(event)) controller.restart();
  }));
  return controller;
}

module.exports = {
  registerAriLsp
};
