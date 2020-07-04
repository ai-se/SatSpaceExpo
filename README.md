# SNAP for building very small test suites

This repo contains the source of SNAP, a tool for building very small test suites.

## Installation
### Step 1. Download and set up Z3 solvers
We used [Z3 v4.8.4](https://github.com/Z3Prover/z3/releases/tag/z3-4.8.4) during the experiment, but newer version could be probably faster.
```
wget https://github.com/Z3Prover/z3/archive/z3-4.8.4.tar.gz
tar -xzf z3-4.8.4.tar.gz
# follow steps in README.md inside the z3-4.8.4 like following...
python scripts/mk_make.py
cd build
make
sudo make install
```
### Step 2. Download Boost C++ libraries if applicable
In most machines, Boost C++ libraries have already been instead. In case they're not instead, you can follow these steps:
```
wget https://dl.bintray.com/boostorg/release/1.73.0/source/boost_1_73_0.tar.gz
tar -xzf boost_1_73_0.tar.gz
# You don't need to compile the whole libraries of boost actually since it's very huge. Continue on Step 3...
```

### Step 3. Download SNAP as well as the benchmarks
```
git clone https://github.com/ai-se/SatSpaceExpo.git
git clone https://github.com/RafaelTupynamba/quicksampler.git # for the benchamarks
cp -rf quicksampler/Benchmarks SatSpaceExpo
boost_path="/path/to/boost/include"
sed -i "1s/.*/CC=g++ -I$boost_path/" SatSpaceExpo/Makefile
``` 

### Step 4. Understanding the Benchmark ID
In the SNAP, for convenient, we use the ID to represent the benchmarks,  you can also replace or add more test cases by editing the `src/commons/utility/utility.h`.
Be default, the id shows as 
```
static std::vector<std::string> benchmark_models{
    "Benchmarks/Blasted_Real/blasted_case47.cnf", // 0
    "Benchmarks/Blasted_Real/blasted_case110.cnf", // 1
    "Benchmarks/V7/s820a_7_4.cnf", // 2
    "Benchmarks/V15/s820a_15_7.cnf", // 3
    "Benchmarks/V3/s1238a_3_2.cnf", // 4
    "Benchmarks/V3/s1196a_3_2.cnf", // 5
    "Benchmarks/V15/s832a_15_7.cnf", // 6
    "Benchmarks/Blasted_Real/blasted_case_1_b12_2.cnf", // 7
    "Benchmarks/Blasted_Real/blasted_squaring16.cnf", // 8
    "Benchmarks/Blasted_Real/blasted_squaring7.cnf", // 9
    "Benchmarks/70.sk_3_40.cnf", // 10
    "Benchmarks/ProcessBean.sk_8_64.cnf", // 11
    "Benchmarks/56.sk_6_38.cnf", // 12
    "Benchmarks/35.sk_3_52.cnf", // 13
    "Benchmarks/80.sk_2_48.cnf", // 14
    "Benchmarks/7.sk_4_50.cnf", // 15
    "Benchmarks/doublyLinkedList.sk_8_37.cnf", // 16
    "Benchmarks/19.sk_3_48.cnf", // 17
    "Benchmarks/29.sk_3_45.cnf", // 18
    "Benchmarks/isolateRightmost.sk_7_481.cnf", //19
    "Benchmarks/17.sk_3_45.cnf", // 20
    "Benchmarks/81.sk_5_51.cnf", // 21
    "Benchmarks/LoginService2.sk_23_36.cnf", // 22
    "Benchmarks/sort.sk_8_52.cnf", // 23
    "Benchmarks/parity.sk_11_11.cnf", // 24
    "Benchmarks/77.sk_3_44.cnf", // 25
    "Benchmarks/20.sk_1_51.cnf", // 26
    "Benchmarks/enqueueSeqSK.sk_10_42.cnf", // 27
    "Benchmarks/karatsuba.sk_7_41.cnf", // 28
    "Benchmarks/tutorial3.sk_4_31.cnf" // 29
};
```

### Step 5. Compile SNAP and execution
```
cd /path/to/SatSpaceExpo
make snap
/path/to/SatSpaceExpo/bin/snap -i ID@Step4

# To clean up all compiled binaries
cd /path/to/SatSpaceExpo
make clean
```

## Other referencing
- Boost installation [https://www.boost.org/users/history/version_1_73_0.html](https://www.boost.org/users/history/version_1_73_0.html)
- Z3 introduction [https://github.com/Z3Prover/z3](https://github.com/Z3Prover/z3)

## Citation
*The paper has been submitted and under reviewed in IEEE Transactions on Software Engineering*
```
@article{chen2019building,
  title={Building Very Small Test Suites (with Snap)},
  author={Chen, Jianfeng and Shen, Xipeng and Menzies, Tim},
  journal={arXiv preprint arXiv:1905.05358},
  year={2019}
}
```
