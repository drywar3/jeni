
MODE="build"
TEST_DIR="./tests"
EXE_ARGS=""

debug () {
    printf "note: debugging with 'gf2'\n"
    gf2 --args ./build/jeni $EXE_ARGS
    printf "done:"
}

build () {
    printf "note: refreshing cache\n"
    cmake -B ./build  1> /dev/null
    printf "note: building\n"
    cmake --build ./build/  1> /dev/null
    printf "note: done\n"
}

run () {
    build
    printf ">> program-output:\n"
    ./build/jeni $EXE_ARGS
    printf ">> done"
}

run_tests () {
    build
    stat $TEST_DIR &> /dev/null
    if [ $? == 1 ]; then
        printf "note: test directory does not exist\n"
        printf "note: creating test directory at './tests/'\n"
        mkdir -p $TEST_DIR
        exit 1
    fi

    for file in `find $TEST_DIR -type f -name '*.jeni'`; do
        ./build/jeni $file -o $file.test
        if [ $? == 0 ]; then
            if $file.test; then
                printf "test '$file' passed\n"
            else
                printf "test '$file' failed\n"
            fi
        else
            printf "error: failed to compile '$file'\n"
        fi
    done
    printf "note: running test finished\n"
}

index=1
for arg in $@; do
    if [ $arg == "-run" ]; then
        MODE="run"
    elif [ $arg == "-build" ]; then
        MODE="build"
    elif [ $arg == "-debug" ]; then
        MODE="debug"
    elif [ $arg == "-run-tests" ]; then
        MODE="tests"
    elif [ $arg == "--" ]; then
        EXE_ARGS=${@:((index + 1))}
        break
    else
        printf "error: invalid argument: '$arg'\n";
    fi
    ((index++))
done

if [ $MODE == "run" ]; then
    run
    exit
elif [ $MODE == "build" ]; then
    build
    exit
elif [ $MODE == "tests" ]; then
    run_tests
    exit
elif [ $MODE == "debug" ]; then
    debug
    exit
fi
