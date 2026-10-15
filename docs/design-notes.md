# Design and implementation notes

## Material provenance

The source material consists of one C source file and the **CPT109 Group Report: Planet Catalogue System**. The report identifies a team project. This repository makes no claim about which team member implemented a particular function.

| Repository material | Origin |
|---|---|
| `src/planet_catalogue.c` | Adapted from the supplied C source. The data model, menus and core operations are inherited; portability, input and persistence repairs are listed below. |
| `src/portable.h` | Newly written adapters and input helpers for repository preparation. |
| `README.md`, `src/README.md`, this document | Newly written explanations checked against the supplied source and report, with preparation changes identified separately. They are not the original report. |
| `Makefile`, `.gitignore`, `.gitattributes` | Newly added build and repository configuration. |
| `examples/demo-input.txt`, `tests/test_catalogue.py` | Newly written demonstration and regression checks using fictional accounts and synthetic planet records. Fixed fixture timestamps are arbitrary test values. |

The supplied source already contains `increasepl()`, `modifyPlanet()` and `deletePlanet()` for record management, `searchPlanet()` for the seven search criteria, the registration/login/password functions, `viewStatistics()` and `findcontribution()`. The report discusses these operations in Sections 2.3, 3.3–3.5 and 5. The fixes and passing automated checks below describe the prepared repository version, not the condition of the original submission. No original runtime data files were supplied.

## Data model and operations

`struct planet` stores a name, diameter, mass text, rotation period, density, age, distance from a star, modification time, discovered/observed flags and an adding account. `struct account` stores credentials, an administrator flag and a linked-list pointer. A linked-list cache supports the contribution view; most catalogue operations scan the persisted file.

Adding a record is linear in the existing file size. Search, statistics and contribution queries also scan records or their loaded list. Modification and deletion load file lines into memory and rewrite the retained records with recalculated totals. No timing benchmarks were performed.

The source declares a `history` structure but does not implement an edit-history feature. Likewise, the report's broader requirements mention planet types and account deletion, but those operations are absent from the supplied code and are not presented as features here.

## Local files

All paths are relative to the process working directory. Runtime data is excluded from Git.

| File | Format |
|---|---|
| `land.txt` | One `account password isAdmin` record per line; role 1 is administrator, 0 is researcher |
| `Researchers.txt` | `REGISTERED RESEARCHERS: N`, counting researcher registrations rather than administrators |
| `planetcat.txt` | Three count headers followed by tab-separated planet rows |

Planet columns, in order:

```text
name  diameter  mass  rotation_period  density  age  star_distance
last_modified  discovered  observed  added_by
```

Rows use tabs, not the spaces shown above. Timestamps use `YYYY-MM-DD HH:MM:SS`. The supplied conventions label diameter and star distance in AU, period in hours, density in kg/m^3 and age in years. They are retained as application field labels, not verified scientific data. Mass remains text; numeric mass searches should use standard notation such as `2e6`, not an expression such as `2*10^6`.

## Focused preparation changes

The supplied `.c.txt` was renamed to `src/planet_catalogue.c`. The original menu structure, field model, operation set and linked-list/text-file approach remain recognizable. The original report and its screenshots, student identifiers and account examples are not redistributed.

Repairs made for a reproducible repository:

- Added standard-C equivalents for Microsoft-specific file, string and time calls, explicit function prototypes and correct call arguments.
- Initialized missing runtime files without seeding real accounts or observations.
- Replaced fixed 100-line editing buffers with dynamically growing line storage, bounded record fields and canonical count headers. Counts are calculated from retained records, including when the deleted row is not the final row.
- Enforced the declared 100-planet insertion limit, refreshed the contribution cache after catalogue changes and reloaded accounts before duplicate-name checks.
- Truncated account-file rewrites to prevent stale trailing records after shorter password changes. Corrected resource cleanup and a failed-allocation path.
- Made input-buffer clearing EOF-aware, retried invalid numeric conversions and rejected overlong/tab-containing line input. Menu returns no longer recursively reopen the main menu. Invalid edit choices leave the file unchanged.
- Added a Makefile, a fictional replay and tests. These are preparation work, not claims about what was submitted for assessment.

## Verification record

On October 15, 2026, Apple Clang 21.0.0 compiled the distributed source as C17 with warnings treated as errors. Eight end-to-end test cases passed against the normal build and an AddressSanitizer/UndefinedBehaviorSanitizer build. The example input also completed in a fresh temporary directory. Other operating systems and compilers were not executed during this preparation.

During the material audit on October 16, 2026, both eight-case suites and the demonstration were rerun successfully with the same compiler. The demonstration output matched the lines quoted in the README. Passing these checks does not establish scientific accuracy or complete input validation.

## Remaining boundaries

This is a single-user coursework prototype. The administrator/researcher menus are not a production authorization boundary: registration permits choosing the administrator role, credentials are plain text, account-password editing is not identity-restricted, and the adding account is entered manually.

Float precision, two-decimal storage and exact numeric comparisons limit scientific use. Input and file parsing are not a complete validation layer; malformed rows may be ignored or omitted during rewrites. File rewrites are not transactional and there is no concurrent-access control or recovery journal. The application has no network service, graphical interface, validated astronomy dataset or implemented account-deletion/history subsystem.
