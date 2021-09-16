#! /bin/sh

################################################################################
# Set up color support
# stolen from /test/measurement/config/test-framework.sh
################################################################################
color_enabled=no
if [ "$AM_COLOR_TESTS" = no ]; then
    color_enabled=no
elif [ "$AM_COLOR_TESTS" = always ]; then
    color_enabled=yes
elif [ "$TERM" != dumb ] && [ -t 1 ]; then
    color_enabled=yes
fi

if test $color_enabled = yes; then
    # e for ascii escape sequence. Add some prefix to avoid name clashes when being sourced.
    e_r='\033[0;31m'
    e_g='\033[0;32m'
    e_b='\033[1m'
    e_rb="$e_r$e_b"
    e_gb="$e_g$e_b"
    e_n='\033[0m'
else
    e_r=''
    e_g=''
    e_b=''
    e_rb=''
    e_gb=''
    e_n=''
fi


################################################################################
# Utils
################################################################################
test_passed_flag=0
test_failed_flag=1
test_skipped_flag=2

test_message()
{
    local level="$1"
    case $level in
        "info") local color=$e_b;;
        "debug") local color=$e_n;;
        "error") local color=$e_r;;
        "success") local color=$e_g;;
        *) local color=$e_n;;
    esac
    local format="$2"
    shift
    shift
    printf "TIMER-TEST: ${color}${format}${e_n}" $@
}


################################################################################
# Individual steps of test_run()
################################################################################
test_load()
{
    for test_file in set.sh test.c; do
        if [ ! -f "${test_folder}/${test_file}" ]; then
            test_message "error" "Test ${test_name} is incomplete: ${test_folder}/${test_file} not found\n"
            return $test_skipped_flag
        fi
    done

    nprocs=1
    nthreads=1
    expect_failure=
    . ${test_folder}/set.sh
    local rc=$?
    if [ $rc -ne 0 ] ; then
        test_message "error" " ${test_folder}/set.sh is invalid\n"
        return $test_skipped_flag
    fi

    test_message "info" "Running test '$test_name' on $nprocs MPI processes with $nthreads OMP threads each\n"
}


clean_test_artifacts()
{
    test_message "debug" "Cleaning test artifacts\n"
    rm -f traces*.txt
    rm -rf scorep_timer-test*
    rm -f timer-test_*_trace.log
}


clean_build_artifacts()
{
    test_message "debug" "Cleaning build artifacts\n"
    make clean
}


test_build()
{
    test_message "info" "Building the instrumented executable\n"
    make TEST=$test_name
    # See if make had an error
    local rc=$?
    if [ $rc -ne 0 ]; then
        test_message "error" "Building test $test_name failed: make returned %d\n" $rc
        return $test_failed_flag
    fi
}


test_trace()
{
    test_message "info" "Tracing the executable\n"
    export OMP_NUM_THREADS=$nthreads && scan -q -t mpirun -n $nprocs ./timer-test.x

    # See if scan itself had an error
    local rc=$?
    if [ $rc -ne 0 ]; then
        test_message "error" "scan failed with return code %d\n" $rc
        return $test_failed_flag
    fi
}


test_validate_scorep_log()
{
    # Name of the output folder for traces:
    # This depends on the name of the executable and
    # the number of processes the number of threads.
    local scorep_out="scorep_timer-test_${nprocs}x${nthreads}_trace"

    test_message "info" "Validating the output of the executable\n"
    # See if the scorep log file exists.
    # This contains the output of our tested program.
    local scorep_log="${scorep_out}/scorep.log"
    if [ ! -f $scorep_log ]; then
        test_message "error" "Could not find the scorep log file %s\n" "$scorep_log"
        return $test_failed_flag
    fi

    # Check if Scorep had any warnings about thread level exceeding MPI_THREAD_FUNNELED
    local mpi_thread_level_warnings="$(sed -r -e '/[Score-P].*Warning:.*MPI_THREAD_FUNNELED/!d' $scorep_log)"
    if [ "$mpi_thread_level_warnings" ]; then
        test_message "error" "scorep produced warnings:\n${mpi_thread_level_warnings}\n"
        return $test_failed_flag
    fi

    # Get name, status and return code of the test program
    local pattern='\| Master \| Test ([a-zA-Z_]+) (FAILED|SUCCEEDED) with final status ([[:digit:]]+)'
    # this command with the weird quoting is necessary to work around the stupid history expansion
    local sed_remove_nonmatching="/${pattern}/"'!d'
    local test_status="$(sed -r "$sed_remove_nonmatching" "$scorep_log" | sed -r "s/${pattern}/\1 \2 \3/")"
    local pattern='([a-zA-Z_]+) (FAILED|SUCCEEDED) ([[:digit:]]+)'
    local parsed_test_name=$(echo "$test_status" | sed -r "s/${pattern}/\1/")
    local parsed_test_result=$(echo "$test_status" | sed -r "s/${pattern}/\2/")
    local parsed_test_status=$(echo "$test_status" | sed -r "s/${pattern}/\3/")

    # Check if we could parse the test output correctly
    if [ ! $parsed_test_name ] || [ ! $parsed_test_result ] || [ ! $parsed_test_status ]; then
        test_message "error" "Error while parsing test output:\n${test_status}\n"
        return $test_failed_flag
    fi

    # Check if the name we parsed from the test output
    # and the name passed as argument to this function are identical
    if [ "$parsed_test_name" != "$test_name" ]; then
        test_message "error" "The output seems to come from test %s, but this is test %s\n" "$parsed_test_name" "$test_name"
        return $test_failed_flag
    fi

    # Check if the test failed
    if [ $parsed_test_status -ne 0 ] || [ $parsed_test_result != "SUCCEEDED" ]; then
        test_message "error" "Test %s %s with status %s\n" "$parsed_test_name" "$parsed_test_result" "$parsed_test_status"
        return $test_failed_flag
    fi
}


