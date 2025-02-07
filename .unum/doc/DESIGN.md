## unum - design
This document describes the design for the `unum` systemic platform.  It is
supported by both the work in the manifesto and language documents and should
be considered a _reference implementation_ until otherwise noted.

# Overview
## Vocabulary
* **deployment**: the complete set of hardware and software allocated to an unum
systemic programming architecture

* **basis**: also, _architectural basis_, refers to the sum of shared compute 
resources assigned to an unum deployment.  The basis is first created when the
`unum` base repository is cloned and further maintained during development.

* **kernel**: the platform-specific executable called `unum` which implements
the toolchain and runtime for all systemic operations.

* **bootstrapping**: the process of cloning the `unum` baseline repository 
from VCS (eg. Git) and building its foundation with a local C++ compiler, which
analyzes the platform, captures references to the C++ and linker executables,
and builds the baseline kernel.


## Objectives
1. **Minimal dimensionality**

The platform must primarily support the simplified, holistic development of 
interconnected platforms.  This is accomplished by actively reducing the
cognitive costs of linkage in all forms during an applied development process.

2. **Unmatched performance**

The product of any development, especially in linkage, must far exceed those 
using existing conventional techniques.  The introduction of a systemic, 
distributed compiler must enable unexpected and shocking speed of execution.

3. **Version independence**

Developers must not be beholden to the whims of organizations who build their
platforms and tools to accept the costs of continued upgrades.

4. **Zero cycle time**

The time it takes to deploy a one-line change will deliver instantaneous 
results.

5. **No-friction understanding**

Communicate the highest ideals of systemic programming without introducing 
obstacles to understanding from dependencies, tools, coding style or 
documentation.


## On C++
The choice of C++ for unum is an intentional upgrade over a prior approach
using only C.  The limitations of C, particularly in the areas of modularity 
and encapsulation made it hard to justify its use for what could be a 
significant amount of code to support a massively systemic platform.  While it 
is still a great language and underpins countless projects, it doesn't feel like
a good choice for this scenario, especially when the goal is to move often
between a new language of `unum` and a native language for integration.  It is 
also the choice here to *not* use the standard C++ library at all in this repo, 
instead favoring a coding style and locally-implemented supporting types that 
mirrors the unum code that is transpiled into the C++.  Pursuing the objectives
above and the strategy below motivated a holistic, single mono-repo for as much
behavior as possible, recommending that the significant version differences of 
C++ as a language over the years not influence the architectural decisions here.


## Strategy

* All code for tooling, platform and product for a systemic architecture exists
in a single repository.  External dependencies are only permitted when (a) they
are universally-accepted common runtime components (eg. SQLite), (b) are 
uniquely trusted to implement critical industry-accepted algorithms 
(eg. OpenSSL) or (c) are the only means of integrating with non-unum systems
(eg. PostgreSQL).  This encourages version independence because except for
intentional or minimal  external integrations, everything is the choice of the 
developer who owns a repository.  Not even their toolchain is outside their
control.

* Linkage reliability is systemic, not isolated.  Developers don't encode or
accommodate the uncertainties of distributed communication into their 
software.  This actively destroys the greatest source of dimensionality.

* The systemic language is `unum` but the runtime language is specialized `C++`.
More specifically, `C++` is not used to interpret `unum`, but rather `unum` is
used to write highly-optimal `C++`.  The entire repository assumes C++, not 
machine language is the 'executable'.  This mindset ensures that nearly every 
element of a deployment may be understood and maintained by adopters using 
long-proven technologies.  Those who wish to work entirely in `unum` may do so,
but are not restricted from adapting or extending their deployment as desired.
This actively supports all objectives by standardizing on the fastest, most 
mature, general-purpose programming language available.

* Concepts from programming to tooling to system management adopt a philosophy 
of hierarchical precision.  It is intended that common tasks are very easy and
fully automatic, while more demanding ones may be accomplished by increasing 
the precision of their definition with increasingly descriptive syntax.


## Tactics

* Systemic `unum` products _clone_ a repository and do not _initialize_ it in
a `git`, `npm`, `pip`, etc. manner.  _Cloning is initialization_.

* An unum repository is bootstrapped with a simple makefile and _ingests_ the
C++ compiler environment exposed by that makefile to use for future deployments.
Makefiles need not be maintained when the developer is not implementing kernel
enhancements (language, basis, analytics, etc).

