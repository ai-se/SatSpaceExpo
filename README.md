## SNAP for building very small test suites

This repo contains the code of SNAP, a tool for building very small tests suites.

To exeucte the code, please install the Z3 solvers in your machine. For details of Z3 installation, please see the lastest Microsoft document at https://github.com/Z3Prover/z3/wiki.

This repo also replies on the Boost libraries. For most systems, such as the MacOS, compiled binaries are avaialbel. Details on configuring the Boost can be found at https://www.boost.org.
After the installation, please update the g++ arguments -I/.../boost/ at `Makefile`.

Benchmark is large and not included in this repo. To run the benchmark in the paper, please download [this folder](https://github.com/RafaelTupynamba/quicksampler/tree/master/Benchmarks) and put that as the neighbour of README.md.

- To build the snap, run `make snap`.
- To clean up the binaries, run `make clean`.
- All benchmarks in the paper were assigned the id. IDs are the index of vector `benchmark_models` in the file `src/commons/utility/utility.h`.
- Kernel of snap can be run (`bin` folder is created after the compilation) by `bin/snap -i ID`