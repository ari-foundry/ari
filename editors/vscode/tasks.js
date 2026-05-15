const vscode = require('vscode');

const TASK_TYPE = 'ari';

const TASKS = [
  { target: 'build', label: 'Ari: make', args: [] },
  { target: 'check', label: 'Ari: make check', args: ['check'] },
  { target: 'tools', label: 'Ari: make tools', args: ['tools'] },
  { target: 'check-tools', label: 'Ari: make check-tools', args: ['check-tools'] },
  { target: 'lint', label: 'Ari: make lint', args: ['lint'] },
  { target: 'lsp', label: 'Ari: make lsp', args: ['lsp'] }
];

function workspaceFolder() {
  return vscode.workspace.workspaceFolders && vscode.workspace.workspaceFolders[0];
}

function makeTask(definition) {
  const folder = workspaceFolder();
  const scope = folder || vscode.TaskScope.Workspace;
  const task = new vscode.Task(
    { type: TASK_TYPE, target: definition.target },
    scope,
    definition.label,
    'ari',
    new vscode.ShellExecution('make', definition.args, folder ? { cwd: folder.uri.fsPath } : undefined),
    []
  );
  task.group = definition.target === 'build' ? vscode.TaskGroup.Build : vscode.TaskGroup.Test;
  return task;
}

function findDefinition(target) {
  return TASKS.find((task) => task.target === target);
}

class AriTaskProvider {
  provideTasks() {
    return TASKS.map(makeTask);
  }

  resolveTask(task) {
    const target = task.definition && task.definition.target;
    const definition = findDefinition(target);
    if (!definition) return undefined;
    return makeTask(definition);
  }
}

function registerAriTasks(context) {
  context.subscriptions.push(vscode.tasks.registerTaskProvider(TASK_TYPE, new AriTaskProvider()));
}

module.exports = {
  registerAriTasks
};
