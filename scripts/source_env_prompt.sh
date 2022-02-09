#!/bin/sh

# Finds a unify_env.source file from the root of the unify repository
# and source it to make it possible to run Rust applications that depends on Unify libs without installing the libs.
SCRIPT_DIR="$(dirname "${BASH_SOURCE[0]}")"
UNIFY_ROOT="$(dirname "$SCRIPT_DIR")"

# find build folders and sort them by last accessed.
env_scripts="$(find $UNIFY_ROOT -name  unify_env.source -maxdepth 2 | xargs ls -lt1ud)"
if [ -z "$env_scripts" ]
then
    yellow='\033[0;33m'
    NC='\033[0m'

    echo "${yellow}Warning! Could not load unify rust environment..\n"\
    "-> Please make sure you ran cmake sucessfully!${NC}"
    exit 0
fi

read -a options <<< $env_scripts
if [ "${#options[@]}" -eq "1" ]; then
    choice=0
else
    echo "please select the correct environment to source:"
    for i in ${!options[@]}; do
        echo "  [$i]\t $(dirname "${options[$i]}")"
    done
    read -a choice
fi

. "${options[$choice]}"
echo "sourced ${options[$choice]}!"

