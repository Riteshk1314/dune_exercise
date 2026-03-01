# DUNE Fine-Grained Storage Benchmark

Benchmark comparing three storage strategies for DUNE warm-up exercise. Tests writing/reading lots of small records.

Two implementations:
- **cppVersion/** - C++ implementation with full validation
- **pythonVersion/** - Python notebook version (quicker to iterate)

## C++ Quick Start

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
./dune_benchmark
```

On Windows use the VS generator or NMake. Linux/mac just need cmake and a compiler.

## Python Version

The Python version is in `pythonVersion/storage_benchmark.ipynb`. Just open the notebook and run the cells. Needs Python 3.8+ and matplotlib for the plots.

```bash
cd pythonVersion
pip install matplotlib
jupyter notebook storage_benchmark.ipynb
```

## Overview

Tests 3 storage approaches with 100k records (1-2KB each):
- **SingleFile** - all in one file + index
- **Chunked** - 1000 records per chunk
- **Individual** - one file per record (slow but simple)

## Features

- Data validation
- Timing & throughput metrics
- Fixed seed so results are reproducible
- Cross-platform


## How it works
- Generate 100k random records once (fixed seed for reproducibility).
- Run each strategy:
  - write all records
  - read everything sequentially
  - read 1000 records at random positions
  - verify every read matches what was written
  - report timings, throughput, disk usage, file counts
- Clean up the files for that strategy before moving to the next one.

## Requirements

C++ version:
- C++17 compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.15+
- ~200MB disk space

Python version:
- Python 3.8+


## Results

- Write/read times in seconds and MB/s
- Disk usage and file counts
- Verification status
