const { resolveWorkspacePath } = require('./paths');

function modulePathArgs(modulePaths) {
  const args = [];
  for (const modulePath of modulePaths || []) {
    args.push('-I');
    args.push(resolveWorkspacePath(modulePath));
  }
  return args;
}

function lintRuleArgs(lintRules) {
  const args = [];
  for (const [rule, severity] of Object.entries(lintRules || {})) {
    if (!severity) continue;
    args.push('--rule');
    args.push(`${rule}=${severity}`);
  }
  return args;
}

module.exports = {
  lintRuleArgs,
  modulePathArgs
};
