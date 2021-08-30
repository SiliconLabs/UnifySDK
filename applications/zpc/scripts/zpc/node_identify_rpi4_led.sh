#!/bin/sh

#############################################################################
# This script implements a "Node Identify Raspberry Pi 4 LED driver" by
# configuring LED blinks timings
#
# The script will spawn itself into a background process that will control
# the blink sequence.
#
# Call as:
#    script_name <on_time_ms> <off_time_ms> <num_cycles>
#
# Parameters
#    on_time_ms:  ON duration (in milliseconds) for a single blink cycle.
#                 If on_time is zero the indicator will be turned off and
#                 the background process will terminate immediately.
#    off_time_ms: OFF duration (in milliseconds) for a single blink cycle.
#    num_cycles:  Number of blink cycles. If num_cycles is zero the
#                 background script will terminate immediately after
#                 configuring the LED. I.e. the LED will blink "forever" or
#                 until re-configured.
#############################################################################

# Make sure SCRIPTNAME points to where the script is installed
SCRIPTNAME=@CMAKE_INSTALL_PREFIX@/zpc/node_identify_rpi4_led.sh
# The LED to control. On Raspberry Pi 4, led0 (the one next to power led) is available.
LEDPATH=/sys/class/leds/led0
PIDFILE=/var/run/zipgateway-node-identify.pid

# Divide integer by 1000 without calling external program.
# (/bin/sh does not support floating point math)
ms2sec() {
    num_digits=${#1}
    case $num_digits in
        0) res=0.0 ;;
        1) res=0.00$1 ;;
        2) res=0.0$1 ;;
        3) res=0.$1 ;;
        *) secpart=${1%???}
           mspart=${1#$secpart}
           res=$secpart.$mspart
        ;;
    esac
    echo $res
}

# Commands to configure Raspberry Pi 4 LED to blink
indicator_config() {
    echo none > $LEDPATH/trigger
    echo timer > $LEDPATH/trigger
    echo $1 > $LEDPATH/delay_on
    echo $2 > $LEDPATH/delay_off
}

# Commands to turn off Raspberry Pi 4 LED
indicator_off() {
    echo none > $LEDPATH/trigger
}

# bg_blinker is running in a background process
bg_blinker() {
    on_time_ms=$1
    off_time_ms=$2
    num_cycles=$3

    if [ $on_time_ms -eq 0 ]; then
        indicator_off
        return 0
    fi

    sequence_duration_ms=$(( (on_time_ms + off_time_ms) * num_cycles - off_time_ms))
    sequence_duration_sec=$(ms2sec $sequence_duration_ms)

    indicator_config $on_time_ms $off_time_ms
    if [ $num_cycles -ne 0 ]; then
        sleep ${sequence_duration_sec}s
        indicator_off
    fi

    return 0
}

# Kill the process with pid found in $PIDFILE
kill_bg_blinker() {
    if [ -r "$PIDFILE" ]; then
        read pid < "$PIDFILE"
        if [ -n "${pid:-}" ]; then
            rm "$PIDFILE"
            if $(kill -0 "${pid:-}" 2> /dev/null); then
                kill "${pid:-}" && return 0
            fi
        fi
    fi
}


# Here we go!

# If called with "magic" string (by ourself) we are running in the background
if [ "$1" = "bg_blinker" ]; then
    shift
    bg_blinker "$@"
else
    # This is the normal entry point when calling the script
    if [ $# -ne 3 ]; then
        echo 'Required parameters: <on_time_ms> <off_time_ms> <num_cycles>' >&2
        return 1
    else
        # Kill current blinker process (if any)
        kill_bg_blinker

        # Start bg_blinker in background and save its process id
        $SCRIPTNAME bg_blinker "$@" &
        echo $! > $PIDFILE

        return 0
    fi
fi
