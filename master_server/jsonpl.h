/*
	JSON parser library version 1.0.0 2019-10-31 by Santtu Nyman.
	git repository https://github.com/Santtu-Nyman/jsonpl

	Description
		Cross-Platform C library without dependencies for parsing JSON text.

		The library provides only one function for parsing the JSON text and
		defines a data structure to represent value tree of JSON text contents.

		Documentation of the function and the data structure is provided in the file "jsonpl.h".
		
	Version history
		version 1.0.0 2019-10-31
			First publicly available version.
*/

#ifndef JSONPL_H
#define JSONPL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#define JSONPL_TYPE_ERROR   0
#define JSONPL_TYPE_OBJECT  1
#define JSONPL_TYPE_ARRAY   2
#define JSONPL_TYPE_STRING  3
#define JSONPL_TYPE_NUMBER  4
#define JSONPL_TYPE_BOOLEAN 5
#define JSONPL_TYPE_NULL    6

typedef struct jsonpl_value_t
{
	size_t size;
	int type;
	union
	{
		struct
		{
			size_t value_count;
			struct jsonpl_value_t** table;
		} array;
		struct jsonpl_object_t
		{
			size_t value_count;
			struct
			{
				size_t name_length;
				char* name;
				struct jsonpl_value_t* value;
			}* table;
		} object;
		struct
		{
			size_t length;
			char* value;
		} string;
		double number_value;
		int boolean_value;
	};
} jsonpl_value_t;
/*
	Description
		The jsonpl_value_t structure defines format of value tree that represents the contents of the JSON text.

	Members
		size
			The size of this structure, in bytes.
			The size includes contents pointed by member variables and sizes of child values.

		type
			The type of this value.
			This member can be one of the following constants.

				JSON_TYPE_OBJECT
					The specified value is an object.

				JSON_TYPE_ARRAY
					The specified value is an array.

				JSON_TYPE_STRING
					The specified value is a string.

				JSON_TYPE_NUMBER
					The specified value is a number.

				JSON_TYPE_BOOLEAN
					The specified value is a boolean.

			The constant JSON_TYPE_ERROR is not used. It is for internal uses of this library only.

		array.value_count
			Value of this member is only valid if type of this value is an array.
			The number of values in this array.

		array.table
			Value of this member is only valid if type of this value is an array.
			A pointer to beginning of an array of pointers to child values.

		object.value_count
			Value of this member is only valid if type of this value is an object.
			The number of values in this object.

		object.table
			Value of this member is only valid if type of this value is an object.
			A pointer to beginning of an array of pointers to child values.
			Objects pointed in this array are structures of tree members that represents name value pairs.
			The member name_length specifies size of this string in bytes not including null terminating character or padding bytes.
			The member name specifies beginning address of null terminated UTF-8 string that specifies name in a name value pair.
			The member value specifies address of a child value.

		string.length
			Value of this member is only valid if type of this value is a string.
			The size of this string in bytes not including null terminating character or padding bytes.

		string.value
			Value of this member is only valid if type of this value is a string.
			A pointer to beginning of this string.
			The format of the string is null terminated UTF-8.

		number_value
			Value of this member is only valid if type of this value is a number.
			The value of this number.

		boolean_value
			Value of this member is only valid if type of this value is a boolean.
			The value of this boolean.
*/

size_t jsonpl_parse_text(size_t json_text_size, const char* json_text, size_t value_buffer_size, jsonpl_value_t* value_buffer);
/*
	Description
		The json_parse_text function converts a JSON text to tree structure that represents the contents of the JSON text.
		The parser function unescapes strings and decodes numbers.

	Parameters
		json_text_size
			Size of JSON text in bytes.
			Characters of JSON text are UTF-8 encoded allowing characters to take multiple bytes.
		json_text
			Pointer to buffer that contains the JSON text.
			The text may be optionally null terminated.
		value_buffer_size
			Size of buffer pointed by parameter value_buffer in bytes.

			If the buffer is not large enough to hold the tree data, the function returns required buffer size in bytes.
			Required size of the buffer can't be zero for any JSON text.
		value_buffer
			A pointer to a buffer that receives the value tree if size of the buffer is sufficiently large.
			The whole tree is written to this buffer and will not contain any pointers to any memory outside of the value tree buffer.

			All strings in the value tree are null terminated, aligned to size of pointer and
			if next address after the null character is not aligned to size of pointer,
			zero bytes are appended after the null terminating character until next address is aligned to size of pointer.
			This allows strings to be read in pointer sized blocks.

			If the buffer is not large enough to hold the tree data, the contents of the tree value buffer are undefined.

			If the buffer size is zero, this parameter is ignored.
	Return
		If the JSON text is successfully parsed, the return value is size of value three in bytes and zero otherwise.

		If the returned size is not zero and not greater than the size of the value tree buffer,
		the buffer will contain value tree representing the contents of the JSON text.
*/

#ifdef __cplusplus
}
#endif

#endif