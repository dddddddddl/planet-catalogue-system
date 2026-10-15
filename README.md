# Planet Catalogue System

A C command-line application for maintaining planet records, researcher accounts and catalogue statistics, originally developed as a team coursework project for **CPT109**. It uses structures, linked lists, dynamic memory and text-file persistence.

This repository is a prepared edition of the supplied coursework source and group report. Documentation, build files, automated tests and the fictional demonstration were added during repository preparation, along with the repairs listed in the [material provenance and preparation notes](docs/design-notes.md#material-provenance). These additions are separate from the original coursework submission.

## What it does

| Area | Implemented behavior |
|---|---|
| Planet records | Add, edit and delete entries with physical attributes and discovered/observed flags |
| Search | Query by name, diameter, mass, rotation period, density, age or distance from a star |
| Accounts | Register an administrator or researcher, log in and update an account password |
| Statistics | Count catalogued, discovered and observed planets, plus registered researchers |
| Catalogue contributions | List entries attributed to a supplied account name |

The two login menus expose different operations. This is an educational local application: role selection is available during registration, passwords are stored as plain text, and password changes are not restricted to the current account. Use fictional accounts only.

## Build and run

A C17 compiler and Make are sufficient; there are no third-party C dependencies.

```bash
make
./build/planet_catalogue
```

Or compile directly:

```bash
cc -std=c17 -Wall -Wextra -Wpedantic src/planet_catalogue.c -o planet_catalogue
./planet_catalogue
```

On first run, the program creates `land.txt`, `Researchers.txt` and `planetcat.txt` in the **current working directory**. No initial dataset or default account is required. Create an account using main-menu option **2**, then choose **1** for an administrator or **2** for a researcher. After an operation, enter `Y` to return to the main menu or `N` to exit.

## Try the demonstration

[examples/demo-input.txt](examples/demo-input.txt) creates two fictional accounts, adds `Demo-A`, searches for it, displays statistics and queries its recorded contributor. The values are synthetic software examples, not astronomical observations.

No original runtime dataset was supplied. The demonstration and test fixtures were written for this repository; their account names, planet values and fixed test timestamps are not historical project records.

From the repository root, run the script in a fresh temporary directory:

```bash
project_dir="$PWD"
demo_dir="$(mktemp -d)"
(cd "$demo_dir" && "$project_dir/build/planet_catalogue" < "$project_dir/examples/demo-input.txt")
```

The replay prints a planet record and statistics including:

```text
Name: Demo-A
TOTAL NUMBER OF CATALOGUED PLANETS: 1
TOTAL NUMBER OF DISCOVERED PLANETS: 1
TOTAL NUMBER OF OBSERVED PLANETS: 0
TOTAL NUMBER OF REGISTERED RESEARCHERS: 1
```

## Read the code

| File | Purpose |
|---|---|
| [src/planet_catalogue.c](src/planet_catalogue.c) | Original menu-driven application, with focused portability and persistence repairs |
| [src/portable.h](src/portable.h) | Standard-C adapters and interactive input helpers |
| [docs/design-notes.md](docs/design-notes.md) | Data structures, storage format, preparation changes and limitations |
| [tests/test_catalogue.py](tests/test_catalogue.py) | Isolated tests of the compiled interactive program |

Start with `mainmenu()`, the administrator/researcher menus and the `planet`/`account` structures. The catalogue operations use file records; linked lists hold accounts and the contribution-view cache. This is file-based storage, not a database-server application.

## Verification

```bash
make test
```

The **8 automated test cases** cover registration/login, duplicate accounts after restart, all seven search criteria, multiword planet names, add/edit/delete persistence, refreshed contribution results, password rewrites, cancelled operations, invalid numeric input/EOF and the 100-record boundary. They run in temporary directories and require Python 3 only as a test driver.

The C17 build passed on Apple Silicon macOS with Apple Clang 21.0.0, including `-Wall -Wextra -Wpedantic -Werror`. The same tests passed with AddressSanitizer and UndefinedBehaviorSanitizer enabled. This is newly executed repository validation, separate from the coursework report's historical test screenshots.

## Scope

The course report was used to summarize the project and is **not included**. Documentation describes the prepared implementation: it does not claim planet-type classification, account deletion, a working history log or astronomy-grade numerical accuracy. Numeric fields retain the original `float` representation and two-decimal file formatting; numeric searches use exact equality.

The original coursework's menu and storage design is retained. Repository preparation added build instructions, portable function adapters, focused persistence/input fixes and tests; details are in [design notes](docs/design-notes.md).