test_validate_scout_log()
{
    test_message "info" "Validating the output of the analyzer\n"

    # Name of the output folder for traces:
    # This depends on the name of the executable and
    # the number of processes the number of threads.
    local scorep_out="scorep_timer-test_${nprocs}x${nthreads}_trace"

    # See if the scout log file exists.
    # This contains the output of the scan command.
    local scout_log="${scorep_out}/scout.log"
    if [ ! -f $scout_log ]; then
        test_message "error" "Could not find the scout log file %s\n" "$scout_log"
        return $test_failed_flag
    fi

    # Check for errors and warnings in the scout log file

    # Check for warnings
    # We treat all warnings as errors
    local scout_warnings="$(sed -r -e '/WARNING/!d' -e 'n' $scout_log)"
    if [ "$scout_warnings" ]; then
        test_message "error" "scan produced some warnings:\n${scout_warnings}\n"
        return $test_failed_flag
    fi
}


test_validate_traces()
{
    test_message "info" "Validating the trace file\n"

    local scorep_out="scorep_timer-test_${nprocs}x${nthreads}_trace"
    local trace_file="${scorep_out}/traces.otf2"

    # Does the trace file exist?
    if [ ! -f $trace_file ]; then
        test_message "error" "Could not find the traces file %s\n" "$trace_file"
        return $test_failed_flag
    fi

    # TODO: make the python otf2 module in vendor available here
    python3 validate_trace_omp.py --input=$trace_file --num_processes=$nprocs --num_threads=$nthreads --test_name=$test_name

    local rc=$?
    if [ $rc -ne 0 ]; then
        test_message "error" "Validation of the trace file failed\n"
        return $test_failed_flag
    fi
}

test_expectation()
{
    local rc=$?

    if [ "$expect_failure" ]; then
        case $rc in
            "$test_failed_flag")
            test_message "info" "This test is expected to fail, which it did.\n"
            return $test_passed_flag;;
            "$test_passed_flag")
            test_message "error" "This test is expected to fail, but it didn't.\n"
            return $test_failed_flag;;
            *) return $rc;;
        esac
    fi
    test_message "debug" "This test is expected to pass.\n"
    return $rc
}


################################################################################
# Functions to be called from run-logical-timer-tests.sh
################################################################################
test_init()
{
    printf "
-----------------------------
Test suite for logical timers
-----------------------------

"
    num_tests_passed=0
    num_tests_failed=0
    num_tests_skipped=0
    export 'SCOREP_TIMER=logical'
}


test_finalize()
{
    local num_tests_run=$(($num_tests_passed + $num_tests_failed))
    local color="$e_r"
    local return_status=1
    if [ $num_tests_failed -eq 0 ]; then
        local color="$e_g"
        local return_status='0'
    fi
    printf "
-----------------------------
Test summary
-----------------------------
TESTS RUN     = $num_tests_run
TESTS SKIPPED = $num_tests_skipped

${e_g}TESTS PASSED  = $num_tests_passed${e_n}
${color}TESTS FAILED  = $num_tests_failed${e_n}
-----------------------------
"
    clean_build_artifacts
    clean_test_artifacts
    exit "$return_status"
}


test_eval()
{
    local rc=$?
    if [ $rc -eq $test_passed_flag ]; then
        num_tests_passed=$(($num_tests_passed+1))
        test_message "success" "Test '%s' PASSED\n" "$test_name"
        return
    fi
    if [ $rc -eq $test_failed_flag ]; then
        num_tests_failed=$(($num_tests_failed+1))
        test_message "error" "Test '%s' FAILED\n" "$test_name"
        return
    fi
    if [ $rc -eq $test_skipped_flag ]; then
        num_tests_skipped=$(($num_tests_skipped+1))
        test_message "info" "Test '%s' SKIPPED\n" "$test_name"
        return
    fi
}


test_run()
{
    test_folder="$1"
    test_name="$2"

    test_load \
    && clean_build_artifacts \
    && clean_test_artifacts \
    && test_build \
    && test_trace \
    && test_validate_scorep_log \
    && test_validate_scout_log \
    && test_validate_traces

    test_expectation
}
