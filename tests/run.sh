#!/bin/bash

S=`basename $0`
#P=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
P="$( dirname "$( readlink -f "$0" )" )"

ARCH=$(gcc -dumpmachine | awk 'BEGIN { FS = "-" } ; { print $1 }')
BINPATH="${P}/../bin/${ARCH}"
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${BINPATH}"

cd "$P"
# could be set to STDOUT, SUBUNIT, TAP or XML.
export CMOCKA_MESSAGE_OUTPUT=STDOUT

"${P}/runtests.exe" 2>&1 | \
    sed ''/OK/s//$(printf "\033[32mOK\033[0m")/'' | \
    sed ''/PASSED/s//$(printf "\033[32mPASSED\033[0m")/'' | \
    sed ''/FAILED/s//$(printf "\033[31mFAILED\033[0m")/'' | \
    sed ''/ERROR/s//$(printf "\033[31mERROR\033[0m")/''

#gdb --args "${P}/runtests.exe"