* The implementation of standards will be performed 'as-needed' with an 
understanding that the implemention is a subset, not an official fully-compliant
form as a rule.  The eventual full realization of compliance in any one area
will be an eventual goal, not an immediate priority at the time of first
introduction.


# Specifics

## Manifest

* YAML format, but just a subset to support deployment.

* named .unum/config/manifest.umy (unum manifest YAML format)

* Organized as an _ordered_ 'mapping' of file categories to file lists.  All 
filenames are expressed in directory notation using POSIX format _relative_ to 
top of the repo.

* File categories are expressed as a textual word with the categories 'core',
'kernel', and 'build' reserved. 

* File mapping entries are organized in the file in order of build priority
with the most fundamental dependencies first in the file and the highest level
abstractions last.  This naturally means the file with the main() function is
necessarily always has the maximum line number of any source file.

* The `core` category describes files that are sufficient to perform a full 
deployment of `unum` from source code in both the basis and the primary 
repository hierarchy (unum source).  During bootstrapping the core is built in
isolation with limited features, which then rebuilds the entire kernel.

* The 'kernel' category describes all source, both native foundational files 
under the basis, and .un/.cc source code in the deployment hierarchy that
fully defines the unum deployment.  It is assumed to include all of the source
also in the `core` category.

* The 'build' category describes custom build rules and behavior for the basis
and the configured compiler.  It supports a single sub-category of 'include' 
that defines a list of C++ include diretories to use for compilation.

## Bootstrapping

When the unum repository is first cloned or wishes to perform a clean rebuild,
it first performs bootstrapping before the unum kernel then takes over
responsibility for deployment activities.  The bootstrapping process aims for 
immediate self-sufficiency of the toolchain from a single primitive - the C++ 
compiler suite.  Much of the needless complexity of modern development is 
derived from the vast graph of inter-dependencies of modern tools and 
third-party libraries.  The principle of 'one' in unum addresses this by 
embedding as much of what is needed for complete development in this single 
repository's codebase.  Unum purposely advances the 'batteries-included' 
philosophy to ensure its developers maximize their autonomy.

	NOTE:
	In the best interest of unum consumers, there are two exceptions to
	this embedding strategy:

	1.  The C++ compiler and related tools.  The compiler is the bridge into
	the local platform ecosystem and is expected to offer the best possible
	results for general workloads.  Additionally, the choice of C++ for unum for
	both internal and transpilation of its language is very intentional to 
	ensure developers are not unknowingly committed to custom third-party 
	compilers or toolchains at some later date.

	2.  Highly-specialized third-parties.  Most notably with security 
	implementations where mistakes could have serious consequences, but 
	additionally with standardized complex processing often found in databases
	(ie. SQLite, PostgreSQL, etc), a third-party will be integrated over
	building customized implementations.  This will be a complex gray area that
	requires significant rigor to minimize these dependencies.  In this case, 
	the balance must be drawn between correctness/safety and developer
	independencies.

Bootstrapping involves three phases of ingestion, pre-kernel and deployment.

Ingestion initiates the process through conventional native tooling (typically
`make`) to detect the local compiler and encode it in to unum for deployment.

Pre-kernel is the action of a simplified program (uboot) to compile only the 
most fundamental parts of the `unum` kernel used for deploying respository 
code to then rebuild the full kernel.

Deployment is the rebuilding of the unum kernel with all source in the 
repository for runtime access.

The bootstrapping sequence follows this pseudo-code, assuming a recently
cloned repository:
	* run `make` in the root
	  * `make` creates `uboot`
	  * `make` executes `uboot` passing the current compiler tools to it
	    * `uboot` identifies the system type and creates a shared header
	    * `uboot` compiles `unum` as a pre-kernel, with only the code necessary
	      for deployment using the manifest in the config directory of of the 
		  basis directory `./.unum`
	  * `make` executes the `unum` pre-kernel to deploy itself
	    * `unum` pre-kernel rebuilds with all source in the manifest and 
		  replaces its own binary
		* `unum` pre-kernel re-executes the kernel again, requesting deployment
		  verification, completing the process.

It is allowable to re-run the top-level `make` file after bootstrapping, which
will implicitly re-deploy using the most recent kernel unless the configuration
or boot program has been modified.

