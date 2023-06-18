#!/bin/bash
set -eo pipefail
shopt -s inherit_errexit

function clear_colors {
	unset COLOR_MINOR COLOR_MINOR2 COLOR_MAJOR COLOR_NONE
}

CALLER_NAME=`basename ${0^^}`
if [[ $GNU_BUILD_WRAPPER_DEBUG -eq 1 ]]; then
	if [[ $GNU_BUILD_WRAPPER_COLOR -eq 	1 ]]; then
		COLOR_MINOR="${COLOR_MINOR:-\e[2;33m}"
		COLOR_MINOR2="${COLOR_MINOR2:-\e[2;36m}"
		COLOR_MAJOR="${COLOR_MAJOR:-\e[1;32m}"
		COLOR_NONE="${COLOR_NONE:-\e[0m}"
	else
		clear_colors;
	fi
	echo -e ${COLOR_MINOR}GNU ${CALLER_NAME} INPUT${COLOR_NONE}: "$@" 1>&2
else
	clear_colors;
fi


function wrapper_exec {	
	shopt -s extglob
	if [[ -v GNU_BUILD_CMD_FILE ]]; then
		LAST_PWD_FILE="${GNU_BUILD_CMD_FILE}.tmpcurdir"
		CUR_PWD=`pwd`
		if [[ ! -e "$LAST_PWD_FILE" || "$CUR_PWD" != "`cat $LAST_PWD_FILE`" ]]; then
			echo -e cd "$CUR_PWD" >> "$GNU_BUILD_CMD_FILE"
			echo -n $CUR_PWD > "$LAST_PWD_FILE"
		fi
		echo -e "${@//'\e'\[*([0-9;])m/}" >> "$GNU_BUILD_CMD_FILE"
	fi
	if [[ $GNU_BUILD_WRAPPER_DEBUG -eq 1 ]]; then	
		echo -e "${COLOR_MINOR}GNU ${CALLER_NAME} OUTPUT${COLOR_NONE}: " "$@" $STD_DECLARE $linker_opts 1>&2
	fi
	exec "${@//'\e'\[*([0-9;])m/}"  #strip ansi color strings out
	shopt -u extglob

}

