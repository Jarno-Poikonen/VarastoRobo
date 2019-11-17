/*
	JSON parser library version 1.0.0 2019-10-31 by Santtu Nyman.
	git repository https://github.com/Santtu-Nyman/jsonpl
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "jsonpl.h"

#if defined(_MSC_VER)
#define JSONPL_INLINE __forceinline
#elif defined(__GNUC__)
#define JSONPL_INLINE inline __attribute__((always_inline))
#else
#define JSONPL_INLINE inline
#endif

static const size_t jsonpl_value_alignment_minus_one = ((sizeof(double) > sizeof(void*)) ? sizeof(double) : ((sizeof(int) > sizeof(void*)) ? sizeof(int) : sizeof(void*))) - 1;

static JSONPL_INLINE size_t jsonpl_round_size(size_t size) { return (size + jsonpl_value_alignment_minus_one) & ~jsonpl_value_alignment_minus_one; }

static JSONPL_INLINE int jsonpl_is_white_space(char character) { return character == ' ' || character == '\t' || character == '\n' || character == '\r'; }

static JSONPL_INLINE int jsonpl_is_structural_character(char character) { return character == '[' || character == '{' || character == ']' || character == '}' || character == ':' || character == ',' || character == '"'; }

static JSONPL_INLINE int jsonpl_is_decimal_value(char character) { return character >= '0' && character <= '9'; }

static JSONPL_INLINE int jsonpl_is_hex_value(char character) { return (character >= '0' && character <= '9') || (character >= 'A' && character <= 'F') || (character >= 'a' && character <= 'f'); }

static JSONPL_INLINE int jsonpl_decimal_value(char character) { return character - '0'; }

static JSONPL_INLINE int jsonpl_hex_value(char character) { return (character <= '9') ? (character - '0') : ((character <= 'F') ? (character - ('A' - 10)) : (character - ('f' - 10))); }

static size_t jsonpl_white_space_length(size_t json_text_size, const char* json_text);

static size_t jsonpl_printable_space_length(size_t json_text_size, const char* json_text);

static size_t jsonpl_quoted_string_lenght(size_t json_text_size, const char* json_text);

static size_t jsonpl_decode_boolean(size_t json_text_size, const char* json_text, int* value);

static size_t jsonpl_decode_quoted_string(size_t json_text_size, const char* json_text, size_t string_buffer_size, char* string_buffer, size_t* string_size);

static size_t jsonpl_decode_number(size_t json_text_size, const char* json_text, double* number_value);

static size_t jsonpl_object_text_size(size_t json_text_size, const char* json_text);

static size_t jsonpl_array_text_size(size_t json_text_size, const char* json_text);

static int jsonpl_decide_value_type(size_t json_text_size, const char* json_text, size_t* object_text_size);

static size_t jsonpl_name_value_pair_size(size_t json_text_size, const char* json_text, size_t* name_size, int* value_type, size_t* value_offset, size_t* value_size);

static size_t jsonpl_count_object_name_value_pairs(size_t json_text_size, const char* json_text);

static size_t jsonpl_count_array_values(size_t json_text_size, const char* json_text);

static void jsonpl_terminator_string(char* string_end);

static size_t jsonpl_create_tree_from_text(size_t json_text_size, const char* json_text, size_t value_buffer_size, jsonpl_value_t* value_buffer, size_t* required_value_buffer_size);

static size_t jsonpl_white_space_length(size_t json_text_size, const char* json_text)
{
	const char* end = json_text + json_text_size;
	const char* read = json_text;
	while (read != end && jsonpl_is_white_space(*read))
		++read;
	return (size_t)((uintptr_t)read - (uintptr_t)json_text);
}

static size_t jsonpl_printable_space_length(size_t json_text_size, const char* json_text)
{
	const char* end = json_text + json_text_size;
	const char* read = json_text;
	while (read != end && !jsonpl_is_white_space(*read))
		++read;
	return (size_t)((uintptr_t)read - (uintptr_t)json_text);
}

static size_t jsonpl_quoted_string_lenght(size_t json_text_size, const char* json_text)
{
	const char* end = json_text + json_text_size;
	const char* read = json_text + 1;
	if (!json_text_size || *json_text != '\"')
		return 0;
	for (int escape = 0; read != end;)
	{
		char character = *read++;
		if (!escape && character == '\"')
			return (size_t)((uintptr_t)read - (uintptr_t)json_text);
		escape = character == '\\';
	}
	return 0;
}

static size_t jsonpl_decode_boolean(size_t json_text_size, const char* json_text, int* value)
{
	if ((json_text_size == 5 && json_text[0] == 'f' && json_text[1] == 'a' && json_text[2] == 'l' && json_text[3] == 's' && json_text[4] == 'e') ||
		(json_text_size > 5 && json_text[0] == 'f' && json_text[1] == 'a' && json_text[2] == 'l' && json_text[3] == 's' && json_text[4] == 'e' &&
		(jsonpl_is_white_space(json_text[5]) || jsonpl_is_structural_character(json_text[5]))))
	{
		if (value)
			*value = 0;
		return 5;
	}
	else if ((json_text_size == 4 && json_text[0] == 't' && json_text[1] == 'r' && json_text[2] == 'u' && json_text[3] == 'e') ||
		(json_text_size > 4 && json_text[0] == 't' && json_text[1] == 'r' && json_text[2] == 'u' && json_text[3] == 'e' &&
		(jsonpl_is_white_space(json_text[4]) || jsonpl_is_structural_character(json_text[4]))))
	{
		if (value)
			*value = 1;
		return 4;
	}
	else
		return 0;
}

static size_t jsonpl_decode_quoted_string(size_t json_text_size, const char* json_text, size_t string_buffer_size, char* string_buffer, size_t* string_size)
{
	json_text_size = jsonpl_quoted_string_lenght(json_text_size, json_text);
	const char* end = json_text + json_text_size - 1;
	const char* read = json_text + 1;
	size_t string_length = 0;
	if (!json_text_size)
		return 0;
	json_text_size -= 2;
	json_text += 1;
	while (read != end)
	{
		char character = *read;
		if (character != '\\' || read + 1 == end)
		{
			++read;
			if (string_length < string_buffer_size)
				*string_buffer++ = character;
			++string_length;
		}
		else
		{
			character = *(read + 1);
			switch (character)
			{
				case '\'' :
					read += 2;
					if (string_length < string_buffer_size)
						*string_buffer++ = '\'';
					++string_length;
					break;
				case '"' :
					read += 2;
					if (string_length < string_buffer_size)
						*string_buffer++ = '"';
					++string_length;
					break;
				case '\\' :
					read += 2;
					if (string_length < string_buffer_size)
						*string_buffer++ = '\\';
					++string_length;
					break;
				case 'n':
					read += 2;
					if (string_length < string_buffer_size)
						*string_buffer++ = '\n';
					++string_length;
					break;
				case 'r':
					read += 2;
					if (string_length < string_buffer_size)
						*string_buffer++ = '\r';
					++string_length;
					break;
				case 't':
					read += 2;
					if (string_length < string_buffer_size)
						*string_buffer++ = '\t';
					++string_length;
					break;
				case 'b':
					read += 2;
					if (string_length < string_buffer_size)
						*string_buffer++ = '\b';
					++string_length;
					break;
				case 'f':
					read += 2;
					if (string_length < string_buffer_size)
						*string_buffer++ = '\f';
					++string_length;
					break;
				case 'u':
					if (read + 5 != end && jsonpl_is_hex_value(*(read + 2)) && jsonpl_is_hex_value(*(read + 3)) && jsonpl_is_hex_value(*(read + 4)) && jsonpl_is_hex_value(*(read + 5)))
					{
						uint32_t unicode_character = ((uint32_t)jsonpl_hex_value(*(read + 2)) << 12) | ((uint32_t)jsonpl_hex_value(*(read + 3)) << 8) | ((uint32_t)jsonpl_hex_value(*(read + 4)) << 4) | (uint32_t)jsonpl_hex_value(*(read + 5));
						if (unicode_character > 0x7FF)
						{
							if (string_length < string_buffer_size)
								*(uint8_t*)string_buffer++ = (uint8_t)(unicode_character >> 12) | 0xE0;
							++string_length;
							if (string_length < string_buffer_size)
								*(uint8_t*)string_buffer++ = (uint8_t)((unicode_character >> 6) & 0x3F) | 0x80;
							++string_length;
							if (string_length < string_buffer_size)
								*(uint8_t*)string_buffer++ = (uint8_t)(unicode_character & 0x3F) | 0x80;
							++string_length;
						}
						else if (unicode_character > 0x7F)
						{
							if (string_length < string_buffer_size)
								*(uint8_t*)string_buffer++ = (uint8_t)(unicode_character >> 6) | 0xC0;
							++string_length;
							if (string_length < string_buffer_size)
								*(uint8_t*)string_buffer++ = (uint8_t)(unicode_character & 0x3F) | 0x80;
							++string_length;
						}
						else
						{
							if (string_length < string_buffer_size)
								*(uint8_t*)string_buffer++ = (uint8_t)unicode_character;
							++string_length;
						}
						read += 6;
					}
					else
					{
						++read;
						if (string_length < string_buffer_size)
							*string_buffer++ = '\\';
						++string_length;
					}
					break;
				default:
					++read;
					if (string_length < string_buffer_size)
						*string_buffer++ = '\\';
					++string_length;
					break;
			}
		}
	}
	*string_size = string_length;
	return json_text_size + 2;
}

static size_t jsonpl_decode_number(size_t json_text_size, const char* json_text, double* number_value)
{
	size_t remaining_size = json_text_size;
	const char* read = json_text;
	if (!remaining_size)
		return 0;
	int is_negative = *read == '-';
	if (is_negative)
	{
		--remaining_size;
		++read;
	}
	if (!remaining_size || !jsonpl_is_decimal_value(*read))
		return 0;
	size_t integer_digit_count = 1;
	const char* integer_digits = read;
	while (integer_digit_count != remaining_size && jsonpl_is_decimal_value(integer_digits[integer_digit_count]))
		++integer_digit_count;
	if (integer_digit_count > 1 && *integer_digits == '0')
		return 0;
	remaining_size -= integer_digit_count;
	read += integer_digit_count;
	size_t fraction_digit_count = 0;
	const char* fraction_digits = 0;
	if (remaining_size && *read == '.')
	{
		--remaining_size;
		++read;
		if (!remaining_size || !jsonpl_is_decimal_value(*read))
			return 0;
		fraction_digit_count = 1;
		fraction_digits = read;
		while (fraction_digit_count != remaining_size && jsonpl_is_decimal_value(fraction_digits[fraction_digit_count]))
			++fraction_digit_count;
		remaining_size -= fraction_digit_count;
		read += fraction_digit_count;
	}
	int exponent_is_negative = 0;
	size_t exponent_digit_count = 0;
	const char* exponent_digits = 0;
	if (remaining_size && (*read == 'E' || *read == 'e'))
	{
		--remaining_size;
		++read;
		if (!remaining_size)
			return 0;
		if (*read == '+')
		{
			--remaining_size;
			++read;
		}
		else if (*read == '-')
		{
			exponent_is_negative = 1;
			--remaining_size;
			++read;
		}
		if (!remaining_size || !jsonpl_is_decimal_value(*read))
			return 0;
		exponent_digit_count = 1;
		exponent_digits = read;
		while (exponent_digit_count != remaining_size && jsonpl_is_decimal_value(exponent_digits[exponent_digit_count]))
			++exponent_digit_count;
		remaining_size -= exponent_digit_count;
		read += exponent_digit_count;
	}
	if (remaining_size && !jsonpl_is_white_space(*read) && !jsonpl_is_structural_character(*read))
		return 0;
	if (number_value)
	{
		double value = 0.0;
		double decimal_shift;
		if (fraction_digit_count)
		{
			decimal_shift = 1.0;
			for (size_t i = fraction_digit_count; i--;)
				decimal_shift /= 10.0;
			for (size_t i = fraction_digit_count; i--; decimal_shift *= 10.0)
				value += decimal_shift * (double)jsonpl_decimal_value(fraction_digits[i]);
		}
		decimal_shift = 1.0;
		for (size_t i = integer_digit_count; i--; decimal_shift *= 10.0)
			value += decimal_shift * (double)jsonpl_decimal_value(integer_digits[i]);
		if (exponent_digit_count)
		{
			if (exponent_digit_count < 4)
			{
				int exponent_value = 0;
				for (int i = 0; i != (int)exponent_digit_count; ++i)
					exponent_value = 10 * exponent_value + jsonpl_decimal_value(exponent_digits[i]);
				if (exponent_value)
				{
					decimal_shift = 1.0;
					for (size_t i = exponent_value; i--;)
						decimal_shift *= 10.0;
					if (exponent_is_negative)
						value /= decimal_shift;
					else
						value *= decimal_shift;
				}
			}
			else
			{
				if (exponent_is_negative)
					value = 0.0;
				else
				{
					const uint64_t hex_infinity = 0x7FF0000000000000;
					value = *(const double*)&hex_infinity;
				}
			}
		}
		if (value != 0.0 && is_negative)
			value = -value;
		*number_value = value;
	}
	return (size_t)((uintptr_t)read - (uintptr_t)json_text);
}

static size_t jsonpl_object_text_size(size_t json_text_size, const char* json_text)
{
	const char* end = json_text + json_text_size;
	const char* read = json_text + 1;
	int depth = 1;
	if (!json_text_size || *json_text != '{')
		return 0;
	while (read != end)
	{
		char character = *read++;
		if (character == '}')
		{
			--depth;
			if (!depth)
				return (size_t)((uintptr_t)read - (uintptr_t)json_text);
		}
		else if (character == '{')
			++depth;
		else if (character == '"')
		{
			--read;
			size_t quoted_string_lenght = jsonpl_quoted_string_lenght((size_t)((uintptr_t)end - (uintptr_t)read), read);
			if (quoted_string_lenght)
				read += quoted_string_lenght;
			else
				return 0;
		}
	}
	return 0;
}

static size_t jsonpl_array_text_size(size_t json_text_size, const char* json_text)
{
	const char* end = json_text + json_text_size;
	const char* read = json_text + 1;
	int depth = 1;
	if (!json_text_size || *json_text != '[')
		return 0;
	while (read != end)
	{
		char character = *read++;
		if (character == ']')
		{
			--depth;
			if (!depth)
				return (size_t)((uintptr_t)read - (uintptr_t)json_text);
		}
		else if (character == '[')
			++depth;
		else if (character == '"')
		{
			--read;
			size_t quoted_string_lenght = jsonpl_quoted_string_lenght((size_t)((uintptr_t)end - (uintptr_t)read), read);
			if (quoted_string_lenght)
				read += quoted_string_lenght;
			else
				return 0;
		}
	}
	return 0;
}

static int jsonpl_decide_value_type(size_t json_text_size, const char* json_text, size_t* object_text_size)
{
	if ((json_text_size == 4 && json_text[0] == 'n' && json_text[1] == 'u' && json_text[2] == 'l' && json_text[3] == 'l') ||
		(json_text_size > 4 && json_text[0] == 'n' && json_text[1] == 'u' && json_text[2] == 'l' && json_text[3] == 'l' &&
		(jsonpl_is_white_space(json_text[4]) || jsonpl_is_structural_character(json_text[4]))))
	{
		*object_text_size = 4;
		return JSONPL_TYPE_NULL;
	}
	size_t length = jsonpl_decode_boolean(json_text_size, json_text, 0);
	if (length)
	{
		*object_text_size = length;
		return JSONPL_TYPE_BOOLEAN;
	}
	length = jsonpl_object_text_size(json_text_size, json_text);
	if (length)
	{
		*object_text_size = length;
		return JSONPL_TYPE_OBJECT;
	}
	length = jsonpl_array_text_size(json_text_size, json_text);
	if (length)
	{
		*object_text_size = length;
		return JSONPL_TYPE_ARRAY;
	}
	length = jsonpl_decode_number(json_text_size, json_text, 0);
	if (length)
	{
		*object_text_size = length;
		return JSONPL_TYPE_NUMBER;
	}
	length = jsonpl_quoted_string_lenght(json_text_size, json_text);
	if (length)
	{
		*object_text_size = length;
		return JSONPL_TYPE_STRING;
	}
	*object_text_size = 0;
	return JSONPL_TYPE_ERROR;
}

static size_t jsonpl_name_value_pair_size(size_t json_text_size, const char* json_text, size_t* name_size, int* value_type, size_t* value_offset, size_t* value_size)
{
	size_t value_lenght;
	size_t name_lenght = jsonpl_quoted_string_lenght(json_text_size, json_text);
	if (name_size)
		*name_size = name_lenght;
	const char* read = json_text;
	const char* end = json_text + json_text_size;
	if (!name_lenght)
		return 0;
	read += name_lenght;
	read += jsonpl_white_space_length((size_t)((uintptr_t)end - (uintptr_t)read), read);
	if (read == end || *read != ':')
		return 0;
	read += 1;
	read += jsonpl_white_space_length((size_t)((uintptr_t)end - (uintptr_t)read), read);
	if (read == end)
		return 0;
	size_t object_offset = (size_t)((uintptr_t)read - (uintptr_t)json_text);
	if (value_offset)
		*value_offset = object_offset;
	int value_object_type = jsonpl_decide_value_type((size_t)((uintptr_t)end - (uintptr_t)read), read, &value_lenght);
	if (value_object_type == JSONPL_TYPE_ERROR)
		return 0;
	if (value_type)
		*value_type = value_object_type;
	if (value_size)
		*value_size = value_lenght;
	return object_offset + value_lenght;
}

static size_t jsonpl_count_object_name_value_pairs(size_t json_text_size, const char* json_text)
{
	json_text_size = jsonpl_object_text_size(json_text_size, json_text);
	if (!json_text_size)
		return (size_t)~0;
	const char* end = json_text + json_text_size - 1;
	const char* read = json_text + 1;
	size_t object_count = 0;
	int expecting_new_object = 1;
	while (read != end)
	{
		char character = *read;
		if (expecting_new_object)
		{
			if (character == '"')
			{
				size_t name_value_pair_size = jsonpl_name_value_pair_size((size_t)((uintptr_t)end - (uintptr_t)read), read, 0, 0, 0, 0);
				if (!name_value_pair_size)
					return (size_t)~0;
				read += name_value_pair_size;
				++object_count;
				expecting_new_object = 0;
			}
			else if (jsonpl_is_white_space(character))
				++read;
			else
				return (size_t)~0;
		}
		else
		{
			if (character == ',')
				expecting_new_object = 1;
			else if (!jsonpl_is_white_space(character))
				return (size_t)~0;
			++read;
		}
	}
	return (!object_count || !expecting_new_object) ? object_count : (size_t)~0;
}

static size_t jsonpl_count_array_values(size_t json_text_size, const char* json_text)
{
	json_text_size = jsonpl_array_text_size(json_text_size, json_text);
	if (!json_text_size)
		return (size_t)~0;
	const char* end = json_text + json_text_size - 1;
	const char* read = json_text + 1;
	size_t object_count = 0;
	int expecting_new_object = 1;
	while (read != end)
	{
		char character = *read;
		if (expecting_new_object)
		{
			if (jsonpl_is_white_space(character))
				++read;
			else
			{
				size_t value_lenght;
				int value_type = jsonpl_decide_value_type((size_t)((uintptr_t)end - (uintptr_t)read), read, &value_lenght);
				if (value_type == JSONPL_TYPE_ERROR)
					return (size_t)~0;
				read += value_lenght;
				++object_count;
				expecting_new_object = 0;
			}
		}
		else
		{
			if (character == ',')
				expecting_new_object = 1;
			else if (!jsonpl_is_white_space(character))
				return (size_t)~0;
			++read;
		}
	}
	return (!object_count || !expecting_new_object) ? object_count : (size_t)~0;
}

static void jsonpl_terminator_string(char* string_end)
{
	*string_end++ = 0;
	while ((uintptr_t)string_end & (uintptr_t)jsonpl_value_alignment_minus_one)
		*string_end++ = 0;
}

static size_t jsonpl_create_tree_from_text(size_t json_text_size, const char* json_text, size_t value_buffer_size, jsonpl_value_t* value_buffer, size_t* required_value_buffer_size)
{
	size_t value_text_size;
	int value_type = jsonpl_decide_value_type(json_text_size, json_text, &value_text_size);
	int expecting_sub_value;
	int boolean_value;
	double number_value;
	size_t value_size = jsonpl_round_size(sizeof(jsonpl_value_t));
	size_t string_length;
	size_t string_size;
	size_t sub_value_text_size;
	size_t sub_value_count;
	size_t sub_value_size;
	uintptr_t polymorphic_content = (uintptr_t)value_buffer + value_size;
	jsonpl_value_t* sub_value_content;
	const char* end = json_text + value_text_size - 1;
	const char* read = json_text + 1;
	switch (value_type)
	{
		case JSONPL_TYPE_OBJECT:
			sub_value_count = jsonpl_count_object_name_value_pairs(value_text_size, json_text);
			if (sub_value_count == (size_t)~0)
				return 0;
			value_size += jsonpl_round_size(sub_value_count * sizeof(*value_buffer->object.table));
			sub_value_content = (jsonpl_value_t*)((uintptr_t)value_buffer + value_size);
			expecting_sub_value = 1;
			if (value_size <= value_buffer_size)
			{
				value_buffer->object.value_count = sub_value_count;
				*(uintptr_t*)&value_buffer->object.table = polymorphic_content;
			}
			for (size_t sub_object_index = 0; sub_object_index != sub_value_count;)
			{
				char character = *read;
				if (expecting_sub_value)
				{
					if (character == '"')
					{
						size_t name_length;
						size_t value_offset;
						size_t value_lenght;
						size_t name_value_pair_length = jsonpl_name_value_pair_size((size_t)((uintptr_t)end - (uintptr_t)read), read, &name_length, 0, &value_offset, &value_lenght);
						if (!name_value_pair_length || !jsonpl_decode_quoted_string(name_length, read, (value_size <= value_buffer_size) ? (value_buffer_size - value_size) : 0, (char*)sub_value_content, &string_length))
							return 0;
						string_size = jsonpl_round_size(string_length + 1);
						value_size += string_size;
						if (value_size <= value_buffer_size)
						{
							jsonpl_value_t* next_sub_value_content = (jsonpl_value_t*)((uintptr_t)sub_value_content + string_size);
							value_buffer->object.table[sub_object_index].name_length = string_length;
							value_buffer->object.table[sub_object_index].name = (char*)sub_value_content;
							value_buffer->object.table[sub_object_index].value = next_sub_value_content;
							jsonpl_terminator_string((char*)sub_value_content + string_length);
							sub_value_content = next_sub_value_content;
						}
						size_t value_text_lenght = jsonpl_create_tree_from_text(value_lenght, read + value_offset, (value_size <= value_buffer_size) ? (value_buffer_size - value_size) : 0, sub_value_content, &sub_value_size);
						if (!value_text_lenght)
							return 0;
						value_size += sub_value_size;
						sub_value_content = (jsonpl_value_t*)((uintptr_t)sub_value_content + sub_value_size);
						++sub_object_index;
						read += name_value_pair_length;
						expecting_sub_value = 0;
					}
					else if (jsonpl_is_white_space(character))
						++read;
					else
						return 0;
				}
				else
				{
					if (character == ',')
						expecting_sub_value = 1;
					else if (!jsonpl_is_white_space(character))
						return 0;
					++read;
				}
			}
			if (value_size <= value_buffer_size)
			{
				value_buffer->type = value_type;
				value_buffer->size = value_size;
			}
			*required_value_buffer_size = value_size;
			return value_text_size;
		case JSONPL_TYPE_ARRAY:
			sub_value_count = jsonpl_count_array_values(value_text_size, json_text);
			if (sub_value_count == (size_t)~0)
				return 0;
			value_size += jsonpl_round_size(sub_value_count * sizeof(jsonpl_value_t*));
			sub_value_content = (jsonpl_value_t*)((uintptr_t)value_buffer + value_size);
			expecting_sub_value = 1;
			for (size_t sub_object_index = 0; sub_object_index != sub_value_count;)
			{
				char character = *read;
				if (expecting_sub_value)
				{
					if (jsonpl_is_white_space(character))
						++read;
					else
					{
						sub_value_text_size = jsonpl_create_tree_from_text((size_t)((uintptr_t)end - (uintptr_t)read), read, (value_size <= value_buffer_size) ? (value_buffer_size - value_size) : 0, sub_value_content, &sub_value_size);
						if (!sub_value_text_size)
							return 0;
						if (value_size <= value_buffer_size)
						{
							*((jsonpl_value_t**)polymorphic_content + sub_object_index) = sub_value_content;
							sub_value_content = (jsonpl_value_t*)((uintptr_t)sub_value_content + sub_value_size);
						}
						value_size += sub_value_size;
						read += sub_value_text_size;
						++sub_object_index;
						expecting_sub_value = 0;
					}
				}
				else
				{
					if (character == ',')
						expecting_sub_value = 1;
					else if (!jsonpl_is_white_space(character))
						return 0;
					++read;
				}
			}
			if (value_size <= value_buffer_size)
			{
				value_buffer->array.value_count = sub_value_count;
				value_buffer->array.table = (jsonpl_value_t**)polymorphic_content;
				value_buffer->type = value_type;
				value_buffer->size = value_size;
			}
			*required_value_buffer_size = value_size;
			return value_text_size;
		case JSONPL_TYPE_STRING:
			if (!jsonpl_decode_quoted_string(value_text_size, json_text, (value_size <= value_buffer_size) ? (value_buffer_size - value_size) : 0, (char*)polymorphic_content, &string_length))
				return 0;
			value_size += jsonpl_round_size(string_length + 1);
			if (value_size <= value_buffer_size)
			{
				jsonpl_terminator_string((char*)((uintptr_t)polymorphic_content + string_length));
				value_buffer->string.length = string_length;
				value_buffer->string.value = (char*)polymorphic_content;
				value_buffer->type = value_type;
				value_buffer->size = value_size;
			}
			*required_value_buffer_size = value_size;
			return value_text_size;
		case JSONPL_TYPE_NUMBER:
			if (!jsonpl_decode_number(value_text_size, json_text, &number_value))
				return 0;
			if (value_size <= value_buffer_size)
			{
				value_buffer->number_value = number_value;
				value_buffer->type = value_type;
				value_buffer->size = value_size;
			}
			*required_value_buffer_size = value_size;
			return value_text_size;
		case JSONPL_TYPE_BOOLEAN:
			if (!jsonpl_decode_boolean(value_text_size, json_text, &boolean_value))
				return 0;
			if (value_size <= value_buffer_size)
			{
				value_buffer->boolean_value = boolean_value;
				value_buffer->type = value_type;
				value_buffer->size = value_size;
			}
			*required_value_buffer_size = value_size;
			return value_text_size;
		case JSONPL_TYPE_NULL:
			if (value_size <= value_buffer_size)
			{
				value_buffer->type = value_type;
				value_buffer->size = value_size;
			}
			*required_value_buffer_size = value_size;
			return value_text_size;
		default:
			return 0;
	}
}

size_t jsonpl_parse_text(size_t json_text_size, const char* json_text, size_t value_buffer_size, jsonpl_value_t* value_buffer)
{
	size_t beginning_white_space = jsonpl_white_space_length(json_text_size, json_text);
	size_t tree_size;
	if (jsonpl_create_tree_from_text(json_text_size - beginning_white_space, json_text + beginning_white_space, value_buffer_size, value_buffer, &tree_size))
		return tree_size;
	else
		return 0;
}

#ifdef __cplusplus
}
#endif