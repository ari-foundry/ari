# Documentation Ownership

This note records Ari documentation ownership before any project or site split.
It is a policy guide for where documentation should live while the ecosystem is
still mostly bundled in this repository.

## Policy

- The top-level Ari site is a portal only. It owns project overview, install
  entry points, release links, compatibility information, and links to
  project-specific docs. It does not own detailed language, compiler, standard
  library, package-manager, lint, LSP, or editor documentation.
- The `ari` repository keeps the Ari language docs, compiler docs, standard
  library docs, developer docs, and current Ari-written compiler/bootstrap
  notes.
- `docs/dev` remains Ari project-owned. The developer guide belongs to the Ari
  compiler project, is published as part of the Ari project docs, and is not
  being split into a separate repository at this stage.
- `arix` is the Ari package manager project. It starts separately and owns
  package-manager documentation and implementation outside this repository.
- Lint, LSP, and editor docs stay bundled with `ari` until their projects
  split. While the tools are bundled here, their docs may remain here with clear
  status notes.
