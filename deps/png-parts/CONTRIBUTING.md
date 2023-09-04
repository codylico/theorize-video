# Contributing Guidelines

Thank you for considering contribution to this repository. This
document describes how to contribute to this repository.

Any source code provided to this project should be licensable under
this repository's license, which can be found at the
[LICENSE document](LICENSE.txt). Contributors should hold the
copyrights necessary to license their source code contributions to
this repository's license.

## Roadmap

The goals of this project can be found in the
[README document](README.md). This project tracks the project goals
through the GitHub project board, hosted at
[the project board](https://github.com/codylico/png-parts/projects).

The types of contributions accepted for this project include the
following:

- Bug fixes

- In-line documentation

- Test programs

- Improvements and enhancements to the project goals

- Proposals for new features

- Implementations of features based on current project goals

- Improvements to this guidelines document

## Issue Reports

This project uses the repository issue tracker provided by GitHub. The
issue tracker can be found
[here](https://github.com/codylico/png-parts/issues).
Issue reports should be clear and properly serve project contributors
to find, understand and fix bugs in the project. Good issue reports
include the following:

- An identification of the program or library where the issue
  manifested

- A description of the expected behavior from the code

- A description of the actual observed behavior from the code

- Steps to reproduce the observed behavior (if known)

- Information about the commit or version of the code showing the
  issue

- Operating system and environment information, with build
  configuration settings

Post the issue to the issue tracker using the template which can be
found at
[bug_report.md](.github/ISSUE_TEMPLATE/bug_report.md)
for bug reports, or
[feature_request.md](.github/ISSUE_TEMPLATE/feature_request.md)
for feature requests.

## Pull Requests
Contributions to this repository should follow these guidelines:

- Commit messages should have a single line at the top (maximum 50
  characters) that briefly describes the purpose of the commit. The
  second line of the message should be empty. The following lines
  should elaborate on the short description at the top.

- Separate functionalities and bug fixes should be in separate
  commits.

- Do not merge or push directly into the `master` branch. Use a pull
  request, with the template available at
  [PULL_REQUEST_TEMPLATE.md](PULL_REQUEST_TEMPLATE.md).

- Formatting changes of code already present should be in their own
  separate commit.

## Feature Suggestions

Feature requests should also be posted through the repository's issue
tracker mentioned earlier. Requests for features that can be
implemented across platforms are preferred. (This does not mean that
features requiring platform-specific code are to be avoided, but as a
cross-platform project, single platform features might not fit the
goals of this project, and may best be served in a separate project.)
A feature request for this repository ideally should mention the
following:

- Purpose of the feature

- How the feature relates to the current project goals

- Plan for implementation (i.e. which steps would lead to completing
  the feature)

## Coding Style

Below are some suggestions for formatting the source code. Consistency
with these suggestions will help the code base retain easier
readability, by minimizing the potential distraction of shifting
coding styles. Additional coding style suggestions are welcome.

- Any changes in the formatting of source code should be in separate
  commits from implementation of features or bug fixes.

- Avoid pushing commits with half-working code to the repository. If
  you push a commit where the code fails to compile or run, indicate
  within the commit message that "The build is broken."

- Assume only the C 89 standard for C code, unless you use
  preprocessors to detect otherwise.

- Assume only the C++ 2003 standard for C++ code, unless you use
  preprocessors or CMake configuration options to detect otherwise.

- Indent by spaces instead of tabs.

- Omit any line break between a block statement (i.e. `if`, `while`,
  `do`, `switch` ...) and its opening brace, unless the condition
  spans multiple lines.

- If a condition spans multiple lines, put the opening brace on a
  separate line by itself.

- In C code, only use multiline comments.

- Struct tag names should not end with `... _t`. The `_t` ending
  conflicts with POSIX.

- Use `lowercase_snake_case` for variable and function names.

- Use `UPPERCASE_SNAKE_CASE` for enumerations and preprocessor macros.

- Set compile-time configuration options through the CMake project
  file when possible.

- Place declarations with documentation for each public function in
  the header files.

- Functions private to a particular source file should be declared
  `static` at the top of that source file. However, if you find
  yourself copying the private function across to multiple source
  files, it probably should become a public "utility" function.

- Keep source code lines below 80 columns.

## Testing and Running

Compiling the project uses CMake at least version 3.
You can obtain CMake from the link below:

[https://cmake.org/download/](https://cmake.org/download/)

To use CMake with this project, first make a diretory to hold the build results.
Then run CMake in the directory with a path to the source code.
On UNIX, the commands would look like the following:
```
mkdir build
cd build
cmake ../png-parts/src
```

Running CMake should create a build project, which then can be processed using other
tools. Usually, the tool would be Makefile or a IDE project.
For Makefiles, use the following command to build the project:
```
make
```
For IDE projects, the IDE must be installed and ready to use. Open the project
within the IDE.

## Contributor List

Contributors who have provided their e-mail here with their name are
willing to accept questions regarding assistance with using this
project as well as contributions to this repository.

- Cody Licorish (`svgmovement+contributing <at> gmail <dot> com`)

