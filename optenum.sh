function option_complete {
	#(( $# == 3 )) || return 1

	command=${1}

	stem=${2}
	#typeset -a COMPREPLY
	[[ ${stem} = -* ]] || return 124

	[[ ${stem%-*} = - ]] && enum_opt="-2"
	[[ ${stem%-*} = "" ]] && enum_opt="-1"

	index=0;
	while read option; do 
		[[ ${option} == ${stem}* ]] || continue
		#echo $option
		COMPREPLY[${index}]=${option}
		index=$((1+$index))
	done << _EOMAN
		$(optenum ${enum_opt} $(which ${command}))
_EOMAN
}

