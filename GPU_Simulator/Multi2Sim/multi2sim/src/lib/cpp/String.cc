/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdarg>
#include <cstring>
#include <sstream>

#include "Misc.h"
#include "String.h"


namespace misc
{


static StringMap string_error_map =
{
	{ "ok", StringErrorOK },
	{ "invalid base", StringErrorBase },
	{ "invalid format", StringErrorFormat },
	{ "integer out of range", StringErrorRange }
};


const char *StringErrorToString(StringError error)
{
	return string_error_map.MapValue(error);
}


std::string fmt(const char *fmt_str, ...)
{
	char buf[1024];
	va_list va;

	va_start(va, fmt_str);
	vsnprintf(buf, sizeof buf, fmt_str, va);
	return buf;
}


void StringTrimLeft(std::string &s, const std::string &set)
{
	size_t startpos = s.find_first_not_of(set);
	if (std::string::npos != startpos)
		s = s.substr(startpos);
}


void StringTrimRight(std::string& s, const std::string &set)
{
	size_t endpos = s.find_last_not_of(set);
	if (std::string::npos != endpos)
		s = s.substr(0, endpos + 1);
}


void StringTrim(std::string &s, const std::string &set)
{
	StringTrimLeft(s, set);
	StringTrimRight(s, set);
}


void StringSingleSpaces(std::string &s, const std::string &set)
{
	int src = 0;
	int dest = 0;
	bool is_space = false;
	bool was_space = false;
	bool started = false;
	for (unsigned i = 0; i < s.length(); i++)
	{
		is_space = StringHasChar(set, s[i]);
		if (is_space)
		{
			src++;
			if (!was_space && started)
				s[dest++] = ' ';
		}
		else
		{
			s[dest++] = s[src++];
			started = true;
		}
		was_space = is_space;
	}

	// Get rid of possible extra space at the end
	if (dest && is_space)
		dest--;

	// Erase trailing characters
	s.erase(dest);
}


void StringToLower(std::string &s)
{
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);
}


void StringToUpper(std::string &s)
{
	std::transform(s.begin(), s.end(), s.begin(), ::toupper);
}


bool StringPrefix(const std::string &s, const std::string &prefix)
{
	return s.length() >= prefix.length() &&
			s.substr(0, prefix.length()) == prefix;
}


bool StringSuffix(const std::string &s, const std::string &suffix)
{
	return s.length() >= suffix.length() &&
			s.substr(s.length() - suffix.length(),
			suffix.length()) == suffix;
}


void StringTokenize(const std::string &s, std::vector<std::string> &tokens,
		const std::string &set)
{
	// Extract tokens
	int token_start = -1;
	std::string token = "";
	for (unsigned i = 0; i <= s.length(); i++)
	{
		// End of string
		bool is_end = i == s.length();

		// Start a token
		if (!is_end && !StringHasChar(set, s[i])
				&& token_start == -1)
			token_start = i;

		// End a token
		if (token_start > -1 && (is_end || StringHasChar(set, s[i])))
		{
			token = s.substr(token_start, i - token_start);
			tokens.push_back(token);
			token_start = -1;
		}
	}
}


int StringDigitToInt(char digit, int base)
{
	StringError error;
	return StringDigitToInt(digit, base, error);
}


int StringDigitToInt(char digit, int base, StringError &error)
{
	int result;

	// Assume no error
	error = StringErrorOK;

	// Check base
	if (base != 2 && base != 8 && base != 10 && base != 16)
	{
		error = StringErrorBase;
		return 0;
	}

	// Parse digit
	result = 0;
	digit = tolower(digit);
	if (digit >= '0' && digit <= '9')
	{
		result = digit - '0';
	}
	else if (digit >= 'a' && digit <= 'f')
	{
		result = digit - 'a' + 10;
	}
	else
	{
		error = StringErrorFormat;
		return 0;
	}

	// Check digit range
	if (result >= base)
	{
		error = StringErrorFormat;
		return 0;
	}

	// Return
	return result;
}


int StringToInt(const std::string &s)
{
	StringError error;
	return StringToInt(s, error);
}


