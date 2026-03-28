# 0714-02-CSE-2100
Course Code: 0714 02 CSE 2100 || Course Title : Advanced Programming Laboratory
 || ID : 240204 , 240215
 
# Gambit Chess Engine: C to C++ Transition with OOD and SOLID Implementation

**Course:** Advanced Programming Lab (2nd Year CSE)  
**Project:** Gambit Chess Engine   

---

## 1. Executive Overview

This report presents a structured transition plan from legacy C-style implementation to modern C++ architecture for the Gambit Chess Engine, with explicit focus on:

1. Object-Oriented Design (OOD)
2. SOLID principles implementation
3. Incremental migration without gameplay regression
4. Maintainability, testability, and extensibility

The goal is not only to modernize syntax, but to redesign responsibility boundaries so the engine remains correct, scalable, and easier to evolve.

---

## 2. Why Transition from C to C++

The transition is justified by both engineering and academic value.

### Engineering motivations

1. Better encapsulation and state safety through classes and access control.
2. Cleaner module boundaries using interfaces and polymorphism.
3. Reduced memory risk via RAII (`std::vector`, `std::unique_ptr`, `std::array`).
4. Easier feature extension (new evaluators, renderers, protocols, time controls).
5. Better testability through dependency injection and interface contracts.

### Academic motivations

1. Demonstrates practical application of OOD in systems-level software.
2. Demonstrates applied SOLID in a non-trivial domain (chess engine).
3. Demonstrates design pattern usage under performance constraints.

---

## 3. Transition Objectives

### Primary objectives

1. Preserve chess correctness and protocol behavior.
2. Replace broad shared state with cohesive domain classes.
3. Introduce clear abstraction layers:
- UI and protocols
- Engine services
- Core chess domain
4. Implement all five SOLID principles in concrete code structure.

### Non-objectives

1. No risky big-bang rewrite.
2. No immediate algorithmic redesign of search/evaluation logic.
3. No behavior-breaking API removal before compatibility layer.

---

## 4. Target OOD Architecture

### Layered model

1. **Presentation Layer**
- SDL GUI
- UCI protocol
- XBoard protocol

2. **Application/Service Layer**
- Search service
- Evaluation service
- Game controller/service orchestration

3. **Domain Layer**
- Board state and rules
- Move generation and execution
- Attack and validation logic

4. **Infrastructure Layer**
- Hash table
- Opening book IO
- Timer, logging, utility services

### Dependency rule

Dependencies flow downward only:

`Presentation -> Services -> Domain -> Infrastructure`

No domain module depends on presentation concerns.

---

## 5. Core Class and Interface Design

### Key classes (concrete)

1. `ChessBoard`
2. `SearchEngine`
3. `StaticEvaluator`
4. `HashTable`
5. `OpeningBook`
6. `GameController`

### Key interfaces (contracts)

1. `IBoardQuery` (read-only board state)
2. `IBoardModifier` (state mutation operations)
3. `IBoardSetup` (initialization/reset/FEN setup)
4. `IEvaluator` (position scoring)
5. `ISearchService` (best-move search contract)
6. `IProtocol` (UCI/XBoard loop abstraction)
7. `IRenderer` (UI backend abstraction)

This split is designed to enforce both ISP and DIP.

---

## 6. SOLID Implementation Plan

## 6.1 S: Single Responsibility Principle (SRP)

### Problem pattern
Large files and large state objects mixing unrelated concerns.

### Implementation
1. Separate GUI responsibilities:
- rendering
- input handling
- timer management
- move history tracking
- game flow control

2. Separate board state concerns from search/session concerns.

3. Separate protocol parsing from engine execution service.

### Expected result
Each class has one clear reason to change.

## 6.2 O: Open/Closed Principle (OCP)

### Implementation
1. Use interface-driven extension points for:
- evaluator strategies
- search strategies
- renderer backends
- protocol handlers

2. Add new behavior by creating new classes implementing interfaces, not modifying stable core classes.

### Expected result
New features are added with minimal risk to existing modules.

## 6.3 L: Liskov Substitution Principle (LSP)

### Implementation
1. Define strict behavioral contracts for every interface.
2. Ensure all implementations are substitutable (same preconditions/postconditions).
3. Add interface-level test suites reused by all implementations.

### Expected result
Any evaluator/search/protocol implementation can be swapped safely.

## 6.4 I: Interface Segregation Principle (ISP)

### Implementation
1. Split broad board API into small role-based interfaces:
- query-only
- mutation-only
- setup-only

2. Make modules depend only on the methods they actually need.

### Expected result
Reduced coupling and smaller compile-time dependency surface.

## 6.5 D: Dependency Inversion Principle (DIP)

### Implementation
1. High-level modules depend on abstractions (`IEvaluator`, `ISearchService`, `IBoardQuery`).
2. Concrete implementations are injected via constructor or factory wiring.
3. Remove direct calls from UI/protocol code into concrete low-level internals.

### Expected result
Architecture becomes testable, replaceable, and stable under change.

---

## 7. Design Patterns Used

