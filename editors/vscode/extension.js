const { registerAriCommands } = require('./commands');
const { registerAriLsp } = require('./lsp');
const { registerAriTasks } = require('./tasks');

let lspController;

function activate(context) {
  registerAriCommands(context);
  registerAriTasks(context);
  lspController = registerAriLsp(context);
}

function deactivate() {
  if (!lspController) return undefined;
  return lspController.dispose();
}

module.exports = {
  activate,
  deactivate
};
