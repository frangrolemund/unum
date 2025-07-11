## Project Status
This work is in early pre-production.

- Its justification is mostly in place with the MANIFESTO and a concept for the
DESIGN of transpiling to C++ is reasonbly solid.

- Tree organization, source structure is getting clearer.

- The bootstrapping is functional for creating the barebones unum kernel.


## Overview
The `unum` repository is an implementation of a purely systemic toolchain as
outlined in its [manifesto](./.unum/doc/MANIFESTO.md) and described in its [design](./.unum/doc/DESIGN.md).  

It is intended to be cloned, modified and maintained by its [licensees](./.unum/LICENSES).


## Supported Platforms

- macOS


## Prerequisites

- GNU make
- C++ compiler (specifically [Xcode](https://developer.apple.com/xcode/) 
w/ command-line tools on macOS)


## Bootstrapping

This toolchain ingests the local C++ references and takes over all building of
the included C++ source in this tree through a process of 'bootstrapping' that
is performed at least once.  To bootstrap, in the root of the the repository
ensure that you have access to your toolchain and run `make` from a terminal
prompt to build the initial version of the `unum` service.

Bootstrapping must precede general development workflow, including the use of
IDEs.


## Development
General development is performed by maintaining source code for your customized
deployment under this root directory, or upgrading the unum 'kernel' under the
basis sub-directory `./.unum`.  All deployment activities are performed by 
invoking the binary found at `./.unum/deployed/bin/unum`.
