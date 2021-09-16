#! /bin/sh
. "./test-framework.sh"

all_tests()
{
    find ../tests -type d -not -name "tests"  -not -name ".*" -printf "%f "
}


print_help_message_and_exit()
{
echo "\
Usage: run-logical-timer-tests.sh
   or: run-logical-timer-tests.sh TEST [TEST]...
   or: run-logical-timer-tests.sh print-tests

(1): Run all tests in ../tests/ ( must be directories that do not start with '.')
(2): Run the specified tests ( all assumed to be in ../tests/ )
(3): Print all available tests

Please set the environment variable SCOREP to the path of the scorep executable

Please set the environment variable PYTHONPATH to the path of the otf2 python package
"
exit
}


if [ $# -ge 1 ]; then
    if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
        print_help_message_and_exit
    fi
    if [ "$1" = "print-tests" ]; then
        printf "%s\n" $(all_tests)
        exit
    fi

    # If some arguments were passed, treat them as names of tests to run
    tests_to_run=""
    for arg in $@; do
        tests_to_run="$tests_to_run $arg"
    done
# No arguments were passed: run all tests we can find in the tests folder
else
    # All directories in the ../tests folder
    # that are not the ../tests folder itself
    # and that do not begin with a "."
    tests_to_run=$(all_tests)
fi

test_init

for test_name in $tests_to_run; do
    test_folder="../tests/$test_name"
    test_run $test_folder $test_name
    test_eval
done

test_finalize
