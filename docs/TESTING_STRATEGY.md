# Testing Strategy

This repository uses a layered test strategy: unit, integration, and performance validation.

## Goals

- Catch regressions early at subsystem boundaries.
- Keep contracts stable across backends/modules.
- Validate that performance-sensitive paths stay within budget.

## 1) Unit testing

Scope:

- pure logic
- lifecycle state machines
- descriptor parsing/validation
- utility/helpers

Guidelines:

- fast, deterministic, isolated
- no external process/network dependencies
- run on every pull request

## 2) Integration testing

Scope:

- cross-subsystem behavior (`core` + `platform`, module manager flows)
- backend factory selection and fallback behavior
- dependency/conflict resolution for modules

Guidelines:

- exercise real integration points, not mocks everywhere
- focus on high-value user/runtime scenarios
- run on every pull request in debug configuration

## 3) Contract testing

Scope:

- public interface compliance for backends/modules
- ABI/API compatibility checks (module API version gates)

Guidelines:

- required for any new backend or swappable module
- must fail clearly with actionable diagnostics

## 4) Performance validation

Scope:

- startup/initialization time
- frame-critical code paths
- module load/unload overhead

Guidelines:

- define baseline metrics and regression thresholds
- run in controlled environment for trend tracking
- treat large regressions as release blockers

## CI expectations

- Build and test matrix across Linux and Windows toolchains.
- Formatting/linting gates must pass before merge.
- New public contracts require corresponding tests in the same PR.

## PR checklist

- [ ] Added/updated unit tests for changed logic.
- [ ] Added/updated integration or contract tests for changed interfaces.
- [ ] Documented performance impact for hot-path changes.
- [ ] Verified CI matrix and lint/format gates are green.
