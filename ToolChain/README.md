# Clang SaturnOS-specific toolchain

The entire operating system is being built with a modified version of clang that includes a custom `x86_64-saturnos` target.

## Patching

Simply apply the provided patch to the `llvmorg-20.1.8` branch of [LLVM's source code](https://github.com/llvm/llvm-project).

## Building

There are three essential parts that need to be built.

### Clang, LLVM and LLDB

Navigate to a separate, empty directory, outside of LLVM's source tree, and run the following commands:

```sh
cmake \
-DCMAKE_BUILD_TYPE=Release \
-DLLVM_ENABLE_PROJECTS="clang;lld;lldb" \
-DLLVM_BUILD_TOOLS=ON \
-DLLVM_BUILD_UTILS=OFF \
-DLLVM_INCLUDE_TESTS=OFF \
-DLLVM_INCLUDE_EXAMPLES=OFF \
-DLLVM_INCLUDE_BENCHMARKS=OFF \
-DLLVM_DEFAULT_TARGET_TRIPLE=x86_64-saturnos \
path/to/llvm
```

Replacing `path/to/llvm` with the actual path to the `llvm` subdirectory of the patched source.

```sh
cmake --build .
```

Actually building the entire project is going to take **hours**, so feel free to add the `-j <job_count>` parameter,
where `<job_count>` is the number of parallel jobs to run. In my experience 8 should take the build time down to about half an hour,
although this can vary between CPUs.


```sh
sudo cmake -DCMAKE_INSTALL_PREFIX=prefix/to/choose -P cmake_install.cmake
```

Replacing `prefix/to/choose` with the directory where the built toolchain will reside.
This should be added to your path before the next steps.

### Compiler RT for the userspace

Navigate to another separate and empty directory, outside of LLVM's source tree, and run the following commands:

```sh
cmake \
-DCMAKE_BUILD_TYPE=Release \
-DCMAKE_C_FLAGS="-ffreestanding -nostdlib" \
-DCMAKE_CXX_FLAGS="-ffreestanding -nostdlib" \
-DCMAKE_ASM_COMPILER_TARGET="x86_64-saturnos" \
-DCMAKE_C_COMPILER_TARGET="x86_64-saturnos" \
-DCMAKE_C_COMPILER=clang \
-DCMAKE_CXX_COMPILER=clang++ \
-DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" \
-DCOMPILER_RT_BUILD_BUILTINS=ON \
-DCOMPILER_RT_BUILD_LIBFUZZER=OFF \
-DCOMPILER_RT_BUILD_MEMPROF=OFF \
-DCOMPILER_RT_BUILD_PROFILE=OFF \
-DCOMPILER_RT_BUILD_SANITIZERS=OFF \
-DCOMPILER_RT_BUILD_XRAY=OFF \
-DCOMPILER_RT_BUILD_ORC=OFF \
-DCOMPILER_RT_BUILD_CTX_PROFILE=OFF \
-DCOMPILER_RT_BUILD_CRT=OFF \
-DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON \
-DCOMPILER_RT_BAREMETAL_BUILD=ON \
-DCOMPILER_RT_CXX_LIBRARY=none \
-DCOMPILER_RT_USE_LIBCXX=OFF \
path/to/compiler-rt
```

Replacing `path/to/compiler-rt` with the actual path to the `compiler-rt` subdirectory of the patched source.

```sh
cmake --build .
```

If necessary, the number of jobs can also be provided here, as instructed earlier.

### Compiler RT for the kernel

Contrary to the userspace the kernel is built without the red zone, which is why its Compiler RT also needs to be built without it.
Navigate to another separate and empty directory, outside of LLVM's source tree, and run the following commands:

```sh
cmake \
-DCMAKE_BUILD_TYPE=Release \
-DCMAKE_C_FLAGS="-ffreestanding -nostdlib -mno-red-zone" \
-DCMAKE_CXX_FLAGS="-ffreestanding -nostdlib -mno-red-zone" \
-DCMAKE_ASM_COMPILER_TARGET="x86_64-saturnos" \
-DCMAKE_C_COMPILER_TARGET="x86_64-saturnos" \
-DCMAKE_C_COMPILER=clang \
-DCMAKE_CXX_COMPILER=clang++ \
-DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" \
-DCOMPILER_RT_BUILD_BUILTINS=ON \
-DCOMPILER_RT_BUILD_LIBFUZZER=OFF \
-DCOMPILER_RT_BUILD_MEMPROF=OFF \
-DCOMPILER_RT_BUILD_PROFILE=OFF \
-DCOMPILER_RT_BUILD_SANITIZERS=OFF \
-DCOMPILER_RT_BUILD_XRAY=OFF \
-DCOMPILER_RT_BUILD_ORC=OFF \
-DCOMPILER_RT_BUILD_CTX_PROFILE=OFF \
-DCOMPILER_RT_BUILD_CRT=OFF \
-DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON \
-DCOMPILER_RT_BAREMETAL_BUILD=ON \
-DCOMPILER_RT_CXX_LIBRARY=none \
-DCOMPILER_RT_USE_LIBCXX=OFF \
path/to/compiler-rt
```

Replacing `path/to/compiler-rt` with the actual path to the `compiler-rt` subdirectory of the patched source.

```sh
cmake --build .
```

If necessary, the number of jobs can also be provided here, as instructed earlier.
