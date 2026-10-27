# CampusRule Compiler — A Domain-Specific Language for College Rule Automation

[![Language](https://img.shields.io/badge/Language-C11-blue.svg)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))
[![Standard](https://img.shields.io/badge/Standard-ISO%2FIEC%209899%3A2011-green.svg)]()
[![Build System](https://img.shields.io/badge/Build-GNU%20Make-orange.svg)](https://www.gnu.org/software/make/)
[![Architecture](https://img.shields.io/badge/Architecture-Modular%20Compiler%20Pipeline-purple.svg)]()
[![Tests](https://img.shields.io/badge/Tests-143%20Passed-brightgreen.svg)]()

> **Course Based Project Report**  
> **Course:** Compiler Design and Computer Networks Laboratory (22PC2CS301)  
> **Degree:** Bachelor of Technology in Information Technology  
> **Institution:** Department of Information Technology, Vallurupalli Nageswara Rao Vignana Jyothi Institute of Engineering & Technology (VNRVJIET), Hyderabad  
> **Original Authors:** M. Vibhas (24071A1233), B. Bhanu Prasad (25075A1201), B. Sai Hansika (25075A1202), G. Akshitha (25075A1203)  
> **Supervisors:** Dr. B. Srivani, Mrs. S. Renu Deepthi  
> **C Re-Implementation & Architecture:** Senior Compiler & Systems Engineering Team

---

## 1. Project Introduction

**CampusRule Compiler** is a complete, production-grade Domain-Specific Language (DSL) processing system implemented from scratch in **C11**. It automates the evaluation of complex college academic rules governing:
- **Examination Eligibility**
- **Placement Eligibility**
- **Scholarship Eligibility**

Evaluating student eligibility rules manually in educational institutions is tedious, error-prone, and difficult to maintain as institutional policies evolve. CampusRule Compiler solves this by providing a clean, human-readable DSL that college administrators and department heads can use to express eligibility policies without writing low-level code.

The system embodies the textbook compiler design pipeline:
$$\text{Input Source} \longrightarrow \text{Lexical Analysis} \longrightarrow \text{Syntax Analysis} \longrightarrow \text{AST Construction} \longrightarrow \text{Semantic Analysis} \longrightarrow \text{Three-Address Code (TAC)} \longrightarrow \text{Rule Evaluation} \longrightarrow \text{Multi-Category Verdict}$$

---

## 2. Problem Statement

Educational institutions handle thousands of student academic records each semester. Institutional policies require evaluating composite criteria involving attendance records, cumulative grade point averages (CGPA), subject marks, and backlog counts. 

Traditional approaches rely on manual checking or scattered hardcoded scripts in spreadsheets. This leads to:
1. **Human Error**: Overlooking backlogs or miscalculating threshold boundaries.
2. **Lack of Transparency**: Students and auditors receive unexplained rejection verdicts.
3. **Rigid Policies**: Changing a threshold requires code deployment or manual database modification.
4. **Category Conflation**: An exam eligibility failure can ambiguously mask an independent placement or scholarship qualification.

---

## 3. Objectives and Scope

### Primary Objectives
- Provide a domain-specific, case-insensitive, readable grammar (`IF ... THEN ... END`).
- Demonstrate every fundamental compiler phase with authentic intermediate data structures:
  - Lexical Token Stream with coordinates (line, column, position).
  - Concrete Abstract Syntax Tree (AST) with clean recursive traversal and destruction.
  - Rigorous Semantic Type and Range checking.
  - Three-Address Code (TAC) generation with temporary variables and jump labels.
  - Deterministic multi-category rule evaluation engine.
- Deliver both an interactive desktop Graphical User Interface (GUI) and a scriptable Command-Line Interface (CLI).
- Implement strict memory safety in C with zero memory leaks and robust defensive checks.

### Scope
- Supported variables: `ATTENDANCE`, `CGPA`, `MARKS`, `BACKLOGS`.
- Supported actions: `ELIGIBLE EXAM`, `NOT ELIGIBLE EXAM`, `ELIGIBLE PLACEMENT`, `NOT ELIGIBLE PLACEMENT`, `ELIGIBLE SCHOLARSHIP`, `NOT ELIGIBLE SCHOLARSHIP`.
- Multi-rule compilation in a single compilation unit.
- Multi-category conflict detection and deterministic resolution policy.

---

## 4. Key Features

- **Strict C11 Implementation**: Written using standard C11 (`-std=c11 -Wall -Wextra -Wpedantic`).
- **Zero Mocking / Fake Output**: All 7 inspection tabs display actual tokens, parsed AST nodes, semantic diagnostics, TAC instructions, and evaluation traces computed dynamically at runtime.
- **Multi-Category Independence**: Independent decision evaluation for `EXAM`, `PLACEMENT`, and `SCHOLARSHIP`.
- **Deterministic Conflict Policy**: Explicit detection of contradictory rules (`CONFLICT`), positive eligibility (`ELIGIBLE`), negative eligibility (`NOT ELIGIBLE`), and unaddressed categories (`NO DECISION`).
- **Extended Scholarship Grammar**: Adds full language and compiler support for scholarship eligibility.
- **Parenthesized Compound Logic**: Fully supports nested expressions like `IF (ATTENDANCE >= 75 OR CGPA >= 8.5) AND BACKLOGS == 0 THEN ... END`.
- **Dual GUI Support**:
  - **Native Win32 GUI**: Runs natively on Windows without third-party dependencies.
  - **GTK 3 GUI**: Built with GTK 3 for cross-platform Linux and Unix environments.
- **Headless CLI Mode**: Fast, headless compilation and batch testing for terminal and CI/CD workflows.
- **100% Automated Test Suite**: 143 unit test assertions covering every compiler module.

---

## 5. Technology Stack

| Component | Technology | Rationale |
|---|---|---|
| **Language** | C (C11 Standard) | Native performance, memory control, standard academic compiler language |
| **Compiler** | GCC 16.2 / Clang | High optimization, strict diagnostic warnings (`-Wall -Wextra -Wpedantic`) |
| **Build System** | GNU Make 4.4 | Deterministic dependency tracking, multi-target compilation |
| **Desktop GUI (Windows)** | Native Win32 API & Common Controls | Zero external dependencies, native speed, identical visual fidelity |
| **Desktop GUI (Linux/Unix)** | GTK+ 3.0 via C bindings | Cross-platform GNOME desktop standard |
| **Testing** | Custom C Test Harness | High-speed, isolated unit test runner with per-phase assertion reporting |

---

## 6. Architecture and Module Responsibilities

The codebase follows a strictly modular architecture with clear ownership boundaries:

```
CampusRuleCompiler/
├── include/
│   ├── common.h         # Centralized academic ranges, memory helpers, string builder
│   ├── token.h          # CRTokenType enum, Token structure, TokenList dynamic array
│   ├── lexer.h          # Lexical analyzer interface
│   ├── ast.h            # Tagged-union AST node hierarchy and tree formatting
│   ├── parser.h         # Recursive descent parser interface
│   ├── semantic.h       # Semantic analyzer and student data validator
│   ├── tac.h            # Three-Address Code IR structures and generator
│   ├── evaluator.h      # Rule evaluation engine and category decision policy
│   ├── compiler.h       # Pipeline orchestrator and sample rule registry
│   └── gui.h            # GUI launcher interface
├── src/
│   ├── main.c           # CLI argument parsing, sample execution, GUI dispatch
│   ├── token.c          # Token allocation, formatting, and cleanup
│   ├── lexer.c          # Lexical tokenizer and coordinate tracker
│   ├── ast.c            # AST construction, deep tree destruction, tree printer
│   ├── parser.c         # Recursive descent parser with operator precedence
│   ├── semantic.c       # Semantic range checking and duplicate rule detection
│   ├── tac.c            # TAC intermediate representation generation
│   ├── evaluator.c      # Condition tree evaluation and multi-category policy
│   ├── compiler.c       # 7-phase compilation pipeline orchestrator
│   ├── gui.c            # Platform GUI dispatcher (GTK3 or Win32)
│   ├── gui_gtk.c        # GTK 3 desktop application implementation
│   └── gui_win32.c      # Native Win32 desktop application implementation
├── tests/
│   ├── test_lexer.c     # Lexer unit tests (keywords, numbers, operators, errors)
│   ├── test_parser.c    # Parser unit tests (precedence, parentheses, syntax errors)
│   ├── test_ast.c       # AST unit tests (node creation, tree printing, destruction)
│   ├── test_semantic.c  # Semantic tests (ranges, unknown vars, student bounds)
│   ├── test_tac.c       # TAC tests (temporaries, labels, jumps, print)
│   ├── test_evaluator.c # Evaluator tests (comparisons, AND/OR, conflicts)
│   └── test_main.c      # Unified test runner with colorized statistics
├── examples/            # Built-in and standalone .cr rule files
├── Makefile             # GNU Make build configuration
└── README.md            # Comprehensive project documentation
```

---

## 7. DSL Grammar Specification

The language grammar is defined in Backus-Naur Form (BNF):

```bnf
<program>              ::= <rule>+ <EOF>

<rule>                 ::= "IF" <expression> "THEN" <action> "END"

<expression>           ::= <or_expression>

<or_expression>        ::= <and_expression> ( "OR" <and_expression> )*

<and_expression>       ::= <primary> ( "AND" <primary> )*

<primary>              ::= <comparison>
                         | "(" <expression> ")"

<comparison>           ::= <identifier> <comparison_operator> <number>

<comparison_operator>  ::= ">" | "<" | ">=" | "<=" | "==" | "!="

<action>               ::= [ "NOT" ] "ELIGIBLE" [ "FOR" ] <category>

<category>             ::= "EXAM" | "PLACEMENT" | "SCHOLARSHIP"
```

### Precedence & Associativity
1. **Parentheses `( ... )`**: Highest precedence, groups nested logic.
2. **Comparison Operators (`>`, `<`, `>=`, `<=`, `==`, `!=`)**: Non-associative binary relations.
3. **Logical `AND`**: Left-associative, higher precedence than `OR`.
4. **Logical `OR`**: Left-associative, lowest logical precedence.

---

## 8. Academic Variables & Permitted Ranges

All academic variables are strictly typed and range-checked:

| Variable | Type | Permitted Range | Description | Constraints |
|---|---|---|---|---|
| `ATTENDANCE` | Numeric (`double`) | `0.0` to `100.0` | Class attendance percentage | Non-negative, $\le 100\%$ |
| `CGPA` | Numeric (`double`) | `0.0` to `10.0` | Cumulative Grade Point Average | 10-point scale |
| `MARKS` | Numeric (`double`) | `0.0` to `100.0` | Aggregate academic marks | Non-negative percentage |
| `BACKLOGS` | Integer (`double`) | `0` to `50` | Active backlog count | **Must be a whole number** (no decimals) |

---

## 9. Compiler Phases Detailed

### Phase 1: Input (`PHASE_INPUT`)
Captures and displays the raw DSL source text exactly as provided by the user or file.

### Phase 2: Lexical Analysis (`PHASE_LEXICAL`)
- Tokenizes character stream into typed tokens.
- Preserves exact source location: `line`, `col`, and 1-based character position `pos`.
- Normalizes keywords and identifiers to uppercase for case-insensitivity.
- Validates numeric literals (integer and floating-point).
- Rejects unsupported characters (e.g. `&`, `|`, `@`, `$`, isolated `=`).
- Example output:
  ```
   1. IF 'IF'
   2. IDENTIFIER 'ATTENDANCE'
   3. GREATER_EQUAL '>='
   4. NUMBER '75'
   5. THEN 'THEN'
   6. ELIGIBLE 'ELIGIBLE'
   7. EXAM 'EXAM'
   8. END 'END'
   9. EOF ''
  ```

### Phase 3: Syntax Analysis (`PHASE_PARSING`)
- Constructs concrete rule structures using recursive descent.
- Enforces strict statement ordering: `IF` $\to$ `<expression>` $\to$ `THEN` $\to$ `<action>` $\to$ `END`.
- Returns clear diagnostic messages if expected tokens are missing:
  `Syntax Error (token 5): Expected THEN but found ELIGIBLE`

### Phase 4: Abstract Syntax Tree (`PHASE_AST`)
- Produces a structural tagged-union tree.
- Renders indented hierarchical visualization:
  ```
  Program (1 rule(s))
    Rule 1 (IF)
      Condition
        AND
          ATTENDANCE >= 75.0
          BACKLOGS == 0.0
      Action: ELIGIBLE FOR EXAM
  ```

### Phase 5: Semantic Analysis (`PHASE_SEMANTIC`)
- Verifies that referenced variables exist in the centralized academic symbol table.
- Validates that thresholds fall within `[MIN, MAX]`.
- Enforces that `BACKLOGS` is an integer:
  `[ERROR] BACKLOGS must be a whole number, found 1.5`
- Detects duplicate rules for the same action:
  `[ERROR] Duplicate rule: 'ELIGIBLE FOR EXAM' is already defined by an earlier rule`
- Validates student data separately (checks for NaN, Inf, and bounds).

### Phase 6: Three-Address Code (`PHASE_TAC`)
- Generates linearized quadruples/triples using temporary variables (`t1`, `t2`, ...) and labels (`L1`, `L2`, ...):
  ```
  Rule 1:
      t1 = ATTENDANCE >= 75.0
      t2 = BACKLOGS == 0.0
      t3 = t1 AND t2
      IF_FALSE t3 GOTO L1
      PRINT "ELIGIBLE FOR EXAM"
  L1:
  ```

### Phase 7: Rule Evaluation & Results (`PHASE_RESULT`)
- Recursively evaluates condition trees against the student record.
- Prints execution trace showing individual condition values and rule triggers.
- Computes multi-category verdicts and produces the final banner decision.

---

## 10. Multi-Category Decision Policy

A major innovation of this C implementation is **Category Independence**:

Each category (`EXAM`, `PLACEMENT`, `SCHOLARSHIP`) is evaluated independently according to a deterministic four-state policy:

```
                  +--------------------------------+
                  | Evaluate Category Rules Fired  |
                  +---------------+----------------+
                                  |
         +------------------------+-----------------------+
         |                        |                       |
Positive Only Fired      Negative Only Fired     Both Positive & Negative
         |                        |                       |
         v                        v                       v
    [ ELIGIBLE ]           [ NOT ELIGIBLE ]          [ CONFLICT ]
                                                          |
                                      No Rules Fired ---> v
                                                    [ NO DECISION ]
```

### Deterministic Rules:
1. **`ELIGIBLE`**: Only positive rules fired for this category (e.g., `ELIGIBLE EXAM`).
2. **`NOT ELIGIBLE`**: Only negative rules fired for this category (e.g., `NOT ELIGIBLE EXAM`).
3. **`CONFLICT`**: Both positive and negative rules fired for the same category (e.g., student attendance qualifies, but backlog count fails a contradictory negative rule).
4. **`NO DECISION`**: No rules in the DSL addressed this category.

---

## 11. Command-Line Interface (CLI) Usage

The application includes a standalone CLI binary `campusrule_cli.exe` (and `campusrule.exe --cli`):

### Run All Built-In Example Programs:
```bash
make run-cli
# or directly:
./bin/campusrule_cli.exe --cli
```

### Compile a Specific Rule File with Default Student Data:
```bash
./bin/campusrule_cli.exe --cli examples/multiple_rules.cr
```

### Compile with Custom Student Data:
```bash
./bin/campusrule_cli.exe --cli examples/multiple_rules.cr --attendance 85 --cgpa 9.2 --marks 88 --backlogs 0
```

### Display Help and Options:
```bash
./bin/campusrule_cli.exe --help
```

---

## 12. Desktop Graphical Interface (GUI)

The GUI faithfully recreates and enhances the Java Swing interface demonstrated in the report (pages 29–36):

```
+----------------------------------------------------------------------------------------------------+
| Pipeline: [ Input ] [ Lexical ] [ Parsing ] [ AST ] [ Semantic ] [ TAC ] [ Result ]                |
+---------------------------------------------+------------------------------------------------------+
| Rule input (what the user types):           | [ Input ] [ Lexical ] [ Parsing ] [ AST ] [ ... ]    |
| [ 1. Simple exam rule                    v] |------------------------------------------------------|
| +-----------------------------------------+ |                                                      |
| | IF ATTENDANCE >= 75 AND BACKLOGS == 0   | | Rule 1: (ATTENDANCE >= 75.0 AND BACKLOGS == 0.0)     |
| | THEN                                    | |         -> TRUE                                    |
| | ELIGIBLE EXAM                           | |         => ELIGIBLE FOR EXAM                       |
| | END                                     | |                                                      |
| +-----------------------------------------+ | FINAL RESULT: ELIGIBLE FOR EXAM                      |
|                                             |                                                      |
| Student academic data                       | CATEGORY BREAKDOWN:                                  |
| ATTENDANCE: [ 78.0 ]    CGPA:  [ 8.2  ]     |   - EXAM:        ELIGIBLE                            |
| MARKS:      [ 82.0 ]    BACKS: [ 0.0  ]     |   - PLACEMENT:   NO DECISION                         |
|                                             |   - SCHOLARSHIP: NO DECISION                         |
| [       Compile & Run       ] [ Reset ]     |                                                      |
+---------------------------------------------+------------------------------------------------------+
|                     ELIGIBLE FOR EXAM  (GREEN BANNER)                                              |
+----------------------------------------------------------------------------------------------------+
```

### GUI Components:
1. **Pipeline Status Chips**: Green (`#2E7D32`) when OK, Red (`#C62828`) when Failed, Grey (`#9E9E9E`) when Skipped.
2. **Rule Editor**: Monospaced editable multiline text area.
3. **Sample Selector**: Dropdown to load pre-configured DSL rules instantly.
4. **Student Data Form**: Interactive inputs for Attendance, CGPA, Marks, and Backlogs.
5. **Phase Tabs**: 7 independent inspection tabs. On failure, the failing tab title turns into `Phase (error)` and is automatically selected!
6. **Verdict Banner**: Prominent colored banner showing the verdict:
   - **Green**: Eligible
   - **Amber**: Not eligible or Conflict
   - **Red**: Compilation error (lexical, syntax, or semantic)

### Launch the GUI:
```bash
make run
# or directly:
./bin/campusrule.exe
```

---

## 13. Build Process & Makefile Targets

The project builds cleanly with GNU Make and GCC.

| Command | Action |
|---|---|
| `make` / `make all` | Compiles GUI executable, CLI executable, and executes automated test suite |
| `make cli` | Compiles `bin/campusrule_cli.exe` |
| `make gui` | Compiles `bin/campusrule.exe` |
| `make test` | Compiles `bin/test_runner.exe` and runs all 143 unit test assertions |
| `make run` | Launches the graphical desktop application |
| `make run-cli` | Runs the CLI test suite against all built-in sample programs |
| `make clean` | Cleans `obj/` and `bin/` directories |
| `make debug` | Builds with debug symbols (`-g -O0 -DDEBUG`) |

---

## 14. Automated Test Suite & Verification Results

The test suite consists of 143 granular assertions across 6 test suites:

```
====================================================
  CampusRule Compiler - Automated Test Suite (C11)  
====================================================

Running Lexer Tests...
  Lexer Tests: 43/43 passed

Running Parser Tests...
  Parser Tests: 24/24 passed

Running AST Tests...
  AST Tests: 17/17 passed

Running Semantic Tests...
  Semantic Tests: 23/23 passed

Running TAC Tests...
  TAC Tests: 18/18 passed

Running Evaluator Tests...
  Evaluator Tests: 18/18 passed

====================================================
  ALL TEST SUITES PASSED SUCCESSFULLY! (0 failures)
====================================================
```

### Test Coverage Highlights:
- **Lexer**: Validates all keywords, operators, delimiters, numbers, identifiers, case-insensitivity, line/col tracking, and error detection on `&`, single `=`, etc.
- **Parser**: Single rules, compound rules, AND/OR precedence, parentheses grouping, missing `THEN`, missing `END`, empty input.
- **AST**: Dynamic allocation, hierarchical child links, condition text serialization, indented tree rendering, and recursive destruction.
- **Semantic**: Valid range checking, unknown variables, fractional backlog detection, duplicate rules, and student input boundaries.
- **TAC**: Temporaries allocation (`t1`, `t2`), label counters (`L1`, `L2`), conditional jumps (`IF_FALSE`), and PRINT instructions.
- **Evaluator**: All 6 relational operators, short-circuit boolean logic, positive vs negative triggers, multi-category independence, and conflict resolution.

---

## 15. Example Programs Catalog

The `examples/` directory contains complete sample programs illustrating every language feature:

1. [`exam_rule.cr`](examples/exam_rule.cr): Simple examination eligibility rule.
2. [`compound_and.cr`](examples/compound_and.cr): Compound condition combining attendance and backlogs.
3. [`placement_rule.cr`](examples/placement_rule.cr): Multi-criteria placement rule (CGPA, marks, backlogs).
4. [`scholarship_rule.cr`](examples/scholarship_rule.cr): High academic achievement scholarship rule.
5. [`multiple_rules.cr`](examples/multiple_rules.cr): Program containing Exam, Placement, Scholarship, and Negative rules.
6. [`negative_rule.cr`](examples/negative_rule.cr): Negative action `NOT ELIGIBLE EXAM`.
7. [`parentheses_rule.cr`](examples/parentheses_rule.cr): Grouped logic with parentheses.
8. [`conflict_rule.cr`](examples/conflict_rule.cr): Contradictory rules demonstrating conflict resolution.
9. [`semantic_error.cr`](examples/semantic_error.cr): Out-of-bounds attendance and CGPA triggering semantic rejection.
10. [`syntax_error.cr`](examples/syntax_error.cr): Missing `THEN` keyword triggering syntax rejection.
11. [`lexical_error.cr`](examples/lexical_error.cr): Invalid character `&` triggering lexical rejection.
12. [`unknown_var.cr`](examples/unknown_var.cr): Unknown variable name `CREDITS`.
13. [`fractional_backlog.cr`](examples/fractional_backlog.cr): Decimal backlog threshold `1.5`.
14. [`no_match.cr`](examples/no_match.cr): Unmatched criteria demonstrating `NO DECISION`.

---

## 16. Memory Management Strategy & Ownership Contracts

Because this project is written in standard C, memory management is strictly enforced:

1. **Explicit Allocation & Ownership**:
   - `TokenList`: Allocated by `lexer_tokenize()`, owned by caller, freed by `token_list_free()`.
   - `ASTNode`: Allocated via node constructors (`ast_create_compare`, etc.), owned by parent AST or caller, recursively freed by `ast_free()`.
   - `SemanticResult`: Buffers freed by `semantic_result_free()`.
   - `TACProgram`: Intermediate representation freed by `tac_program_free()`.
   - `EvaluationResult`: Fired action strings and trace buffers freed by `evaluation_result_free()`.
   - `CompilerOutput`: Central result structure freed by `compiler_free_output()`.
2. **Defensive Reallocation**:
   - Dynamic arrays (`TokenList`, `ASTProgram`, `StringBuilder`, `TACBuilder`) use geometrical resizing ($2\times$) with realloc failure checks.
3. **No Dangling Pointers**:
   - All free helpers nullify pointers after release.
4. **Leak-Free Compilation**:
   - Each compilation run releases all intermediate data structures, retaining only the text buffers required for output display.

---

## 17. Differences & Improvements Over Original Java Implementation

| Feature | Original Java Implementation | New C11 Implementation |
|---|---|---|
| **Language** | Java (JVM dependent) | Pure C11 (ISO C standard, native binary) |
| **Scholarship Category** | Mentioned in report objectives, but missing in Java `Parser.java` | **Fully implemented**: parsed, checked semantically, emitted in TAC, and evaluated |
| **Parenthesized Logic** | Not supported in Java parser | **Fully supported** with recursive descent precedence parsing |
| **Category Decisions** | Single combined string verdict | **Independent category breakdown** (`EXAM`, `PLACEMENT`, `SCHOLARSHIP`) |
| **Conflict Resolution** | Undefined (silent overwrite) | **Explicit `CONFLICT` status** when opposing rules fire |
| **Desktop GUI** | Java Swing | **Native Win32 desktop application** + **GTK 3 support** |
| **CLI Support** | Fixed sample iteration | **Full CLI** supporting custom rule files and CLI student parameters |
| **Diagnostics** | Basic exception strings | **Structured diagnostics** with line numbers, column offsets, and token indices |
| **Memory Footprint** | ~60MB JVM heap | **< 4MB native RAM** |

---

## 18. References

1. Aho, A. V., Lam, M. S., Sethi, R., & Ullman, J. D. (2006). *Compilers: Principles, Techniques, and Tools* (2nd ed.). Addison-Wesley.
2. ISO/IEC 9899:2011. *Information technology — Programming languages — C*.
3. GNOME Foundation. *GTK+ 3 Reference Manual*. https://docs.gtk.org/gtk3/
4. Microsoft Corporation. *Windows API Reference: Desktop Application Development*. https://learn.microsoft.com/en-us/windows/win32/
5. Department of Information Technology, VNRVJIET (2026). *Campus Rule Compiler — A Course Based Project Report*.
