
The Data Transfer Syntax.

The data transfered is always a string with a prefix name, type and action.

name:type:action data

To transfer more than one set of data in a single UDP packet use the ';' to separate them, like.

name:type:action data;name:type:action data

The name part is a unique name without whitespaces, use something like '-', '.' or '_' to make it a more readable name.

The type part is one od the following types. Can use a single letter for type instead of the complete word.

	n[umber]	= A signed interger value, like -12345 or 12345.
	d[ouble]	= A double value, like 123.456 or -123.456
	s[tring]	= A string of chars, like 'Hello world'
				  A string can contain spaces and single quotes are required.
	b[oolean]	= A boolean value. This can be any true or false type like true or false or yes or no or 1 or 0
				  Upper or lower case and single letter like T, F, Y, N

if data type is a string then enclose string in single quote marks, ie 'Kelly Wiles'.

The action is one of the following. Can use a single letter for action instead of the complete word.

	s[et]		= Set value.
	g[et]		= Get value.
	c[ompare]	= Compare value, returns -1, 0 or 1, meaning lessthan, equal to or greater than.
	i[ncrease]	= Increase number by value.
	d[ecrease]  = Decrease number by value.

The returning value is in the format of:

	name:type data

And for multiple requests:

	name:type data;name:type data
