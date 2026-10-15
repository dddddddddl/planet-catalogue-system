# Source guide

The application remains a small C program rather than a multi-module framework.

- `mainmenu()`, `adminmenu()`, `researchermenu()` and `land()` handle navigation and login.
- `createacc()`, `loadAccounts()`, `match()` and `modifyAccount()` manage the local account list/file.
- `increasepl()`, `modifyPlanet()`, `deletePlanet()` and `searchPlanet()` implement catalogue operations.
- `viewStatistics()` counts records; `findcontribution()` lists records for an adding account.
- Shared storage helpers near the top of `planet_catalogue.c` parse rows, resize line storage and write count headers.
- `portable.h` provides standard-C file/string/time adapters and input helpers.

Build/run instructions and current limitations are in the [root README](../README.md). No external C libraries are required.
