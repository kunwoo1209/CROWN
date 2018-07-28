=======================================================================
Written by Hyunwoo Kim on 2017.07.15

llvm, clang version (<= 3.4) is required to execute make.
(at least, makefile works well on 3.4 version)

makefile doesn't work for the version 3.5 or higher.

Note that llvm path must be set according to your environment.

How to install llvm/clang and set the path are described below.

=======================================================================

llvm/clang Installation

0. Move to direcotry to install llvm/clang.
   Let's call this directory "<base-directory>"

   cd <base-directory>

1. Download source codes of both llvm and clang (<= 3.4 version) in
   "http://releases.llvm.org/download.html"

   For 3.4 version, you can get sources by following commands.
   clang-3.4: "wget http://releases.llvm.org/3.4/clang-3.4.src.tar.gz"
   llvm-3.4: "wget http://releases.llvm.org/3.4/llvm-3.4.src.tar.gz"

2. Uncompress downloaded files.

   For 3.4 version,
   tar -xvzf clang-3.4.src.tar.gz
   tar -xvzf llvm-3.4.src.tar.gz

3. Rename clang-3.4 to clang and move it to llvm-3.4/tools/

   After second step, 2 directories will be generated.
   (clang-3.4, llvm-3.4 for 3.4 version)

4. Run build in llvm directory.

    For 3.4 version, move into llvm-3.4 and build it.

    cd llvm-3.4
    ./configure
    make

    Setting path in makefile

    1. set the variable "LLVM_BASE_PATH" to <base-directory>
    2. set the variable "LLVM_SRC_DIRECTORY_NAME" and
       "LLVM_BUILD_DIRECTORY_NAME" to llvm directory (default "llvm-3.4")


To do list
1. real number format printed in new_coverage is different from original code.
   (done) when extracting expression, using getSourceText API
          insead of printPretty API to bring source code directly.

2. type casting printed in new_coverage needs to be removed.