1. **Strategy Pattern**
- interchangeable evaluators and search policies

2. **Factory Pattern**
- centralized creation of board/service/protocol objects

3. **Facade Pattern**
- simplified API for game loop orchestration

4. **Command Pattern** (optional expansion)
- move command history for structured undo/redo

5. **Observer Pattern** (optional expansion)
- notifications for game events (check, mate, draw, timeout)

---

## 8. Migration Roadmap (Phased)

## Phase 1: Baseline and Safety Net

1. Freeze functional baseline.
2. Add perft snapshots and protocol smoke checks.
3. Add build checks for target modules.

**Deliverable:** deterministic baseline report.

## Phase 2: Type Modernization

1. Convert key macros/constants to `constexpr` and `enum class` where safe.
2. Introduce typed wrappers for move/board primitives.

**Deliverable:** modernized type layer with no behavior change.

## Phase 3: Interface Introduction

1. Add `IBoardQuery`, `IBoardModifier`, `IBoardSetup`, `IEvaluator`, `ISearchService`.
2. Keep adapter layer for compatibility.

**Deliverable:** compile-safe abstraction layer.

## Phase 4: Core Refactor

1. Migrate core modules from direct concrete access to interface calls.
2. Encapsulate mutable board internals behind methods.

**Deliverable:** reduced direct field coupling.

## Phase 5: Service and UI Decoupling

1. Route protocol and GUI flows through service interfaces.
2. Remove protocol/UI direct dependency on core internals.

**Deliverable:** clean Presentation -> Service boundaries.

## Phase 6: Hardening and Cleanup

1. Apply RAII ownership cleanup.
2. Remove dead compatibility paths.
3. Final static analysis and review.

**Deliverable:** production-ready OOD/SOLID architecture.

---

## 9. Risk Management

### Key risks

1. Functional regression in move legality or search behavior.
2. Hidden dependency breakage during encapsulation.
3. Performance regression from abstraction overhead.

### Mitigations

1. Compile-and-test after every small patch batch.
2. Perft regression gates at fixed depths.
3. Protocol golden-output checks for UCI/XBoard.
4. Nodes-per-second benchmark comparison before/after each phase.
5. Compatibility layer during transition window.

---

## 10. Validation and Quality Gates

Transition is accepted only if all checks pass:

1. Core compile gates pass.
2. Perft baselines match expected values.
3. UCI and XBoard smoke tests pass.
4. GUI basic interaction works (if environment dependencies exist).
5. No new critical static-analysis findings.
6. Performance regression remains under accepted threshold.

---

## 11. **AI Prompt Set for Execution**


## Prompt 1 - Migration Inventory
"Analyze the chess engine and list where we still use old C-style code. For each file, suggest the C++ replacement in simple terms and explain the risk level. Give us a safe order to start, so we can begin with low-risk files first."

## Prompt 2 - Baseline Lock
"Create a clear baseline before we refactor anything. Include compile checks, perft checkpoints, and quick UCI/XBoard tests with expected output. This baseline will help us prove that behavior did not break after changes."

## Prompt 3 - Interface Scaffold
"Introduce core interfaces like IBoardQuery, IBoardModifier, IBoardSetup, IEvaluator, and ISearchService. Keep adapters so old code can still run while we migrate step by step. Do this with minimal behavior change so we stay stable."

## Prompt 4 - Encapsulation Pass
"Replace direct board field access with getter and setter methods in core modules. After each safe batch, make those fields private so outside code cannot change them directly. Keep this gradual and compile after every batch."

## Prompt 5 - SRP Decomposition
"Find the three biggest files that are doing too many jobs. Split each file into smaller focused classes or components, where each one has one main responsibility. Keep behavior exactly the same while doing this split."

## Prompt 6 - DIP Enforcement
"Refactor protocol and GUI code so they talk to interfaces, not concrete engine internals. Remove direct calls to specific evaluation and search implementations. This will reduce coupling and make testing easier."

## Prompt 7 - RAII and Ownership
"Replace manual memory handling with RAII-based C++ structures. Use safe ownership tools like standard containers and smart pointers where needed. Keep object lifetime behavior the same as before."

## Prompt 8 - Regression Gate
"After every refactor batch, run compile checks, perft checks, and protocol checks. Compare results with the baseline and show pass or fail clearly. If there is any difference, point to the exact place where it changed."

## Prompt 9 - Performance Gate
"Benchmark engine speed before and after refactoring. Measure search throughput and depth-time behavior using the same settings. Flag any slowdown above the accepted threshold and report where it appears."

## Prompt 10 - Final SOLID Audit
"Run a final code audit focused on SOLID quality. Rank issues by severity and explain each issue in simple words. For every remaining violation, suggest a concrete and practical fix."



## Conclusion

This transition plan establishes a disciplined path from legacy C-style code to modern C++ architecture with OOD and SOLID at the center. The design balances correctness, maintainability, and extensibility through incremental refactoring, strict validation gates, and interface-driven boundaries.

The final outcome is a chess engine architecture suitable for both academic evaluation and long-term engineering evolution.
