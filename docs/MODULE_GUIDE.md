# Module Guide

Use this guide to add a new swappable module under `engine/modules`.

## 1) Decide module category

Pick the swap policy in the module descriptor:

- `RuntimeSwappable`: safe to replace while engine is running.
- `RestartRequired`: requires restart due to global state/resource ownership.

If uncertain, start with `RestartRequired` and justify any move to runtime-swappable.

## 2) Implement lifecycle contract

Implement the `IModule` lifecycle methods:

- `onLoad`
- `onStart`
- `onStop`
- `onUnload`

Lifecycle must be idempotent where practical and fail fast with clear diagnostics.

## 3) Define module descriptor metadata

Populate module manifest/descriptor fields:

- module name and semantic version
- dependencies
- conflicts
- API compatibility version
- swap policy

The module manager validates these during registration/activation.

## 4) Provide stable ABI entry points (for dynamic modules)

Export the C ABI required by the contract:

- `CreateModule`
- `DestroyModule`
- `QueryModuleApiVersion`

Ensure binary compatibility checks happen before activation.

## 5) Isolate dependencies

- Prefer contract/event interfaces over direct module-to-module calls.
- Do not depend on devtools internals.
- Keep optional feature logic in the module itself.

## 6) Testing requirements

For each module add:

- unit tests for module-local logic
- contract tests for descriptor validation and lifecycle ordering
- integration tests for dependency/conflict behavior with `ModuleManager`
- hot-swap tests if policy is `RuntimeSwappable`

## 7) Documentation and review

- Update relevant subsystem docs when adding new contracts.
- Add ADR/RFC when the module introduces a new compatibility or lifecycle policy.
- Request review from modules owner and affected subsystem owners.
