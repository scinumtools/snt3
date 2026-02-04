#!/bin/bash

set -e

# load settings and export variables
set -a          
source settings.env 
set +a

DIR_ROOT=$(pwd)

NUM_SYSTEM_CORES=$(getconf _NPROCESSORS_ONLN)
NUM_MAKE_CORES=$NUM_SYSTEM_CORES
CMAKE_BUILD_TYPE=Release
ENABLE_CLANG_TIDY=OFF

CMAKE_FLAGS=(
  -DENABLE_SNT=ON
  -DENABLE_SNT_GTEST=ON
  -DENABLE_SNT_PYTEST=ON

  -DENABLE_EXS=ON
  -DENABLE_EXS_GTEST=ON
  -DENABLE_EXS_PYTEST=OFF

  -DENABLE_VAL=ON
  -DENABLE_VAL_GTEST=ON
  -DENABLE_VAL_PYTEST=OFF

  -DENABLE_PUQ=ON
  -DENABLE_PUQ_GTEST=ON
  -DENABLE_PUQ_PYTEST=ON
  -DENABLE_PUQ_MAGNITUDE=value

  -DENABLE_DIP=ON
  -DENABLE_DIP_GTEST=ON
  -DENABLE_DIP_PYTEST=ON

  -DENABLE_MAT=OFF
  -DENABLE_MAT_GTEST=ON
  -DENABLE_MAT_PYTEST=OFF
)

OS="$(uname -s)"

function clean_code {
    if [[ -d $DIR_BUILD ]]; then
	    rm -r $DIR_BUILD
    fi
}

function build_code {    
    if [[ ! -d $DIR_BUILD ]]; then
	    mkdir $DIR_BUILD
    fi
    if [[ $ENABLE_CLANG_TIDY == "ON" ]]; then
	clang_tidy_flag="clang-tidy;-warnings-as-errors=*"
	CMAKE_FLAGS+=(
	    -DENABLE_CLANG_TIDY="${ENABLE_CLANG_TIDY}"
	    -DCMAKE_EXPORT_ENABLE_COMMANDS=ON
	    -DCMAKE_CXX_CLANG_TIDY="${clang_tidy_flag}"
	)
    fi
    if [[ $CMAKE_BUILD_TYPE == Debug ]]; then
	CMAKE_FLAGS+=(
	    -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
	)
    fi
    cmake -B $DIR_BUILD "${CMAKE_FLAGS[@]}"
    cd $DIR_BUILD
    make -j $NUM_MAKE_CORES
    cd $DIR_ROOT
}

function install_code {
    cd $DIR_BUILD
    make install
    cd $DIR_ROOT
}

function uninstall_code {
    cd $DIR_BUILD
    sudo xargs rm < install_manifest.txt
    cd $DIR_ROOT    
}

function test_code {
    EXEC_FLAGS=""
    if [[ $CMAKE_BUILD_TYPE == Debug ]]; then
	if [[ "$OS" == "Darwin" ]]; then
	    EXEC_PROGRAM="lldb -o run --"
	elif [[ "$OS" == "Linux" ]]; then
	    EXEC_PROGRAM="gdb -ex run --args"
	fi
	EXEC_FLAGS="${EXEC_FLAGS} --gtest_break_on_failure --gtest_catch_exceptions=0"
    fi

    IFS='.' read -ra parts <<< "$1"
    if [[ "${parts[0]}" == "gtest" ]]; then     
	cd $DIR_BUILD
    	if [[ "${parts[2]}" != "" && "${parts[3]}" != "" ]]; then       # eg. gtest.exs.Expression.Initialization
	    $EXEC_PROGRAM ./bin/gtest-${parts[1]} $EXEC_FLAGS --gtest_filter="${parts[2]}.${parts[3]}"
    	elif [[ "${parts[2]}" != "" ]]; then                            # eg. gtest.exs.Expression
	    $EXEC_PROGRAM ./bin/gtest-${parts[1]} $EXEC_FLAGS --gtest_filter="${parts[2]}.*"
	else                                 # eg. gtest.exs
	    $EXEC_PROGRAM ./bin/gtest-${parts[1]} $EXEC_FLAGS
    	fi
    elif [[ "${parts[0]}" == "pytest" ]]; then
	pytest pytest/${parts[1]}/${parts[2]}.py::test_${parts[3]}
    else
	cd $DIR_BUILD
	ctest --output-on-failure
    fi
}

function run_code {
    ./$DIR_BUILD/$1
}

function compile_docs {
    cd docs
    doxygen
    sphinx-build -b html source build/html
}

function grep_code {
    if [[ "${2}" != "" ]]; then
	grep -Inr --color "${2}" ./src ./gtest ./exec
    fi
}

function setup_clang_format {
    find src -name '*.cpp' -o -name '*.h' | xargs clang-format -i
    find exec -name '*.cpp' -o -name '*.h' | xargs clang-format -i
    find gtest -name '*.cpp' -o -name '*.h' | xargs clang-format -i
    find pybind -name '*.cpp' -o -name '*.h' | xargs clang-format -i
}

function refactor_code {
    grep -rl "${1}" ./ --exclude-dir=build --exclude-dir=.venv | xargs sed -i '' "s/${1}/${2}/g"
}

function show_help {
    echo "Scientific Numerical Tools (SNT, scinumtools)"
    echo ""
    echo "Options:"
    echo " -c|--clean              clean the build directory"
    echo " -b|--build              build code"
    echo " -i|--install            install dip"
    echo " -u|--uninstall          uninstall dip"
    echo " -r|--run <example>      run an example code"
    echo " -t|--test [<test>]      run all/specific tests"
    echo " -d|--docs               compile documentation"
    echo " -g|--grep <expr>        find expression in a code"
    echo " -h|--help               show this help"
    echo " --debug                 run in a debug mode"
    echo " --clang-tidy            run clang-tidy during compilation"
    echo " --clang-format          run clang-format"
    echo " --num-cores N           number of compiliation cores"
    echo " --refactor <from> <to>  replace <from> a string <to> another string"
    echo ""
    echo "Examples:"
    echo "./setup.sh -c -b               clean and build the code"
    echo "./setup.sh -t Output.Data      run 'Output.Data' test"
    echo "./setup.sh -g class            find 'class' in the code or test"
}

if [[ "${1}" == "" ]]; then
    show_help
fi
while [[ $# -gt 0 ]]; do
    case $1 in
	-c|--clean)
	    clean_code; shift;;
	-b|--build)
	    build_code; shift;;
	-i|--install)
	    install_code; shift;;
	-u|--uninstall)
	    uninstall_code; shift;;
	-r|--run)
	    run_code $2; shift; shift;;
	-t|--test)
	    test_code $2 $3;
	    set +e; shift; shift; shift; set -e;; # shift with no parameters causes exit 1
	-d|--docs)
	    compile_docs; shift;;
	-g|--grep)
	    grep_code $2; shift; shift;;
	-h|--help)
	    show_help; shift;;
	--debug)
	    CMAKE_BUILD_TYPE=Debug; shift;;
	--clang-tidy)
	    ENABLE_CLANG_TIDY=ON; shift;;
	--clang-format)
	    setup_clang_format; shift;;
	--num-cores)
	    NUM_SYSTEM_CORES=$2; shift; shift;;
	--refactor)
	    refactor_code $2 $3; shift; shift; shift;;
	-*|--*)
	    show_help; exit 1;;
	*)
	    show_help; shift;;
    esac
done
