#include <maxscript/maxscript.h>
#include <maxscript/foundation/numbers.h>
#include <maxscript/foundation/arrays.h>
#include <maxscript/util/listener.h>

#include "cmdex.h"

void CmdexInit()
{
	//Todo: Place initialization code here. This gets called when Maxscript goes live
	//during max startup.
}

// Declare C++ function and register it with MAXScript
#include <maxscript\macros\define_instantiation_functions.h>
	def_visible_primitive(cmdexRun, "cmdexRun");

Value* cmdexRun_cf(Value **arg_list, int count) {
	//--------------------------------------------------------
	//Maxscript usage:
	//--------------------------------------------------------
	// cmdex <command line:String>
	check_arg_count(cmdexRun2, 1, count);
	Value* pCommand = arg_list[0];

	//First example of how to type check an argument
	if (!(is_string(pCommand)))
	{
		throw RuntimeError(_T("Expected a String for the first argument"));
	}

	const wchar_t* command = pCommand->to_string();

	auto output = cmdex::run(command);
	the_listener->edit_stream->puts(output);

	return &true_value;
}