int StringToInt(const std::string &_s, StringError &error)
{
	int sign;
	int base;
	int result;
	int digit;
	int num_digits;
	int factor;

	// Initialize
	std::string s = _s;
	StringTrim(s);
	error = StringErrorOK;

	// Sign
	sign = 1;
	if (s[0] == '+')
	{
		sign = 1;
		s.erase(0, 1);
	}
	else if (s[0] == '-')
	{
		sign = -1;
		s.erase(0, 1);
	}

	// Base
	base = 10;
	if (s.length() >= 2 && s[0] == '0' && s[1] == 'x')
	{
		base = 16;
		s.erase(0, 2);
	}
	else if (s.length() > 1 && s[0] == '0')
	{
		base = 8;
		s.erase(0, 1);
	}

	// Empty string
	if (s.length() == 0)
	{
		error = StringErrorFormat;
		return 0;
	}

	// Suffixes (only for base 10)
	factor = 1;
	assert(s.length() > 0);
	if (base == 10)
	{
		switch (s[s.length() - 1])
		{
		case 'k':
			factor = 1024;
			s.erase(s.length() - 1);
			break;

		case 'K':
			factor = 1000;
			s.erase(s.length() - 1);
			break;

		case 'm':
			factor = 1024 * 1024;
			s.erase(s.length() - 1);
			break;

		case 'M':
			factor = 1000 * 1000;
			s.erase(s.length() - 1);
			break;

		case 'g':
			factor = 1024 * 1024 * 1024;
			s.erase(s.length() - 1);
			break;

		case 'G':
			factor = 1000 * 1000 * 1000;
			s.erase(s.length() - 1);
			break;
		}
	}

	// Remove leading 0s
	while (s.length() && s[0] == '0')
		s.erase(0, 1);
	if (!s.length())
		return 0;

	// Start converting
	result = 0;
	num_digits = 0;
	while (s.length())
	{
		// Get one digit
		digit = StringDigitToInt(s[0], base, error);
		num_digits++;
		if (error)
			return 0;

		// Underflow
		if (sign < 0 && INT_MIN / base > result)
		{
			error = StringErrorRange;
			return 0;
		}

		// Overflow
		if (sign > 0 && UINT_MAX / base < (unsigned int) result)
		{
			error = StringErrorRange;
			return 0;
		}

		// Multiply by base
		result *= base;

		// Underflow
		if (sign < 0 && INT_MIN + digit > result)
		{
			error = StringErrorRange;
			return 0;
		}

		// Overflow
		if (sign > 0 && UINT_MAX - digit < (unsigned int) result)
		{
			error = StringErrorRange;
			return 0;
		}

		// Add digit
		result += digit * sign;

		// Next character
		s.erase(0, 1);
	}

	// Multiplying factor
	if (factor != 1)
	{
		// Underflow
		if (sign < 0 && INT_MIN / factor > result)
		{
			error = StringErrorRange;
			return 0;
		}

		// Overflow
		if (sign > 0 && UINT_MAX / factor < (unsigned int) result)
		{
			error = StringErrorRange;
			return 0;
		}

		// Multiply by factor
		result *= factor;
	}

	// Return
	return result;
}


long long StringToInt64(const std::string &s)
{
	StringError error;
	return StringToInt64(s, error);
}


long long StringToInt64(const std::string &_s, StringError &error)
{
	int sign;
	int base;
	int digit;
	int num_digits;
	int factor;

	long long result;

	// Initialize
	std::string s = _s;
	StringTrim(s);
	error = StringErrorOK;

	// Sign
	sign = 1;
	if (s[0] == '+')
	{
		sign = 1;
		s.erase(0, 1);
	}
	else if (s[0] == '-')
	{
		sign = -1;
		s.erase(0, 1);
	}

	// Base
	base = 10;
	if (s.length() >= 2 && s[0] == '0' && s[1] == 'x')
	{
		base = 16;
		s.erase(0, 2);
	}
	else if (s.length() > 1 && s[0] == '0')
	{
		base = 8;
		s.erase(0, 1);
	}

	// Empty string
	if (s.length() == 0)
	{
		error = StringErrorFormat;
		return 0;
	}

	// Suffixes (only for base 10)
	factor = 1;
	assert(s.length() > 0);
	if (base == 10)
	{
		switch (s[s.length() - 1])
		{
		case 'k':
			factor = 1024;
			s.erase(s.length() - 1);
			break;

		case 'K':
			factor = 1000;
			s.erase(s.length() - 1);
			break;

		case 'm':
			factor = 1024 * 1024;
			s.erase(s.length() - 1);
			break;

		case 'M':
			factor = 1000 * 1000;
			s.erase(s.length() - 1);
			break;

		case 'g':
			factor = 1024 * 1024 * 1024;
			s.erase(s.length() - 1);
			break;

		case 'G':
			factor = 1000 * 1000 * 1000;
			s.erase(s.length() - 1);
			break;
		}
	}

	// Remove leading 0s
	while (s.length() && s[0] == '0')
		s.erase(0, 1);
	if (!s.length())
		return 0;

	// Start converting
	result = 0;
	num_digits = 0;
	while (s.length())
	{
		// Get one digit
		digit = StringDigitToInt(s[0], base, error);
		num_digits++;
		if (error)
			return 0;

		// Underflow
		if (sign < 0 && LLONG_MIN / base > result)
		{
			error = StringErrorRange;
			return 0;
		}

		// Overflow
		if (sign > 0 && ULLONG_MAX / base < (unsigned int) result)
		{
			error = StringErrorRange;
			return 0;
		}

		// Multiply by base
		result *= base;

		// Underflow
		if (sign < 0 && LLONG_MIN + digit > result)
		{
			error = StringErrorRange;
			return 0;
		}

		// Overflow
		if (sign > 0 && ULLONG_MAX - digit < (unsigned int) result)
		{
			error = StringErrorRange;
			return 0;
		}

		// Add digit
		result += digit * sign;

		// Next character
		s.erase(0, 1);
	}

	// Multiplying factor
	if (factor != 1)
	{
		// Underflow
		if (sign < 0 && LLONG_MIN / factor > result)
		{
			error = StringErrorRange;
			return 0;
		}

		// Overflow
		if (sign > 0 && ULLONG_MAX / factor < (unsigned int) result)
		{
			error = StringErrorRange;
			return 0;
		}

		// Multiply by factor
		result *= factor;
	}

	// Return
	return result;
}


