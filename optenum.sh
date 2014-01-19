# Copyright (C) 2013 Matt Boyer.
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the project nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

function option_complete {
	local command=${1} stem=${2} enum_opt index=0;
	# TODO In future, handle the case where the previous token on the
	# command line is an option that expects a mandatry argument
	[[ ${stem} = -* ]] || return 124

	[[ ${stem%-*} = - ]] && enum_opt="-2"
	[[ ${stem%-*} = "" ]] && enum_opt="-1"

	enum_opt="${enum_opt} -b"
	while read option; do 
		[[ ${option} == ${stem}* ]] || continue
		COMPREPLY[${index}]=${option}
		index=$((1+$index))
	done << _EOMAN
		$(optenum ${enum_opt} $(which ${command} 2>/dev/null) 2>/dev/null)
_EOMAN
	unset option
}

complete -D -F option_complete -o default

# vim:set tabstop=8 softtabstop=8 shiftwidth=8 noexpandtab list:
