const fs = require('fs');
const path = require('path');
const { spawnSync } = require('child_process');

const extensionRoot = path.resolve(__dirname, '..');
const repoRoot = path.resolve(extensionRoot, '..', '..');
const packageJson = require(path.join(extensionRoot, 'package.json'));
const buildDir = path.join(repoRoot, 'build', 'vscode');
const outputPath = path.join(buildDir, `${packageJson.name}-${packageJson.version}.vsix`);
const runtimeStage = path.join(buildDir, '.ari-vscode-runtime');

const runtimePackages = [
  'balanced-match',
  'brace-expansion',
  'minimatch',
  'semver',
  'vscode-jsonrpc',
  'vscode-languageclient',
  'vscode-languageserver-protocol',
  'vscode-languageserver-types'
];

function commandName(base) {
  return process.platform === 'win32' ? `${base}.cmd` : base;
}

function run(command, args, options = {}) {
  const result = spawnSync(command, args, {
    cwd: options.cwd || extensionRoot,
    stdio: 'inherit',
    shell: false
  });
  if (result.status !== 0) {
    process.exit(result.status || 1);
  }
}

function ensureRuntimeDeps() {
  const missing = runtimePackages.filter((name) => {
    return !fs.existsSync(path.join(extensionRoot, 'node_modules', name));
  });
  if (missing.length > 0) {
    console.error(`Missing VS Code runtime dependencies: ${missing.join(', ')}`);
    console.error('Run npm install from editors/vscode before packaging.');
    process.exit(1);
  }
}

function stageRuntimeDeps() {
  fs.rmSync(runtimeStage, { recursive: true, force: true });
  const nodeModulesStage = path.join(runtimeStage, 'extension', 'node_modules');
  fs.mkdirSync(nodeModulesStage, { recursive: true });
  for (const name of runtimePackages) {
    fs.cpSync(
      path.join(extensionRoot, 'node_modules', name),
      path.join(nodeModulesStage, name),
      { recursive: true }
    );
  }
}

function packageBaseVsix() {
  fs.mkdirSync(buildDir, { recursive: true });
  run(commandName('npx'), [
    'vsce',
    'package',
    '--allow-missing-repository',
    '--skip-license',
    '--no-dependencies',
    '--out',
    outputPath
  ]);
}

function addRuntimeDepsToVsix() {
  run('zip', ['-qr', outputPath, 'extension/node_modules'], { cwd: runtimeStage });
}

ensureRuntimeDeps();
packageBaseVsix();
stageRuntimeDeps();
addRuntimeDepsToVsix();
fs.rmSync(runtimeStage, { recursive: true, force: true });
console.log(`Packaged installable Ari VS Code extension: ${outputPath}`);
