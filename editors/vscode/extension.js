const vscode = require('vscode');
const { LanguageClient } = require('vscode-languageclient/node');
const { registerAriCommands, resolveWorkspacePath } = require('./commands');

let client;

function activate(context) {
  registerAriCommands(context);

  const config = vscode.workspace.getConfiguration('ari');
  const compilerPath = resolveWorkspacePath(config.get('compilerPath', 'build/ari'));
  const lspPath = resolveWorkspacePath(config.get('lspPath', 'build/ari-lsp'));
  const modulePaths = config.get('modulePaths', []);

  const args = ['--ari', compilerPath];
  for (const modulePath of modulePaths) {
    args.push('-I');
    args.push(resolveWorkspacePath(modulePath));
  }

  client = new LanguageClient(
    'ari-lsp',
    'Ari Language Server',
    {
      command: lspPath,
      args,
      options: {
        cwd: vscode.workspace.workspaceFolders && vscode.workspace.workspaceFolders[0]
          ? vscode.workspace.workspaceFolders[0].uri.fsPath
          : undefined
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

  client.start();
  context.subscriptions.push({
    dispose: () => {
      if (client) client.stop();
    }
  });
}

function deactivate() {
  if (!client) return undefined;
  return client.stop();
}

module.exports = {
  activate,
  deactivate
};
