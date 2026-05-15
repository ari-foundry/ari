const vscode = require('vscode');
const { LanguageClient } = require('vscode-languageclient/node');
const { lintRuleArgs, modulePathArgs } = require('./config');
const { resolveWorkspacePath, workspaceRoot } = require('./paths');

function lspArgs(config) {
  const compilerPath = resolveWorkspacePath(config.get('compilerPath', 'build/ari'));
  return [
    '--ari',
    compilerPath,
    ...modulePathArgs(config.get('modulePaths', [])),
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
    event.affectsConfiguration('ari.lintRules');
}

class AriLspController {
  constructor() {
    this.client = undefined;
    this.operation = Promise.resolve();
  }

  start() {
    this.client = createClient();
    this.client.start();
  }

  stop() {
    if (!this.client) return Promise.resolve();
    const client = this.client;
    this.client = undefined;
    return client.stop();
  }

  restart() {
    this.operation = this.operation
      .then(() => this.stop())
      .then(() => this.start());
    return this.operation;
  }

  dispose() {
    return this.stop();
  }
}

function registerAriLsp(context) {
  const controller = new AriLspController();
  controller.start();
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
