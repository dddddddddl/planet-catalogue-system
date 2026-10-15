# Design and implementation notes

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

## Remaining boundaries

This is a single-user coursework prototype. The administrator/researcher menus are not a production authorization boundary: registration permits choosing the administrator role, credentials are plain text, account-password editing is not identity-restricted, and the adding account is entered manually.

Float precision, two-decimal storage and exact numeric comparisons limit scientific use. Input and file parsing are not a complete validation layer; malformed rows may be ignored or omitted during rewrites. File rewrites are not transactional and there is no concurrent-access control or recovery journal. The application has no network service, graphical interface, validated astronomy dataset or implemented account-deletion/history subsystem.
