#!/usr/bin/env bash

MODE="build"
TEST_DIR="./tests"
EXE_ARGS=()
USE_VALGRIND="no"
FRESH_BUILD="no"

build() {
    printf "note: refreshing cache\n"
    #cmake -B ./build > /dev/null $EXE_ARGS
    cmake -B ./build $EXE_ARGS

    if [ "$FRESH_BUILD" = "yes" ]; then
        make -C ./vendor/libtcc/ clean
    fi


    printf "note: building libtcc\n"
    if [ ! -f "./vendor/libtcc/config.mak" ]; then
        (cd ./vendor/libtcc && ./configure)
    fi
    #make -C ./vendor/libtcc/ libtcc.a
    make -C ./vendor/libtcc/ libtcc.a libtcc1.a CFLAGS="-Wall -O2 -g -DTCC_TARGET_X86_64"

    printf "note: building\n"
    local build_cmd=(cmake --build ./build/ --parallel)
    if [ "$FRESH_BUILD" = "yes" ]; then
        build_cmd+=(--clean-first)
    fi

    #"${build_cmd[@]}" > /dev/null
    "${build_cmd[@]}"
    printf "note: done\n"
}

debug() {
    build
    printf "note: debugging with 'gf2'\n"
    gf2 --args ./build/jeni "${EXE_ARGS[@]}"
    printf "done:\n"
}

run() {
    build
    printf ">> program-output"

    if [ "$USE_VALGRIND" = "yes" ]; then
        printf " (valgrind):\n"
        valgrind --leak-check=full ./build/jeni "${EXE_ARGS[@]}"
    else
        printf ":\n"
        ./build/jeni "${EXE_ARGS[@]}"
    fi
    printf ">> done\n"
}

run_tests() {
    build
    if [ ! -d "$TEST_DIR" ]; then
        printf "note: test directory does not exist\n"
        printf "note: creating test directory at '%s'\n" "$TEST_DIR"
        mkdir -p "$TEST_DIR"
        exit 1
    fi

    local passed=0
    local failed=0

    while IFS= read -r -d '' file; do
        local test_bin="${file}.test"
        if ./build/jeni "$file" -o "$test_bin" > /dev/null 2>&1; then
            if "$test_bin"; then
                printf "test '%s' passed\n" "$file"
                ((passed++))
            else
                printf "test '%s' failed\n" "$file"
                ((failed++))
            fi
            rm -f "$test_bin"
        else
            printf "error: failed to compile '%s'\n" "$file"
            ((failed++))
        fi
    done < <(find "$TEST_DIR" -type f -name '*.jeni' -print0)

    printf "note: running tests finished (%d passed, %d failed)\n" "$passed" "$failed"
}

format () {
    for file in `find ./src/ -type f -name '*.cc'` `find ./src/ -type f -name '*.h'`; do
        clang-format -i $file
    done
}

# Parse command line options
while [[ $# -gt 0 ]]; do
    case "$1" in
        -run)       MODE="run" ;;
        -build)     MODE="build" ;;
        -debug)     MODE="debug" ;;
        -run-tests) MODE="tests" ;;
        -format)    MODE="format" ;;
        -valgrind)  USE_VALGRIND="yes" ;;
        -f)         FRESH_BUILD="yes" ;;
        --)
            shift
            EXE_ARGS=("$@")
            break
            ;;
        *)
            printf "error: invalid argument: '%s'\n" "$1"
            exit 1
            ;;
    esac
    shift
done

case "$MODE" in
    run)   run ;;
    build) build ;;
    tests) run_tests ;;
    debug) debug ;;
    format) format ;;
esac
