# docs

## Scope
Project knowledge base:
- architecture decision records (ADRs)
- contributor onboarding guides
- coding standards and review policies
- subsystem ownership map

## Core documents
- `ARCHITECTURE.md` — layering rules and dependency direction.
- `MODULE_GUIDE.md` — process for adding a new swappable module.
- `CODING_STANDARD.md` — naming, error handling, logging, assertions.
- `TESTING_STRATEGY.md` — unit/integration/contract/performance validation.
- `RFC_TEMPLATE.md` — design proposal template for major technical changes.
- `ADR_TEMPLATE.md` — architecture decision record template.

## Ownership
- **Primary owner:** Architecture Council.
- **Contributors:** every team updates docs with behavior or interface changes.

## Extension points
- Store decisions in ADR format with context, decision, and consequences.
- Keep onboarding docs task-oriented (first build, debug, test, contribution flow).
- Version coding standards and enforce them in CI/tooling over time.

## Parallel-work rules
- Significant technical changes must land with matching documentation updates.
- Deprecated guidance should be removed or marked with replacement links.
