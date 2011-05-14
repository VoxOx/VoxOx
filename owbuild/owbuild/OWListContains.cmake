# - ow_list_contains(list string result)
# Internal function, checks if ${string} is in ${list}
#
# Example:
# set(list
# 	"bonjour"
# 	"hello"
# 	"guten tag"
# 	"ciao"
# 	"hola"
# )
# ow_list_contains(${list} "hello" result)
# message("list contains hello?" ${result})
#
# Output generated:
# list contains hello? YES
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_list_contains list string result)

	set(${result} NO)

	foreach (loop ${list})
		if (loop MATCHES ${string})
			set(${result} YES)
		endif (loop MATCHES ${string})
	endforeach (loop)

endmacro (ow_list_contains)
