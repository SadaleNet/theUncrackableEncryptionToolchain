#ifndef PARAM_H
#define PARAM_H

#include <memory>

class Argument;

struct Param_t{
	std::shared_ptr<Argument> arg; //loaded by core.h
	uni::string value; //can be used to store file name, loaded by the class Argument
	virtual ~Param_t(){} //enable inheritance for trivial struct
};

/* Table of the Meaning of Fields of Different Param Types
	type			.value		.pipe			.pairPtr			.argPointer
	INPUT			(1)			pipeData		ptrToItsOUTPUT		argPointer
	AUX_INPUT		(1)			pipeData		-					argPointer
	MOD_INPUT		(1)			pipeData		-					argPointer
	OUTPUT			(1)			pipeData		ptrToItsINPUT(2)	argPointer
	OUTPUT_NO_INPUT: (will be branched to two param)
		param 1		0			0				param 2				argPointer
		param 2		(1)			pipeData		param 1				argPointer
	OPTION:
		optional	&rawStr[1]	-				-					-1
		compulsory	rawStr		-				-					argPointer
	OPTION_PARAM	rawStr		optionTableId	-					argPointer

	(1):
	if the argument match is a sort of {fileName} :
		return fileName
	else if the argument match is a sort of {i} :
		return [to be loaded from stdin]
	else if the argument match is a sort of {[(*)]} :
		return -
	else if the argument match nothing above :
		return rawStr

	(2): unused in The Uncrackable Encryption Toolchain. Maybe useful in other programs.
*/


#endif
