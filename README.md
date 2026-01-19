# valex
valex - value expression
## PLAN
all scopes are expressions,so are if conditions etc.

## Getting Started

Clone the repository:

```sh
git clone https://github.com/JustAChillGuy14/valex
cd repo
```

Build & Run:

```sh
make
./main
```

## Error Handling

This interpreter currently uses a **fail-fast error model**.

On encountering a syntax, runtime, or internal error, the program prints a diagnostic message to `stderr` and exits immediately. No error recovery is attempted.

This is an intentional design choice at the current stage of development, allowing the implementation to remain simple and correct while core language features are being built.

Future versions may introduce structured error types, source-location reporting, and non-fatal diagnostics.
