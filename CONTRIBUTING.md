# Contributing to FOSSredder

Thanks for taking the time to contribute.

This document describes the preferred workflow for working on FOSSredder so that changes stay consistent, reviewable, and release-friendly.

## Branch model

- `master` is the release line.
- `develop` is the integration line for ongoing work.
- Feature, bugfix, refactor, and release-related work should normally target `develop`.
- Release work moves from `develop` to `master` only when a release is being prepared.

## Workflows

### Feature work

- Create a feature branch from `develop`.
- Open a pull request back into `develop`.
- Use the `Feature request` issue form for new product work or meaningful workflow changes.

### Bug fixes

- Create a branch from `develop` unless a hotfix against a released version is explicitly required.
- Open a pull request back into `develop`.
- Use the `Bug report` issue form for reproducible defects or regressions.

### Refactors

- Use the `Refactor` issue form for architecture cleanup, structural changes, or codebase improvements that should preserve the intended behavior.
- Keep refactors focused and behavior-preserving unless the issue explicitly states otherwise.

### Release work

- Use the `Release task` issue form for versioning, tagging, release verification, packaging checks, or milestone closure.
- Release tasks belong to the milestone they help complete.
- Final release pull requests should move from `develop` to `master`.

## Issue creation

- Prefer the GitHub issue forms instead of free-form issues.
- Choose the template that best matches the work:
  - `Feature request`
  - `Bug report`
  - `Refactor`
  - `Release task`
- Fill in the fields with concrete, actionable information.
- Include acceptance criteria or a clear checklist where possible.

## Labels

- Use labels to describe the issue type, priority, and current status.
- Keep label usage consistent with the repository conventions.
- When in doubt, mark the issue for triage instead of guessing.

Recommended label groups:

- `type/*` for the kind of work
- `prio/*` for priority
- `status/*` for the current workflow state

## Pull requests

- Keep pull requests small and focused whenever possible.
- Link the related issue in the pull request description.
- Include testing notes when behavior changes or risk is non-trivial.
- Prefer squashable, reviewable changes over large mixed-purpose commits.

## Quality expectations

Before opening a pull request, make sure the change is in a good state:

- formatting is consistent
- tests were added or updated when appropriate
- documentation was updated when behavior or usage changed
- no unnecessary magic values or hidden assumptions were introduced
- architecture changes stay aligned with the existing core design

## Documentation

- Update `docs/` when the change affects architecture, workflow, or product behavior.
- Update the README when setup, usage, or release-related instructions change.
- Keep code and documentation aligned.

## Releases

- A milestone represents a version target.
- A release task closes out the version target by handling the release-specific work.
- Use the release task to finish tagging, packaging checks, release notes, and final verification.

## Questions and triage

- If something is unclear, create an issue with the best matching template and add enough context for triage.
- If the work does not fit an existing template, open a discussion or ask a maintainer to triage it.
