# Project Guidelines

## Language

- Use English for all project communication, documentation, comments, and user-facing text.

## Architecture and important directories

- `src/fpot.c` contains the command-line program and fast-decoupled AC power-flow calculations. `src/inpanrd.c` parses input cases, and `src/fpot.h` defines shared data structures and declarations.
- `examples/` contains sample `.dad` power-system cases. `docs/manual.md` describes the input format implemented by the parser; keep it in sync with parser changes.
- The project uses GNU Autotools: `configure.ac` and the `Makefile.am` files are sources; `configure`, `Makefile.in`, and configured Makefiles are generated build files.

## Build and test

- Configure and build with `./configure && make`. If generated Autotools files need to be refreshed from their sources, run `./bootstrap.sh` first.
- Run `make check` as the standard check target. There are currently no automated test cases configured, so also smoke-test solver changes with a representative sample, for example `./src/fpot examples/sis9.dad 0` (`phi` is in degrees).
- Do not claim behavior is tested solely because `make check` succeeds; report when no automated tests cover a change.

## Coding conventions

- Implement in C and follow the existing style in `src/`: two-space indentation, snake_case identifiers, and declarations in `src/fpot.h` when shared across source files.
- Preserve the fixed-width `.dad` record format and existing parser behavior unless the change explicitly intends to alter it. Consult `docs/manual.md` before changing parsing or supported input records.

## Completing a change

- Make the smallest coherent change and preserve existing command-line and sample-case behavior unless requirements call for a change.
- For changes to input parsing or supported records, update `docs/manual.md` and exercise a relevant sample or focused parser case.
- Run the relevant build and checks above, and summarize what was changed, what was tested, and any remaining test limitations.