std::string StringParagraph(const std::string &text,
		int indent, int first_indent, int width)
{
	std::string word;
	bool first_word_in_line = true;
	bool first_word_in_paragraph = true;

	// Check good values for 'first_indent' and 'indent'
	if (first_indent >= width || indent >= width)
		panic("%s: invalid indentation values", __FUNCTION__);

	// Add initial indentation
	std::string output = std::string(first_indent, ' ');
	int line_length = first_indent;

	// Process text
	char last = 0;
	char curr = 0;
	for (int i = 0; i <= (int) text.length(); i++)
	{
		// Current character
		last = curr;
		curr = i == (int) text.length() ? 0 : text[i];

		// New paragraph
		if (curr == '\n' && last == '\n' && !first_word_in_paragraph)
		{
			first_word_in_paragraph = true;
			first_word_in_line = true;
			output += "\n\n";
			output += std::string(indent, ' ');
			line_length = indent;
		}

		// Space-like character
		if (curr == ' ' || curr == '\t' || curr == '\n' || !curr)
		{
			// No new word, ignore
			if (!word.length())
				continue;

			// If word doesn't fit, add new line
			if (line_length + 1 + (int) word.length() > width)
			{
				output += '\n';
				output += std::string(indent, ' ');
				line_length = indent;
				first_word_in_line = true;
			}

			// Split word in chunks that fit in line
			while (line_length + (int) word.length() > width)
			{
				output += word.substr(0, width - line_length);
				output += '\n';
				word.erase(0, width - line_length);
				output += std::string(indent, ' ');
				line_length = indent;
				first_word_in_line = true;
			}
			
			// Space if needed
			if (!first_word_in_line)
			{
				output += ' ';
				line_length++;
			}

			// Rest of the word
			first_word_in_line = false;
			first_word_in_paragraph = false;
			output += word;
			line_length += word.length();
			word.clear();
			continue;
		}

		// Other character just gets added to the current word
		word += curr;
	}

	// Final output
	output += '\n';
	return output;
}


std::string StringBinaryBuffer(char *buffer, int size, int truncate)
{
	// Truncate size to
	bool truncated = false;
	if (size > truncate && truncate)
	{
		size = truncate;
		truncated = true;
	}

	// Create output
	std::string result;
	while (size)
	{
		unsigned char c = *buffer;
		if (c >= 32)
			result += c;
		else if (!c)
			result += "\\0";
		else if (c == '\n')
			result += "\\n";
		else if (c == '\t')
			result += "\\t";
		else
			result += misc::fmt("\\%02x", c);

		// Next byte
		buffer++;
		size--;
	}

	// Ellipsis if truncated;
	if (truncated)
		result += "...";
	
	// Return
	return result;
}




//
// StringMap Class
//


void StringMap::Dump(std::ostream &os) const
{
	os << '{';
	std::string sep;
	for (auto &item : items)
	{
		if (!item.text)
			continue;
		os << sep << item.text;
		sep = ",";
	}
	os << '}';
}

std::string StringMap::toString() const
{
	std::ostringstream ss;
	ss << *this;
	return ss.str();
}
	

const char *StringMap::MapValue(int value, bool &error) const
{
	// Find value
	error = false;
	for (auto &item : items)
		if (item.value == value)
			return item.text;

	// Not found
	error = true;
	return "<unknown>";
}

int StringMap::MapString(const std::string &s, bool &error) const
{
	// Find string
	error = false;
	for (auto &item : items)
		if (item.text == s)
			return item.value;
	
	// Not found
	error = true;
	return 0;
}

int StringMap::MapStringCase(const std::string &s, bool &error) const
{
	// Find string
	error = false;
	for (auto &item : items)
		if (!strcasecmp(item.text, s.c_str()))
			return item.value;
	
	// Not found
	error = true;
	return 0;
}


std::string StringMap::MapFlags(unsigned flags) const
{
	std::stringstream s;
	std::string comma;
	s << '{';
	for (int i = 0; i < 32; i++)
	{
		if (flags & (1 << i))
		{
			s << comma;
			bool error;
			const char *text = MapValue(1 << i, error);
			if (error)
				s << (1 << i);
			else
				s << text;
			comma = "|";
		}
	}
	s << '}';

	// Return created text
	return s.str();
}


}  // namespace Misc

