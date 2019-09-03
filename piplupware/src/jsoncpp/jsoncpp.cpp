/// Json-cpp amalgated source (http://jsoncpp.sourceforge.net/).
/// It is intended to be used with #include "json/json.h"

// //////////////////////////////////////////////////////////////////////
// Beginning of content of file: LICENSE
// //////////////////////////////////////////////////////////////////////

/*
The JsonCpp library's source code, including accompanying documentation,
tests and demonstration applications, are licensed under the following
conditions...

The author (Baptiste Lepilleur) explicitly disclaims copyright in all
jurisdictions which recognize such a disclaimer. In such jurisdictions,
this software is released into the Public Domain.

In jurisdictions which do not recognize Public Domain property (e.g. Germany as of
2010), this software is Copyright (c) 2007-2010 by Baptiste Lepilleur, and is
released under the terms of the MIT License (see below).

In jurisdictions which recognize Public Domain property, the user of this
software may choose to accept it either as 1) Public Domain, 2) under the
conditions of the MIT License (see below), or 3) under the terms of dual
Public Domain/MIT License conditions described here, as they choose.

The MIT License is about as close to Public Domain as a license can get, and is
described in clear, concise terms at:

   http://en.wikipedia.org/wiki/MIT_License

The full text of the MIT License follows:

========================================================================
Copyright (c) 2007-2010 Baptiste Lepilleur

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
========================================================================
(END LICENSE TEXT)

The MIT license is compatible with both the GPL and commercial
software, affording one all of the rights of Public Domain with the
minor nuisance of being required to keep the above copyright notice
and license text in the source code. Note also that by accepting the
Public Domain "license" you can re-license your copy using whatever
license you like.

*/

// //////////////////////////////////////////////////////////////////////
// End of content of file: LICENSE
// //////////////////////////////////////////////////////////////////////






#include "json.h"

#ifndef JSON_IS_AMALGAMATION
#error "Compile with -I PATH_TO_JSON_DIRECTORY"
#endif


// //////////////////////////////////////////////////////////////////////
// Beginning of content of file: src/lib_json/json_tool.h
// //////////////////////////////////////////////////////////////////////

// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef LIB_JSONCPP_JSON_TOOL_H_INCLUDED
#define LIB_JSONCPP_JSON_TOOL_H_INCLUDED

#ifndef NO_LOCALE_SUPPORT
#include <clocale>
#endif

/* This header provides common string manipulation support, such as UTF-8,
 * portable conversion from/to string...
 *
 * It is an internal header that must not be exposed.
 */

namespace Json {
	static char getDecimalPoint() {
#ifdef NO_LOCALE_SUPPORT
		return '\0';
#else
		struct lconv* lc = localeconv();
		return lc ? *(lc->decimal_point) : '\0';
#endif
	}

	/// Converts a unicode code-point to UTF-8.
	static inline JSONCPP_STRING codePointToUTF8(unsigned int cp) {
		JSONCPP_STRING result;

		// based on description from http://en.wikipedia.org/wiki/UTF-8

		if (cp <= 0x7f) {
			result.resize(1);
			result[0] = static_cast<char>(cp);
		}
		else if (cp <= 0x7FF) {
			result.resize(2);
			result[1] = static_cast<char>(0x80 | (0x3f & cp));
			result[0] = static_cast<char>(0xC0 | (0x1f & (cp >> 6)));
		}
		else if (cp <= 0xFFFF) {
			result.resize(3);
			result[2] = static_cast<char>(0x80 | (0x3f & cp));
			result[1] = static_cast<char>(0x80 | (0x3f & (cp >> 6)));
			result[0] = static_cast<char>(0xE0 | (0xf & (cp >> 12)));
		}
		else if (cp <= 0x10FFFF) {
			result.resize(4);
			result[3] = static_cast<char>(0x80 | (0x3f & cp));
			result[2] = static_cast<char>(0x80 | (0x3f & (cp >> 6)));
			result[1] = static_cast<char>(0x80 | (0x3f & (cp >> 12)));
			result[0] = static_cast<char>(0xF0 | (0x7 & (cp >> 18)));
		}

		return result;
	}

	/// Returns true if ch is a control character (in range [1,31]).
	static inline bool isControlCharacter(char ch) { return ch > 0 && ch <= 0x1F; }

	enum {
		/// Constant that specify the size of the buffer that must be passed to
		/// uintToString.
		uintToStringBufferSize = 3 * sizeof(LargestUInt) + 1
	};

	// Defines a char buffer for use with uintToString().
	typedef char UIntToStringBuffer[uintToStringBufferSize];

	/** Converts an unsigned integer to string.
	 * @param value Unsigned interger to convert to string
	 * @param current Input/Output string buffer.
	 *        Must have at least uintToStringBufferSize chars free.
	 */
	static inline void uintToString(LargestUInt value, char*& current) {
		*--current = 0;
		do {
			*--current = static_cast<char>(value % 10U + static_cast<unsigned>('0'));
			value /= 10;
		} while (value != 0);
	}

	/** Change ',' to '.' everywhere in buffer.
	 *
	 * We had a sophisticated way, but it did not work in WinCE.
	 * @see https://github.com/open-source-parsers/jsoncpp/pull/9
	 */
	static inline void fixNumericLocale(char* begin, char* end) {
		while (begin < end) {
			if (*begin == ',') {
				*begin = '.';
			}
			++begin;
		}
	}

	static inline void fixNumericLocaleInput(char* begin, char* end) {
		char decimalPoint = getDecimalPoint();
		if (decimalPoint != '\0' && decimalPoint != '.') {
			while (begin < end) {
				if (*begin == '.') {
					*begin = decimalPoint;
				}
				++begin;
			}
		}
	}

} // namespace Json {

#endif // LIB_JSONCPP_JSON_TOOL_H_INCLUDED

// //////////////////////////////////////////////////////////////////////
// End of content of file: src/lib_json/json_tool.h
// //////////////////////////////////////////////////////////////////////






// //////////////////////////////////////////////////////////////////////
// Beginning of content of file: src/lib_json/json_reader.cpp
// //////////////////////////////////////////////////////////////////////

// Copyright 2007-2011 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#if !defined(JSON_IS_AMALGAMATION)
#include <json/assertions.h>
#include <json/reader.h>
#include <json/value.h>
#include "json_tool.h"
#endif // if !defined(JSON_IS_AMALGAMATION)
#include <utility>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <istream>
#include <sstream>
#include <memory>
#include <set>
#include <limits>

#if defined(_MSC_VER)
#if !defined(WINCE) && defined(__STDC_SECURE_LIB__) && _MSC_VER >= 1500 // VC++ 9.0 and above 
#define snprintf sprintf_s
#elif _MSC_VER >= 1900 // VC++ 14.0 and above
#define snprintf std::snprintf
#else
#define snprintf _snprintf
#endif
#elif defined(__ANDROID__) || defined(__QNXNTO__)
#define snprintf snprintf
#elif __cplusplus >= 201103L
#if !defined(__MINGW32__) && !defined(__CYGWIN__)
#define snprintf std::snprintf
#endif
#endif

#if defined(__QNXNTO__)
#define sscanf std::sscanf
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1400 // VC++ 8.0
// Disable warning about strdup being deprecated.
#pragma warning(disable : 4996)
#endif

static int const stackLimit_g = 1000;
static int       stackDepth_g = 0;  // see readValue()

namespace Json {

#if __cplusplus >= 201103L || (defined(_CPPLIB_VER) && _CPPLIB_VER >= 520)
	typedef std::unique_ptr<CharReader> CharReaderPtr;
#else
	typedef std::auto_ptr<CharReader>   CharReaderPtr;
#endif

	// Implementation of class Features
	// ////////////////////////////////

	Features::Features()
		: allowComments_(true), strictRoot_(false),
		allowDroppedNullPlaceholders_(false), allowNumericKeys_(false) {}

	Features Features::all() { return Features(); }

	Features Features::strictMode() {
		Features features;
		features.allowComments_ = false;
		features.strictRoot_ = true;
		features.allowDroppedNullPlaceholders_ = false;
		features.allowNumericKeys_ = false;
		return features;
	}

	// Implementation of class Reader
	// ////////////////////////////////

	static bool containsNewLine(Reader::Location begin, Reader::Location end) {
		for (; begin < end; ++begin)
			if (*begin == '\n' || *begin == '\r')
				return true;
		return false;
	}

	// Class Reader
	// //////////////////////////////////////////////////////////////////

	Reader::Reader()
		: errors_(), document_(), begin_(), end_(), current_(), lastValueEnd_(),
		lastValue_(), commentsBefore_(), features_(Features::all()),
		collectComments_() {}

	Reader::Reader(const Features& features)
		: errors_(), document_(), begin_(), end_(), current_(), lastValueEnd_(),
		lastValue_(), commentsBefore_(), features_(features), collectComments_() {
	}

	bool
		Reader::parse(const std::string& document, Value& root, bool collectComments) {
		JSONCPP_STRING documentCopy(document.data(), document.data() + document.capacity());
		std::swap(documentCopy, document_);
		const char* begin = document_.c_str();
		const char* end = begin + document_.length();
		return parse(begin, end, root, collectComments);
	}

	bool Reader::parse(std::istream& sin, Value& root, bool collectComments) {
		// std::istream_iterator<char> begin(sin);
		// std::istream_iterator<char> end;
		// Those would allow streamed input from a file, if parse() were a
		// template function.

		// Since JSONCPP_STRING is reference-counted, this at least does not
		// create an extra copy.
		JSONCPP_STRING doc;
		std::getline(sin, doc, (char)EOF);
		return parse(doc.data(), doc.data() + doc.size(), root, collectComments);
	}

	bool Reader::parse(const char* beginDoc,
		const char* endDoc,
		Value& root,
		bool collectComments) {
		if (!features_.allowComments_) {
			collectComments = false;
		}

		begin_ = beginDoc;
		end_ = endDoc;
		collectComments_ = collectComments;
		current_ = begin_;
		lastValueEnd_ = 0;
		lastValue_ = 0;
		commentsBefore_ = "";
		errors_.clear();
		while (!nodes_.empty())
			nodes_.pop();
		nodes_.push(&root);

		stackDepth_g = 0;  // Yes, this is bad coding, but options are limited.
		bool successful = readValue();
		Token token;
		skipCommentTokens(token);
		if (collectComments_ && !commentsBefore_.empty())
			root.setComment(commentsBefore_, commentAfter);
		if (features_.strictRoot_) {
			if (!root.isArray() && !root.isObject()) {
				// Set error location to start of doc, ideally should be first token found
				// in doc
				token.type_ = tokenError;
				token.start_ = beginDoc;
				token.end_ = endDoc;
				addError(
					"A valid JSON document must be either an array or an object value.",
					token);
				return false;
			}
		}
		return successful;
	}

	bool Reader::readValue() {
		// This is a non-reentrant way to support a stackLimit. Terrible!
		// But this deprecated class has a security problem: Bad input can
		// cause a seg-fault. This seems like a fair, binary-compatible way
		// to prevent the problem.
		if (stackDepth_g >= stackLimit_g) throwRuntimeError("Exceeded stackLimit in readValue().");
		++stackDepth_g;

		Token token;
		skipCommentTokens(token);
		bool successful = true;

		if (collectComments_ && !commentsBefore_.empty()) {
			currentValue().setComment(commentsBefore_, commentBefore);
			commentsBefore_ = "";
		}

		switch (token.type_) {
		case tokenObjectBegin:
			successful = readObject(token);
			currentValue().setOffsetLimit(current_ - begin_);
			break;
		case tokenArrayBegin:
			successful = readArray(token);
			currentValue().setOffsetLimit(current_ - begin_);
			break;
		case tokenNumber:
			successful = decodeNumber(token);
			break;
		case tokenString:
			successful = decodeString(token);
			break;
		case tokenTrue:
		{
			Value v(true);
			currentValue().swapPayload(v);
			currentValue().setOffsetStart(token.start_ - begin_);
			currentValue().setOffsetLimit(token.end_ - begin_);
		}
		break;
		case tokenFalse:
		{
			Value v(false);
			currentValue().swapPayload(v);
			currentValue().setOffsetStart(token.start_ - begin_);
			currentValue().setOffsetLimit(token.end_ - begin_);
		}
		break;
		case tokenNull:
		{
			Value v;
			currentValue().swapPayload(v);
			currentValue().setOffsetStart(token.start_ - begin_);
			currentValue().setOffsetLimit(token.end_ - begin_);
		}
		break;
		case tokenArraySeparator:
		case tokenObjectEnd:
		case tokenArrayEnd:
			if (features_.allowDroppedNullPlaceholders_) {
				// "Un-read" the current token and mark the current value as a null
				// token.
				current_--;
				Value v;
				currentValue().swapPayload(v);
				currentValue().setOffsetStart(current_ - begin_ - 1);
				currentValue().setOffsetLimit(current_ - begin_);
				break;
			} // Else, fall through...
		default:
			currentValue().setOffsetStart(token.start_ - begin_);
			currentValue().setOffsetLimit(token.end_ - begin_);
			return addError("Syntax error: value, object or array expected.", token);
		}

		if (collectComments_) {
			lastValueEnd_ = current_;
			lastValue_ = &currentValue();
		}

		--stackDepth_g;
		return successful;
	}

	void Reader::skipCommentTokens(Token& token) {
		if (features_.allowComments_) {
			do {
				readToken(token);
			} while (token.type_ == tokenComment);
		}
		else {
			readToken(token);
		}
	}

	bool Reader::readToken(Token& token) {
		skipSpaces();
		token.start_ = current_;
		Char c = getNextChar();
		bool ok = true;
		switch (c) {
		case '{':
			token.type_ = tokenObjectBegin;
			break;
		case '}':
			token.type_ = tokenObjectEnd;
			break;
		case '[':
			token.type_ = tokenArrayBegin;
			break;
		case ']':
			token.type_ = tokenArrayEnd;
			break;
		case '"':
			token.type_ = tokenString;
			ok = readString();
			break;
		case '/':
			token.type_ = tokenComment;
			ok = readComment();
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '-':
			token.type_ = tokenNumber;
			readNumber();
			break;
		case 't':
			token.type_ = tokenTrue;
			ok = match("rue", 3);
			break;
		case 'f':
			token.type_ = tokenFalse;
			ok = match("alse", 4);
			break;
		case 'n':
			token.type_ = tokenNull;
			ok = match("ull", 3);
			break;
		case ',':
			token.type_ = tokenArraySeparator;
			break;
		case ':':
			token.type_ = tokenMemberSeparator;
			break;
		case 0:
			token.type_ = tokenEndOfStream;
			break;
		default:
			ok = false;
			break;
		}
		if (!ok)
			token.type_ = tokenError;
		token.end_ = current_;
		return true;
	}

	void Reader::skipSpaces() {
		while (current_ != end_) {
			Char c = *current_;
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
				++current_;
			else
				break;
		}
	}

	bool Reader::match(Location pattern, int patternLength) {
		if (end_ - current_ < patternLength)
			return false;
		int index = patternLength;
		while (index--)
			if (current_[index] != pattern[index])
				return false;
		current_ += patternLength;
		return true;
	}

	bool Reader::readComment() {
		Location commentBegin = current_ - 1;
		Char c = getNextChar();
		bool successful = false;
		if (c == '*')
			successful = readCStyleComment();
		else if (c == '/')
			successful = readCppStyleComment();
		if (!successful)
			return false;

		if (collectComments_) {
			CommentPlacement placement = commentBefore;
			if (lastValueEnd_ && !containsNewLine(lastValueEnd_, commentBegin)) {
				if (c != '*' || !containsNewLine(commentBegin, current_))
					placement = commentAfterOnSameLine;
			}

			addComment(commentBegin, current_, placement);
		}
		return true;
	}

	static JSONCPP_STRING normalizeEOL(Reader::Location begin, Reader::Location end) {
		JSONCPP_STRING normalized;
		normalized.reserve(static_cast<size_t>(end - begin));
		Reader::Location current = begin;
		while (current != end) {
			char c = *current++;
			if (c == '\r') {
				if (current != end && *current == '\n')
					// convert dos EOL
					++current;
				// convert Mac EOL
				normalized += '\n';
			}
			else {
				normalized += c;
			}
		}
		return normalized;
	}

	void
		Reader::addComment(Location begin, Location end, CommentPlacement placement) {
		assert(collectComments_);
		const JSONCPP_STRING& normalized = normalizeEOL(begin, end);
		if (placement == commentAfterOnSameLine) {
			assert(lastValue_ != 0);
			lastValue_->setComment(normalized, placement);
		}
		else {
			commentsBefore_ += normalized;
		}
	}

	bool Reader::readCStyleComment() {
		while ((current_ + 1) < end_) {
			Char c = getNextChar();
			if (c == '*' && *current_ == '/')
				break;
		}
		return getNextChar() == '/';
	}

	bool Reader::readCppStyleComment() {
		while (current_ != end_) {
			Char c = getNextChar();
			if (c == '\n')
				break;
			if (c == '\r') {
				// Consume DOS EOL. It will be normalized in addComment.
				if (current_ != end_ && *current_ == '\n')
					getNextChar();
				// Break on Moc OS 9 EOL.
				break;
			}
		}
		return true;
	}

	void Reader::readNumber() {
		const char* p = current_;
		char c = '0'; // stopgap for already consumed character
		// integral part
		while (c >= '0' && c <= '9')
			c = (current_ = p) < end_ ? *p++ : '\0';
		// fractional part
		if (c == '.') {
			c = (current_ = p) < end_ ? *p++ : '\0';
			while (c >= '0' && c <= '9')
				c = (current_ = p) < end_ ? *p++ : '\0';
		}
		// exponential part
		if (c == 'e' || c == 'E') {
			c = (current_ = p) < end_ ? *p++ : '\0';
			if (c == '+' || c == '-')
				c = (current_ = p) < end_ ? *p++ : '\0';
			while (c >= '0' && c <= '9')
				c = (current_ = p) < end_ ? *p++ : '\0';
		}
	}

	bool Reader::readString() {
		Char c = '\0';
		while (current_ != end_) {
			c = getNextChar();
			if (c == '\\')
				getNextChar();
			else if (c == '"')
				break;
		}
		return c == '"';
	}

	bool Reader::readObject(Token& tokenStart) {
		Token tokenName;
		JSONCPP_STRING name;
		Value init(objectValue);
		currentValue().swapPayload(init);
		currentValue().setOffsetStart(tokenStart.start_ - begin_);
		while (readToken(tokenName)) {
			bool initialTokenOk = true;
			while (tokenName.type_ == tokenComment && initialTokenOk)
				initialTokenOk = readToken(tokenName);
			if (!initialTokenOk)
				break;
			if (tokenName.type_ == tokenObjectEnd && name.empty()) // empty object
				return true;
			name = "";
			if (tokenName.type_ == tokenString) {
				if (!decodeString(tokenName, name))
					return recoverFromError(tokenObjectEnd);
			}
			else if (tokenName.type_ == tokenNumber && features_.allowNumericKeys_) {
				Value numberName;
				if (!decodeNumber(tokenName, numberName))
					return recoverFromError(tokenObjectEnd);
				name = JSONCPP_STRING(numberName.asCString());
			}
			else {
				break;
			}

			Token colon;
			if (!readToken(colon) || colon.type_ != tokenMemberSeparator) {
				return addErrorAndRecover(
					"Missing ':' after object member name", colon, tokenObjectEnd);
			}
			Value& value = currentValue()[name];
			nodes_.push(&value);
			bool ok = readValue();
			nodes_.pop();
			if (!ok) // error already set
				return recoverFromError(tokenObjectEnd);

			Token comma;
			if (!readToken(comma) ||
				(comma.type_ != tokenObjectEnd && comma.type_ != tokenArraySeparator &&
					comma.type_ != tokenComment)) {
				return addErrorAndRecover(
					"Missing ',' or '}' in object declaration", comma, tokenObjectEnd);
			}
			bool finalizeTokenOk = true;
			while (comma.type_ == tokenComment && finalizeTokenOk)
				finalizeTokenOk = readToken(comma);
			if (comma.type_ == tokenObjectEnd)
				return true;
		}
		return addErrorAndRecover(
			"Missing '}' or object member name", tokenName, tokenObjectEnd);
	}

	bool Reader::readArray(Token& tokenStart) {
		Value init(arrayValue);
		currentValue().swapPayload(init);
		currentValue().setOffsetStart(tokenStart.start_ - begin_);
		skipSpaces();
		if (current_ != end_ && *current_ == ']') // empty array
		{
			Token endArray;
			readToken(endArray);
			return true;
		}
		int index = 0;
		for (;;) {
			Value& value = currentValue()[index++];
			nodes_.push(&value);
			bool ok = readValue();
			nodes_.pop();
			if (!ok) // error already set
				return recoverFromError(tokenArrayEnd);

			Token token;
			// Accept Comment after last item in the array.
			ok = readToken(token);
			while (token.type_ == tokenComment && ok) {
				ok = readToken(token);
			}
			bool badTokenType =
				(token.type_ != tokenArraySeparator && token.type_ != tokenArrayEnd);
			if (!ok || badTokenType) {
				return addErrorAndRecover(
					"Missing ',' or ']' in array declaration", token, tokenArrayEnd);
			}
			if (token.type_ == tokenArrayEnd)
				break;
		}
		return true;
	}

	bool Reader::decodeNumber(Token& token) {
		Value decoded;
		if (!decodeNumber(token, decoded))
			return false;
		currentValue().swapPayload(decoded);
		currentValue().setOffsetStart(token.start_ - begin_);
		currentValue().setOffsetLimit(token.end_ - begin_);
		return true;
	}

	bool Reader::decodeNumber(Token& token, Value& decoded) {
		// Attempts to parse the number as an integer. If the number is
		// larger than the maximum supported value of an integer then
		// we decode the number as a double.
		Location current = token.start_;
		bool isNegative = *current == '-';
		if (isNegative)
			++current;
		// TODO: Help the compiler do the div and mod at compile time or get rid of them.
		Value::LargestUInt maxIntegerValue =
			isNegative ? Value::LargestUInt(Value::maxLargestInt) + 1
			: Value::maxLargestUInt;
		Value::LargestUInt threshold = maxIntegerValue / 10;
		Value::LargestUInt value = 0;
		while (current < token.end_) {
			Char c = *current++;
			if (c < '0' || c > '9')
				return decodeDouble(token, decoded);
			Value::UInt digit(static_cast<Value::UInt>(c - '0'));
			if (value >= threshold) {
				// We've hit or exceeded the max value divided by 10 (rounded down). If
				// a) we've only just touched the limit, b) this is the last digit, and
				// c) it's small enough to fit in that rounding delta, we're okay.
				// Otherwise treat this number as a double to avoid overflow.
				if (value > threshold || current != token.end_ ||
					digit > maxIntegerValue % 10) {
					return decodeDouble(token, decoded);
				}
			}
			value = value * 10 + digit;
		}
		if (isNegative && value == maxIntegerValue)
			decoded = Value::minLargestInt;
		else if (isNegative)
			decoded = -Value::LargestInt(value);
		else if (value <= Value::LargestUInt(Value::maxInt))
			decoded = Value::LargestInt(value);
		else
			decoded = value;
		return true;
	}

	bool Reader::decodeDouble(Token& token) {
		Value decoded;
		if (!decodeDouble(token, decoded))
			return false;
		currentValue().swapPayload(decoded);
		currentValue().setOffsetStart(token.start_ - begin_);
		currentValue().setOffsetLimit(token.end_ - begin_);
		return true;
	}

	bool Reader::decodeDouble(Token& token, Value& decoded) {
		double value = 0;
		JSONCPP_STRING buffer(token.start_, token.end_);
		JSONCPP_ISTRINGSTREAM is(buffer);
		if (!(is >> value))
			return addError("'" + JSONCPP_STRING(token.start_, token.end_) +
				"' is not a number.",
				token);
		decoded = value;
		return true;
	}

	bool Reader::decodeString(Token& token) {
		JSONCPP_STRING decoded_string;
		if (!decodeString(token, decoded_string))
			return false;
		Value decoded(decoded_string);
		currentValue().swapPayload(decoded);
		currentValue().setOffsetStart(token.start_ - begin_);
		currentValue().setOffsetLimit(token.end_ - begin_);
		return true;
	}

	bool Reader::decodeString(Token& token, JSONCPP_STRING& decoded) {
		decoded.reserve(static_cast<size_t>(token.end_ - token.start_ - 2));
		Location current = token.start_ + 1; // skip '"'
		Location end = token.end_ - 1;       // do not include '"'
		while (current != end) {
			Char c = *current++;
			if (c == '"')
				break;
			else if (c == '\\') {
				if (current == end)
					return addError("Empty escape sequence in string", token, current);
				Char escape = *current++;
				switch (escape) {
				case '"':
					decoded += '"';
					break;
				case '/':
					decoded += '/';
					break;
				case '\\':
					decoded += '\\';
					break;
				case 'b':
					decoded += '\b';
					break;
				case 'f':
					decoded += '\f';
					break;
				case 'n':
					decoded += '\n';
					break;
				case 'r':
					decoded += '\r';
					break;
				case 't':
					decoded += '\t';
					break;
				case 'u': {
					unsigned int unicode;
					if (!decodeUnicodeCodePoint(token, current, end, unicode))
						return false;
					decoded += codePointToUTF8(unicode);
				} break;
				default:
					return addError("Bad escape sequence in string", token, current);
				}
			}
			else {
				decoded += c;
			}
		}
		return true;
	}

	bool Reader::decodeUnicodeCodePoint(Token& token,
		Location& current,
		Location end,
		unsigned int& unicode) {

		if (!decodeUnicodeEscapeSequence(token, current, end, unicode))
			return false;
		if (unicode >= 0xD800 && unicode <= 0xDBFF) {
			// surrogate pairs
			if (end - current < 6)
				return addError(
					"additional six characters expected to parse unicode surrogate pair.",
					token,
					current);
			unsigned int surrogatePair;
			if (*(current++) == '\\' && *(current++) == 'u') {
				if (decodeUnicodeEscapeSequence(token, current, end, surrogatePair)) {
					unicode = 0x10000 + ((unicode & 0x3FF) << 10) + (surrogatePair & 0x3FF);
				}
				else
					return false;
			}
			else
				return addError("expecting another \\u token to begin the second half of "
					"a unicode surrogate pair",
					token,
					current);
		}
		return true;
	}

	bool Reader::decodeUnicodeEscapeSequence(Token& token,
		Location& current,
		Location end,
		unsigned int& ret_unicode) {
		if (end - current < 4)
			return addError(
				"Bad unicode escape sequence in string: four digits expected.",
				token,
				current);
		int unicode = 0;
		for (int index = 0; index < 4; ++index) {
			Char c = *current++;
			unicode *= 16;
			if (c >= '0' && c <= '9')
				unicode += c - '0';
			else if (c >= 'a' && c <= 'f')
				unicode += c - 'a' + 10;
			else if (c >= 'A' && c <= 'F')
				unicode += c - 'A' + 10;
			else
				return addError(
					"Bad unicode escape sequence in string: hexadecimal digit expected.",
					token,
					current);
		}
		ret_unicode = static_cast<unsigned int>(unicode);
		return true;
	}

	bool
		Reader::addError(const JSONCPP_STRING& message, Token& token, Location extra) {
		ErrorInfo info;
		info.token_ = token;
		info.message_ = message;
		info.extra_ = extra;
		errors_.push_back(info);
		return false;
	}

	bool Reader::recoverFromError(TokenType skipUntilToken) {
		size_t const errorCount = errors_.size();
		Token skip;
		for (;;) {
			if (!readToken(skip))
				errors_.resize(errorCount); // discard errors caused by recovery
			if (skip.type_ == skipUntilToken || skip.type_ == tokenEndOfStream)
				break;
		}
		errors_.resize(errorCount);
		return false;
	}

	bool Reader::addErrorAndRecover(const JSONCPP_STRING& message,
		Token& token,
		TokenType skipUntilToken) {
		addError(message, token);
		return recoverFromError(skipUntilToken);
	}

	Value& Reader::currentValue() { return *(nodes_.top()); }

	Reader::Char Reader::getNextChar() {
		if (current_ == end_)
			return 0;
		return *current_++;
	}

	void Reader::getLocationLineAndColumn(Location location,
		int& line,
		int& column) const {
		Location current = begin_;
		Location lastLineStart = current;
		line = 0;
		while (current < location && current != end_) {
			Char c = *current++;
			if (c == '\r') {
				if (*current == '\n')
					++current;
				lastLineStart = current;
				++line;
			}
			else if (c == '\n') {
				lastLineStart = current;
				++line;
			}
		}
		// column & line start at 1
		column = int(location - lastLineStart) + 1;
		++line;
	}

	JSONCPP_STRING Reader::getLocationLineAndColumn(Location location) const {
		int line, column;
		getLocationLineAndColumn(location, line, column);
		char buffer[18 + 16 + 16 + 1];
		snprintf(buffer, sizeof(buffer), "Line %d, Column %d", line, column);
		return buffer;
	}

	// Deprecated. Preserved for backward compatibility
	JSONCPP_STRING Reader::getFormatedErrorMessages() const {
		return getFormattedErrorMessages();
	}

	JSONCPP_STRING Reader::getFormattedErrorMessages() const {
		JSONCPP_STRING formattedMessage;
		for (Errors::const_iterator itError = errors_.begin();
			itError != errors_.end();
			++itError) {
			const ErrorInfo& error = *itError;
			formattedMessage +=
				"* " + getLocationLineAndColumn(error.token_.start_) + "\n";
			formattedMessage += "  " + error.message_ + "\n";
			if (error.extra_)
				formattedMessage +=
				"See " + getLocationLineAndColumn(error.extra_) + " for detail.\n";
		}
		return formattedMessage;
	}

	std::vector<Reader::StructuredError> Reader::getStructuredErrors() const {
		std::vector<Reader::StructuredError> allErrors;
		for (Errors::const_iterator itError = errors_.begin();
			itError != errors_.end();
			++itError) {
			const ErrorInfo& error = *itError;
			Reader::StructuredError structured;
			structured.offset_start = error.token_.start_ - begin_;
			structured.offset_limit = error.token_.end_ - begin_;
			structured.message = error.message_;
			allErrors.push_back(structured);
		}
		return allErrors;
	}

	bool Reader::pushError(const Value& value, const JSONCPP_STRING& message) {
		ptrdiff_t const length = end_ - begin_;
		if (value.getOffsetStart() > length
			|| value.getOffsetLimit() > length)
			return false;
		Token token;
		token.type_ = tokenError;
		token.start_ = begin_ + value.getOffsetStart();
		token.end_ = end_ + value.getOffsetLimit();
		ErrorInfo info;
		info.token_ = token;
		info.message_ = message;
		info.extra_ = 0;
		errors_.push_back(info);
		return true;
	}

	bool Reader::pushError(const Value& value, const JSONCPP_STRING& message, const Value& extra) {
		ptrdiff_t const length = end_ - begin_;
		if (value.getOffsetStart() > length
			|| value.getOffsetLimit() > length
			|| extra.getOffsetLimit() > length)
			return false;
		Token token;
		token.type_ = tokenError;
		token.start_ = begin_ + value.getOffsetStart();
		token.end_ = begin_ + value.getOffsetLimit();
		ErrorInfo info;
		info.token_ = token;
		info.message_ = message;
		info.extra_ = begin_ + extra.getOffsetStart();
		errors_.push_back(info);
		return true;
	}

	bool Reader::good() const {
		return !errors_.size();
	}

	// exact copy of Features
	class OurFeatures {
	public:
		static OurFeatures all();
		bool allowComments_;
		bool strictRoot_;
		bool allowDroppedNullPlaceholders_;
		bool allowNumericKeys_;
		bool allowSingleQuotes_;
		bool failIfExtra_;
		bool rejectDupKeys_;
		bool allowSpecialFloats_;
		int stackLimit_;
	};  // OurFeatures

	// exact copy of Implementation of class Features
	// ////////////////////////////////

	OurFeatures OurFeatures::all() { return OurFeatures(); }

	// Implementation of class Reader
	// ////////////////////////////////

	// exact copy of Reader, renamed to OurReader
	class OurReader {
	public:
		typedef char Char;
		typedef const Char* Location;
		struct StructuredError {
			ptrdiff_t offset_start;
			ptrdiff_t offset_limit;
			JSONCPP_STRING message;
		};

		OurReader(OurFeatures const& features);
		bool parse(const char* beginDoc,
			const char* endDoc,
			Value& root,
			bool collectComments = true);
		JSONCPP_STRING getFormattedErrorMessages() const;
		std::vector<StructuredError> getStructuredErrors() const;
		bool pushError(const Value& value, const JSONCPP_STRING& message);
		bool pushError(const Value& value, const JSONCPP_STRING& message, const Value& extra);
		bool good() const;

	private:
		OurReader(OurReader const&);  // no impl
		void operator=(OurReader const&);  // no impl

		enum TokenType {
			tokenEndOfStream = 0,
			tokenObjectBegin,
			tokenObjectEnd,
			tokenArrayBegin,
			tokenArrayEnd,
			tokenString,
			tokenNumber,
			tokenTrue,
			tokenFalse,
			tokenNull,
			tokenNaN,
			tokenPosInf,
			tokenNegInf,
			tokenArraySeparator,
			tokenMemberSeparator,
			tokenComment,
			tokenError
		};

		class Token {
		public:
			TokenType type_;
			Location start_;
			Location end_;
		};

		class ErrorInfo {
		public:
			Token token_;
			JSONCPP_STRING message_;
			Location extra_;
		};

		typedef std::deque<ErrorInfo> Errors;

		bool readToken(Token& token);
		void skipSpaces();
		bool match(Location pattern, int patternLength);
		bool readComment();
		bool readCStyleComment();
		bool readCppStyleComment();
		bool readString();
		bool readStringSingleQuote();
		bool readNumber(bool checkInf);
		bool readValue();
		bool readObject(Token& token);
		bool readArray(Token& token);
		bool decodeNumber(Token& token);
		bool decodeNumber(Token& token, Value& decoded);
		bool decodeString(Token& token);
		bool decodeString(Token& token, JSONCPP_STRING& decoded);
		bool decodeDouble(Token& token);
		bool decodeDouble(Token& token, Value& decoded);
		bool decodeUnicodeCodePoint(Token& token,
			Location& current,
			Location end,
			unsigned int& unicode);
		bool decodeUnicodeEscapeSequence(Token& token,
			Location& current,
			Location end,
			unsigned int& unicode);
		bool addError(const JSONCPP_STRING& message, Token& token, Location extra = 0);
		bool recoverFromError(TokenType skipUntilToken);
		bool addErrorAndRecover(const JSONCPP_STRING& message,
			Token& token,
			TokenType skipUntilToken);
		void skipUntilSpace();
		Value& currentValue();
		Char getNextChar();
		void
			getLocationLineAndColumn(Location location, int& line, int& column) const;
		JSONCPP_STRING getLocationLineAndColumn(Location location) const;
		void addComment(Location begin, Location end, CommentPlacement placement);
		void skipCommentTokens(Token& token);

		typedef std::stack<Value*> Nodes;
		Nodes nodes_;
		Errors errors_;
		JSONCPP_STRING document_;
		Location begin_;
		Location end_;
		Location current_;
		Location lastValueEnd_;
		Value* lastValue_;
		JSONCPP_STRING commentsBefore_;
		int stackDepth_;

		OurFeatures const features_;
		bool collectComments_;
	};  // OurReader

	// complete copy of Read impl, for OurReader

	OurReader::OurReader(OurFeatures const& features)
		: errors_(), document_(), begin_(), end_(), current_(), lastValueEnd_(),
		lastValue_(), commentsBefore_(),
		stackDepth_(0),
		features_(features), collectComments_() {
	}

	bool OurReader::parse(const char* beginDoc,
		const char* endDoc,
		Value& root,
		bool collectComments) {
		if (!features_.allowComments_) {
			collectComments = false;
		}

		begin_ = beginDoc;
		end_ = endDoc;
		collectComments_ = collectComments;
		current_ = begin_;
		lastValueEnd_ = 0;
		lastValue_ = 0;
		commentsBefore_ = "";
		errors_.clear();
		while (!nodes_.empty())
			nodes_.pop();
		nodes_.push(&root);

		stackDepth_ = 0;
		bool successful = readValue();
		Token token;
		skipCommentTokens(token);
		if (features_.failIfExtra_) {
			if ((features_.strictRoot_ || token.type_ != tokenError) && token.type_ != tokenEndOfStream) {
				addError("Extra non-whitespace after JSON value.", token);
				return false;
			}
		}
		if (collectComments_ && !commentsBefore_.empty())
			root.setComment(commentsBefore_, commentAfter);
		if (features_.strictRoot_) {
			if (!root.isArray() && !root.isObject()) {
				// Set error location to start of doc, ideally should be first token found
				// in doc
				token.type_ = tokenError;
				token.start_ = beginDoc;
				token.end_ = endDoc;
				addError(
					"A valid JSON document must be either an array or an object value.",
					token);
				return false;
			}
		}
		return successful;
	}

	bool OurReader::readValue() {
		if (stackDepth_ >= features_.stackLimit_) throwRuntimeError("Exceeded stackLimit in readValue().");
		++stackDepth_;
		Token token;
		skipCommentTokens(token);
		bool successful = true;

		if (collectComments_ && !commentsBefore_.empty()) {
			currentValue().setComment(commentsBefore_, commentBefore);
			commentsBefore_ = "";
		}

		switch (token.type_) {
		case tokenObjectBegin:
			successful = readObject(token);
			currentValue().setOffsetLimit(current_ - begin_);
			break;
		case tokenArrayBegin:
			successful = readArray(token);
			currentValue().setOffsetLimit(current_ - begin_);
			break;
		case tokenNumber:
			successful = decodeNumber(token);
			break;
		case tokenString:
			successful = decodeString(token);
			break;
		case tokenTrue:
		{
			Value v(true);
			currentValue().swapPayload(v);
			currentValue().setOffsetStart(token.start_ - begin_);
			currentValue().setOffsetLimit(token.end_ - begin_);
		}
		break;
		case tokenFalse:
		{
			Value v(false);
			currentValue().swapPayload(v);
			currentValue().setOffsetStart(token.start_ - begin_);
			currentValue().setOffsetLimit(token.end_ - begin_);
		}
		break;
		case tokenNull:
		{
			Value v;
			currentValue().swapPayload(v);
			currentValue().setOffsetStart(token.start_ - begin_);
			currentValue().setOffsetLimit(token.end_ - begin_);
		}
		break;
		case tokenNaN:
		{
			Value v(std::numeric_limits<double>::quiet_NaN());
			currentValue().swapPayload(v);
			currentValue().setOffsetStart(token.start_ - begin_);
			currentValue().setOffsetLimit(token.end_ - begin_);
		}
		break;
		case tokenPosInf:
		{
			Value v(std::numeric_limits<double>::infinity());
			currentValue().swapPayload(v);
			currentValue().setOffsetStart(token.start_ - begin_);
			currentValue().setOffsetLimit(token.end_ - begin_);
		}
		break;
		case tokenNegInf:
		{
			Value v(-std::numeric_limits<double>::infinity());
			currentValue().swapPayload(v);
			currentValue().setOffsetStart(token.start_ - begin_);
			currentValue().setOffsetLimit(token.end_ - begin_);
		}
		break;
		case tokenArraySeparator:
		case tokenObjectEnd:
		case tokenArrayEnd:
			if (features_.allowDroppedNullPlaceholders_) {
				// "Un-read" the current token and mark the current value as a null
				// token.
				current_--;
				Value v;
				currentValue().swapPayload(v);
				currentValue().setOffsetStart(current_ - begin_ - 1);
				currentValue().setOffsetLimit(current_ - begin_);
				break;
			} // else, fall through ...
		default:
			currentValue().setOffsetStart(token.start_ - begin_);
			currentValue().setOffsetLimit(token.end_ - begin_);
			return addError("Syntax error: value, object or array expected.", token);
		}

		if (collectComments_) {
			lastValueEnd_ = current_;
			lastValue_ = &currentValue();
		}

		--stackDepth_;
		return successful;
	}

	void OurReader::skipCommentTokens(Token& token) {
		if (features_.allowComments_) {
			do {
				readToken(token);
			} while (token.type_ == tokenComment);
		}
		else {
			readToken(token);
		}
	}

	bool OurReader::readToken(Token& token) {
		skipSpaces();
		token.start_ = current_;
		Char c = getNextChar();
		bool ok = true;
		switch (c) {
		case '{':
			token.type_ = tokenObjectBegin;
			break;
		case '}':
			token.type_ = tokenObjectEnd;
			break;
		case '[':
			token.type_ = tokenArrayBegin;
			break;
		case ']':
			token.type_ = tokenArrayEnd;
			break;
		case '"':
			token.type_ = tokenString;
			ok = readString();
			break;
		case '\'':
			if (features_.allowSingleQuotes_) {
				token.type_ = tokenString;
				ok = readStringSingleQuote();
				break;
			} // else continue
		case '/':
			token.type_ = tokenComment;
			ok = readComment();
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			token.type_ = tokenNumber;
			readNumber(false);
			break;
		case '-':
			if (readNumber(true)) {
				token.type_ = tokenNumber;
			}
			else {
				token.type_ = tokenNegInf;
				ok = features_.allowSpecialFloats_ && match("nfinity", 7);
			}
			break;
		case 't':
			token.type_ = tokenTrue;
			ok = match("rue", 3);
			break;
		case 'f':
			token.type_ = tokenFalse;
			ok = match("alse", 4);
			break;
		case 'n':
			token.type_ = tokenNull;
			ok = match("ull", 3);
			break;
		case 'N':
			if (features_.allowSpecialFloats_) {
				token.type_ = tokenNaN;
				ok = match("aN", 2);
			}
			else {
				ok = false;
			}
			break;
		case 'I':
			if (features_.allowSpecialFloats_) {
				token.type_ = tokenPosInf;
				ok = match("nfinity", 7);
			}
			else {
				ok = false;
			}
			break;
		case ',':
			token.type_ = tokenArraySeparator;
			break;
		case ':':
			token.type_ = tokenMemberSeparator;
			break;
		case 0:
			token.type_ = tokenEndOfStream;
			break;
		default:
			ok = false;
			break;
		}
		if (!ok)
			token.type_ = tokenError;
		token.end_ = current_;
		return true;
	}

	void OurReader::skipSpaces() {
		while (current_ != end_) {
			Char c = *current_;
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
				++current_;
			else
				break;
		}
	}

	bool OurReader::match(Location pattern, int patternLength) {
		if (end_ - current_ < patternLength)
			return false;
		int index = patternLength;
		while (index--)
			if (current_[index] != pattern[index])
				return false;
		current_ += patternLength;
		return true;
	}

	bool OurReader::readComment() {
		Location commentBegin = current_ - 1;
		Char c = getNextChar();
		bool successful = false;
		if (c == '*')
			successful = readCStyleComment();
		else if (c == '/')
			successful = readCppStyleComment();
		if (!successful)
			return false;

		if (collectComments_) {
			CommentPlacement placement = commentBefore;
			if (lastValueEnd_ && !containsNewLine(lastValueEnd_, commentBegin)) {
				if (c != '*' || !containsNewLine(commentBegin, current_))
					placement = commentAfterOnSameLine;
			}

			addComment(commentBegin, current_, placement);
		}
		return true;
	}

	void
		OurReader::addComment(Location begin, Location end, CommentPlacement placement) {
		assert(collectComments_);
		const JSONCPP_STRING& normalized = normalizeEOL(begin, end);
		if (placement == commentAfterOnSameLine) {
			assert(lastValue_ != 0);
			lastValue_->setComment(normalized, placement);
		}
		else {
			commentsBefore_ += normalized;
		}
	}

	bool OurReader::readCStyleComment() {
		while ((current_ + 1) < end_) {
			Char c = getNextChar();
			if (c == '*' && *current_ == '/')
				break;
		}
		return getNextChar() == '/';
	}

	bool OurReader::readCppStyleComment() {
		while (current_ != end_) {
			Char c = getNextChar();
			if (c == '\n')
				break;
			if (c == '\r') {
				// Consume DOS EOL. It will be normalized in addComment.
				if (current_ != end_ && *current_ == '\n')
					getNextChar();
				// Break on Moc OS 9 EOL.
				break;
			}
		}
		return true;
	}

	bool OurReader::readNumber(bool checkInf) {
		const char* p = current_;
		if (checkInf && p != end_ && *p == 'I') {
			current_ = ++p;
			return false;
		}
		char c = '0'; // stopgap for already consumed character
		// integral part
		while (c >= '0' && c <= '9')
			c = (current_ = p) < end_ ? *p++ : '\0';
		// fractional part
		if (c == '.') {
			c = (current_ = p) < end_ ? *p++ : '\0';
			while (c >= '0' && c <= '9')
				c = (current_ = p) < end_ ? *p++ : '\0';
		}
		// exponential part
		if (c == 'e' || c == 'E') {
			c = (current_ = p) < end_ ? *p++ : '\0';
			if (c == '+' || c == '-')
				c = (current_ = p) < end_ ? *p++ : '\0';
			while (c >= '0' && c <= '9')
				c = (current_ = p) < end_ ? *p++ : '\0';
		}
		return true;
	}
	bool OurReader::readString() {
		Char c = 0;
		while (current_ != end_) {
			c = getNextChar();
			if (c == '\\')
				getNextChar();
			else if (c == '"')
				break;
		}
		return c == '"';
	}


	bool OurReader::readStringSingleQuote() {
		Char c = 0;
		while (current_ != end_) {
			c = getNextChar();
			if (c == '\\')
				getNextChar();
			else if (c == '\'')
				break;
		}
		return c == '\'';
	}

	bool OurReader::readObject(Token& tokenStart) {
		Token tokenName;
		JSONCPP_STRING name;
		Value init(objectValue);
		currentValue().swapPayload(init);
		currentValue().setOffsetStart(tokenStart.start_ - begin_);
		while (readToken(tokenName)) {
			bool initialTokenOk = true;
			while (tokenName.type_ == tokenComment && initialTokenOk)
				initialTokenOk = readToken(tokenName);
			if (!initialTokenOk)
				break;
			if (tokenName.type_ == tokenObjectEnd && name.empty()) // empty object
				return true;
			name = "";
			if (tokenName.type_ == tokenString) {
				if (!decodeString(tokenName, name))
					return recoverFromError(tokenObjectEnd);
			}
			else if (tokenName.type_ == tokenNumber && features_.allowNumericKeys_) {
				Value numberName;
				if (!decodeNumber(tokenName, numberName))
					return recoverFromError(tokenObjectEnd);
				name = numberName.asString();
			}
			else {
				break;
			}

			Token colon;
			if (!readToken(colon) || colon.type_ != tokenMemberSeparator) {
				return addErrorAndRecover(
					"Missing ':' after object member name", colon, tokenObjectEnd);
			}
			if (name.length() >= (1U << 30)) throwRuntimeError("keylength >= 2^30");
			if (features_.rejectDupKeys_ && currentValue().isMember(name)) {
				JSONCPP_STRING msg = "Duplicate key: '" + name + "'";
				return addErrorAndRecover(
					msg, tokenName, tokenObjectEnd);
			}
			Value& value = currentValue()[name];
			nodes_.push(&value);
			bool ok = readValue();
			nodes_.pop();
			if (!ok) // error already set
				return recoverFromError(tokenObjectEnd);

			Token comma;
			if (!readToken(comma) ||
				(comma.type_ != tokenObjectEnd && comma.type_ != tokenArraySeparator &&
					comma.type_ != tokenComment)) {
				return addErrorAndRecover(
					"Missing ',' or '}' in object declaration", comma, tokenObjectEnd);
			}
			bool finalizeTokenOk = true;
			while (comma.type_ == tokenComment && finalizeTokenOk)
				finalizeTokenOk = readToken(comma);
			if (comma.type_ == tokenObjectEnd)
				return true;
		}
		return addErrorAndRecover(
			"Missing '}' or object member name", tokenName, tokenObjectEnd);
	}

	bool OurReader::readArray(Token& tokenStart) {
		Value init(arrayValue);
		currentValue().swapPayload(init);
		currentValue().setOffsetStart(tokenStart.start_ - begin_);
		skipSpaces();
		if (current_ != end_ && *current_ == ']') // empty array
		{
			Token endArray;
			readToken(endArray);
			return true;
		}
		int index = 0;
		for (;;) {
			Value& value = currentValue()[index++];
			nodes_.push(&value);
			bool ok = readValue();
			nodes_.pop();
			if (!ok) // error already set
				return recoverFromError(tokenArrayEnd);

			Token token;
			// Accept Comment after last item in the array.
			ok = readToken(token);
			while (token.type_ == tokenComment && ok) {
				ok = readToken(token);
			}
			bool badTokenType =
				(token.type_ != tokenArraySeparator && token.type_ != tokenArrayEnd);
			if (!ok || badTokenType) {
				return addErrorAndRecover(
					"Missing ',' or ']' in array declaration", token, tokenArrayEnd);
			}
			if (token.type_ == tokenArrayEnd)
				break;
		}
		return true;
	}

	bool OurReader::decodeNumber(Token& token) {
		Value decoded;
		if (!decodeNumber(token, decoded))
			return false;
		currentValue().swapPayload(decoded);
		currentValue().setOffsetStart(token.start_ - begin_);
		currentValue().setOffsetLimit(token.end_ - begin_);
		return true;
	}

	bool OurReader::decodeNumber(Token& token, Value& decoded) {
		// Attempts to parse the number as an integer. If the number is
		// larger than the maximum supported value of an integer then
		// we decode the number as a double.
		Location current = token.start_;
		bool isNegative = *current == '-';
		if (isNegative)
			++current;
		// TODO: Help the compiler do the div and mod at compile time or get rid of them.
		Value::LargestUInt maxIntegerValue =
			isNegative ? Value::LargestUInt(-Value::minLargestInt)
			: Value::maxLargestUInt;
		Value::LargestUInt threshold = maxIntegerValue / 10;
		Value::LargestUInt value = 0;
		while (current < token.end_) {
			Char c = *current++;
			if (c < '0' || c > '9')
				return decodeDouble(token, decoded);
			Value::UInt digit(static_cast<Value::UInt>(c - '0'));
			if (value >= threshold) {
				// We've hit or exceeded the max value divided by 10 (rounded down). If
				// a) we've only just touched the limit, b) this is the last digit, and
				// c) it's small enough to fit in that rounding delta, we're okay.
				// Otherwise treat this number as a double to avoid overflow.
				if (value > threshold || current != token.end_ ||
					digit > maxIntegerValue % 10) {
					return decodeDouble(token, decoded);
				}
			}
			value = value * 10 + digit;
		}
		if (isNegative)
			decoded = -Value::LargestInt(value);
		else if (value <= Value::LargestUInt(Value::maxInt))
			decoded = Value::LargestInt(value);
		else
			decoded = value;
		return true;
	}

	bool OurReader::decodeDouble(Token& token) {
		Value decoded;
		if (!decodeDouble(token, decoded))
			return false;
		currentValue().swapPayload(decoded);
		currentValue().setOffsetStart(token.start_ - begin_);
		currentValue().setOffsetLimit(token.end_ - begin_);
		return true;
	}

	bool OurReader::decodeDouble(Token& token, Value& decoded) {
		double value = 0;
		const int bufferSize = 32;
		int count;
		ptrdiff_t const length = token.end_ - token.start_;

		// Sanity check to avoid buffer overflow exploits.
		if (length < 0) {
			return addError("Unable to parse token length", token);
		}
		size_t const ulength = static_cast<size_t>(length);

		// Avoid using a string constant for the format control string given to
		// sscanf, as this can cause hard to debug crashes on OS X. See here for more
		// info:
		//
		//     http://developer.apple.com/library/mac/#DOCUMENTATION/DeveloperTools/gcc-4.0.1/gcc/Incompatibilities.html
		char format[] = "%lf";

		if (length <= bufferSize) {
			Char buffer[bufferSize + 1];
			memcpy(buffer, token.start_, ulength);
			buffer[length] = 0;
			fixNumericLocaleInput(buffer, buffer + length);
			count = sscanf(buffer, format, &value);
		}
		else {
			JSONCPP_STRING buffer(token.start_, token.end_);
			count = sscanf(buffer.c_str(), format, &value);
		}

		if (count != 1)
			return addError("'" + JSONCPP_STRING(token.start_, token.end_) +
				"' is not a number.",
				token);
		decoded = value;
		return true;
	}

	bool OurReader::decodeString(Token& token) {
		JSONCPP_STRING decoded_string;
		if (!decodeString(token, decoded_string))
			return false;
		Value decoded(decoded_string);
		currentValue().swapPayload(decoded);
		currentValue().setOffsetStart(token.start_ - begin_);
		currentValue().setOffsetLimit(token.end_ - begin_);
		return true;
	}

	bool OurReader::decodeString(Token& token, JSONCPP_STRING& decoded) {
		decoded.reserve(static_cast<size_t>(token.end_ - token.start_ - 2));
		Location current = token.start_ + 1; // skip '"'
		Location end = token.end_ - 1;       // do not include '"'
		while (current != end) {
			Char c = *current++;
			if (c == '"')
				break;
			else if (c == '\\') {
				if (current == end)
					return addError("Empty escape sequence in string", token, current);
				Char escape = *current++;
				switch (escape) {
				case '"':
					decoded += '"';
					break;
				case '/':
					decoded += '/';
					break;
				case '\\':
					decoded += '\\';
					break;
				case 'b':
					decoded += '\b';
					break;
				case 'f':
					decoded += '\f';
					break;
				case 'n':
					decoded += '\n';
					break;
				case 'r':
					decoded += '\r';
					break;
				case 't':
					decoded += '\t';
					break;
				case 'u': {
					unsigned int unicode;
					if (!decodeUnicodeCodePoint(token, current, end, unicode))
						return false;
					decoded += codePointToUTF8(unicode);
				} break;
				default:
					return addError("Bad escape sequence in string", token, current);
				}
			}
			else {
				decoded += c;
			}
		}
		return true;
	}

	bool OurReader::decodeUnicodeCodePoint(Token& token,
		Location& current,
		Location end,
		unsigned int& unicode) {

		if (!decodeUnicodeEscapeSequence(token, current, end, unicode))
			return false;
		if (unicode >= 0xD800 && unicode <= 0xDBFF) {
			// surrogate pairs
			if (end - current < 6)
				return addError(
					"additional six characters expected to parse unicode surrogate pair.",
					token,
					current);
			unsigned int surrogatePair;
			if (*(current++) == '\\' && *(current++) == 'u') {
				if (decodeUnicodeEscapeSequence(token, current, end, surrogatePair)) {
					unicode = 0x10000 + ((unicode & 0x3FF) << 10) + (surrogatePair & 0x3FF);
				}
				else
					return false;
			}
			else
				return addError("expecting another \\u token to begin the second half of "
					"a unicode surrogate pair",
					token,
					current);
		}
		return true;
	}

	bool OurReader::decodeUnicodeEscapeSequence(Token& token,
		Location& current,
		Location end,
		unsigned int& ret_unicode) {
		if (end - current < 4)
			return addError(
				"Bad unicode escape sequence in string: four digits expected.",
				token,
				current);
		int unicode = 0;
		for (int index = 0; index < 4; ++index) {
			Char c = *current++;
			unicode *= 16;
			if (c >= '0' && c <= '9')
				unicode += c - '0';
			else if (c >= 'a' && c <= 'f')
				unicode += c - 'a' + 10;
			else if (c >= 'A' && c <= 'F')
				unicode += c - 'A' + 10;
			else
				return addError(
					"Bad unicode escape sequence in string: hexadecimal digit expected.",
					token,
					current);
		}
		ret_unicode = static_cast<unsigned int>(unicode);
		return true;
	}

	bool
		OurReader::addError(const JSONCPP_STRING& message, Token& token, Location extra) {
		ErrorInfo info;
		info.token_ = token;
		info.message_ = message;
		info.extra_ = extra;
		errors_.push_back(info);
		return false;
	}

	bool OurReader::recoverFromError(TokenType skipUntilToken) {
		size_t errorCount = errors_.size();
		Token skip;
		for (;;) {
			if (!readToken(skip))
				errors_.resize(errorCount); // discard errors caused by recovery
			if (skip.type_ == skipUntilToken || skip.type_ == tokenEndOfStream)
				break;
		}
		errors_.resize(errorCount);
		return false;
	}

	bool OurReader::addErrorAndRecover(const JSONCPP_STRING& message,
		Token& token,
		TokenType skipUntilToken) {
		addError(message, token);
		return recoverFromError(skipUntilToken);
	}

	Value& OurReader::currentValue() { return *(nodes_.top()); }

	OurReader::Char OurReader::getNextChar() {
		if (current_ == end_)
			return 0;
		return *current_++;
	}

	void OurReader::getLocationLineAndColumn(Location location,
		int& line,
		int& column) const {
		Location current = begin_;
		Location lastLineStart = current;
		line = 0;
		while (current < location && current != end_) {
			Char c = *current++;
			if (c == '\r') {
				if (*current == '\n')
					++current;
				lastLineStart = current;
				++line;
			}
			else if (c == '\n') {
				lastLineStart = current;
				++line;
			}
		}
		// column & line start at 1
		column = int(location - lastLineStart) + 1;
		++line;
	}

	JSONCPP_STRING OurReader::getLocationLineAndColumn(Location location) const {
		int line, column;
		getLocationLineAndColumn(location, line, column);
		char buffer[18 + 16 + 16 + 1];
		snprintf(buffer, sizeof(buffer), "Line %d, Column %d", line, column);
		return buffer;
	}

	JSONCPP_STRING OurReader::getFormattedErrorMessages() const {
		JSONCPP_STRING formattedMessage;
		for (Errors::const_iterator itError = errors_.begin();
			itError != errors_.end();
			++itError) {
			const ErrorInfo& error = *itError;
			formattedMessage +=
				"* " + getLocationLineAndColumn(error.token_.start_) + "\n";
			formattedMessage += "  " + error.message_ + "\n";
			if (error.extra_)
				formattedMessage +=
				"See " + getLocationLineAndColumn(error.extra_) + " for detail.\n";
		}
		return formattedMessage;
	}

	std::vector<OurReader::StructuredError> OurReader::getStructuredErrors() const {
		std::vector<OurReader::StructuredError> allErrors;
		for (Errors::const_iterator itError = errors_.begin();
			itError != errors_.end();
			++itError) {
			const ErrorInfo& error = *itError;
			OurReader::StructuredError structured;
			structured.offset_start = error.token_.start_ - begin_;
			structured.offset_limit = error.token_.end_ - begin_;
			structured.message = error.message_;
			allErrors.push_back(structured);
		}
		return allErrors;
	}

	bool OurReader::pushError(const Value& value, const JSONCPP_STRING& message) {
		ptrdiff_t length = end_ - begin_;
		if (value.getOffsetStart() > length
			|| value.getOffsetLimit() > length)
			return false;
		Token token;
		token.type_ = tokenError;
		token.start_ = begin_ + value.getOffsetStart();
		token.end_ = end_ + value.getOffsetLimit();
		ErrorInfo info;
		info.token_ = token;
		info.message_ = message;
		info.extra_ = 0;
		errors_.push_back(info);
		return true;
	}

	bool OurReader::pushError(const Value& value, const JSONCPP_STRING& message, const Value& extra) {
		ptrdiff_t length = end_ - begin_;
		if (value.getOffsetStart() > length
			|| value.getOffsetLimit() > length
			|| extra.getOffsetLimit() > length)
			return false;
		Token token;
		token.type_ = tokenError;
		token.start_ = begin_ + value.getOffsetStart();
		token.end_ = begin_ + value.getOffsetLimit();
		ErrorInfo info;
		info.token_ = token;
		info.message_ = message;
		info.extra_ = begin_ + extra.getOffsetStart();
		errors_.push_back(info);
		return true;
	}

	bool OurReader::good() const {
		return !errors_.size();
	}


	class OurCharReader : public CharReader {
		bool const collectComments_;
		OurReader reader_;
	public:
		OurCharReader(
			bool collectComments,
			OurFeatures const& features)
			: collectComments_(collectComments)
			, reader_(features)
		{}
		bool parse(
			char const* beginDoc, char const* endDoc,
			Value* root, JSONCPP_STRING* errs) JSONCPP_OVERRIDE {
			bool ok = reader_.parse(beginDoc, endDoc, *root, collectComments_);
			if (errs) {
				*errs = reader_.getFormattedErrorMessages();
			}
			return ok;
		}
	};

	CharReaderBuilder::CharReaderBuilder()
	{
		setDefaults(&settings_);
	}
	CharReaderBuilder::~CharReaderBuilder()
	{}
	CharReader* CharReaderBuilder::newCharReader() const
	{
		bool collectComments = settings_["collectComments"].asBool();
		OurFeatures features = OurFeatures::all();
		features.allowComments_ = settings_["allowComments"].asBool();
		features.strictRoot_ = settings_["strictRoot"].asBool();
		features.allowDroppedNullPlaceholders_ = settings_["allowDroppedNullPlaceholders"].asBool();
		features.allowNumericKeys_ = settings_["allowNumericKeys"].asBool();
		features.allowSingleQuotes_ = settings_["allowSingleQuotes"].asBool();
		features.stackLimit_ = settings_["stackLimit"].asInt();
		features.failIfExtra_ = settings_["failIfExtra"].asBool();
		features.rejectDupKeys_ = settings_["rejectDupKeys"].asBool();
		features.allowSpecialFloats_ = settings_["allowSpecialFloats"].asBool();
		return new OurCharReader(collectComments, features);
	}
	static void getValidReaderKeys(std::set<JSONCPP_STRING>* valid_keys)
	{
		valid_keys->clear();
		valid_keys->insert("collectComments");
		valid_keys->insert("allowComments");
		valid_keys->insert("strictRoot");
		valid_keys->insert("allowDroppedNullPlaceholders");
		valid_keys->insert("allowNumericKeys");
		valid_keys->insert("allowSingleQuotes");
		valid_keys->insert("stackLimit");
		valid_keys->insert("failIfExtra");
		valid_keys->insert("rejectDupKeys");
		valid_keys->insert("allowSpecialFloats");
	}
	bool CharReaderBuilder::validate(Json::Value* invalid) const
	{
		Json::Value my_invalid;
		if (!invalid) invalid = &my_invalid;  // so we do not need to test for NULL
		Json::Value& inv = *invalid;
		std::set<JSONCPP_STRING> valid_keys;
		getValidReaderKeys(&valid_keys);
		Value::Members keys = settings_.getMemberNames();
		size_t n = keys.size();
		for (size_t i = 0; i < n; ++i) {
			JSONCPP_STRING const& key = keys[i];
			if (valid_keys.find(key) == valid_keys.end()) {
				inv[key] = settings_[key];
			}
		}
		return 0u == inv.size();
	}
	Value& CharReaderBuilder::operator[](JSONCPP_STRING key)
	{
		return settings_[key];
	}
	// static
	void CharReaderBuilder::strictMode(Json::Value* settings)
	{
		//! [CharReaderBuilderStrictMode]
		(*settings)["allowComments"] = false;
		(*settings)["strictRoot"] = true;
		(*settings)["allowDroppedNullPlaceholders"] = false;
		(*settings)["allowNumericKeys"] = false;
		(*settings)["allowSingleQuotes"] = false;
		(*settings)["stackLimit"] = 1000;
		(*settings)["failIfExtra"] = true;
		(*settings)["rejectDupKeys"] = true;
		(*settings)["allowSpecialFloats"] = false;
		//! [CharReaderBuilderStrictMode]
	}
	// static
	void CharReaderBuilder::setDefaults(Json::Value* settings)
	{
		//! [CharReaderBuilderDefaults]
		(*settings)["collectComments"] = true;
		(*settings)["allowComments"] = true;
		(*settings)["strictRoot"] = false;
		(*settings)["allowDroppedNullPlaceholders"] = false;
		(*settings)["allowNumericKeys"] = false;
		(*settings)["allowSingleQuotes"] = false;
		(*settings)["stackLimit"] = 1000;
		(*settings)["failIfExtra"] = false;
		(*settings)["rejectDupKeys"] = false;
		(*settings)["allowSpecialFloats"] = false;
		//! [CharReaderBuilderDefaults]
	}

	//////////////////////////////////
	// global functions

	bool parseFromStream(
		CharReader::Factory const& fact, JSONCPP_ISTREAM& sin,
		Value* root, JSONCPP_STRING* errs)
	{
		JSONCPP_OSTRINGSTREAM ssin;
		ssin << sin.rdbuf();
		JSONCPP_STRING doc = ssin.str();
		char const* begin = doc.data();
		char const* end = begin + doc.size();
		// Note that we do not actually need a null-terminator.
		CharReaderPtr const reader(fact.newCharReader());
		return reader->parse(begin, end, root, errs);
	}

	JSONCPP_ISTREAM& operator>>(JSONCPP_ISTREAM& sin, Value& root) {
		CharReaderBuilder b;
		JSONCPP_STRING errs;
		bool ok = parseFromStream(b, sin, &root, &errs);
		if (!ok) {
			fprintf(stderr,
				"Error from reader: %s",
				errs.c_str());

			throwRuntimeError(errs);
		}
		return sin;
	}

} // namespace Json

// //////////////////////////////////////////////////////////////////////
// End of content of file: src/lib_json/json_reader.cpp
// //////////////////////////////////////////////////////////////////////






// //////////////////////////////////////////////////////////////////////
// Beginning of content of file: src/lib_json/json_valueiterator.inl
// //////////////////////////////////////////////////////////////////////

// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

// included by json_value.cpp

namespace Json {

	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// class ValueIteratorBase
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////

	ValueIteratorBase::ValueIteratorBase()
		: current_(), isNull_(true) {
	}

	ValueIteratorBase::ValueIteratorBase(
		const Value::ObjectValues::iterator& current)
		: current_(current), isNull_(false) {}

	Value& ValueIteratorBase::deref() const {
		return current_->second;
	}

	void ValueIteratorBase::increment() {
		++current_;
	}

	void ValueIteratorBase::decrement() {
		--current_;
	}

	ValueIteratorBase::difference_type
		ValueIteratorBase::computeDistance(const SelfType& other) const {
#ifdef JSON_USE_CPPTL_SMALLMAP
		return other.current_ - current_;
#else
		// Iterator for null value are initialized using the default
		// constructor, which initialize current_ to the default
		// std::map::iterator. As begin() and end() are two instance
		// of the default std::map::iterator, they can not be compared.
		// To allow this, we handle this comparison specifically.
		if (isNull_ && other.isNull_) {
			return 0;
		}

		// Usage of std::distance is not portable (does not compile with Sun Studio 12
		// RogueWave STL,
		// which is the one used by default).
		// Using a portable hand-made version for non random iterator instead:
		//   return difference_type( std::distance( current_, other.current_ ) );
		difference_type myDistance = 0;
		for (Value::ObjectValues::iterator it = current_; it != other.current_;
			++it) {
			++myDistance;
		}
		return myDistance;
#endif
	}

	bool ValueIteratorBase::isEqual(const SelfType& other) const {
		if (isNull_) {
			return other.isNull_;
		}
		return current_ == other.current_;
	}

	void ValueIteratorBase::copy(const SelfType& other) {
		current_ = other.current_;
		isNull_ = other.isNull_;
	}

	Value ValueIteratorBase::key() const {
		const Value::CZString czstring = (*current_).first;
		if (czstring.data()) {
			if (czstring.isStaticString())
				return Value(StaticString(czstring.data()));
			return Value(czstring.data(), czstring.data() + czstring.length());
		}
		return Value(czstring.index());
	}

	UInt ValueIteratorBase::index() const {
		const Value::CZString czstring = (*current_).first;
		if (!czstring.data())
			return czstring.index();
		return Value::UInt(-1);
	}

	JSONCPP_STRING ValueIteratorBase::name() const {
		char const* keey;
		char const* end;
		keey = memberName(&end);
		if (!keey) return JSONCPP_STRING();
		return JSONCPP_STRING(keey, end);
	}

	char const* ValueIteratorBase::memberName() const {
		const char* cname = (*current_).first.data();
		return cname ? cname : "";
	}

	char const* ValueIteratorBase::memberName(char const** end) const {
		const char* cname = (*current_).first.data();
		if (!cname) {
			*end = NULL;
			return NULL;
		}
		*end = cname + (*current_).first.length();
		return cname;
	}

	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// class ValueConstIterator
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////

	ValueConstIterator::ValueConstIterator() {}

	ValueConstIterator::ValueConstIterator(
		const Value::ObjectValues::iterator& current)
		: ValueIteratorBase(current) {}

	ValueConstIterator::ValueConstIterator(ValueIterator const& other)
		: ValueIteratorBase(other) {}

	ValueConstIterator& ValueConstIterator::
		operator=(const ValueIteratorBase& other) {
		copy(other);
		return *this;
	}

	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// class ValueIterator
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////

	ValueIterator::ValueIterator() {}

	ValueIterator::ValueIterator(const Value::ObjectValues::iterator& current)
		: ValueIteratorBase(current) {}

	ValueIterator::ValueIterator(const ValueConstIterator& other)
		: ValueIteratorBase(other) {
		throwRuntimeError("ConstIterator to Iterator should never be allowed.");
	}

	ValueIterator::ValueIterator(const ValueIterator& other)
		: ValueIteratorBase(other) {}

	ValueIterator& ValueIterator::operator=(const SelfType& other) {
		copy(other);
		return *this;
	}

} // namespace Json

// //////////////////////////////////////////////////////////////////////
// End of content of file: src/lib_json/json_valueiterator.inl
// //////////////////////////////////////////////////////////////////////






// //////////////////////////////////////////////////////////////////////
// Beginning of content of file: src/lib_json/json_value.cpp
// //////////////////////////////////////////////////////////////////////

// Copyright 2011 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#if !defined(JSON_IS_AMALGAMATION)
#include <json/assertions.h>
#include <json/value.h>
#include <json/writer.h>
#endif // if !defined(JSON_IS_AMALGAMATION)
#include <math.h>
#include <sstream>
#include <utility>
#include <cstring>
#include <cassert>
#ifdef JSON_USE_CPPTL
#include <cpptl/conststring.h>
#endif
#include <cstddef> // size_t
#include <algorithm> // min()

#define JSON_ASSERT_UNREACHABLE assert(false)

namespace Json {

	// This is a walkaround to avoid the static initialization of Value::null.
	// kNull must be word-aligned to avoid crashing on ARM.  We use an alignment of
	// 8 (instead of 4) as a bit of future-proofing.
#if defined(__ARMEL__)
#define ALIGNAS(byte_alignment) __attribute__((aligned(byte_alignment)))
#else
#define ALIGNAS(byte_alignment)
#endif
//static const unsigned char ALIGNAS(8) kNull[sizeof(Value)] = { 0 };
//const unsigned char& kNullRef = kNull[0];
//const Value& Value::null = reinterpret_cast<const Value&>(kNullRef);
//const Value& Value::nullRef = null;

// static
	Value const& Value::nullSingleton()
	{
		static Value const nullStatic;
		return nullStatic;
	}

	// for backwards compatibility, we'll leave these global references around, but DO NOT
	// use them in JSONCPP library code any more!
	Value const& Value::null = Value::nullSingleton();
	Value const& Value::nullRef = Value::nullSingleton();

	const Int Value::minInt = Int(~(UInt(-1) / 2));
	const Int Value::maxInt = Int(UInt(-1) / 2);
	const UInt Value::maxUInt = UInt(-1);
#if defined(JSON_HAS_INT64)
	const Int64 Value::minInt64 = Int64(~(UInt64(-1) / 2));
	const Int64 Value::maxInt64 = Int64(UInt64(-1) / 2);
	const UInt64 Value::maxUInt64 = UInt64(-1);
	// The constant is hard-coded because some compiler have trouble
	// converting Value::maxUInt64 to a double correctly (AIX/xlC).
	// Assumes that UInt64 is a 64 bits integer.
	static const double maxUInt64AsDouble = 18446744073709551615.0;
#endif // defined(JSON_HAS_INT64)
	const LargestInt Value::minLargestInt = LargestInt(~(LargestUInt(-1) / 2));
	const LargestInt Value::maxLargestInt = LargestInt(LargestUInt(-1) / 2);
	const LargestUInt Value::maxLargestUInt = LargestUInt(-1);

#if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
	template <typename T, typename U>
	static inline bool InRange(double d, T min, U max) {
		// The casts can lose precision, but we are looking only for
		// an approximate range. Might fail on edge cases though. ~cdunn
		//return d >= static_cast<double>(min) && d <= static_cast<double>(max);
		return d >= min && d <= max;
	}
#else  // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
	static inline double integerToDouble(Json::UInt64 value) {
		return static_cast<double>(Int64(value / 2)) * 2.0 + static_cast<double>(Int64(value & 1));
	}

	template <typename T> static inline double integerToDouble(T value) {
		return static_cast<double>(value);
	}

	template <typename T, typename U>
	static inline bool InRange(double d, T min, U max) {
		return d >= integerToDouble(min) && d <= integerToDouble(max);
	}
#endif // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)

	/** Duplicates the specified string value.
	 * @param value Pointer to the string to duplicate. Must be zero-terminated if
	 *              length is "unknown".
	 * @param length Length of the value. if equals to unknown, then it will be
	 *               computed using strlen(value).
	 * @return Pointer on the duplicate instance of string.
	 */
	static inline char* duplicateStringValue(const char* value,
		size_t length)
	{
		// Avoid an integer overflow in the call to malloc below by limiting length
		// to a sane value.
		if (length >= static_cast<size_t>(Value::maxInt))
			length = Value::maxInt - 1;

		char* newString = static_cast<char*>(malloc(length + 1));
		if (newString == NULL) {
			throwRuntimeError(
				"in Json::Value::duplicateStringValue(): "
				"Failed to allocate string value buffer");
		}
		memcpy(newString, value, length);
		newString[length] = 0;
		return newString;
	}

	/* Record the length as a prefix.
	 */
	static inline char* duplicateAndPrefixStringValue(
		const char* value,
		unsigned int length)
	{
		// Avoid an integer overflow in the call to malloc below by limiting length
		// to a sane value.
		JSON_ASSERT_MESSAGE(length <= static_cast<unsigned>(Value::maxInt) - sizeof(unsigned) - 1U,
			"in Json::Value::duplicateAndPrefixStringValue(): "
			"length too big for prefixing");
		unsigned actualLength = length + static_cast<unsigned>(sizeof(unsigned)) + 1U;
		char* newString = static_cast<char*>(malloc(actualLength));
		if (newString == 0) {
			throwRuntimeError(
				"in Json::Value::duplicateAndPrefixStringValue(): "
				"Failed to allocate string value buffer");
		}
		*reinterpret_cast<unsigned*>(newString) = length;
		memcpy(newString + sizeof(unsigned), value, length);
		newString[actualLength - 1U] = 0; // to avoid buffer over-run accidents by users later
		return newString;
	}
	inline static void decodePrefixedString(
		bool isPrefixed, char const* prefixed,
		unsigned* length, char const** value)
	{
		if (!isPrefixed) {
			*length = static_cast<unsigned>(strlen(prefixed));
			*value = prefixed;
		}
		else {
			*length = *reinterpret_cast<unsigned const*>(prefixed);
			*value = prefixed + sizeof(unsigned);
		}
	}
	/** Free the string duplicated by duplicateStringValue()/duplicateAndPrefixStringValue().
	 */
#if JSONCPP_USING_SECURE_MEMORY
	static inline void releasePrefixedStringValue(char* value) {
		unsigned length = 0;
		char const* valueDecoded;
		decodePrefixedString(true, value, &length, &valueDecoded);
		size_t const size = sizeof(unsigned) + length + 1U;
		memset(value, 0, size);
		free(value);
	}
	static inline void releaseStringValue(char* value, unsigned length) {
		// length==0 => we allocated the strings memory
		size_t size = (length == 0) ? strlen(value) : length;
		memset(value, 0, size);
		free(value);
	}
#else // !JSONCPP_USING_SECURE_MEMORY
	static inline void releasePrefixedStringValue(char* value) {
		free(value);
	}
	static inline void releaseStringValue(char* value, unsigned) {
		free(value);
	}
#endif // JSONCPP_USING_SECURE_MEMORY

} // namespace Json

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// ValueInternals...
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
#if !defined(JSON_IS_AMALGAMATION)

#include "json_valueiterator.inl"
#endif // if !defined(JSON_IS_AMALGAMATION)

namespace Json {

	Exception::Exception(JSONCPP_STRING const& msg)
		: msg_(msg)
	{}
	Exception::~Exception() throw()
	{}
	char const* Exception::what() const throw()
	{
		return msg_.c_str();
	}
	RuntimeError::RuntimeError(JSONCPP_STRING const& msg)
		: Exception(msg)
	{}
	LogicError::LogicError(JSONCPP_STRING const& msg)
		: Exception(msg)
	{}
	JSONCPP_NORETURN void throwRuntimeError(JSONCPP_STRING const& msg)
	{
		throw RuntimeError(msg);
	}
	JSONCPP_NORETURN void throwLogicError(JSONCPP_STRING const& msg)
	{
		throw LogicError(msg);
	}

	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// class Value::CommentInfo
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////

	Value::CommentInfo::CommentInfo() : comment_(0)
	{}

	Value::CommentInfo::~CommentInfo() {
		if (comment_)
			releaseStringValue(comment_, 0u);
	}

	void Value::CommentInfo::setComment(const char* text, size_t len) {
		if (comment_) {
			releaseStringValue(comment_, 0u);
			comment_ = 0;
		}
		JSON_ASSERT(text != 0);
		JSON_ASSERT_MESSAGE(
			text[0] == '\0' || text[0] == '/',
			"in Json::Value::setComment(): Comments must start with /");
		// It seems that /**/ style comments are acceptable as well.
		comment_ = duplicateStringValue(text, len);
	}

	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// class Value::CZString
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////

	// Notes: policy_ indicates if the string was allocated when
	// a string is stored.

	Value::CZString::CZString(ArrayIndex aindex) : cstr_(0), index_(aindex) {}

	Value::CZString::CZString(char const* str, unsigned ulength, DuplicationPolicy allocate)
		: cstr_(str) {
		// allocate != duplicate
		storage_.policy_ = allocate & 0x3;
		storage_.length_ = ulength & 0x3FFFFFFF;
	}

	Value::CZString::CZString(const CZString& other) {
		cstr_ = (other.storage_.policy_ != noDuplication && other.cstr_ != 0
			? duplicateStringValue(other.cstr_, other.storage_.length_)
			: other.cstr_);
		storage_.policy_ = static_cast<unsigned>(other.cstr_
			? (static_cast<DuplicationPolicy>(other.storage_.policy_) == noDuplication
				? noDuplication : duplicate)
			: static_cast<DuplicationPolicy>(other.storage_.policy_)) & 3U;
		storage_.length_ = other.storage_.length_;
	}

#if JSON_HAS_RVALUE_REFERENCES
	Value::CZString::CZString(CZString&& other)
		: cstr_(other.cstr_), index_(other.index_) {
		other.cstr_ = nullptr;
	}
#endif

	Value::CZString::~CZString() {
		if (cstr_ && storage_.policy_ == duplicate) {
			releaseStringValue(const_cast<char*>(cstr_), storage_.length_ + 1u); //+1 for null terminating character for sake of completeness but not actually necessary
		}
	}

	void Value::CZString::swap(CZString& other) {
		std::swap(cstr_, other.cstr_);
		std::swap(index_, other.index_);
	}

	Value::CZString& Value::CZString::operator=(CZString other) {
		swap(other);
		return *this;
	}

	bool Value::CZString::operator<(const CZString& other) const {
		if (!cstr_) return index_ < other.index_;
		//return strcmp(cstr_, other.cstr_) < 0;
		// Assume both are strings.
		unsigned this_len = this->storage_.length_;
		unsigned other_len = other.storage_.length_;
		unsigned min_len = std::min(this_len, other_len);
		JSON_ASSERT(this->cstr_ && other.cstr_);
		int comp = memcmp(this->cstr_, other.cstr_, min_len);
		if (comp < 0) return true;
		if (comp > 0) return false;
		return (this_len < other_len);
	}

	bool Value::CZString::operator==(const CZString& other) const {
		if (!cstr_) return index_ == other.index_;
		//return strcmp(cstr_, other.cstr_) == 0;
		// Assume both are strings.
		unsigned this_len = this->storage_.length_;
		unsigned other_len = other.storage_.length_;
		if (this_len != other_len) return false;
		JSON_ASSERT(this->cstr_ && other.cstr_);
		int comp = memcmp(this->cstr_, other.cstr_, this_len);
		return comp == 0;
	}

	ArrayIndex Value::CZString::index() const { return index_; }

	//const char* Value::CZString::c_str() const { return cstr_; }
	const char* Value::CZString::data() const { return cstr_; }
	unsigned Value::CZString::length() const { return storage_.length_; }
	bool Value::CZString::isStaticString() const { return storage_.policy_ == noDuplication; }

	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// class Value::Value
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////

	/*! \internal Default constructor initialization must be equivalent to:
	 * memset( this, 0, sizeof(Value) )
	 * This optimization is used in ValueInternalMap fast allocator.
	 */
	Value::Value(ValueType vtype) {
		static char const empty[] = "";
		initBasic(vtype);
		switch (vtype) {
		case nullValue:
			break;
		case intValue:
		case uintValue:
			value_.int_ = 0;
			break;
		case realValue:
			value_.real_ = 0.0;
			break;
		case stringValue:
			// allocated_ == false, so this is safe.
			value_.string_ = const_cast<char*>(static_cast<char const*>(empty));
			break;
		case arrayValue:
		case objectValue:
			value_.map_ = new ObjectValues();
			break;
		case booleanValue:
			value_.bool_ = false;
			break;
		default:
			JSON_ASSERT_UNREACHABLE;
		}
	}

	Value::Value(Int value) {
		initBasic(intValue);
		value_.int_ = value;
	}

	Value::Value(UInt value) {
		initBasic(uintValue);
		value_.uint_ = value;
	}
#if defined(JSON_HAS_INT64)
	Value::Value(Int64 value) {
		initBasic(intValue);
		value_.int_ = value;
	}
	Value::Value(UInt64 value) {
		initBasic(uintValue);
		value_.uint_ = value;
	}
#endif // defined(JSON_HAS_INT64)

	Value::Value(double value) {
		initBasic(realValue);
		value_.real_ = value;
	}

	Value::Value(const char* value) {
		initBasic(stringValue, true);
		value_.string_ = duplicateAndPrefixStringValue(value, static_cast<unsigned>(strlen(value)));
	}

	Value::Value(const char* beginValue, const char* endValue) {
		initBasic(stringValue, true);
		value_.string_ =
			duplicateAndPrefixStringValue(beginValue, static_cast<unsigned>(endValue - beginValue));
	}

	Value::Value(const JSONCPP_STRING& value) {
		initBasic(stringValue, true);
		value_.string_ =
			duplicateAndPrefixStringValue(value.data(), static_cast<unsigned>(value.length()));
	}

	Value::Value(const StaticString& value) {
		initBasic(stringValue);
		value_.string_ = const_cast<char*>(value.c_str());
	}

#ifdef JSON_USE_CPPTL
	Value::Value(const CppTL::ConstString& value) {
		initBasic(stringValue, true);
		value_.string_ = duplicateAndPrefixStringValue(value, static_cast<unsigned>(value.length()));
	}
#endif

	Value::Value(bool value) {
		initBasic(booleanValue);
		value_.bool_ = value;
	}

	Value::Value(Value const& other)
		: type_(other.type_), allocated_(false)
		,
		comments_(0), start_(other.start_), limit_(other.limit_)
	{
		switch (type_) {
		case nullValue:
		case intValue:
		case uintValue:
		case realValue:
		case booleanValue:
			value_ = other.value_;
			break;
		case stringValue:
			if (other.value_.string_ && other.allocated_) {
				unsigned len;
				char const* str;
				decodePrefixedString(other.allocated_, other.value_.string_,
					&len, &str);
				value_.string_ = duplicateAndPrefixStringValue(str, len);
				allocated_ = true;
			}
			else {
				value_.string_ = other.value_.string_;
				allocated_ = false;
			}
			break;
		case arrayValue:
		case objectValue:
			value_.map_ = new ObjectValues(*other.value_.map_);
			break;
		default:
			JSON_ASSERT_UNREACHABLE;
		}
		if (other.comments_) {
			comments_ = new CommentInfo[numberOfCommentPlacement];
			for (int comment = 0; comment < numberOfCommentPlacement; ++comment) {
				const CommentInfo& otherComment = other.comments_[comment];
				if (otherComment.comment_)
					comments_[comment].setComment(
						otherComment.comment_, strlen(otherComment.comment_));
			}
		}
	}

#if JSON_HAS_RVALUE_REFERENCES
	// Move constructor
	Value::Value(Value&& other) {
		initBasic(nullValue);
		swap(other);
	}
#endif

	Value::~Value() {
		switch (type_) {
		case nullValue:
		case intValue:
		case uintValue:
		case realValue:
		case booleanValue:
			break;
		case stringValue:
			if (allocated_)
				releasePrefixedStringValue(value_.string_);
			break;
		case arrayValue:
		case objectValue:
			delete value_.map_;
			break;
		default:
			JSON_ASSERT_UNREACHABLE;
		}

		delete[] comments_;

		value_.uint_ = 0;
	}

	Value& Value::operator=(Value other) {
		swap(other);
		return *this;
	}

	void Value::swapPayload(Value& other) {
		ValueType temp = type_;
		type_ = other.type_;
		other.type_ = temp;
		std::swap(value_, other.value_);
		int temp2 = allocated_;
		allocated_ = other.allocated_;
		other.allocated_ = temp2 & 0x1;
	}

	void Value::swap(Value& other) {
		swapPayload(other);
		std::swap(comments_, other.comments_);
		std::swap(start_, other.start_);
		std::swap(limit_, other.limit_);
	}

	ValueType Value::type() const { return type_; }

	int Value::compare(const Value& other) const {
		if (*this < other)
			return -1;
		if (*this > other)
			return 1;
		return 0;
	}

	bool Value::operator<(const Value& other) const {
		int typeDelta = type_ - other.type_;
		if (typeDelta)
			return typeDelta < 0 ? true : false;
		switch (type_) {
		case nullValue:
			return false;
		case intValue:
			return value_.int_ < other.value_.int_;
		case uintValue:
			return value_.uint_ < other.value_.uint_;
		case realValue:
			return value_.real_ < other.value_.real_;
		case booleanValue:
			return value_.bool_ < other.value_.bool_;
		case stringValue:
		{
			if ((value_.string_ == 0) || (other.value_.string_ == 0)) {
				if (other.value_.string_) return true;
				else return false;
			}
			unsigned this_len;
			unsigned other_len;
			char const* this_str;
			char const* other_str;
			decodePrefixedString(this->allocated_, this->value_.string_, &this_len, &this_str);
			decodePrefixedString(other.allocated_, other.value_.string_, &other_len, &other_str);
			unsigned min_len = std::min(this_len, other_len);
			JSON_ASSERT(this_str && other_str);
			int comp = memcmp(this_str, other_str, min_len);
			if (comp < 0) return true;
			if (comp > 0) return false;
			return (this_len < other_len);
		}
		case arrayValue:
		case objectValue: {
			int delta = int(value_.map_->size() - other.value_.map_->size());
			if (delta)
				return delta < 0;
			return (*value_.map_) < (*other.value_.map_);
		}
		default:
			JSON_ASSERT_UNREACHABLE;
		}
		return false; // unreachable
	}

	bool Value::operator<=(const Value& other) const { return !(other < *this); }

	bool Value::operator>=(const Value& other) const { return !(*this < other); }

	bool Value::operator>(const Value& other) const { return other < *this; }

	bool Value::operator==(const Value& other) const {
		// if ( type_ != other.type_ )
		// GCC 2.95.3 says:
		// attempt to take address of bit-field structure member `Json::Value::type_'
		// Beats me, but a temp solves the problem.
		int temp = other.type_;
		if (type_ != temp)
			return false;
		switch (type_) {
		case nullValue:
			return true;
		case intValue:
			return value_.int_ == other.value_.int_;
		case uintValue:
			return value_.uint_ == other.value_.uint_;
		case realValue:
			return value_.real_ == other.value_.real_;
		case booleanValue:
			return value_.bool_ == other.value_.bool_;
		case stringValue:
		{
			if ((value_.string_ == 0) || (other.value_.string_ == 0)) {
				return (value_.string_ == other.value_.string_);
			}
			unsigned this_len;
			unsigned other_len;
			char const* this_str;
			char const* other_str;
			decodePrefixedString(this->allocated_, this->value_.string_, &this_len, &this_str);
			decodePrefixedString(other.allocated_, other.value_.string_, &other_len, &other_str);
			if (this_len != other_len) return false;
			JSON_ASSERT(this_str && other_str);
			int comp = memcmp(this_str, other_str, this_len);
			return comp == 0;
		}
		case arrayValue:
		case objectValue:
			return value_.map_->size() == other.value_.map_->size() &&
				(*value_.map_) == (*other.value_.map_);
		default:
			JSON_ASSERT_UNREACHABLE;
		}
		return false; // unreachable
	}

	bool Value::operator!=(const Value& other) const { return !(*this == other); }

	const char* Value::asCString() const {
		JSON_ASSERT_MESSAGE(type_ == stringValue,
			"in Json::Value::asCString(): requires stringValue");
		if (value_.string_ == 0) return 0;
		unsigned this_len;
		char const* this_str;
		decodePrefixedString(this->allocated_, this->value_.string_, &this_len, &this_str);
		return this_str;
	}

#if JSONCPP_USING_SECURE_MEMORY
	unsigned Value::getCStringLength() const {
		JSON_ASSERT_MESSAGE(type_ == stringValue,
			"in Json::Value::asCString(): requires stringValue");
		if (value_.string_ == 0) return 0;
		unsigned this_len;
		char const* this_str;
		decodePrefixedString(this->allocated_, this->value_.string_, &this_len, &this_str);
		return this_len;
	}
#endif

	bool Value::getString(char const** str, char const** cend) const {
		if (type_ != stringValue) return false;
		if (value_.string_ == 0) return false;
		unsigned length;
		decodePrefixedString(this->allocated_, this->value_.string_, &length, str);
		*cend = *str + length;
		return true;
	}

	JSONCPP_STRING Value::asString() const {
		switch (type_) {
		case nullValue:
			return "";
		case stringValue:
		{
			if (value_.string_ == 0) return "";
			unsigned this_len;
			char const* this_str;
			decodePrefixedString(this->allocated_, this->value_.string_, &this_len, &this_str);
			return JSONCPP_STRING(this_str, this_len);
		}
		case booleanValue:
			return value_.bool_ ? "true" : "false";
		case intValue:
			return valueToString(value_.int_);
		case uintValue:
			return valueToString(value_.uint_);
		case realValue:
			return valueToString(value_.real_);
		default:
			JSON_FAIL_MESSAGE("Type is not convertible to string");
		}
	}

#ifdef JSON_USE_CPPTL
	CppTL::ConstString Value::asConstString() const {
		unsigned len;
		char const* str;
		decodePrefixedString(allocated_, value_.string_,
			&len, &str);
		return CppTL::ConstString(str, len);
	}
#endif

	Value::Int Value::asInt() const {
		switch (type_) {
		case intValue:
			JSON_ASSERT_MESSAGE(isInt(), "LargestInt out of Int range");
			return Int(value_.int_);
		case uintValue:
			JSON_ASSERT_MESSAGE(isInt(), "LargestUInt out of Int range");
			return Int(value_.uint_);
		case realValue:
			JSON_ASSERT_MESSAGE(InRange(value_.real_, minInt, maxInt),
				"double out of Int range");
			return Int(value_.real_);
		case nullValue:
			return 0;
		case booleanValue:
			return value_.bool_ ? 1 : 0;
		default:
			break;
		}
		JSON_FAIL_MESSAGE("Value is not convertible to Int.");
	}

	Value::UInt Value::asUInt() const {
		switch (type_) {
		case intValue:
			JSON_ASSERT_MESSAGE(isUInt(), "LargestInt out of UInt range");
			return UInt(value_.int_);
		case uintValue:
			JSON_ASSERT_MESSAGE(isUInt(), "LargestUInt out of UInt range");
			return UInt(value_.uint_);
		case realValue:
			JSON_ASSERT_MESSAGE(InRange(value_.real_, 0, maxUInt),
				"double out of UInt range");
			return UInt(value_.real_);
		case nullValue:
			return 0;
		case booleanValue:
			return value_.bool_ ? 1 : 0;
		default:
			break;
		}
		JSON_FAIL_MESSAGE("Value is not convertible to UInt.");
	}

#if defined(JSON_HAS_INT64)

	Value::Int64 Value::asInt64() const {
		switch (type_) {
		case intValue:
			return Int64(value_.int_);
		case uintValue:
			JSON_ASSERT_MESSAGE(isInt64(), "LargestUInt out of Int64 range");
			return Int64(value_.uint_);
		case realValue:
			JSON_ASSERT_MESSAGE(InRange(value_.real_, minInt64, maxInt64),
				"double out of Int64 range");
			return Int64(value_.real_);
		case nullValue:
			return 0;
		case booleanValue:
			return value_.bool_ ? 1 : 0;
		default:
			break;
		}
		JSON_FAIL_MESSAGE("Value is not convertible to Int64.");
	}

	Value::UInt64 Value::asUInt64() const {
		switch (type_) {
		case intValue:
			JSON_ASSERT_MESSAGE(isUInt64(), "LargestInt out of UInt64 range");
			return UInt64(value_.int_);
		case uintValue:
			return UInt64(value_.uint_);
		case realValue:
			JSON_ASSERT_MESSAGE(InRange(value_.real_, 0, maxUInt64),
				"double out of UInt64 range");
			return UInt64(value_.real_);
		case nullValue:
			return 0;
		case booleanValue:
			return value_.bool_ ? 1 : 0;
		default:
			break;
		}
		JSON_FAIL_MESSAGE("Value is not convertible to UInt64.");
	}
#endif // if defined(JSON_HAS_INT64)

	LargestInt Value::asLargestInt() const {
#if defined(JSON_NO_INT64)
		return asInt();
#else
		return asInt64();
#endif
	}

	LargestUInt Value::asLargestUInt() const {
#if defined(JSON_NO_INT64)
		return asUInt();
#else
		return asUInt64();
#endif
	}

	double Value::asDouble() const {
		switch (type_) {
		case intValue:
			return static_cast<double>(value_.int_);
		case uintValue:
#if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
			return static_cast<double>(value_.uint_);
#else  // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
			return integerToDouble(value_.uint_);
#endif // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
		case realValue:
			return value_.real_;
		case nullValue:
			return 0.0;
		case booleanValue:
			return value_.bool_ ? 1.0 : 0.0;
		default:
			break;
		}
		JSON_FAIL_MESSAGE("Value is not convertible to double.");
	}

	float Value::asFloat() const {
		switch (type_) {
		case intValue:
			return static_cast<float>(value_.int_);
		case uintValue:
#if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
			return static_cast<float>(value_.uint_);
#else  // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
			// This can fail (silently?) if the value is bigger than MAX_FLOAT.
			return static_cast<float>(integerToDouble(value_.uint_));
#endif // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
		case realValue:
			return static_cast<float>(value_.real_);
		case nullValue:
			return 0.0;
		case booleanValue:
			return value_.bool_ ? 1.0f : 0.0f;
		default:
			break;
		}
		JSON_FAIL_MESSAGE("Value is not convertible to float.");
	}

	bool Value::asBool() const {
		switch (type_) {
		case booleanValue:
			return value_.bool_;
		case nullValue:
			return false;
		case intValue:
			return value_.int_ ? true : false;
		case uintValue:
			return value_.uint_ ? true : false;
		case realValue:
			// This is kind of strange. Not recommended.
			return (value_.real_ != 0.0) ? true : false;
		default:
			break;
		}
		JSON_FAIL_MESSAGE("Value is not convertible to bool.");
	}

	bool Value::isConvertibleTo(ValueType other) const {
		switch (other) {
		case nullValue:
			return (isNumeric() && asDouble() == 0.0) ||
				(type_ == booleanValue && value_.bool_ == false) ||
				(type_ == stringValue && asString() == "") ||
				(type_ == arrayValue && value_.map_->size() == 0) ||
				(type_ == objectValue && value_.map_->size() == 0) ||
				type_ == nullValue;
		case intValue:
			return isInt() ||
				(type_ == realValue && InRange(value_.real_, minInt, maxInt)) ||
				type_ == booleanValue || type_ == nullValue;
		case uintValue:
			return isUInt() ||
				(type_ == realValue && InRange(value_.real_, 0, maxUInt)) ||
				type_ == booleanValue || type_ == nullValue;
		case realValue:
			return isNumeric() || type_ == booleanValue || type_ == nullValue;
		case booleanValue:
			return isNumeric() || type_ == booleanValue || type_ == nullValue;
		case stringValue:
			return isNumeric() || type_ == booleanValue || type_ == stringValue ||
				type_ == nullValue;
		case arrayValue:
			return type_ == arrayValue || type_ == nullValue;
		case objectValue:
			return type_ == objectValue || type_ == nullValue;
		}
		JSON_ASSERT_UNREACHABLE;
		return false;
	}

	/// Number of values in array or object
	ArrayIndex Value::size() const {
		switch (type_) {
		case nullValue:
		case intValue:
		case uintValue:
		case realValue:
		case booleanValue:
		case stringValue:
			return 0;
		case arrayValue: // size of the array is highest index + 1
			if (!value_.map_->empty()) {
				ObjectValues::const_iterator itLast = value_.map_->end();
				--itLast;
				return (*itLast).first.index() + 1;
			}
			return 0;
		case objectValue:
			return ArrayIndex(value_.map_->size());
		}
		JSON_ASSERT_UNREACHABLE;
		return 0; // unreachable;
	}

	bool Value::empty() const {
		if (isNull() || isArray() || isObject())
			return size() == 0u;
		else
			return false;
	}

	bool Value::operator!() const { return isNull(); }

	void Value::clear() {
		JSON_ASSERT_MESSAGE(type_ == nullValue || type_ == arrayValue ||
			type_ == objectValue,
			"in Json::Value::clear(): requires complex value");
		start_ = 0;
		limit_ = 0;
		switch (type_) {
		case arrayValue:
		case objectValue:
			value_.map_->clear();
			break;
		default:
			break;
		}
	}

	void Value::resize(ArrayIndex newSize) {
		JSON_ASSERT_MESSAGE(type_ == nullValue || type_ == arrayValue,
			"in Json::Value::resize(): requires arrayValue");
		if (type_ == nullValue)
			* this = Value(arrayValue);
		ArrayIndex oldSize = size();
		if (newSize == 0)
			clear();
		else if (newSize > oldSize)
			(*this)[newSize - 1];
		else {
			for (ArrayIndex index = newSize; index < oldSize; ++index) {
				value_.map_->erase(index);
			}
			JSON_ASSERT(size() == newSize);
		}
	}

	Value& Value::operator[](ArrayIndex index) {
		JSON_ASSERT_MESSAGE(
			type_ == nullValue || type_ == arrayValue,
			"in Json::Value::operator[](ArrayIndex): requires arrayValue");
		if (type_ == nullValue)
			* this = Value(arrayValue);
		CZString key(index);
		ObjectValues::iterator it = value_.map_->lower_bound(key);
		if (it != value_.map_->end() && (*it).first == key)
			return (*it).second;

		ObjectValues::value_type defaultValue(key, nullSingleton());
		it = value_.map_->insert(it, defaultValue);
		return (*it).second;
	}

	Value& Value::operator[](int index) {
		JSON_ASSERT_MESSAGE(
			index >= 0,
			"in Json::Value::operator[](int index): index cannot be negative");
		return (*this)[ArrayIndex(index)];
	}

	const Value& Value::operator[](ArrayIndex index) const {
		JSON_ASSERT_MESSAGE(
			type_ == nullValue || type_ == arrayValue,
			"in Json::Value::operator[](ArrayIndex)const: requires arrayValue");
		if (type_ == nullValue)
			return nullSingleton();
		CZString key(index);
		ObjectValues::const_iterator it = value_.map_->find(key);
		if (it == value_.map_->end())
			return nullSingleton();
		return (*it).second;
	}

	const Value& Value::operator[](int index) const {
		JSON_ASSERT_MESSAGE(
			index >= 0,
			"in Json::Value::operator[](int index) const: index cannot be negative");
		return (*this)[ArrayIndex(index)];
	}

	void Value::initBasic(ValueType vtype, bool allocated) {
		type_ = vtype;
		allocated_ = allocated;
		comments_ = 0;
		start_ = 0;
		limit_ = 0;
	}

	// Access an object value by name, create a null member if it does not exist.
	// @pre Type of '*this' is object or null.
	// @param key is null-terminated.
	Value& Value::resolveReference(const char* key) {
		JSON_ASSERT_MESSAGE(
			type_ == nullValue || type_ == objectValue,
			"in Json::Value::resolveReference(): requires objectValue");
		if (type_ == nullValue)
			* this = Value(objectValue);
		CZString actualKey(
			key, static_cast<unsigned>(strlen(key)), CZString::noDuplication); // NOTE!
		ObjectValues::iterator it = value_.map_->lower_bound(actualKey);
		if (it != value_.map_->end() && (*it).first == actualKey)
			return (*it).second;

		ObjectValues::value_type defaultValue(actualKey, nullSingleton());
		it = value_.map_->insert(it, defaultValue);
		Value& value = (*it).second;
		return value;
	}

	// @param key is not null-terminated.
	Value& Value::resolveReference(char const* key, char const* cend)
	{
		JSON_ASSERT_MESSAGE(
			type_ == nullValue || type_ == objectValue,
			"in Json::Value::resolveReference(key, end): requires objectValue");
		if (type_ == nullValue)
			* this = Value(objectValue);
		CZString actualKey(
			key, static_cast<unsigned>(cend - key), CZString::duplicateOnCopy);
		ObjectValues::iterator it = value_.map_->lower_bound(actualKey);
		if (it != value_.map_->end() && (*it).first == actualKey)
			return (*it).second;

		ObjectValues::value_type defaultValue(actualKey, nullSingleton());
		it = value_.map_->insert(it, defaultValue);
		Value& value = (*it).second;
		return value;
	}

	Value Value::get(ArrayIndex index, const Value& defaultValue) const {
		const Value* value = &((*this)[index]);
		return value == &nullSingleton() ? defaultValue : *value;
	}

	bool Value::isValidIndex(ArrayIndex index) const { return index < size(); }

	Value const* Value::find(char const* key, char const* cend) const
	{
		JSON_ASSERT_MESSAGE(
			type_ == nullValue || type_ == objectValue,
			"in Json::Value::find(key, end, found): requires objectValue or nullValue");
		if (type_ == nullValue) return NULL;
		CZString actualKey(key, static_cast<unsigned>(cend - key), CZString::noDuplication);
		ObjectValues::const_iterator it = value_.map_->find(actualKey);
		if (it == value_.map_->end()) return NULL;
		return &(*it).second;
	}
	const Value& Value::operator[](const char* key) const
	{
		Value const* found = find(key, key + strlen(key));
		if (!found) return nullSingleton();
		return *found;
	}
	Value const& Value::operator[](JSONCPP_STRING const& key) const
	{
		Value const* found = find(key.data(), key.data() + key.length());
		if (!found) return nullSingleton();
		return *found;
	}

	Value& Value::operator[](const char* key) {
		return resolveReference(key, key + strlen(key));
	}

	Value& Value::operator[](const JSONCPP_STRING& key) {
		return resolveReference(key.data(), key.data() + key.length());
	}

	Value& Value::operator[](const StaticString& key) {
		return resolveReference(key.c_str());
	}

#ifdef JSON_USE_CPPTL
	Value& Value::operator[](const CppTL::ConstString& key) {
		return resolveReference(key.c_str(), key.end_c_str());
	}
	Value const& Value::operator[](CppTL::ConstString const& key) const
	{
		Value const* found = find(key.c_str(), key.end_c_str());
		if (!found) return nullSingleton();
		return *found;
	}
#endif

	Value& Value::append(const Value& value) { return (*this)[size()] = value; }

	Value Value::get(char const* key, char const* cend, Value const& defaultValue) const
	{
		Value const* found = find(key, cend);
		return !found ? defaultValue : *found;
	}
	Value Value::get(char const* key, Value const& defaultValue) const
	{
		return get(key, key + strlen(key), defaultValue);
	}
	Value Value::get(JSONCPP_STRING const& key, Value const& defaultValue) const
	{
		return get(key.data(), key.data() + key.length(), defaultValue);
	}


	bool Value::removeMember(const char* key, const char* cend, Value* removed)
	{
		if (type_ != objectValue) {
			return false;
		}
		CZString actualKey(key, static_cast<unsigned>(cend - key), CZString::noDuplication);
		ObjectValues::iterator it = value_.map_->find(actualKey);
		if (it == value_.map_->end())
			return false;
		*removed = it->second;
		value_.map_->erase(it);
		return true;
	}
	bool Value::removeMember(const char* key, Value* removed)
	{
		return removeMember(key, key + strlen(key), removed);
	}
	bool Value::removeMember(JSONCPP_STRING const& key, Value* removed)
	{
		return removeMember(key.data(), key.data() + key.length(), removed);
	}
	Value Value::removeMember(const char* key)
	{
		JSON_ASSERT_MESSAGE(type_ == nullValue || type_ == objectValue,
			"in Json::Value::removeMember(): requires objectValue");
		if (type_ == nullValue)
			return nullSingleton();

		Value removed;  // null
		removeMember(key, key + strlen(key), &removed);
		return removed; // still null if removeMember() did nothing
	}
	Value Value::removeMember(const JSONCPP_STRING& key)
	{
		return removeMember(key.c_str());
	}

	bool Value::removeIndex(ArrayIndex index, Value* removed) {
		if (type_ != arrayValue) {
			return false;
		}
		CZString key(index);
		ObjectValues::iterator it = value_.map_->find(key);
		if (it == value_.map_->end()) {
			return false;
		}
		*removed = it->second;
		ArrayIndex oldSize = size();
		// shift left all items left, into the place of the "removed"
		for (ArrayIndex i = index; i < (oldSize - 1); ++i) {
			CZString keey(i);
			(*value_.map_)[keey] = (*this)[i + 1];
		}
		// erase the last one ("leftover")
		CZString keyLast(oldSize - 1);
		ObjectValues::iterator itLast = value_.map_->find(keyLast);
		value_.map_->erase(itLast);
		return true;
	}

#ifdef JSON_USE_CPPTL
	Value Value::get(const CppTL::ConstString& key,
		const Value& defaultValue) const {
		return get(key.c_str(), key.end_c_str(), defaultValue);
	}
#endif

	bool Value::isMember(char const* key, char const* cend) const
	{
		Value const* value = find(key, cend);
		return NULL != value;
	}
	bool Value::isMember(char const* key) const
	{
		return isMember(key, key + strlen(key));
	}
	bool Value::isMember(JSONCPP_STRING const& key) const
	{
		return isMember(key.data(), key.data() + key.length());
	}

#ifdef JSON_USE_CPPTL
	bool Value::isMember(const CppTL::ConstString& key) const {
		return isMember(key.c_str(), key.end_c_str());
	}
#endif

	Value::Members Value::getMemberNames() const {
		JSON_ASSERT_MESSAGE(
			type_ == nullValue || type_ == objectValue,
			"in Json::Value::getMemberNames(), value must be objectValue");
		if (type_ == nullValue)
			return Value::Members();
		Members members;
		members.reserve(value_.map_->size());
		ObjectValues::const_iterator it = value_.map_->begin();
		ObjectValues::const_iterator itEnd = value_.map_->end();
		for (; it != itEnd; ++it) {
			members.push_back(JSONCPP_STRING((*it).first.data(),
				(*it).first.length()));
		}
		return members;
	}
	//
	//# ifdef JSON_USE_CPPTL
	// EnumMemberNames
	// Value::enumMemberNames() const
	//{
	//   if ( type_ == objectValue )
	//   {
	//      return CppTL::Enum::any(  CppTL::Enum::transform(
	//         CppTL::Enum::keys( *(value_.map_), CppTL::Type<const CZString &>() ),
	//         MemberNamesTransform() ) );
	//   }
	//   return EnumMemberNames();
	//}
	//
	//
	// EnumValues
	// Value::enumValues() const
	//{
	//   if ( type_ == objectValue  ||  type_ == arrayValue )
	//      return CppTL::Enum::anyValues( *(value_.map_),
	//                                     CppTL::Type<const Value &>() );
	//   return EnumValues();
	//}
	//
	//# endif

	static bool IsIntegral(double d) {
		double integral_part;
		return modf(d, &integral_part) == 0.0;
	}

	bool Value::isNull() const { return type_ == nullValue; }

	bool Value::isBool() const { return type_ == booleanValue; }

	bool Value::isInt() const {
		switch (type_) {
		case intValue:
			return value_.int_ >= minInt && value_.int_ <= maxInt;
		case uintValue:
			return value_.uint_ <= UInt(maxInt);
		case realValue:
			return value_.real_ >= minInt && value_.real_ <= maxInt &&
				IsIntegral(value_.real_);
		default:
			break;
		}
		return false;
	}

	bool Value::isUInt() const {
		switch (type_) {
		case intValue:
			return value_.int_ >= 0 && LargestUInt(value_.int_) <= LargestUInt(maxUInt);
		case uintValue:
			return value_.uint_ <= maxUInt;
		case realValue:
			return value_.real_ >= 0 && value_.real_ <= maxUInt &&
				IsIntegral(value_.real_);
		default:
			break;
		}
		return false;
	}

	bool Value::isInt64() const {
#if defined(JSON_HAS_INT64)
		switch (type_) {
		case intValue:
			return true;
		case uintValue:
			return value_.uint_ <= UInt64(maxInt64);
		case realValue:
			// Note that maxInt64 (= 2^63 - 1) is not exactly representable as a
			// double, so double(maxInt64) will be rounded up to 2^63. Therefore we
			// require the value to be strictly less than the limit.
			return value_.real_ >= double(minInt64) &&
				value_.real_ < double(maxInt64) && IsIntegral(value_.real_);
		default:
			break;
		}
#endif // JSON_HAS_INT64
		return false;
	}

	bool Value::isUInt64() const {
#if defined(JSON_HAS_INT64)
		switch (type_) {
		case intValue:
			return value_.int_ >= 0;
		case uintValue:
			return true;
		case realValue:
			// Note that maxUInt64 (= 2^64 - 1) is not exactly representable as a
			// double, so double(maxUInt64) will be rounded up to 2^64. Therefore we
			// require the value to be strictly less than the limit.
			return value_.real_ >= 0 && value_.real_ < maxUInt64AsDouble &&
				IsIntegral(value_.real_);
		default:
			break;
		}
#endif // JSON_HAS_INT64
		return false;
	}

	bool Value::isIntegral() const {
#if defined(JSON_HAS_INT64)
		return isInt64() || isUInt64();
#else
		return isInt() || isUInt();
#endif
	}

	bool Value::isDouble() const { return type_ == realValue || isIntegral(); }

	bool Value::isNumeric() const { return isIntegral() || isDouble(); }

	bool Value::isString() const { return type_ == stringValue; }

	bool Value::isArray() const { return type_ == arrayValue; }

	bool Value::isObject() const { return type_ == objectValue; }

	void Value::setComment(const char* comment, size_t len, CommentPlacement placement) {
		if (!comments_)
			comments_ = new CommentInfo[numberOfCommentPlacement];
		if ((len > 0) && (comment[len - 1] == '\n')) {
			// Always discard trailing newline, to aid indentation.
			len -= 1;
		}
		comments_[placement].setComment(comment, len);
	}

	void Value::setComment(const char* comment, CommentPlacement placement) {
		setComment(comment, strlen(comment), placement);
	}

	void Value::setComment(const JSONCPP_STRING& comment, CommentPlacement placement) {
		setComment(comment.c_str(), comment.length(), placement);
	}

	bool Value::hasComment(CommentPlacement placement) const {
		return comments_ != 0 && comments_[placement].comment_ != 0;
	}

	JSONCPP_STRING Value::getComment(CommentPlacement placement) const {
		if (hasComment(placement))
			return comments_[placement].comment_;
		return "";
	}

	void Value::setOffsetStart(ptrdiff_t start) { start_ = start; }

	void Value::setOffsetLimit(ptrdiff_t limit) { limit_ = limit; }

	ptrdiff_t Value::getOffsetStart() const { return start_; }

	ptrdiff_t Value::getOffsetLimit() const { return limit_; }

	JSONCPP_STRING Value::toStyledString() const {
		StyledWriter writer;
		return writer.write(*this);
	}

	Value::const_iterator Value::begin() const {
		switch (type_) {
		case arrayValue:
		case objectValue:
			if (value_.map_)
				return const_iterator(value_.map_->begin());
			break;
		default:
			break;
		}
		return const_iterator();
	}

	Value::const_iterator Value::end() const {
		switch (type_) {
		case arrayValue:
		case objectValue:
			if (value_.map_)
				return const_iterator(value_.map_->end());
			break;
		default:
			break;
		}
		return const_iterator();
	}

	Value::iterator Value::begin() {
		switch (type_) {
		case arrayValue:
		case objectValue:
			if (value_.map_)
				return iterator(value_.map_->begin());
			break;
		default:
			break;
		}
		return iterator();
	}

	Value::iterator Value::end() {
		switch (type_) {
		case arrayValue:
		case objectValue:
			if (value_.map_)
				return iterator(value_.map_->end());
			break;
		default:
			break;
		}
		return iterator();
	}

	// class PathArgument
	// //////////////////////////////////////////////////////////////////

	PathArgument::PathArgument() : key_(), index_(), kind_(kindNone) {}

	PathArgument::PathArgument(ArrayIndex index)
		: key_(), index_(index), kind_(kindIndex) {}

	PathArgument::PathArgument(const char* key)
		: key_(key), index_(), kind_(kindKey) {}

	PathArgument::PathArgument(const JSONCPP_STRING& key)
		: key_(key.c_str()), index_(), kind_(kindKey) {}

	// class Path
	// //////////////////////////////////////////////////////////////////

	Path::Path(const JSONCPP_STRING& path,
		const PathArgument& a1,
		const PathArgument& a2,
		const PathArgument& a3,
		const PathArgument& a4,
		const PathArgument& a5) {
		InArgs in;
		in.push_back(&a1);
		in.push_back(&a2);
		in.push_back(&a3);
		in.push_back(&a4);
		in.push_back(&a5);
		makePath(path, in);
	}

	void Path::makePath(const JSONCPP_STRING& path, const InArgs& in) {
		const char* current = path.c_str();
		const char* end = current + path.length();
		InArgs::const_iterator itInArg = in.begin();
		while (current != end) {
			if (*current == '[') {
				++current;
				if (*current == '%')
					addPathInArg(path, in, itInArg, PathArgument::kindIndex);
				else {
					ArrayIndex index = 0;
					for (; current != end && *current >= '0' && *current <= '9'; ++current)
						index = index * 10 + ArrayIndex(*current - '0');
					args_.push_back(index);
				}
				if (current == end || *++current != ']')
					invalidPath(path, int(current - path.c_str()));
			}
			else if (*current == '%') {
				addPathInArg(path, in, itInArg, PathArgument::kindKey);
				++current;
			}
			else if (*current == '.' || *current == ']') {
				++current;
			}
			else {
				const char* beginName = current;
				while (current != end && !strchr("[.", *current))
					++current;
				args_.push_back(JSONCPP_STRING(beginName, current));
			}
		}
	}

	void Path::addPathInArg(const JSONCPP_STRING& /*path*/,
		const InArgs& in,
		InArgs::const_iterator& itInArg,
		PathArgument::Kind kind) {
		if (itInArg == in.end()) {
			// Error: missing argument %d
		}
		else if ((*itInArg)->kind_ != kind) {
			// Error: bad argument type
		}
		else {
			args_.push_back(**itInArg++);
		}
	}

	void Path::invalidPath(const JSONCPP_STRING& /*path*/, int /*location*/) {
		// Error: invalid path.
	}

	const Value& Path::resolve(const Value& root) const {
		const Value* node = &root;
		for (Args::const_iterator it = args_.begin(); it != args_.end(); ++it) {
			const PathArgument& arg = *it;
			if (arg.kind_ == PathArgument::kindIndex) {
				if (!node->isArray() || !node->isValidIndex(arg.index_)) {
					// Error: unable to resolve path (array value expected at position...
					return Value::null;
				}
				node = &((*node)[arg.index_]);
			}
			else if (arg.kind_ == PathArgument::kindKey) {
				if (!node->isObject()) {
					// Error: unable to resolve path (object value expected at position...)
					return Value::null;
				}
				node = &((*node)[arg.key_]);
				if (node == &Value::nullSingleton()) {
					// Error: unable to resolve path (object has no member named '' at
					// position...)
					return Value::null;
				}
			}
		}
		return *node;
	}

	Value Path::resolve(const Value& root, const Value& defaultValue) const {
		const Value* node = &root;
		for (Args::const_iterator it = args_.begin(); it != args_.end(); ++it) {
			const PathArgument& arg = *it;
			if (arg.kind_ == PathArgument::kindIndex) {
				if (!node->isArray() || !node->isValidIndex(arg.index_))
					return defaultValue;
				node = &((*node)[arg.index_]);
			}
			else if (arg.kind_ == PathArgument::kindKey) {
				if (!node->isObject())
					return defaultValue;
				node = &((*node)[arg.key_]);
				if (node == &Value::nullSingleton())
					return defaultValue;
			}
		}
		return *node;
	}

	Value& Path::make(Value& root) const {
		Value* node = &root;
		for (Args::const_iterator it = args_.begin(); it != args_.end(); ++it) {
			const PathArgument& arg = *it;
			if (arg.kind_ == PathArgument::kindIndex) {
				if (!node->isArray()) {
					// Error: node is not an array at position ...
				}
				node = &((*node)[arg.index_]);
			}
			else if (arg.kind_ == PathArgument::kindKey) {
				if (!node->isObject()) {
					// Error: node is not an object at position...
				}
				node = &((*node)[arg.key_]);
			}
		}
		return *node;
	}

} // namespace Json

// //////////////////////////////////////////////////////////////////////
// End of content of file: src/lib_json/json_value.cpp
// //////////////////////////////////////////////////////////////////////






// //////////////////////////////////////////////////////////////////////
// Beginning of content of file: src/lib_json/json_writer.cpp
// //////////////////////////////////////////////////////////////////////

// Copyright 2011 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#if !defined(JSON_IS_AMALGAMATION)
#include <json/writer.h>
#include "json_tool.h"
#endif // if !defined(JSON_IS_AMALGAMATION)
#include <iomanip>
#include <memory>
#include <sstream>
#include <utility>
#include <set>
#include <cassert>
#include <cstring>
#include <cstdio>

#if defined(_MSC_VER) && _MSC_VER >= 1200 && _MSC_VER < 1800 // Between VC++ 6.0 and VC++ 11.0
#include <float.h>
#define isfinite _finite
#elif defined(__sun) && defined(__SVR4) //Solaris
#if !defined(isfinite)
#include <ieeefp.h>
#define isfinite finite
#endif
#elif defined(_AIX)
#if !defined(isfinite)
#include <math.h>
#define isfinite finite
#endif
#elif defined(__hpux)
#if !defined(isfinite)
#if defined(__ia64) && !defined(finite)
#define isfinite(x) ((sizeof(x) == sizeof(float) ? \
                     _Isfinitef(x) : _IsFinite(x)))
#else
#include <math.h>
#define isfinite finite
#endif
#endif
#else
#include <cmath>
#if !(defined(__QNXNTO__)) // QNX already defines isfinite
#define isfinite std::isfinite
#endif
#endif

#if defined(_MSC_VER)
#if !defined(WINCE) && defined(__STDC_SECURE_LIB__) && _MSC_VER >= 1500 // VC++ 9.0 and above
#define snprintf sprintf_s
#elif _MSC_VER >= 1900 // VC++ 14.0 and above
#define snprintf std::snprintf
#else
#define snprintf _snprintf
#endif
#elif defined(__ANDROID__) || defined(__QNXNTO__)
#define snprintf snprintf
#elif __cplusplus >= 201103L
#if !defined(__MINGW32__) && !defined(__CYGWIN__)
#define snprintf std::snprintf
#endif
#endif

#if defined(__BORLANDC__)  
#include <float.h>
#define isfinite _finite
#define snprintf _snprintf
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1400 // VC++ 8.0
// Disable warning about strdup being deprecated.
#pragma warning(disable : 4996)
#endif

namespace Json {

#if __cplusplus >= 201103L || (defined(_CPPLIB_VER) && _CPPLIB_VER >= 520)
	typedef std::unique_ptr<StreamWriter> StreamWriterPtr;
#else
	typedef std::auto_ptr<StreamWriter>   StreamWriterPtr;
#endif

	static bool containsControlCharacter(const char* str) {
		while (*str) {
			if (isControlCharacter(*(str++)))
				return true;
		}
		return false;
	}

	static bool containsControlCharacter0(const char* str, unsigned len) {
		char const* end = str + len;
		while (end != str) {
			if (isControlCharacter(*str) || 0 == *str)
				return true;
			++str;
		}
		return false;
	}

	JSONCPP_STRING valueToString(LargestInt value) {
		UIntToStringBuffer buffer;
		char* current = buffer + sizeof(buffer);
		if (value == Value::minLargestInt) {
			uintToString(LargestUInt(Value::maxLargestInt) + 1, current);
			*--current = '-';
		}
		else if (value < 0) {
			uintToString(LargestUInt(-value), current);
			*--current = '-';
		}
		else {
			uintToString(LargestUInt(value), current);
		}
		assert(current >= buffer);
		return current;
	}

	JSONCPP_STRING valueToString(LargestUInt value) {
		UIntToStringBuffer buffer;
		char* current = buffer + sizeof(buffer);
		uintToString(value, current);
		assert(current >= buffer);
		return current;
	}

#if defined(JSON_HAS_INT64)

	JSONCPP_STRING valueToString(Int value) {
		return valueToString(LargestInt(value));
	}

	JSONCPP_STRING valueToString(UInt value) {
		return valueToString(LargestUInt(value));
	}

#endif // # if defined(JSON_HAS_INT64)

	namespace {
		JSONCPP_STRING valueToString(double value, bool useSpecialFloats, unsigned int precision) {
			// Allocate a buffer that is more than large enough to store the 16 digits of
			// precision requested below.
			char buffer[32];
			int len = -1;

			char formatString[6];
			sprintf(formatString, "%%.%dg", precision);

			// Print into the buffer. We need not request the alternative representation
			// that always has a decimal point because JSON doesn't distingish the
			// concepts of reals and integers.
			if (isfinite(value)) {
				len = snprintf(buffer, sizeof(buffer), formatString, value);
			}
			else {
				// IEEE standard states that NaN values will not compare to themselves
				if (value != value) {
					len = snprintf(buffer, sizeof(buffer), useSpecialFloats ? "NaN" : "null");
				}
				else if (value < 0) {
					len = snprintf(buffer, sizeof(buffer), useSpecialFloats ? "-Infinity" : "-1e+9999");
				}
				else {
					len = snprintf(buffer, sizeof(buffer), useSpecialFloats ? "Infinity" : "1e+9999");
				}
				// For those, we do not need to call fixNumLoc, but it is fast.
			}
			assert(len >= 0);
			fixNumericLocale(buffer, buffer + len);
			return buffer;
		}
	}

	JSONCPP_STRING valueToString(double value) { return valueToString(value, false, 17); }

	JSONCPP_STRING valueToString(bool value) { return value ? "true" : "false"; }

	JSONCPP_STRING valueToQuotedString(const char* value) {
		if (value == NULL)
			return "";
		// Not sure how to handle unicode...
		if (strpbrk(value, "\"\\\b\f\n\r\t") == NULL &&
			!containsControlCharacter(value))
			return JSONCPP_STRING("\"") + value + "\"";
		// We have to walk value and escape any special characters.
		// Appending to JSONCPP_STRING is not efficient, but this should be rare.
		// (Note: forward slashes are *not* rare, but I am not escaping them.)
		JSONCPP_STRING::size_type maxsize =
			strlen(value) * 2 + 3; // allescaped+quotes+NULL
		JSONCPP_STRING result;
		result.reserve(maxsize); // to avoid lots of mallocs
		result += "\"";
		for (const char* c = value; *c != 0; ++c) {
			switch (*c) {
			case '\"':
				result += "\\\"";
				break;
			case '\\':
				result += "\\\\";
				break;
			case '\b':
				result += "\\b";
				break;
			case '\f':
				result += "\\f";
				break;
			case '\n':
				result += "\\n";
				break;
			case '\r':
				result += "\\r";
				break;
			case '\t':
				result += "\\t";
				break;
				// case '/':
				// Even though \/ is considered a legal escape in JSON, a bare
				// slash is also legal, so I see no reason to escape it.
				// (I hope I am not misunderstanding something.
				// blep notes: actually escaping \/ may be useful in javascript to avoid </
				// sequence.
				// Should add a flag to allow this compatibility mode and prevent this
				// sequence from occurring.
			default:
				if (isControlCharacter(*c)) {
					JSONCPP_OSTRINGSTREAM oss;
					oss << "\\u" << std::hex << std::uppercase << std::setfill('0')
						<< std::setw(4) << static_cast<int>(*c);
					result += oss.str();
				}
				else {
					result += *c;
				}
				break;
			}
		}
		result += "\"";
		return result;
	}

	// https://github.com/upcaste/upcaste/blob/master/src/upcore/src/cstring/strnpbrk.cpp
	static char const* strnpbrk(char const* s, char const* accept, size_t n) {
		assert((s || !n) && accept);

		char const* const end = s + n;
		for (char const* cur = s; cur < end; ++cur) {
			int const c = *cur;
			for (char const* a = accept; *a; ++a) {
				if (*a == c) {
					return cur;
				}
			}
		}
		return NULL;
	}
	static JSONCPP_STRING valueToQuotedStringN(const char* value, unsigned length) {
		if (value == NULL)
			return "";
		// Not sure how to handle unicode...
		if (strnpbrk(value, "\"\\\b\f\n\r\t", length) == NULL &&
			!containsControlCharacter0(value, length))
			return JSONCPP_STRING("\"") + value + "\"";
		// We have to walk value and escape any special characters.
		// Appending to JSONCPP_STRING is not efficient, but this should be rare.
		// (Note: forward slashes are *not* rare, but I am not escaping them.)
		JSONCPP_STRING::size_type maxsize =
			length * 2 + 3; // allescaped+quotes+NULL
		JSONCPP_STRING result;
		result.reserve(maxsize); // to avoid lots of mallocs
		result += "\"";
		char const* end = value + length;
		for (const char* c = value; c != end; ++c) {
			switch (*c) {
			case '\"':
				result += "\\\"";
				break;
			case '\\':
				result += "\\\\";
				break;
			case '\b':
				result += "\\b";
				break;
			case '\f':
				result += "\\f";
				break;
			case '\n':
				result += "\\n";
				break;
			case '\r':
				result += "\\r";
				break;
			case '\t':
				result += "\\t";
				break;
				// case '/':
				// Even though \/ is considered a legal escape in JSON, a bare
				// slash is also legal, so I see no reason to escape it.
				// (I hope I am not misunderstanding something.)
				// blep notes: actually escaping \/ may be useful in javascript to avoid </
				// sequence.
				// Should add a flag to allow this compatibility mode and prevent this
				// sequence from occurring.
			default:
				if ((isControlCharacter(*c)) || (*c == 0)) {
					JSONCPP_OSTRINGSTREAM oss;
					oss << "\\u" << std::hex << std::uppercase << std::setfill('0')
						<< std::setw(4) << static_cast<int>(*c);
					result += oss.str();
				}
				else {
					result += *c;
				}
				break;
			}
		}
		result += "\"";
		return result;
	}

	// Class Writer
	// //////////////////////////////////////////////////////////////////
	Writer::~Writer() {}

	// Class FastWriter
	// //////////////////////////////////////////////////////////////////

	FastWriter::FastWriter()
		: yamlCompatiblityEnabled_(false), dropNullPlaceholders_(false),
		omitEndingLineFeed_(false) {}

	void FastWriter::enableYAMLCompatibility() { yamlCompatiblityEnabled_ = true; }

	void FastWriter::dropNullPlaceholders() { dropNullPlaceholders_ = true; }

	void FastWriter::omitEndingLineFeed() { omitEndingLineFeed_ = true; }

	JSONCPP_STRING FastWriter::write(const Value& root) {
		document_ = "";
		writeValue(root);
		if (!omitEndingLineFeed_)
			document_ += "\n";
		return document_;
	}

	void FastWriter::writeValue(const Value& value) {
		switch (value.type()) {
		case nullValue:
			if (!dropNullPlaceholders_)
				document_ += "null";
			break;
		case intValue:
			document_ += valueToString(value.asLargestInt());
			break;
		case uintValue:
			document_ += valueToString(value.asLargestUInt());
			break;
		case realValue:
			document_ += valueToString(value.asDouble());
			break;
		case stringValue:
		{
			// Is NULL possible for value.string_? No.
			char const* str;
			char const* end;
			bool ok = value.getString(&str, &end);
			if (ok) document_ += valueToQuotedStringN(str, static_cast<unsigned>(end - str));
			break;
		}
		case booleanValue:
			document_ += valueToString(value.asBool());
			break;
		case arrayValue: {
			document_ += '[';
			ArrayIndex size = value.size();
			for (ArrayIndex index = 0; index < size; ++index) {
				if (index > 0)
					document_ += ',';
				writeValue(value[index]);
			}
			document_ += ']';
		} break;
		case objectValue: {
			Value::Members members(value.getMemberNames());
			document_ += '{';
			for (Value::Members::iterator it = members.begin(); it != members.end();
				++it) {
				const JSONCPP_STRING& name = *it;
				if (it != members.begin())
					document_ += ',';
				document_ += valueToQuotedStringN(name.data(), static_cast<unsigned>(name.length()));
				document_ += yamlCompatiblityEnabled_ ? ": " : ":";
				writeValue(value[name]);
			}
			document_ += '}';
		} break;
		}
	}

	// Class StyledWriter
	// //////////////////////////////////////////////////////////////////

	StyledWriter::StyledWriter()
		: rightMargin_(74), indentSize_(3), addChildValues_() {}

	JSONCPP_STRING StyledWriter::write(const Value& root) {
		document_ = "";
		addChildValues_ = false;
		indentString_ = "";
		writeCommentBeforeValue(root);
		writeValue(root);
		writeCommentAfterValueOnSameLine(root);
		document_ += "\n";
		return document_;
	}

	void StyledWriter::writeValue(const Value& value) {
		switch (value.type()) {
		case nullValue:
			pushValue("null");
			break;
		case intValue:
			pushValue(valueToString(value.asLargestInt()));
			break;
		case uintValue:
			pushValue(valueToString(value.asLargestUInt()));
			break;
		case realValue:
			pushValue(valueToString(value.asDouble()));
			break;
		case stringValue:
		{
			// Is NULL possible for value.string_? No.
			char const* str;
			char const* end;
			bool ok = value.getString(&str, &end);
			if (ok) pushValue(valueToQuotedStringN(str, static_cast<unsigned>(end - str)));
			else pushValue("");
			break;
		}
		case booleanValue:
			pushValue(valueToString(value.asBool()));
			break;
		case arrayValue:
			writeArrayValue(value);
			break;
		case objectValue: {
			Value::Members members(value.getMemberNames());
			if (members.empty())
				pushValue("{}");
			else {
				writeWithIndent("{");
				indent();
				Value::Members::iterator it = members.begin();
				for (;;) {
					const JSONCPP_STRING& name = *it;
					const Value& childValue = value[name];
					writeCommentBeforeValue(childValue);
					writeWithIndent(valueToQuotedString(name.c_str()));
					document_ += " : ";
					writeValue(childValue);
					if (++it == members.end()) {
						writeCommentAfterValueOnSameLine(childValue);
						break;
					}
					document_ += ',';
					writeCommentAfterValueOnSameLine(childValue);
				}
				unindent();
				writeWithIndent("}");
			}
		} break;
		}
	}

	void StyledWriter::writeArrayValue(const Value& value) {
		unsigned size = value.size();
		if (size == 0)
			pushValue("[]");
		else {
			bool isArrayMultiLine = isMultineArray(value);
			if (isArrayMultiLine) {
				writeWithIndent("[");
				indent();
				bool hasChildValue = !childValues_.empty();
				unsigned index = 0;
				for (;;) {
					const Value& childValue = value[index];
					writeCommentBeforeValue(childValue);
					if (hasChildValue)
						writeWithIndent(childValues_[index]);
					else {
						writeIndent();
						writeValue(childValue);
					}
					if (++index == size) {
						writeCommentAfterValueOnSameLine(childValue);
						break;
					}
					document_ += ',';
					writeCommentAfterValueOnSameLine(childValue);
				}
				unindent();
				writeWithIndent("]");
			}
			else // output on a single line
			{
				assert(childValues_.size() == size);
				document_ += "[ ";
				for (unsigned index = 0; index < size; ++index) {
					if (index > 0)
						document_ += ", ";
					document_ += childValues_[index];
				}
				document_ += " ]";
			}
		}
	}

	bool StyledWriter::isMultineArray(const Value& value) {
		ArrayIndex const size = value.size();
		bool isMultiLine = size * 3 >= rightMargin_;
		childValues_.clear();
		for (ArrayIndex index = 0; index < size && !isMultiLine; ++index) {
			const Value& childValue = value[index];
			isMultiLine = ((childValue.isArray() || childValue.isObject()) &&
				childValue.size() > 0);
		}
		if (!isMultiLine) // check if line length > max line length
		{
			childValues_.reserve(size);
			addChildValues_ = true;
			ArrayIndex lineLength = 4 + (size - 1) * 2; // '[ ' + ', '*n + ' ]'
			for (ArrayIndex index = 0; index < size; ++index) {
				if (hasCommentForValue(value[index])) {
					isMultiLine = true;
				}
				writeValue(value[index]);
				lineLength += static_cast<ArrayIndex>(childValues_[index].length());
			}
			addChildValues_ = false;
			isMultiLine = isMultiLine || lineLength >= rightMargin_;
		}
		return isMultiLine;
	}

	void StyledWriter::pushValue(const JSONCPP_STRING& value) {
		if (addChildValues_)
			childValues_.push_back(value);
		else
			document_ += value;
	}

	void StyledWriter::writeIndent() {
		if (!document_.empty()) {
			char last = document_[document_.length() - 1];
			if (last == ' ') // already indented
				return;
			if (last != '\n') // Comments may add new-line
				document_ += '\n';
		}
		document_ += indentString_;
	}

	void StyledWriter::writeWithIndent(const JSONCPP_STRING& value) {
		writeIndent();
		document_ += value;
	}

	void StyledWriter::indent() { indentString_ += JSONCPP_STRING(indentSize_, ' '); }

	void StyledWriter::unindent() {
		assert(indentString_.size() >= indentSize_);
		indentString_.resize(indentString_.size() - indentSize_);
	}

	void StyledWriter::writeCommentBeforeValue(const Value& root) {
		if (!root.hasComment(commentBefore))
			return;

		document_ += "\n";
		writeIndent();
		const JSONCPP_STRING& comment = root.getComment(commentBefore);
		JSONCPP_STRING::const_iterator iter = comment.begin();
		while (iter != comment.end()) {
			document_ += *iter;
			if (*iter == '\n' &&
				(iter != comment.end() && *(iter + 1) == '/'))
				writeIndent();
			++iter;
		}

		// Comments are stripped of trailing newlines, so add one here
		document_ += "\n";
	}

	void StyledWriter::writeCommentAfterValueOnSameLine(const Value& root) {
		if (root.hasComment(commentAfterOnSameLine))
			document_ += " " + root.getComment(commentAfterOnSameLine);

		if (root.hasComment(commentAfter)) {
			document_ += "\n";
			document_ += root.getComment(commentAfter);
			document_ += "\n";
		}
	}

	bool StyledWriter::hasCommentForValue(const Value& value) {
		return value.hasComment(commentBefore) ||
			value.hasComment(commentAfterOnSameLine) ||
			value.hasComment(commentAfter);
	}

	// Class StyledStreamWriter
	// //////////////////////////////////////////////////////////////////

	StyledStreamWriter::StyledStreamWriter(JSONCPP_STRING indentation)
		: document_(NULL), rightMargin_(74), indentation_(indentation),
		addChildValues_() {}

	void StyledStreamWriter::write(JSONCPP_OSTREAM& out, const Value& root) {
		document_ = &out;
		addChildValues_ = false;
		indentString_ = "";
		indented_ = true;
		writeCommentBeforeValue(root);
		if (!indented_) writeIndent();
		indented_ = true;
		writeValue(root);
		writeCommentAfterValueOnSameLine(root);
		*document_ << "\n";
		document_ = NULL; // Forget the stream, for safety.
	}

	void StyledStreamWriter::writeValue(const Value& value) {
		switch (value.type()) {
		case nullValue:
			pushValue("null");
			break;
		case intValue:
			pushValue(valueToString(value.asLargestInt()));
			break;
		case uintValue:
			pushValue(valueToString(value.asLargestUInt()));
			break;
		case realValue:
			pushValue(valueToString(value.asDouble()));
			break;
		case stringValue:
		{
			// Is NULL possible for value.string_? No.
			char const* str;
			char const* end;
			bool ok = value.getString(&str, &end);
			if (ok) pushValue(valueToQuotedStringN(str, static_cast<unsigned>(end - str)));
			else pushValue("");
			break;
		}
		case booleanValue:
			pushValue(valueToString(value.asBool()));
			break;
		case arrayValue:
			writeArrayValue(value);
			break;
		case objectValue: {
			Value::Members members(value.getMemberNames());
			if (members.empty())
				pushValue("{}");
			else {
				writeWithIndent("{");
				indent();
				Value::Members::iterator it = members.begin();
				for (;;) {
					const JSONCPP_STRING& name = *it;
					const Value& childValue = value[name];
					writeCommentBeforeValue(childValue);
					writeWithIndent(valueToQuotedString(name.c_str()));
					*document_ << " : ";
					writeValue(childValue);
					if (++it == members.end()) {
						writeCommentAfterValueOnSameLine(childValue);
						break;
					}
					*document_ << ",";
					writeCommentAfterValueOnSameLine(childValue);
				}
				unindent();
				writeWithIndent("}");
			}
		} break;
		}
	}

	void StyledStreamWriter::writeArrayValue(const Value& value) {
		unsigned size = value.size();
		if (size == 0)
			pushValue("[]");
		else {
			bool isArrayMultiLine = isMultineArray(value);
			if (isArrayMultiLine) {
				writeWithIndent("[");
				indent();
				bool hasChildValue = !childValues_.empty();
				unsigned index = 0;
				for (;;) {
					const Value& childValue = value[index];
					writeCommentBeforeValue(childValue);
					if (hasChildValue)
						writeWithIndent(childValues_[index]);
					else {
						if (!indented_) writeIndent();
						indented_ = true;
						writeValue(childValue);
						indented_ = false;
					}
					if (++index == size) {
						writeCommentAfterValueOnSameLine(childValue);
						break;
					}
					*document_ << ",";
					writeCommentAfterValueOnSameLine(childValue);
				}
				unindent();
				writeWithIndent("]");
			}
			else // output on a single line
			{
				assert(childValues_.size() == size);
				*document_ << "[ ";
				for (unsigned index = 0; index < size; ++index) {
					if (index > 0)
						* document_ << ", ";
					*document_ << childValues_[index];
				}
				*document_ << " ]";
			}
		}
	}

	bool StyledStreamWriter::isMultineArray(const Value& value) {
		ArrayIndex const size = value.size();
		bool isMultiLine = size * 3 >= rightMargin_;
		childValues_.clear();
		for (ArrayIndex index = 0; index < size && !isMultiLine; ++index) {
			const Value& childValue = value[index];
			isMultiLine = ((childValue.isArray() || childValue.isObject()) &&
				childValue.size() > 0);
		}
		if (!isMultiLine) // check if line length > max line length
		{
			childValues_.reserve(size);
			addChildValues_ = true;
			ArrayIndex lineLength = 4 + (size - 1) * 2; // '[ ' + ', '*n + ' ]'
			for (ArrayIndex index = 0; index < size; ++index) {
				if (hasCommentForValue(value[index])) {
					isMultiLine = true;
				}
				writeValue(value[index]);
				lineLength += static_cast<ArrayIndex>(childValues_[index].length());
			}
			addChildValues_ = false;
			isMultiLine = isMultiLine || lineLength >= rightMargin_;
		}
		return isMultiLine;
	}

	void StyledStreamWriter::pushValue(const JSONCPP_STRING& value) {
		if (addChildValues_)
			childValues_.push_back(value);
		else
			*document_ << value;
	}

	void StyledStreamWriter::writeIndent() {
		// blep intended this to look at the so-far-written string
		// to determine whether we are already indented, but
		// with a stream we cannot do that. So we rely on some saved state.
		// The caller checks indented_.
		*document_ << '\n' << indentString_;
	}

	void StyledStreamWriter::writeWithIndent(const JSONCPP_STRING& value) {
		if (!indented_) writeIndent();
		*document_ << value;
		indented_ = false;
	}

	void StyledStreamWriter::indent() { indentString_ += indentation_; }

	void StyledStreamWriter::unindent() {
		assert(indentString_.size() >= indentation_.size());
		indentString_.resize(indentString_.size() - indentation_.size());
	}

	void StyledStreamWriter::writeCommentBeforeValue(const Value& root) {
		if (!root.hasComment(commentBefore))
			return;

		if (!indented_) writeIndent();
		const JSONCPP_STRING& comment = root.getComment(commentBefore);
		JSONCPP_STRING::const_iterator iter = comment.begin();
		while (iter != comment.end()) {
			*document_ << *iter;
			if (*iter == '\n' &&
				(iter != comment.end() && *(iter + 1) == '/'))
				// writeIndent();  // would include newline
				* document_ << indentString_;
			++iter;
		}
		indented_ = false;
	}

	void StyledStreamWriter::writeCommentAfterValueOnSameLine(const Value& root) {
		if (root.hasComment(commentAfterOnSameLine))
			* document_ << ' ' << root.getComment(commentAfterOnSameLine);

		if (root.hasComment(commentAfter)) {
			writeIndent();
			*document_ << root.getComment(commentAfter);
		}
		indented_ = false;
	}

	bool StyledStreamWriter::hasCommentForValue(const Value& value) {
		return value.hasComment(commentBefore) ||
			value.hasComment(commentAfterOnSameLine) ||
			value.hasComment(commentAfter);
	}

	//////////////////////////
	// BuiltStyledStreamWriter

	/// Scoped enums are not available until C++11.
	struct CommentStyle {
		/// Decide whether to write comments.
		enum Enum {
			None,  ///< Drop all comments.
			Most,  ///< Recover odd behavior of previous versions (not implemented yet).
			All  ///< Keep all comments.
		};
	};

	struct BuiltStyledStreamWriter : public StreamWriter
	{
		BuiltStyledStreamWriter(
			JSONCPP_STRING const& indentation,
			CommentStyle::Enum cs,
			JSONCPP_STRING const& colonSymbol,
			JSONCPP_STRING const& nullSymbol,
			JSONCPP_STRING const& endingLineFeedSymbol,
			bool useSpecialFloats,
			unsigned int precision);
		int write(Value const& root, JSONCPP_OSTREAM* sout) JSONCPP_OVERRIDE;
	private:
		void writeValue(Value const& value);
		void writeArrayValue(Value const& value);
		bool isMultineArray(Value const& value);
		void pushValue(JSONCPP_STRING const& value);
		void writeIndent();
		void writeWithIndent(JSONCPP_STRING const& value);
		void indent();
		void unindent();
		void writeCommentBeforeValue(Value const& root);
		void writeCommentAfterValueOnSameLine(Value const& root);
		static bool hasCommentForValue(const Value& value);

		typedef std::vector<JSONCPP_STRING> ChildValues;

		ChildValues childValues_;
		JSONCPP_STRING indentString_;
		unsigned int rightMargin_;
		JSONCPP_STRING indentation_;
		CommentStyle::Enum cs_;
		JSONCPP_STRING colonSymbol_;
		JSONCPP_STRING nullSymbol_;
		JSONCPP_STRING endingLineFeedSymbol_;
		bool addChildValues_ : 1;
		bool indented_ : 1;
		bool useSpecialFloats_ : 1;
		unsigned int precision_;
	};
	BuiltStyledStreamWriter::BuiltStyledStreamWriter(
		JSONCPP_STRING const& indentation,
		CommentStyle::Enum cs,
		JSONCPP_STRING const& colonSymbol,
		JSONCPP_STRING const& nullSymbol,
		JSONCPP_STRING const& endingLineFeedSymbol,
		bool useSpecialFloats,
		unsigned int precision)
		: rightMargin_(74)
		, indentation_(indentation)
		, cs_(cs)
		, colonSymbol_(colonSymbol)
		, nullSymbol_(nullSymbol)
		, endingLineFeedSymbol_(endingLineFeedSymbol)
		, addChildValues_(false)
		, indented_(false)
		, useSpecialFloats_(useSpecialFloats)
		, precision_(precision)
	{
	}
	int BuiltStyledStreamWriter::write(Value const& root, JSONCPP_OSTREAM* sout)
	{
		sout_ = sout;
		addChildValues_ = false;
		indented_ = true;
		indentString_ = "";
		writeCommentBeforeValue(root);
		if (!indented_) writeIndent();
		indented_ = true;
		writeValue(root);
		writeCommentAfterValueOnSameLine(root);
		*sout_ << endingLineFeedSymbol_;
		sout_ = NULL;
		return 0;
	}
	void BuiltStyledStreamWriter::writeValue(Value const& value) {
		switch (value.type()) {
		case nullValue:
			pushValue(nullSymbol_);
			break;
		case intValue:
			pushValue(valueToString(value.asLargestInt()));
			break;
		case uintValue:
			pushValue(valueToString(value.asLargestUInt()));
			break;
		case realValue:
			pushValue(valueToString(value.asDouble(), useSpecialFloats_, precision_));
			break;
		case stringValue:
		{
			// Is NULL is possible for value.string_? No.
			char const* str;
			char const* end;
			bool ok = value.getString(&str, &end);
			if (ok) pushValue(valueToQuotedStringN(str, static_cast<unsigned>(end - str)));
			else pushValue("");
			break;
		}
		case booleanValue:
			pushValue(valueToString(value.asBool()));
			break;
		case arrayValue:
			writeArrayValue(value);
			break;
		case objectValue: {
			Value::Members members(value.getMemberNames());
			if (members.empty())
				pushValue("{}");
			else {
				writeWithIndent("{");
				indent();
				Value::Members::iterator it = members.begin();
				for (;;) {
					JSONCPP_STRING const& name = *it;
					Value const& childValue = value[name];
					writeCommentBeforeValue(childValue);
					writeWithIndent(valueToQuotedStringN(name.data(), static_cast<unsigned>(name.length())));
					*sout_ << colonSymbol_;
					writeValue(childValue);
					if (++it == members.end()) {
						writeCommentAfterValueOnSameLine(childValue);
						break;
					}
					*sout_ << ",";
					writeCommentAfterValueOnSameLine(childValue);
				}
				unindent();
				writeWithIndent("}");
			}
		} break;
		}
	}

	void BuiltStyledStreamWriter::writeArrayValue(Value const& value) {
		unsigned size = value.size();
		if (size == 0)
			pushValue("[]");
		else {
			bool isMultiLine = (cs_ == CommentStyle::All) || isMultineArray(value);
			if (isMultiLine) {
				writeWithIndent("[");
				indent();
				bool hasChildValue = !childValues_.empty();
				unsigned index = 0;
				for (;;) {
					Value const& childValue = value[index];
					writeCommentBeforeValue(childValue);
					if (hasChildValue)
						writeWithIndent(childValues_[index]);
					else {
						if (!indented_) writeIndent();
						indented_ = true;
						writeValue(childValue);
						indented_ = false;
					}
					if (++index == size) {
						writeCommentAfterValueOnSameLine(childValue);
						break;
					}
					*sout_ << ",";
					writeCommentAfterValueOnSameLine(childValue);
				}
				unindent();
				writeWithIndent("]");
			}
			else // output on a single line
			{
				assert(childValues_.size() == size);
				*sout_ << "[";
				if (!indentation_.empty())* sout_ << " ";
				for (unsigned index = 0; index < size; ++index) {
					if (index > 0)
						* sout_ << ((!indentation_.empty()) ? ", " : ",");
					*sout_ << childValues_[index];
				}
				if (!indentation_.empty())* sout_ << " ";
				*sout_ << "]";
			}
		}
	}

	bool BuiltStyledStreamWriter::isMultineArray(Value const& value) {
		ArrayIndex const size = value.size();
		bool isMultiLine = size * 3 >= rightMargin_;
		childValues_.clear();
		for (ArrayIndex index = 0; index < size && !isMultiLine; ++index) {
			Value const& childValue = value[index];
			isMultiLine = ((childValue.isArray() || childValue.isObject()) &&
				childValue.size() > 0);
		}
		if (!isMultiLine) // check if line length > max line length
		{
			childValues_.reserve(size);
			addChildValues_ = true;
			ArrayIndex lineLength = 4 + (size - 1) * 2; // '[ ' + ', '*n + ' ]'
			for (ArrayIndex index = 0; index < size; ++index) {
				if (hasCommentForValue(value[index])) {
					isMultiLine = true;
				}
				writeValue(value[index]);
				lineLength += static_cast<ArrayIndex>(childValues_[index].length());
			}
			addChildValues_ = false;
			isMultiLine = isMultiLine || lineLength >= rightMargin_;
		}
		return isMultiLine;
	}

	void BuiltStyledStreamWriter::pushValue(JSONCPP_STRING const& value) {
		if (addChildValues_)
			childValues_.push_back(value);
		else
			*sout_ << value;
	}

	void BuiltStyledStreamWriter::writeIndent() {
		// blep intended this to look at the so-far-written string
		// to determine whether we are already indented, but
		// with a stream we cannot do that. So we rely on some saved state.
		// The caller checks indented_.

		if (!indentation_.empty()) {
			// In this case, drop newlines too.
			*sout_ << '\n' << indentString_;
		}
	}

	void BuiltStyledStreamWriter::writeWithIndent(JSONCPP_STRING const& value) {
		if (!indented_) writeIndent();
		*sout_ << value;
		indented_ = false;
	}

	void BuiltStyledStreamWriter::indent() { indentString_ += indentation_; }

	void BuiltStyledStreamWriter::unindent() {
		assert(indentString_.size() >= indentation_.size());
		indentString_.resize(indentString_.size() - indentation_.size());
	}

	void BuiltStyledStreamWriter::writeCommentBeforeValue(Value const& root) {
		if (cs_ == CommentStyle::None) return;
		if (!root.hasComment(commentBefore))
			return;

		if (!indented_) writeIndent();
		const JSONCPP_STRING& comment = root.getComment(commentBefore);
		JSONCPP_STRING::const_iterator iter = comment.begin();
		while (iter != comment.end()) {
			*sout_ << *iter;
			if (*iter == '\n' &&
				(iter != comment.end() && *(iter + 1) == '/'))
				// writeIndent();  // would write extra newline
				* sout_ << indentString_;
			++iter;
		}
		indented_ = false;
	}

	void BuiltStyledStreamWriter::writeCommentAfterValueOnSameLine(Value const& root) {
		if (cs_ == CommentStyle::None) return;
		if (root.hasComment(commentAfterOnSameLine))
			* sout_ << " " + root.getComment(commentAfterOnSameLine);

		if (root.hasComment(commentAfter)) {
			writeIndent();
			*sout_ << root.getComment(commentAfter);
		}
	}

	// static
	bool BuiltStyledStreamWriter::hasCommentForValue(const Value& value) {
		return value.hasComment(commentBefore) ||
			value.hasComment(commentAfterOnSameLine) ||
			value.hasComment(commentAfter);
	}

	///////////////
	// StreamWriter

	StreamWriter::StreamWriter()
		: sout_(NULL)
	{
	}
	StreamWriter::~StreamWriter()
	{
	}
	StreamWriter::Factory::~Factory()
	{}
	StreamWriterBuilder::StreamWriterBuilder()
	{
		setDefaults(&settings_);
	}
	StreamWriterBuilder::~StreamWriterBuilder()
	{}
	StreamWriter* StreamWriterBuilder::newStreamWriter() const
	{
		JSONCPP_STRING indentation = settings_["indentation"].asString();
		JSONCPP_STRING cs_str = settings_["commentStyle"].asString();
		bool eyc = settings_["enableYAMLCompatibility"].asBool();
		bool dnp = settings_["dropNullPlaceholders"].asBool();
		bool usf = settings_["useSpecialFloats"].asBool();
		unsigned int pre = settings_["precision"].asUInt();
		CommentStyle::Enum cs = CommentStyle::All;
		if (cs_str == "All") {
			cs = CommentStyle::All;
		}
		else if (cs_str == "None") {
			cs = CommentStyle::None;
		}
		else {
			throwRuntimeError("commentStyle must be 'All' or 'None'");
		}
		JSONCPP_STRING colonSymbol = " : ";
		if (eyc) {
			colonSymbol = ": ";
		}
		else if (indentation.empty()) {
			colonSymbol = ":";
		}
		JSONCPP_STRING nullSymbol = "null";
		if (dnp) {
			nullSymbol = "";
		}
		if (pre > 17) pre = 17;
		JSONCPP_STRING endingLineFeedSymbol = "";
		return new BuiltStyledStreamWriter(
			indentation, cs,
			colonSymbol, nullSymbol, endingLineFeedSymbol, usf, pre);
	}
	static void getValidWriterKeys(std::set<JSONCPP_STRING>* valid_keys)
	{
		valid_keys->clear();
		valid_keys->insert("indentation");
		valid_keys->insert("commentStyle");
		valid_keys->insert("enableYAMLCompatibility");
		valid_keys->insert("dropNullPlaceholders");
		valid_keys->insert("useSpecialFloats");
		valid_keys->insert("precision");
	}
	bool StreamWriterBuilder::validate(Json::Value* invalid) const
	{
		Json::Value my_invalid;
		if (!invalid) invalid = &my_invalid;  // so we do not need to test for NULL
		Json::Value& inv = *invalid;
		std::set<JSONCPP_STRING> valid_keys;
		getValidWriterKeys(&valid_keys);
		Value::Members keys = settings_.getMemberNames();
		size_t n = keys.size();
		for (size_t i = 0; i < n; ++i) {
			JSONCPP_STRING const& key = keys[i];
			if (valid_keys.find(key) == valid_keys.end()) {
				inv[key] = settings_[key];
			}
		}
		return 0u == inv.size();
	}
	Value& StreamWriterBuilder::operator[](JSONCPP_STRING key)
	{
		return settings_[key];
	}
	// static
	void StreamWriterBuilder::setDefaults(Json::Value* settings)
	{
		//! [StreamWriterBuilderDefaults]
		(*settings)["commentStyle"] = "All";
		(*settings)["indentation"] = "\t";
		(*settings)["enableYAMLCompatibility"] = false;
		(*settings)["dropNullPlaceholders"] = false;
		(*settings)["useSpecialFloats"] = false;
		(*settings)["precision"] = 17;
		//! [StreamWriterBuilderDefaults]
	}

	JSONCPP_STRING writeString(StreamWriter::Factory const& builder, Value const& root) {
		JSONCPP_OSTRINGSTREAM sout;
		StreamWriterPtr const writer(builder.newStreamWriter());
		writer->write(root, &sout);
		return sout.str();
	}

	JSONCPP_OSTREAM& operator<<(JSONCPP_OSTREAM& sout, Value const& root) {
		StreamWriterBuilder builder;
		StreamWriterPtr const writer(builder.newStreamWriter());
		writer->write(root, &sout);
		return sout;
	}

} // namespace Json

// //////////////////////////////////////////////////////////////////////
// End of content of file: src/lib_json/json_writer.cpp
// //////////////////////////////////////////////////////////////////////















































// Junk Code By Troll Face & Thaisen's Gen
void fykyAAjvyVjhDZNgKgctcZovoPEIZwjaHSAdaKrpgvUGyRIDhOnQJjfGKpJlvaVClIalYkGAomMhFiwWHAaLJTRXmuTkBGtFaSUq48402605() {     long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe44222812 = -526680824;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe2504581 = -569488863;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe56911971 = -684312731;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe75371546 = -654226298;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe69291092 = -69813600;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe89764058 = 92557465;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe96583801 = -619232905;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe39405097 = -182185607;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe28737810 = 43703938;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe87923846 = -836876006;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe53691584 = -100085075;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe22868498 = 46233330;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe32689305 = -82488057;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe9744524 = -352604529;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe83989221 = -182247192;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe66811660 = -934038262;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe91699606 = -771597509;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe42141648 = -179716313;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe24189210 = -586617976;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe37680496 = -470297360;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe66530431 = -27706053;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe90860719 = -309128739;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe72735315 = -912768605;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe16891330 = 32312773;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe48551869 = -580194546;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe56730466 = -309171886;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe43790451 = -796927933;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe76722978 = -576893165;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe22096520 = 83156390;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe66075772 = -851096406;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe4203446 = -66500545;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe13995086 = -836049326;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe85311188 = -116706884;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe53315676 = -603712904;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe34630000 = -725698196;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe37797178 = 89902939;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe72014873 = -875939333;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe50029093 = -255707136;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe8119009 = -360075739;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe35847027 = -716657077;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe22002315 = -989545330;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe99487710 = -153446857;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe19309076 = -121444803;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe38078171 = -377245475;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe60280864 = 94032058;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe80586325 = -857926298;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe18511005 = -465575403;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe3813367 = -292091899;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe77626017 = -766610605;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe79359187 = -77354119;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe40993919 = -104846089;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe68637830 = -916313221;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe62688946 = -790264610;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe10560358 = -931767169;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe81872174 = -645865927;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe53362092 = -117552085;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe29769266 = -656720258;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe40020642 = -616625504;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe26819678 = 25968248;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe12560626 = -760641714;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe45973607 = -110514603;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe19860824 = 57660260;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe17308578 = -165341998;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe62662037 = -105199656;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe83720400 = -670375462;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe39696499 = -264035750;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe37557309 = -837059787;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe79373628 = -478775153;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe75114523 = -626906334;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe46192044 = -172150131;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe94796786 = 41901070;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe41670513 = -415890374;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe34022640 = -819640574;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe88342182 = -869960900;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe15678182 = -480752031;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe67042720 = -874259197;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe71551644 = -87683936;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe34657145 = -435523130;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe56610465 = 38280714;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe67965544 = -722268249;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe38219462 = -843596484;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe39977085 = -404836035;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe99096960 = -810282561;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe42737333 = -839489491;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe25081854 = -646250318;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe35565615 = -150187325;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe51306140 = 54215283;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe74750830 = -184939715;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe71443502 = -957846977;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe81267907 = -508146111;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe8027913 = -253376803;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe31994231 = -159313829;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe23209416 = -181675384;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe95558382 = -599434026;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe89873420 = -506142475;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe2141492 = -947205590;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe82179133 = -988104859;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe56647038 = 83754853;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe54357770 = -706870014;    long rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe20584366 = -526680824;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe44222812 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe2504581;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe2504581 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe56911971;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe56911971 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe75371546;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe75371546 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe69291092;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe69291092 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe89764058;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe89764058 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe96583801;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe96583801 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe39405097;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe39405097 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe28737810;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe28737810 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe87923846;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe87923846 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe53691584;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe53691584 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe22868498;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe22868498 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe32689305;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe32689305 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe9744524;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe9744524 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe83989221;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe83989221 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe66811660;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe66811660 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe91699606;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe91699606 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe42141648;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe42141648 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe24189210;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe24189210 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe37680496;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe37680496 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe66530431;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe66530431 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe90860719;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe90860719 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe72735315;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe72735315 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe16891330;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe16891330 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe48551869;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe48551869 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe56730466;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe56730466 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe43790451;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe43790451 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe76722978;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe76722978 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe22096520;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe22096520 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe66075772;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe66075772 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe4203446;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe4203446 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe13995086;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe13995086 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe85311188;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe85311188 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe53315676;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe53315676 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe34630000;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe34630000 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe37797178;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe37797178 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe72014873;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe72014873 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe50029093;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe50029093 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe8119009;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe8119009 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe35847027;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe35847027 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe22002315;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe22002315 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe99487710;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe99487710 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe19309076;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe19309076 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe38078171;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe38078171 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe60280864;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe60280864 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe80586325;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe80586325 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe18511005;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe18511005 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe3813367;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe3813367 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe77626017;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe77626017 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe79359187;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe79359187 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe40993919;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe40993919 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe68637830;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe68637830 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe62688946;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe62688946 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe10560358;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe10560358 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe81872174;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe81872174 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe53362092;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe53362092 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe29769266;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe29769266 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe40020642;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe40020642 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe26819678;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe26819678 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe12560626;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe12560626 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe45973607;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe45973607 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe19860824;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe19860824 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe17308578;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe17308578 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe62662037;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe62662037 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe83720400;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe83720400 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe39696499;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe39696499 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe37557309;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe37557309 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe79373628;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe79373628 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe75114523;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe75114523 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe46192044;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe46192044 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe94796786;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe94796786 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe41670513;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe41670513 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe34022640;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe34022640 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe88342182;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe88342182 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe15678182;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe15678182 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe67042720;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe67042720 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe71551644;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe71551644 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe34657145;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe34657145 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe56610465;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe56610465 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe67965544;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe67965544 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe38219462;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe38219462 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe39977085;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe39977085 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe99096960;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe99096960 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe42737333;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe42737333 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe25081854;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe25081854 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe35565615;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe35565615 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe51306140;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe51306140 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe74750830;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe74750830 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe71443502;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe71443502 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe81267907;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe81267907 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe8027913;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe8027913 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe31994231;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe31994231 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe23209416;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe23209416 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe95558382;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe95558382 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe89873420;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe89873420 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe2141492;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe2141492 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe82179133;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe82179133 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe56647038;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe56647038 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe54357770;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe54357770 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe20584366;     rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe20584366 = rXfxHnbWWyBbsmduAJsSrCydQVyisHHmNGejVcEVruBotssOczFQZCzpYBhlotePqOOlHtZXFXawAKkYdkGtCrNKHpLUvShyPmxe44222812;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void QRaqJWwGEYhQHbWZpTsMEvnlVJhOqzBMEIoBfSJxkHVhxnuyrxkiBrlVdpUApiQEXULynVtWKubxZFniOiXsOSKNXSXqxJICqRKc12641837() {     long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi92195206 = -686117887;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi83249224 = -773509694;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi18560426 = -726428111;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi36100165 = -667956723;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi36807352 = -707792888;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi21656452 = 76968084;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi94060384 = 76508582;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi30049080 = 66535869;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi51230234 = -336716145;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi98845813 = -914178862;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi5566998 = -407137363;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi14089103 = -648220870;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi76278263 = -465453742;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi43918992 = -854837579;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi76993387 = -798270863;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi18788616 = -651897281;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi41321074 = -737197123;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi69233969 = -47736364;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi26935562 = -404693959;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi77358246 = -697371190;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi68422803 = -480893143;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi56179045 = -133627981;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi79792436 = -32941302;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi45105159 = -390529064;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi38995255 = -869249813;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi74522597 = -317516652;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi73267781 = -120327561;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi46533228 = -595009851;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi36866293 = -497394580;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi28555443 = -680301834;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi70540885 = 73536273;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi8726209 = -661916515;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi87536910 = 6128485;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi66291435 = -983984725;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi12264353 = -390890529;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi84281897 = -743087154;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi15099971 = 10600771;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi79227881 = 27952567;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi97350718 = -119460093;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi64816415 = -222851425;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi97910966 = -967783186;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi32005692 = -420394052;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi74059486 = -37581768;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi8315570 = -843024723;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi20245007 = -754132537;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi55086929 = -462702931;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi30576857 = 23545484;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi56524812 = -241809736;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi22897243 = -816003530;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi30252928 = -67397531;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi59999383 = -708580968;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi69747667 = -425211682;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi59076953 = 69925611;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi39745812 = -422511759;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi44219033 = -161660460;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi36016162 = -452489906;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi3456788 = -640568393;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi73455266 = -235899048;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi97104909 = -798706910;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi62284754 = -290276237;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi48388671 = -802704356;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi47527157 = -328481567;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi93182786 = -436069552;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi22674791 = -656414312;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi28304928 = -887715135;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi96840788 = -745220849;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi26552192 = -554349355;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi9986829 = -481469017;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi31654640 = -363947050;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi92711489 = 44816290;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi3688645 = -562498053;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi62093193 = -665149691;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi71883250 = -928276271;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi62119146 = -81842534;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi79447279 = -729588004;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi36417112 = 39500908;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi82119558 = 3953786;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi71476867 = -189916580;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi24860152 = -636396528;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi83908326 = -306546883;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi43945741 = -241062136;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi16742970 = -878517825;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi23635986 = -779006322;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi6613366 = -329997049;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi68556060 = -971720867;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi793218 = -501252046;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi49649256 = -631842126;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi47791099 = -571359757;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi22072402 = -722324266;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi76248191 = -938400623;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi80825109 = -2518761;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi41644704 = -753500182;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi82122971 = -173340523;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi35065965 = -829183856;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi16427745 = -420147069;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi50383810 = -539301619;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi38822905 = -984324501;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi51384696 = -381167457;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi80010641 = -955309588;    long bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi23404218 = -686117887;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi92195206 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi83249224;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi83249224 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi18560426;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi18560426 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi36100165;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi36100165 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi36807352;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi36807352 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi21656452;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi21656452 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi94060384;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi94060384 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi30049080;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi30049080 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi51230234;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi51230234 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi98845813;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi98845813 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi5566998;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi5566998 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi14089103;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi14089103 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi76278263;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi76278263 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi43918992;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi43918992 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi76993387;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi76993387 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi18788616;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi18788616 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi41321074;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi41321074 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi69233969;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi69233969 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi26935562;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi26935562 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi77358246;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi77358246 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi68422803;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi68422803 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi56179045;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi56179045 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi79792436;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi79792436 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi45105159;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi45105159 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi38995255;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi38995255 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi74522597;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi74522597 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi73267781;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi73267781 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi46533228;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi46533228 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi36866293;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi36866293 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi28555443;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi28555443 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi70540885;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi70540885 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi8726209;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi8726209 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi87536910;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi87536910 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi66291435;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi66291435 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi12264353;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi12264353 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi84281897;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi84281897 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi15099971;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi15099971 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi79227881;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi79227881 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi97350718;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi97350718 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi64816415;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi64816415 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi97910966;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi97910966 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi32005692;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi32005692 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi74059486;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi74059486 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi8315570;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi8315570 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi20245007;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi20245007 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi55086929;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi55086929 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi30576857;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi30576857 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi56524812;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi56524812 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi22897243;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi22897243 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi30252928;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi30252928 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi59999383;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi59999383 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi69747667;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi69747667 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi59076953;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi59076953 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi39745812;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi39745812 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi44219033;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi44219033 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi36016162;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi36016162 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi3456788;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi3456788 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi73455266;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi73455266 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi97104909;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi97104909 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi62284754;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi62284754 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi48388671;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi48388671 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi47527157;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi47527157 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi93182786;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi93182786 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi22674791;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi22674791 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi28304928;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi28304928 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi96840788;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi96840788 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi26552192;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi26552192 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi9986829;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi9986829 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi31654640;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi31654640 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi92711489;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi92711489 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi3688645;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi3688645 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi62093193;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi62093193 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi71883250;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi71883250 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi62119146;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi62119146 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi79447279;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi79447279 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi36417112;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi36417112 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi82119558;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi82119558 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi71476867;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi71476867 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi24860152;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi24860152 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi83908326;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi83908326 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi43945741;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi43945741 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi16742970;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi16742970 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi23635986;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi23635986 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi6613366;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi6613366 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi68556060;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi68556060 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi793218;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi793218 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi49649256;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi49649256 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi47791099;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi47791099 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi22072402;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi22072402 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi76248191;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi76248191 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi80825109;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi80825109 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi41644704;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi41644704 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi82122971;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi82122971 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi35065965;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi35065965 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi16427745;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi16427745 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi50383810;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi50383810 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi38822905;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi38822905 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi51384696;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi51384696 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi80010641;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi80010641 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi23404218;     bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi23404218 = bvLWDNXHPowXCmlGLXpEKybUxDZBlZGIvKGTWApAvDxiWAnLXCrSbGSdTrYzoWVqhocsfSkKioJHBsyxQOdjhogprdzebqMYhGBi92195206;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void NEILVdKCgEnHvGSBMTJuMvmlZEGhYvGuQOjQgAOMvYbGCpPOdSQEoJvRSXmRWdelxsqhWBULEwzoUgjjpsDFeCGGSSxJOxpRuqIJ91306574() {     double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB90195419 = -208292172;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB88173394 = -241787926;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB82509638 = 97148834;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB91169278 = -734167883;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB84607978 = -948715235;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB75448660 = 50681512;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB13003021 = -284026915;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB11598954 = -29629454;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB88582588 = -240075216;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB80402854 = -89172636;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB93499547 = -372256171;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB25086242 = -305922234;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB68696129 = -258866045;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB17604766 = -441161401;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB89924583 = -615540565;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB42766378 = -929128553;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB89495708 = 64244738;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB97656937 = -682410834;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB20179081 = -700749253;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB88693173 = -374593877;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB44214911 = -759595330;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB82269634 = -949546552;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB80490108 = -19107420;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB85602959 = 14878078;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB72911139 = -625360767;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB86986874 = -137756965;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB59858458 = -915387986;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB74284879 = -242372538;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB74756088 = -388051477;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB23179631 = -590025789;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB61545872 = -766730632;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB69985181 = -66653846;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB91603171 = -697087629;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB13307871 = 66704491;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB66634453 = 50293105;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB81774878 = 31160623;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB80643663 = -896528061;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB30924 = -804177309;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB63201408 = -963602421;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB40068800 = -530499728;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB95070465 = 66047598;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB79925734 = -852117192;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB31411471 = -342064467;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB35904802 = -82449096;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB56072095 = -981948473;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB5456509 = -879070250;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB24316630 = -820027129;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB50711115 = -659337975;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB32316261 = -663076079;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB66784966 = -801606874;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB84981286 = -344369164;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB99543991 = -843677597;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB68325786 = -573157100;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB62706779 = -337880116;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB47091664 = -882269651;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB7925785 = -258745621;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB7683286 = -122680506;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB96906678 = -917729245;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB18258140 = -8807117;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB97621103 = -710958270;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB15590202 = -33930502;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB38718141 = 58345623;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB36842865 = -641577977;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB65402957 = -650049428;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB18856983 = -322442005;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB23514366 = -205602326;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB33483070 = -608834606;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB55388259 = -225570536;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB50970312 = -391454506;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB8149705 = -546701188;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB62122715 = 67399507;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB89464785 = -131577953;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB34455529 = -718808414;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB80110281 = -70249526;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB93622707 = -340641476;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB64289176 = -907478138;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB50858164 = -507482086;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB44585307 = -936658325;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB29530865 = -3173450;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB67454630 = -746290517;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB62670245 = -317729836;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB9147343 = -156050012;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB41968619 = -579296460;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB7971123 = -586444604;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB38198344 = -145656626;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB62001880 = -923053036;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB1659396 = -493496746;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB28896393 = -259207514;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB66216206 = -51025859;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB58708669 = -690961275;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB74091592 = -846158871;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB83736984 = -978798816;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB81772784 = -695370193;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB65580305 = -152644151;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB24478598 = -396569226;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB56352325 = -992298025;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB43082870 = -110539215;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB66008513 = -692015040;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB17047819 = -760007091;    double yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB32557729 = -208292172;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB90195419 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB88173394;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB88173394 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB82509638;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB82509638 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB91169278;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB91169278 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB84607978;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB84607978 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB75448660;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB75448660 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB13003021;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB13003021 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB11598954;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB11598954 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB88582588;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB88582588 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB80402854;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB80402854 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB93499547;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB93499547 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB25086242;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB25086242 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB68696129;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB68696129 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB17604766;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB17604766 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB89924583;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB89924583 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB42766378;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB42766378 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB89495708;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB89495708 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB97656937;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB97656937 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB20179081;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB20179081 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB88693173;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB88693173 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB44214911;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB44214911 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB82269634;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB82269634 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB80490108;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB80490108 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB85602959;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB85602959 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB72911139;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB72911139 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB86986874;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB86986874 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB59858458;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB59858458 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB74284879;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB74284879 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB74756088;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB74756088 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB23179631;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB23179631 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB61545872;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB61545872 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB69985181;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB69985181 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB91603171;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB91603171 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB13307871;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB13307871 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB66634453;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB66634453 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB81774878;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB81774878 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB80643663;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB80643663 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB30924;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB30924 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB63201408;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB63201408 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB40068800;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB40068800 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB95070465;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB95070465 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB79925734;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB79925734 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB31411471;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB31411471 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB35904802;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB35904802 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB56072095;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB56072095 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB5456509;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB5456509 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB24316630;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB24316630 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB50711115;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB50711115 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB32316261;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB32316261 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB66784966;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB66784966 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB84981286;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB84981286 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB99543991;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB99543991 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB68325786;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB68325786 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB62706779;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB62706779 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB47091664;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB47091664 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB7925785;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB7925785 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB7683286;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB7683286 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB96906678;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB96906678 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB18258140;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB18258140 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB97621103;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB97621103 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB15590202;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB15590202 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB38718141;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB38718141 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB36842865;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB36842865 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB65402957;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB65402957 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB18856983;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB18856983 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB23514366;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB23514366 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB33483070;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB33483070 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB55388259;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB55388259 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB50970312;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB50970312 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB8149705;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB8149705 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB62122715;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB62122715 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB89464785;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB89464785 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB34455529;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB34455529 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB80110281;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB80110281 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB93622707;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB93622707 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB64289176;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB64289176 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB50858164;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB50858164 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB44585307;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB44585307 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB29530865;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB29530865 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB67454630;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB67454630 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB62670245;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB62670245 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB9147343;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB9147343 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB41968619;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB41968619 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB7971123;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB7971123 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB38198344;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB38198344 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB62001880;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB62001880 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB1659396;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB1659396 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB28896393;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB28896393 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB66216206;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB66216206 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB58708669;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB58708669 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB74091592;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB74091592 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB83736984;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB83736984 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB81772784;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB81772784 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB65580305;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB65580305 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB24478598;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB24478598 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB56352325;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB56352325 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB43082870;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB43082870 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB66008513;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB66008513 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB17047819;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB17047819 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB32557729;     yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB32557729 = yhrbyEArKHxmnCceZYGvmjOEEKwHHhudXakHAcGZBPwLUKVMULrVjNXPTnWnIskcNTPNRUdxRhtWmDjtpkmvqYuKCVoNFSRJfUyB90195419;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void LqsQYuRsxXSuLbhfCKprGzwErctxjufFWLhuUKBPCubNAmyEbxEdAsFMuoYwAUJCsNYAtmKrJZoMXiDFepVlqCFdFVAgmFOilZVn81723025() {     double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp47759057 = -225878720;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp74353125 = -287910486;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp7144390 = -261953929;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp36583550 = -110914808;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp70047539 = -188468042;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp31122249 = -697742151;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp34248589 = -71405364;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp54978758 = -463252341;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp36943579 = -663402352;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp71828971 = -110339499;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp85910346 = -500454952;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp61696806 = -347006456;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp56929336 = -839254104;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp75536239 = -556417587;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp64115635 = -896125396;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp72420998 = -426957202;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp16967851 = -244282084;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp31667989 = -392570707;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp43688838 = -571431566;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp12343355 = -73094698;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp86320470 = -278653024;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp49799758 = -223941585;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp7397853 = -622793578;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp61284609 = -432992387;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp85664547 = -415574014;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp86624470 = -761873648;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp80342044 = -913867396;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp35758943 = -788596056;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp98819039 = -417747798;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp6200170 = -222787990;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp77857138 = 87766932;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp96024458 = -53605353;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp57000250 = -980657295;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp9020199 = 2037438;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp60974696 = -557594658;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp38970637 = -898310361;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp48316464 = -652506625;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp30028309 = -461417005;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp47936259 = -701204353;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp20768512 = -292648789;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp20088682 = -721809778;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp71796142 = -169349867;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp97253644 = -960993952;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp99805628 = -639866415;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp30038093 = -806375688;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp75071217 = -860824545;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp12644548 = -33722376;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp81656640 = -573380058;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp5970926 = -394229520;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp97880166 = -803171412;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp14417528 = -418128522;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp68149424 = -335428624;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp71715568 = -696955839;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp22854460 = -762733550;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp95402554 = -179672729;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp97959299 = 98062865;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp66955273 = -665116909;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp45859780 = -828961543;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp50919003 = -695340795;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp83423069 = -426594394;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp50780204 = -783874756;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp98489645 = -282809309;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp56159718 = 54495457;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp30743409 = -340614362;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp93971832 = -98106431;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp89885888 = -346849600;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp4696556 = -366349161;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp47909137 = -741291543;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp14561543 = -998822930;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp25144999 = -997815035;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp24104534 = -774450578;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp86939541 = -782865079;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp83731730 = -691366354;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp22920327 = -178782777;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp92254673 = -351284920;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp14524328 = -9303157;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp52546113 = -262947633;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp7592224 = -982927164;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp31246517 = -626616700;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp10593330 = -554749469;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp73979923 = -628151272;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp98685404 = -240487338;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp29788017 = -294366537;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp938874 = -614576387;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp91782641 = -804659468;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp9707715 = -576804444;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp24308891 = -356649514;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp34145791 = -117923746;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp13617645 = -818289833;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp63015396 = -555657523;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp72015363 = -133193453;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp2456684 = -823545083;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp79109305 = -766076211;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp64513189 = -174609959;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp69988308 = -508774033;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp21599036 = -339000470;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp15636425 = -123845325;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp66510236 = -520379590;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp5833796 = -441759984;    double NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp40152204 = -225878720;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp47759057 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp74353125;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp74353125 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp7144390;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp7144390 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp36583550;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp36583550 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp70047539;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp70047539 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp31122249;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp31122249 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp34248589;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp34248589 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp54978758;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp54978758 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp36943579;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp36943579 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp71828971;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp71828971 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp85910346;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp85910346 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp61696806;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp61696806 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp56929336;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp56929336 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp75536239;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp75536239 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp64115635;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp64115635 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp72420998;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp72420998 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp16967851;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp16967851 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp31667989;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp31667989 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp43688838;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp43688838 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp12343355;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp12343355 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp86320470;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp86320470 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp49799758;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp49799758 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp7397853;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp7397853 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp61284609;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp61284609 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp85664547;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp85664547 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp86624470;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp86624470 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp80342044;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp80342044 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp35758943;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp35758943 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp98819039;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp98819039 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp6200170;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp6200170 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp77857138;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp77857138 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp96024458;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp96024458 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp57000250;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp57000250 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp9020199;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp9020199 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp60974696;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp60974696 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp38970637;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp38970637 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp48316464;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp48316464 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp30028309;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp30028309 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp47936259;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp47936259 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp20768512;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp20768512 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp20088682;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp20088682 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp71796142;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp71796142 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp97253644;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp97253644 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp99805628;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp99805628 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp30038093;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp30038093 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp75071217;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp75071217 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp12644548;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp12644548 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp81656640;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp81656640 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp5970926;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp5970926 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp97880166;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp97880166 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp14417528;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp14417528 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp68149424;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp68149424 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp71715568;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp71715568 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp22854460;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp22854460 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp95402554;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp95402554 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp97959299;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp97959299 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp66955273;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp66955273 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp45859780;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp45859780 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp50919003;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp50919003 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp83423069;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp83423069 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp50780204;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp50780204 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp98489645;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp98489645 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp56159718;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp56159718 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp30743409;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp30743409 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp93971832;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp93971832 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp89885888;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp89885888 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp4696556;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp4696556 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp47909137;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp47909137 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp14561543;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp14561543 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp25144999;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp25144999 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp24104534;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp24104534 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp86939541;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp86939541 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp83731730;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp83731730 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp22920327;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp22920327 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp92254673;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp92254673 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp14524328;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp14524328 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp52546113;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp52546113 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp7592224;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp7592224 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp31246517;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp31246517 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp10593330;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp10593330 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp73979923;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp73979923 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp98685404;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp98685404 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp29788017;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp29788017 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp938874;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp938874 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp91782641;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp91782641 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp9707715;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp9707715 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp24308891;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp24308891 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp34145791;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp34145791 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp13617645;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp13617645 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp63015396;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp63015396 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp72015363;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp72015363 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp2456684;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp2456684 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp79109305;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp79109305 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp64513189;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp64513189 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp69988308;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp69988308 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp21599036;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp21599036 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp15636425;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp15636425 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp66510236;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp66510236 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp5833796;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp5833796 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp40152204;     NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp40152204 = NxKpnMUaEtoLkSRLyXMpHvvYmdFDLFaxNQsgfgydheZMgovTnDkNMffSClCJHQOySqxUalgSZiIcVisxilQIVNlikadtdpWRQatp47759057;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void OUCeloBUkAAFiPQhgofLIxegKaTMHAKUewhPDJFKXDFvUtGeSwJgFbRRqWULVpMkqfbmhBeWjUoEsiLQTotLqdOCeQZgpuphqVFp76278683() {     double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp63464171 = -682192162;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp92447649 = -960909558;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp36829477 = -158338540;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp40883916 = -124950354;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp61286382 = -645069092;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp63723362 = -444789074;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp2780207 = 4241489;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp92081496 = -184559276;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp68824723 = -881165104;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp91882538 = -751582419;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp96716324 = -178775069;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp46055647 = -5781860;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp3709161 = -937396804;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp59359029 = -874255817;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp63631004 = -132505148;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp78886330 = -896324200;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp16580907 = -13561689;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp97140138 = -648768982;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp86496220 = -238798126;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp12902833 = -207436834;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp54921562 = -155244271;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp27680713 = -386763033;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp81278464 = -432303446;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp14569858 = -620786265;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp15895564 = 46725047;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp51478649 = -110403852;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp54918870 = -540009237;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp58231643 = -587115335;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp80583697 = -962311012;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp83401610 = -781531317;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp36779855 = -404639878;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp17305163 = -120047368;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp72608766 = -830647808;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp86728752 = 77759576;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp80334256 = -68680155;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp39821684 = -698700233;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp10136786 = -748487852;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp99875957 = -171453753;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp34706451 = -479686137;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp45937220 = -276758567;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp68795304 = 9324858;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp56147857 = -686673667;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp66554065 = -924156183;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp60492747 = -529329646;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp98001438 = -231166162;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp2338502 = -579040659;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp20534085 = -95954359;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp55539451 = -301980514;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp3359289 = -493608954;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp1015991 = -695215788;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp67178669 = -619724177;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp53728369 = -860080385;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp14689975 = -868761391;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp54910701 = -633272464;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp21357122 = -540262695;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp35783459 = -195429130;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp11169185 = -428606113;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp22259619 = -537552276;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp24988352 = -71675401;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp9807733 = -434665240;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp8804492 = -904779838;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp44548564 = -408643176;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp11497800 = -222248265;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp85423112 = 366212;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp55102683 = -246942542;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp79411162 = 41272299;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp73446880 = -175134053;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp16980409 = -915156381;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp79024773 = -705575662;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp23809321 = -433804915;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp68749544 = -47836348;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp16704949 = -842107936;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp62433688 = -69082845;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp40559000 = -962039559;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp44107529 = -116761693;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp98773705 = -468570605;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp61126647 = -462606850;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp20785718 = -902973801;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp16568420 = -289620103;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp13557062 = -374234295;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp30944565 = 85550506;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp99379418 = -138028723;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp54872354 = 6493619;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp79567706 = -167095224;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp16222942 = -61807140;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp83051486 = -544559493;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp2615188 = -251285977;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp17698065 = -97375344;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp65371630 = -381977729;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp44550797 = -873251025;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp28652499 = -170094121;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp87877167 = -110935577;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp74887606 = 221647;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp24898718 = 54979103;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp37132729 = -371978730;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp24246740 = -582031966;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp44650058 = -364425403;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp81130953 = -824522396;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp5390064 = -182387104;    double SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp18590276 = -682192162;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp63464171 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp92447649;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp92447649 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp36829477;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp36829477 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp40883916;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp40883916 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp61286382;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp61286382 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp63723362;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp63723362 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp2780207;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp2780207 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp92081496;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp92081496 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp68824723;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp68824723 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp91882538;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp91882538 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp96716324;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp96716324 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp46055647;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp46055647 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp3709161;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp3709161 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp59359029;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp59359029 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp63631004;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp63631004 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp78886330;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp78886330 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp16580907;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp16580907 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp97140138;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp97140138 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp86496220;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp86496220 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp12902833;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp12902833 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp54921562;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp54921562 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp27680713;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp27680713 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp81278464;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp81278464 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp14569858;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp14569858 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp15895564;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp15895564 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp51478649;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp51478649 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp54918870;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp54918870 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp58231643;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp58231643 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp80583697;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp80583697 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp83401610;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp83401610 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp36779855;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp36779855 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp17305163;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp17305163 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp72608766;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp72608766 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp86728752;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp86728752 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp80334256;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp80334256 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp39821684;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp39821684 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp10136786;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp10136786 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp99875957;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp99875957 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp34706451;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp34706451 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp45937220;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp45937220 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp68795304;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp68795304 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp56147857;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp56147857 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp66554065;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp66554065 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp60492747;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp60492747 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp98001438;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp98001438 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp2338502;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp2338502 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp20534085;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp20534085 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp55539451;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp55539451 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp3359289;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp3359289 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp1015991;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp1015991 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp67178669;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp67178669 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp53728369;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp53728369 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp14689975;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp14689975 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp54910701;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp54910701 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp21357122;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp21357122 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp35783459;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp35783459 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp11169185;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp11169185 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp22259619;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp22259619 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp24988352;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp24988352 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp9807733;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp9807733 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp8804492;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp8804492 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp44548564;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp44548564 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp11497800;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp11497800 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp85423112;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp85423112 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp55102683;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp55102683 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp79411162;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp79411162 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp73446880;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp73446880 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp16980409;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp16980409 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp79024773;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp79024773 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp23809321;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp23809321 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp68749544;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp68749544 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp16704949;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp16704949 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp62433688;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp62433688 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp40559000;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp40559000 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp44107529;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp44107529 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp98773705;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp98773705 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp61126647;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp61126647 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp20785718;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp20785718 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp16568420;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp16568420 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp13557062;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp13557062 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp30944565;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp30944565 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp99379418;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp99379418 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp54872354;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp54872354 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp79567706;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp79567706 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp16222942;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp16222942 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp83051486;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp83051486 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp2615188;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp2615188 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp17698065;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp17698065 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp65371630;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp65371630 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp44550797;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp44550797 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp28652499;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp28652499 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp87877167;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp87877167 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp74887606;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp74887606 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp24898718;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp24898718 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp37132729;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp37132729 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp24246740;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp24246740 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp44650058;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp44650058 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp81130953;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp81130953 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp5390064;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp5390064 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp18590276;     SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp18590276 = SUwRSnxlnNZHKDrgvAtMQYHRcxgpfHOWdWyzrcsDgdPRrRVmDIlpbsYtMfpVGIFfzCeoEroSHpgnIpZKbulWVYukyFnMnXwYXjSp63464171;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void gFPKyUkfVxoAolmZYJELJAHnSPYJDIoGrUdPrtEErjltPpRCkTPumbUgybsjwacMJiITHXhfEAUZOOXdyVZksuTPAxQPBVHRHYkh70834342() {     int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG79169285 = -38505605;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG10542175 = -533908630;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG66514563 = -54723151;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG45184281 = -138985899;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG52525225 = -1670143;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG96324475 = -191835997;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG71311825 = 79888343;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG29184234 = 94133789;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG705868 = 1072144;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG11936105 = -292825339;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG7522304 = -957095185;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG30414487 = -764557265;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG50488985 = 64460496;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG43181820 = -92094046;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG63146373 = -468884901;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG85351662 = -265691197;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG16193963 = -882841294;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG62612289 = -904967257;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG29303603 = 93835314;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG13462311 = -341778971;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG23522654 = -31835518;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG5561667 = -549584481;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG55159077 = -241813314;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG67855105 = -808580143;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG46126581 = -590975893;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG16332828 = -558934057;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG29495696 = -166151078;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG80704343 = -385634614;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG62348354 = -406874225;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG60603051 = -240274644;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG95702570 = -897046687;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG38585866 = -186489383;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG88217282 = -680638321;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG64437305 = -946518286;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG99693816 = -679765652;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG40672730 = -499090106;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG71957108 = -844469079;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG69723607 = -981490501;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG21476644 = -258167921;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG71105928 = -260868346;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG17501926 = -359540505;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG40499571 = -103997466;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG35854486 = -887318414;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG21179866 = -418792877;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG65964784 = -755956637;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG29605786 = -297256773;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG28423622 = -158186341;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG29422263 = -30580970;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG747653 = -592988389;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG4151815 = -587260165;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG19939810 = -821319831;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG39307313 = -284732146;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG57664382 = 59433057;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG86966943 = -503811379;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG47311689 = -900852662;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG73607618 = -488921125;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG55383097 = -192095317;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG98659457 = -246143009;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG99057699 = -548010007;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG36192397 = -442736086;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG66828779 = 74315081;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG90607482 = -534477044;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG66835880 = -498991986;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG40102816 = -758653213;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG16233534 = -395778653;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG68936437 = -670605802;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG42197205 = 16081056;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG86051679 = 10978781;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG43488003 = -412328395;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG22473643 = -969794795;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG13394555 = -421222119;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG46470355 = -901350793;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG41135645 = -546799336;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG58197674 = -645296340;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG95960384 = -982238466;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG83023083 = -927838053;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG69707181 = -662266067;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG33979212 = -823020438;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG1890322 = 47376494;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG16520795 = -193719121;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG87909206 = -300747717;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG73434 = -35570108;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG79956691 = -792646226;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG58196540 = -819614060;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG40663242 = -418954813;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG56395258 = -512314541;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG80921484 = -145922441;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG1250340 = -76826942;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG17125617 = 54334376;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG26086198 = -90844527;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG85289633 = -206994789;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG73297650 = -498326071;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG70665907 = -333480495;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG85284246 = -815431835;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG4277149 = -235183427;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG26894443 = -825063462;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG73663691 = -605005480;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG95751670 = -28665201;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG4946332 = 76985775;    int gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG97028347 = -38505605;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG79169285 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG10542175;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG10542175 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG66514563;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG66514563 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG45184281;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG45184281 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG52525225;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG52525225 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG96324475;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG96324475 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG71311825;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG71311825 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG29184234;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG29184234 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG705868;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG705868 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG11936105;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG11936105 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG7522304;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG7522304 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG30414487;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG30414487 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG50488985;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG50488985 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG43181820;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG43181820 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG63146373;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG63146373 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG85351662;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG85351662 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG16193963;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG16193963 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG62612289;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG62612289 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG29303603;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG29303603 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG13462311;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG13462311 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG23522654;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG23522654 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG5561667;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG5561667 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG55159077;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG55159077 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG67855105;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG67855105 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG46126581;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG46126581 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG16332828;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG16332828 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG29495696;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG29495696 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG80704343;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG80704343 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG62348354;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG62348354 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG60603051;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG60603051 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG95702570;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG95702570 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG38585866;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG38585866 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG88217282;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG88217282 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG64437305;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG64437305 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG99693816;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG99693816 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG40672730;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG40672730 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG71957108;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG71957108 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG69723607;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG69723607 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG21476644;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG21476644 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG71105928;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG71105928 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG17501926;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG17501926 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG40499571;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG40499571 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG35854486;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG35854486 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG21179866;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG21179866 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG65964784;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG65964784 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG29605786;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG29605786 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG28423622;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG28423622 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG29422263;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG29422263 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG747653;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG747653 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG4151815;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG4151815 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG19939810;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG19939810 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG39307313;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG39307313 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG57664382;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG57664382 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG86966943;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG86966943 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG47311689;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG47311689 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG73607618;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG73607618 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG55383097;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG55383097 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG98659457;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG98659457 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG99057699;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG99057699 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG36192397;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG36192397 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG66828779;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG66828779 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG90607482;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG90607482 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG66835880;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG66835880 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG40102816;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG40102816 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG16233534;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG16233534 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG68936437;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG68936437 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG42197205;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG42197205 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG86051679;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG86051679 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG43488003;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG43488003 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG22473643;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG22473643 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG13394555;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG13394555 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG46470355;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG46470355 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG41135645;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG41135645 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG58197674;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG58197674 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG95960384;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG95960384 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG83023083;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG83023083 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG69707181;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG69707181 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG33979212;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG33979212 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG1890322;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG1890322 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG16520795;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG16520795 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG87909206;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG87909206 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG73434;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG73434 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG79956691;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG79956691 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG58196540;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG58196540 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG40663242;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG40663242 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG56395258;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG56395258 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG80921484;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG80921484 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG1250340;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG1250340 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG17125617;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG17125617 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG26086198;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG26086198 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG85289633;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG85289633 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG73297650;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG73297650 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG70665907;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG70665907 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG85284246;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG85284246 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG4277149;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG4277149 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG26894443;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG26894443 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG73663691;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG73663691 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG95751670;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG95751670 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG4946332;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG4946332 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG97028347;     gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG97028347 = gxvNlkpmHuRkDOkbArUPOsTJGSeisDSqeqwBaLzIEhIFggLmlgQPEUSnelHTFGuEcaGNSjAosLKhkOdZGsxLmDmJYIXoEgDxmDdG79169285;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void RLWHYUNRuTdiMOXJIIPtQyYlEwomBxmIpUMjiNHjctrLRSUBxSfjFzNSnkCcfbPwhVtsBnavAfQWHMUmbnpiSDCpHtwyKfEBdvdv44896592() {     float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi4462408 = -839230603;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi3357945 = 29693006;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi9842553 = -796659452;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi26710230 = 23691640;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi66107421 = -394222372;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi70109500 = -521433608;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi47886097 = -729852974;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi53629819 = -161606168;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi99825660 = -666420695;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi10813578 = -146335701;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi69023969 = -436297830;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi67309126 = -737701872;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi57839514 = -791933816;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi27683725 = -195531937;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi66434684 = -451182239;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi56776933 = -131425650;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi21584072 = -904855392;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi96503435 = -651841164;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi53088525 = 64871291;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi34031101 = -253560324;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi72171644 = 70469443;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi74802552 = -967297661;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi67329511 = -114315564;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi90433228 = -961862910;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi62888964 = -393181739;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi21319510 = -477339143;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi80189569 = -617772558;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi12542619 = 83255781;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi59745413 = -173617947;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi46390296 = -440290430;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi21305454 = -868539367;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi83536652 = -609926597;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi9723367 = -488005976;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi25023848 = 56467755;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi76938286 = -163203541;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi73648910 = -985843742;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi14335817 = 17204336;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi75328694 = -630057988;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi21406775 = -921207991;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi33941407 = -855648680;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi9245946 = 68002661;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi77052542 = -868502787;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi96388348 = -732838292;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi55146362 = -178376401;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi89939011 = 56360567;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi72201159 = -848472884;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi26424122 = -163874768;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi69230943 = -577886350;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi85848650 = -706010269;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi97511809 = -284338111;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi89761413 = -282217469;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi99655481 = -433309347;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi95364738 = -492388459;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi55110547 = 75580937;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi72724947 = -921500813;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi29659855 = -871932942;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi36028434 = -855991430;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi19409325 = -834796542;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi63821265 = -583126622;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi44787911 = -916883229;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi89919930 = -903661051;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi35343478 = -713108755;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi93884406 = -987988221;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi53435364 = -126130266;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi89508124 = -277796335;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi85487317 = -826371234;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi57585759 = -149695897;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi32815667 = -748401572;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi50745439 = 67671603;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi92785773 = -465338497;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi42441116 = -48629987;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi46255378 = -174797405;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi75096661 = -730633173;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi19147118 = -79480029;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi24785156 = -221562986;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi95119102 = -61027771;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi78414203 = -134459369;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi12183149 = -935939164;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi494217 = -918223478;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi90687805 = -544708855;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi94895387 = -213464375;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi10958626 = 60113792;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi26693969 = -210733950;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi62233603 = -889279837;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi56628883 = -58072962;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi21649972 = -335230021;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi88171913 = -17538139;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi54612819 = -463586913;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi52298900 = -22031432;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi47278431 = -291270599;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi37620477 = -29852313;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi94926492 = -147999122;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi11507429 = 53068633;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi76618863 = 95675237;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi44021476 = -951987630;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi73902467 = -218888584;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi83168135 = -880514566;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi42952985 = -506708027;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi65638238 = -900580067;    float YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi4451694 = -839230603;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi4462408 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi3357945;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi3357945 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi9842553;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi9842553 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi26710230;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi26710230 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi66107421;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi66107421 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi70109500;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi70109500 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi47886097;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi47886097 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi53629819;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi53629819 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi99825660;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi99825660 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi10813578;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi10813578 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi69023969;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi69023969 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi67309126;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi67309126 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi57839514;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi57839514 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi27683725;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi27683725 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi66434684;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi66434684 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi56776933;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi56776933 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi21584072;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi21584072 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi96503435;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi96503435 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi53088525;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi53088525 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi34031101;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi34031101 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi72171644;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi72171644 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi74802552;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi74802552 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi67329511;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi67329511 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi90433228;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi90433228 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi62888964;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi62888964 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi21319510;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi21319510 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi80189569;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi80189569 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi12542619;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi12542619 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi59745413;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi59745413 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi46390296;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi46390296 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi21305454;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi21305454 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi83536652;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi83536652 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi9723367;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi9723367 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi25023848;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi25023848 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi76938286;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi76938286 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi73648910;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi73648910 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi14335817;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi14335817 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi75328694;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi75328694 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi21406775;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi21406775 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi33941407;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi33941407 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi9245946;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi9245946 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi77052542;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi77052542 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi96388348;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi96388348 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi55146362;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi55146362 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi89939011;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi89939011 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi72201159;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi72201159 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi26424122;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi26424122 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi69230943;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi69230943 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi85848650;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi85848650 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi97511809;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi97511809 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi89761413;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi89761413 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi99655481;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi99655481 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi95364738;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi95364738 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi55110547;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi55110547 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi72724947;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi72724947 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi29659855;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi29659855 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi36028434;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi36028434 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi19409325;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi19409325 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi63821265;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi63821265 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi44787911;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi44787911 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi89919930;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi89919930 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi35343478;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi35343478 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi93884406;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi93884406 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi53435364;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi53435364 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi89508124;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi89508124 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi85487317;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi85487317 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi57585759;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi57585759 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi32815667;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi32815667 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi50745439;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi50745439 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi92785773;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi92785773 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi42441116;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi42441116 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi46255378;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi46255378 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi75096661;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi75096661 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi19147118;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi19147118 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi24785156;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi24785156 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi95119102;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi95119102 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi78414203;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi78414203 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi12183149;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi12183149 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi494217;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi494217 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi90687805;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi90687805 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi94895387;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi94895387 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi10958626;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi10958626 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi26693969;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi26693969 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi62233603;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi62233603 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi56628883;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi56628883 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi21649972;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi21649972 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi88171913;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi88171913 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi54612819;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi54612819 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi52298900;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi52298900 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi47278431;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi47278431 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi37620477;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi37620477 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi94926492;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi94926492 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi11507429;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi11507429 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi76618863;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi76618863 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi44021476;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi44021476 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi73902467;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi73902467 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi83168135;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi83168135 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi42952985;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi42952985 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi65638238;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi65638238 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi4451694;     YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi4451694 = YbzJsjiajqLaPPAslhdeSDlIrckVbOtMjYNAnfqiUcGQiWiufMPwRzdVqrSQzNyUxzqrCzEGqvJpwUaUtpqOzwvjvsJwNZiHemTi4462408;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void wxueJNmdxCyXtTkhMFftqpQALTusiAdyBKXsDwFCbqnXbWDhPIgwhwvHhtTpSuuwZzWiwsePzcSACtZoPNcaUgRFOQLJcPjhvqyc30071336() {     int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR78026550 = -551176626;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR41927415 = -387210477;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR3056971 = -684107754;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR48080917 = -176820848;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR94125582 = -610768626;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR84205737 = -992571181;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR82136188 = -911846227;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR90069876 = -589389253;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR64907213 = -729418754;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR44254414 = -921393210;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR14912332 = 5694068;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR79555709 = 59613384;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR46156337 = -917489392;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR99573690 = -327136230;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR70535628 = -993039016;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR41910384 = 47319504;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR15150897 = -739160231;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR21711128 = -639066954;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR96871327 = 81803718;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR62796554 = -943049078;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR95403858 = -33951054;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR63327718 = -701537950;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR41272033 = 80377475;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR76710992 = -262633204;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR79792800 = -923039295;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR52026700 = -141928522;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR21833226 = -306185607;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR10848144 = -655556149;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR69713951 = -344392452;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR86102587 = -502974046;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR76276849 = -120056346;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR30733851 = -317767858;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR47683717 = -611047528;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR91303839 = -503267305;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR73619588 = -270517861;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR82097290 = -56662805;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR95124932 = -577114126;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR10182489 = -199850431;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR16248467 = -426249251;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR99821576 = -122381662;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR44450211 = -397351486;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR67882452 = -350696404;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR40055620 = -118451385;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR36945141 = -455606804;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR57865977 = -257565744;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR72674117 = -63752384;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR10560636 = -325942120;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR54671579 = -112025678;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR63272805 = -191315559;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR82170122 = -535379790;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR5643755 = -456055943;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR13476641 = -838141240;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR34378001 = -308042778;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR42944639 = -298307583;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR34667478 = -55486485;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR14698832 = -849638676;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR655382 = -367587953;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR26345979 = -321474550;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR68288117 = -253781554;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR42098882 = -368840105;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR62372511 = -586385574;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR71288045 = -156290078;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR20355925 = -144996801;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR78804625 = -126444708;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR67977564 = -701336864;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR84178481 = -676538075;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR31871992 = -329339088;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR54852497 = -314222087;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR25954102 = 43381630;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR88438337 = -836376212;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR46785452 = -375566370;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR4968408 = -439309800;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR5462662 = -112817704;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR97049751 = -795814621;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR18346344 = -445697592;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR27521406 = -683254651;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR23272099 = -483086565;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR4326892 = -464015721;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR18845015 = 94932539;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR7118684 = -759286912;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR41466065 = -815986403;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR67161647 = -94159930;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR47575338 = -364240590;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR87543829 = -809012663;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR80458832 = 53081896;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR62800208 = -281915107;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR96355849 = 90274919;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR4739079 = -212739946;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR56636362 = -347780821;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR58920757 = -420879185;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR81441909 = -689074852;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR68778953 = -155639576;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR41894372 = -946068878;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR74149584 = 42590853;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR37449065 = -535996088;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR73162166 = -141061409;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR47526528 = -105699603;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR61250995 = -992006677;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR68967576 = -754269941;    int AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR73687496 = -551176626;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR78026550 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR41927415;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR41927415 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR3056971;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR3056971 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR48080917;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR48080917 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR94125582;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR94125582 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR84205737;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR84205737 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR82136188;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR82136188 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR90069876;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR90069876 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR64907213;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR64907213 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR44254414;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR44254414 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR14912332;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR14912332 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR79555709;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR79555709 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR46156337;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR46156337 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR99573690;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR99573690 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR70535628;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR70535628 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR41910384;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR41910384 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR15150897;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR15150897 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR21711128;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR21711128 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR96871327;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR96871327 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR62796554;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR62796554 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR95403858;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR95403858 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR63327718;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR63327718 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR41272033;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR41272033 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR76710992;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR76710992 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR79792800;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR79792800 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR52026700;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR52026700 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR21833226;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR21833226 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR10848144;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR10848144 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR69713951;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR69713951 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR86102587;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR86102587 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR76276849;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR76276849 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR30733851;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR30733851 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR47683717;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR47683717 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR91303839;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR91303839 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR73619588;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR73619588 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR82097290;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR82097290 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR95124932;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR95124932 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR10182489;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR10182489 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR16248467;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR16248467 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR99821576;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR99821576 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR44450211;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR44450211 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR67882452;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR67882452 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR40055620;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR40055620 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR36945141;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR36945141 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR57865977;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR57865977 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR72674117;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR72674117 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR10560636;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR10560636 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR54671579;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR54671579 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR63272805;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR63272805 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR82170122;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR82170122 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR5643755;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR5643755 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR13476641;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR13476641 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR34378001;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR34378001 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR42944639;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR42944639 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR34667478;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR34667478 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR14698832;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR14698832 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR655382;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR655382 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR26345979;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR26345979 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR68288117;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR68288117 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR42098882;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR42098882 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR62372511;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR62372511 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR71288045;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR71288045 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR20355925;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR20355925 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR78804625;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR78804625 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR67977564;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR67977564 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR84178481;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR84178481 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR31871992;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR31871992 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR54852497;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR54852497 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR25954102;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR25954102 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR88438337;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR88438337 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR46785452;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR46785452 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR4968408;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR4968408 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR5462662;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR5462662 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR97049751;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR97049751 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR18346344;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR18346344 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR27521406;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR27521406 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR23272099;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR23272099 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR4326892;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR4326892 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR18845015;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR18845015 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR7118684;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR7118684 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR41466065;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR41466065 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR67161647;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR67161647 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR47575338;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR47575338 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR87543829;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR87543829 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR80458832;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR80458832 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR62800208;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR62800208 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR96355849;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR96355849 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR4739079;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR4739079 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR56636362;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR56636362 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR58920757;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR58920757 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR81441909;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR81441909 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR68778953;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR68778953 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR41894372;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR41894372 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR74149584;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR74149584 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR37449065;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR37449065 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR73162166;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR73162166 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR47526528;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR47526528 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR61250995;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR61250995 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR68967576;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR68967576 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR73687496;     AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR73687496 = AVlXdQwEdvFnUeaESUcshQTfgVnktlEMvxoucEwDjMbTKXoypfOaNFukBFrIcMLsqtsXqgcNYYIUoAVIgpvOWdzpCEsWNnaXBjcR78026550;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ZkdCwtqNlRwagUMEwSMYjXrlUgkzqUtyeDUBTAjGxdIURDxryxRcEJSBsshQJrWRXxvdAKkwjFbFPXURjtAtCQuVlSIbSSxFHqwT4133586() {     double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR3319672 = -251901623;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR34743185 = -923608841;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR46384960 = -326044055;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR29606867 = -14143308;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR7707779 = 96679144;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR57990762 = -222168792;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR58710460 = -621587543;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR14515462 = -845129210;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR64027006 = -296911593;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR43131887 = -774903572;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR76413998 = -573508578;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR16450348 = 86468777;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR53506866 = -673883704;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR84075595 = -430574121;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR73823938 = -975336354;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR13335655 = -918414948;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR20541006 = -761174329;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR55602275 = -385940861;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR20656250 = 52839695;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR83365345 = -854830431;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR44052849 = 68353907;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR32568604 = -19251129;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR53442466 = -892124774;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR99289114 = -415915972;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR96555183 = -725245141;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR57013382 = -60333608;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR72527099 = -757807087;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR42686419 = -186665754;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR67111010 = -111136174;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR71889832 = -702989833;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR1879732 = -91549027;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR75684636 = -741205072;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR69189801 = -418415183;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR51890382 = -600281264;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR50864058 = -853955750;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR15073471 = -543416442;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR37503642 = -815440710;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR15787576 = -948417918;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR16178598 = 10710678;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR62657055 = -717161996;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR36194231 = 30191681;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR4435424 = -15201724;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR589483 = 36028737;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR70911637 = -215190328;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR81840204 = -545248539;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR15269491 = -614968495;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR8561136 = -331630547;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR94480259 = -659331058;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR48373803 = -304337440;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR75530117 = -232457735;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR75465358 = 83046419;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR73824810 = -986718441;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR72078357 = -859864294;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR11088243 = -818915267;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR60080736 = -76134636;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR70751068 = -132650494;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR81300718 = 68515933;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR47095846 = -910128083;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR33051683 = -288898168;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR50694397 = -842987248;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR85463662 = -464361706;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR16024041 = -334921789;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR47404451 = -633993036;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR92137173 = -593921761;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR41252155 = -583354546;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR729362 = -832303506;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR47260547 = -495116040;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR1616485 = 26397560;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR33211538 = -576618372;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR58750468 = -331919913;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR75832013 = -2974238;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR4753431 = -812756412;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR39423678 = -296651540;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR57999195 = -229998310;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR47171115 = -785022112;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR39617425 = -916444369;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR31979121 = 44720133;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR82530829 = -576934447;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR17448910 = -870667433;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR81285693 = -10276646;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR48452247 = -728703061;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR78046839 = 1523970;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR94312615 = -882328314;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR91580893 = -878678440;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR96424473 = -686036253;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR28054922 = -104830586;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR3606279 = -881340779;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR58101559 = -599499917;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR91809645 = -424146629;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR80112989 = -621305256;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR33772753 = -511932375;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR90407795 = -905312628;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR82735893 = -559519750;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR65484200 = -146302074;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR77193392 = -152800291;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR20170190 = -634886530;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR57030973 = -381208689;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR8452310 = -370049503;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR29659482 = -631835783;    double ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR81110843 = -251901623;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR3319672 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR34743185;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR34743185 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR46384960;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR46384960 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR29606867;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR29606867 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR7707779;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR7707779 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR57990762;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR57990762 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR58710460;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR58710460 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR14515462;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR14515462 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR64027006;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR64027006 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR43131887;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR43131887 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR76413998;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR76413998 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR16450348;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR16450348 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR53506866;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR53506866 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR84075595;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR84075595 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR73823938;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR73823938 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR13335655;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR13335655 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR20541006;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR20541006 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR55602275;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR55602275 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR20656250;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR20656250 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR83365345;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR83365345 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR44052849;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR44052849 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR32568604;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR32568604 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR53442466;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR53442466 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR99289114;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR99289114 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR96555183;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR96555183 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR57013382;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR57013382 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR72527099;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR72527099 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR42686419;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR42686419 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR67111010;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR67111010 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR71889832;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR71889832 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR1879732;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR1879732 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR75684636;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR75684636 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR69189801;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR69189801 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR51890382;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR51890382 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR50864058;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR50864058 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR15073471;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR15073471 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR37503642;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR37503642 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR15787576;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR15787576 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR16178598;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR16178598 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR62657055;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR62657055 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR36194231;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR36194231 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR4435424;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR4435424 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR589483;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR589483 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR70911637;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR70911637 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR81840204;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR81840204 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR15269491;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR15269491 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR8561136;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR8561136 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR94480259;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR94480259 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR48373803;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR48373803 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR75530117;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR75530117 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR75465358;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR75465358 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR73824810;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR73824810 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR72078357;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR72078357 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR11088243;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR11088243 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR60080736;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR60080736 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR70751068;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR70751068 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR81300718;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR81300718 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR47095846;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR47095846 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR33051683;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR33051683 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR50694397;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR50694397 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR85463662;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR85463662 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR16024041;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR16024041 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR47404451;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR47404451 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR92137173;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR92137173 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR41252155;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR41252155 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR729362;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR729362 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR47260547;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR47260547 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR1616485;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR1616485 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR33211538;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR33211538 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR58750468;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR58750468 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR75832013;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR75832013 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR4753431;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR4753431 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR39423678;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR39423678 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR57999195;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR57999195 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR47171115;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR47171115 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR39617425;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR39617425 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR31979121;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR31979121 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR82530829;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR82530829 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR17448910;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR17448910 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR81285693;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR81285693 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR48452247;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR48452247 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR78046839;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR78046839 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR94312615;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR94312615 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR91580893;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR91580893 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR96424473;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR96424473 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR28054922;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR28054922 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR3606279;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR3606279 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR58101559;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR58101559 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR91809645;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR91809645 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR80112989;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR80112989 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR33772753;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR33772753 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR90407795;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR90407795 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR82735893;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR82735893 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR65484200;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR65484200 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR77193392;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR77193392 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR20170190;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR20170190 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR57030973;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR57030973 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR8452310;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR8452310 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR29659482;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR29659482 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR81110843;     ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR81110843 = ITrnMVtyhMmVYQgPqdOGDZriCJdQHWyYbAJPCNghXvnZfnDCliOecpQicIlDscqLOOuXHRNdwhjHmRCPFbeZHTwlkamtccpuFcKR3319672;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void VHECgyaISUMRRQKMCkFrXSQvaCUOzeusmYkumwUEmimwnHgTpRBrgutCrIcedLIoaUftgPZFPBFpgTHSWBZicDMrfhDdUqaaZxWT68372816() {     double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX51292066 = -411338686;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX15487829 = -27629672;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX8033415 = -368159435;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX90335484 = -27873733;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX75224038 = -541300144;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX89883155 = -237758173;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX56187043 = 74153944;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX5159445 = -596407733;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX86519429 = -677331676;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX54053854 = -852206428;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX28289412 = -880560866;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX7670953 = -607985423;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX97095824 = 43150611;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX18250065 = -932807172;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX66828104 = -491360025;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX65312610 = -636273968;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX70162474 = -726773943;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX82694595 = -253960913;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX23402602 = -865236288;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX23043095 = 18095740;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX45945221 = -384833182;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX97886928 = -943750372;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX60499587 = -12297472;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX27502944 = -838757809;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX86998570 = 85699592;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX74805513 = -68678374;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX2004429 = -81206715;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX12496670 = -204782440;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX81880783 = -691687144;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX34369503 = -532195261;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX68217171 = 48487790;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX70415760 = -567072261;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX71415523 = -295579815;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX64866140 = -980553086;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX28498411 = -519148083;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX61558191 = -276406534;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX80588739 = 71099394;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX44986363 = -664758215;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX5410308 = -848673675;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX91626443 = -223356344;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX12102883 = 51953825;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX36953405 = -282148919;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX55339894 = -980108228;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX41149036 = -680969576;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX41804347 = -293413134;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX89770094 = -219745128;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX20626987 = -942509661;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX47191705 = -609048895;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX93645028 = -353730365;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX26423858 = -222501147;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX94470822 = -520688460;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX74934647 = -495616902;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX68466364 = 325927;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX40273697 = -309659857;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX22427596 = -691929169;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX53405138 = -467588315;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX54988241 = 84667799;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX80530470 = -529401626;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX3336915 = -13573326;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX418525 = -372621771;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX87878726 = -56551459;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX43690374 = -721063616;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX23278661 = -904720590;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX52149927 = -45136416;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX85836682 = -800694219;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX57873651 = -213488605;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX36255430 = -212405608;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX32229684 = 23703697;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX89751653 = -313659089;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX5269913 = -114953492;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX84723870 = -607373362;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX25176111 = 37984271;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX77284288 = -405287238;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX31776159 = -541879944;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX10940213 = 66141914;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX8991816 = -2684263;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX42547035 = -963642145;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX19350552 = -331327896;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX85698596 = -445344675;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX97228475 = -694555280;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX54178527 = -126168714;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX54812724 = -472157820;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX18851642 = -851052076;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX55456926 = -369185997;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX39898680 = 88493198;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX93282524 = -455895308;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX1949396 = -467398189;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX31141827 = -985919959;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX42438545 = -188623918;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX75093273 = 48440231;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX6569950 = -261074334;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX58269 = -399498980;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX41649449 = -551184889;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX4991783 = -376051905;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX3747717 = -66804886;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX68412509 = -226982559;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX13674745 = -377428330;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX3189968 = -834971813;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX55312353 = -880275357;    double cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX83930695 = -411338686;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX51292066 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX15487829;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX15487829 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX8033415;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX8033415 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX90335484;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX90335484 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX75224038;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX75224038 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX89883155;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX89883155 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX56187043;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX56187043 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX5159445;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX5159445 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX86519429;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX86519429 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX54053854;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX54053854 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX28289412;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX28289412 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX7670953;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX7670953 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX97095824;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX97095824 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX18250065;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX18250065 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX66828104;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX66828104 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX65312610;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX65312610 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX70162474;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX70162474 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX82694595;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX82694595 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX23402602;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX23402602 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX23043095;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX23043095 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX45945221;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX45945221 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX97886928;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX97886928 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX60499587;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX60499587 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX27502944;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX27502944 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX86998570;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX86998570 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX74805513;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX74805513 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX2004429;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX2004429 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX12496670;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX12496670 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX81880783;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX81880783 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX34369503;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX34369503 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX68217171;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX68217171 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX70415760;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX70415760 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX71415523;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX71415523 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX64866140;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX64866140 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX28498411;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX28498411 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX61558191;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX61558191 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX80588739;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX80588739 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX44986363;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX44986363 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX5410308;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX5410308 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX91626443;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX91626443 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX12102883;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX12102883 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX36953405;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX36953405 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX55339894;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX55339894 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX41149036;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX41149036 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX41804347;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX41804347 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX89770094;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX89770094 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX20626987;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX20626987 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX47191705;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX47191705 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX93645028;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX93645028 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX26423858;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX26423858 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX94470822;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX94470822 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX74934647;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX74934647 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX68466364;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX68466364 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX40273697;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX40273697 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX22427596;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX22427596 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX53405138;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX53405138 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX54988241;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX54988241 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX80530470;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX80530470 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX3336915;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX3336915 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX418525;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX418525 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX87878726;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX87878726 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX43690374;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX43690374 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX23278661;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX23278661 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX52149927;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX52149927 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX85836682;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX85836682 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX57873651;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX57873651 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX36255430;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX36255430 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX32229684;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX32229684 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX89751653;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX89751653 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX5269913;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX5269913 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX84723870;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX84723870 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX25176111;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX25176111 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX77284288;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX77284288 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX31776159;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX31776159 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX10940213;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX10940213 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX8991816;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX8991816 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX42547035;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX42547035 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX19350552;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX19350552 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX85698596;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX85698596 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX97228475;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX97228475 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX54178527;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX54178527 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX54812724;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX54812724 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX18851642;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX18851642 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX55456926;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX55456926 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX39898680;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX39898680 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX93282524;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX93282524 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX1949396;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX1949396 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX31141827;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX31141827 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX42438545;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX42438545 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX75093273;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX75093273 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX6569950;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX6569950 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX58269;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX58269 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX41649449;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX41649449 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX4991783;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX4991783 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX3747717;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX3747717 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX68412509;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX68412509 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX13674745;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX13674745 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX3189968;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX3189968 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX55312353;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX55312353 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX83930695;     cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX83930695 = cnmZrwbfoEWXBKIcmaLWInOzMvlZaePixopiOzqgcgDaKfcJixozlzLpYmNvKBEKonZMUZOnkfubknogTbqcpQwydzKbluVacsmX51292066;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void bRnBZOdhTyOihtWghIdpHGmpImxtksCCViZNCSUiNNctptivHkuLaLwUfxtmQjwbvMNyTIeOnubudszOEBlzxqbJmFiEYhSdQBxd44827039() {     long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA73393500 = -448910404;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA57681639 = -214498189;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA45938295 = -490159430;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA56066332 = -43740002;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA8798382 = 90368233;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA26736588 = -573550347;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA51048873 = -270767004;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA21014714 = -137885138;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA74732897 = 80849560;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA28897016 = -477089729;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA92678778 = -86487953;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA50859208 = -652688054;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA63020843 = -546054180;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA99962784 = -144276475;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA5410695 = -249876267;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA98708202 = -114688835;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA69725059 = -418133497;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA78445721 = -639228528;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA6576165 = -728346312;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA88892939 = -659856241;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA14798630 = -101849374;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA51143660 = -936505053;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA35321150 = -657830366;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA31216703 = -716263932;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA55955372 = -443875383;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA22031976 = -958321214;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA47178232 = -423801840;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA50944070 = -885717278;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA65614743 = 79676180;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA99901566 = 31834022;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA82651546 = -938580776;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA50993947 = -976963234;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA67320802 = 17474388;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA30971462 = -368867191;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA98209218 = -205592558;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA88607200 = -481195085;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA54820407 = -420009820;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA58727184 = -336973669;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA77411394 = -741740040;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA93991070 = -874958702;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA97597324 = -460676586;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA32307517 = -101732345;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA11940370 = -125422055;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA44534474 = -412536707;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA57762912 = 22041111;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA33637459 = -618598126;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA3458638 = 87141142;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA48102709 = -110945063;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA3736222 = -753028856;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA43012180 = -626551312;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA49766025 = -509448765;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA67328236 = -514788458;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA90959172 = -863454262;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA89554665 = -258964717;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA30028410 = -621291740;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA22249840 = -512405352;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA22360489 = -556667823;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA14721593 = -773895499;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA110961 = -599864620;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA86766405 = 48689447;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA79558356 = -49748508;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA104804 = -385049727;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA55399970 = -117561318;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA74831331 = -950984462;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA46245470 = -538508953;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA41684831 = -109524720;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA83538405 = -570162442;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA32049382 = -77186990;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA1753567 = -938683917;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA16803495 = -768681182;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA43887795 = -694679016;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA10997875 = 18840172;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA1034328 = -897488489;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA12585094 = -853387610;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA91295614 = -99179655;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA82491112 = 99882970;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA39203290 = -711082999;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA90786675 = -145293660;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA73453790 = -638305044;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA22317913 = -825277257;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA18573339 = -945462356;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA99075522 = -212856777;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA47207848 = -32688422;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA22602563 = -293772508;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA50135541 = -458717214;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA15323310 = -323792319;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA60034774 = -13508973;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA77766137 = -723560896;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA943052 = -747575452;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA75959378 = -693187206;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA66246711 = -924527263;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA40098815 = -646114321;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA58616224 = -737109049;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA90644988 = -690429487;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA14432714 = -725210195;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA97492521 = 24373140;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA76907547 = -984171027;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA37109039 = -174437593;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA98289003 = -874027754;    long fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA16078081 = -448910404;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA73393500 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA57681639;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA57681639 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA45938295;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA45938295 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA56066332;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA56066332 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA8798382;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA8798382 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA26736588;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA26736588 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA51048873;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA51048873 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA21014714;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA21014714 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA74732897;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA74732897 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA28897016;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA28897016 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA92678778;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA92678778 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA50859208;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA50859208 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA63020843;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA63020843 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA99962784;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA99962784 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA5410695;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA5410695 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA98708202;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA98708202 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA69725059;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA69725059 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA78445721;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA78445721 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA6576165;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA6576165 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA88892939;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA88892939 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA14798630;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA14798630 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA51143660;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA51143660 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA35321150;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA35321150 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA31216703;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA31216703 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA55955372;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA55955372 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA22031976;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA22031976 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA47178232;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA47178232 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA50944070;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA50944070 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA65614743;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA65614743 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA99901566;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA99901566 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA82651546;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA82651546 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA50993947;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA50993947 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA67320802;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA67320802 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA30971462;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA30971462 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA98209218;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA98209218 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA88607200;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA88607200 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA54820407;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA54820407 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA58727184;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA58727184 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA77411394;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA77411394 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA93991070;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA93991070 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA97597324;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA97597324 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA32307517;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA32307517 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA11940370;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA11940370 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA44534474;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA44534474 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA57762912;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA57762912 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA33637459;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA33637459 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA3458638;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA3458638 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA48102709;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA48102709 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA3736222;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA3736222 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA43012180;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA43012180 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA49766025;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA49766025 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA67328236;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA67328236 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA90959172;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA90959172 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA89554665;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA89554665 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA30028410;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA30028410 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA22249840;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA22249840 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA22360489;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA22360489 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA14721593;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA14721593 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA110961;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA110961 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA86766405;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA86766405 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA79558356;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA79558356 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA104804;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA104804 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA55399970;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA55399970 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA74831331;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA74831331 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA46245470;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA46245470 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA41684831;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA41684831 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA83538405;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA83538405 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA32049382;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA32049382 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA1753567;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA1753567 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA16803495;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA16803495 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA43887795;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA43887795 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA10997875;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA10997875 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA1034328;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA1034328 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA12585094;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA12585094 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA91295614;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA91295614 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA82491112;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA82491112 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA39203290;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA39203290 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA90786675;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA90786675 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA73453790;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA73453790 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA22317913;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA22317913 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA18573339;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA18573339 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA99075522;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA99075522 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA47207848;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA47207848 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA22602563;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA22602563 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA50135541;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA50135541 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA15323310;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA15323310 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA60034774;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA60034774 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA77766137;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA77766137 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA943052;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA943052 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA75959378;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA75959378 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA66246711;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA66246711 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA40098815;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA40098815 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA58616224;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA58616224 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA90644988;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA90644988 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA14432714;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA14432714 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA97492521;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA97492521 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA76907547;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA76907547 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA37109039;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA37109039 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA98289003;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA98289003 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA16078081;     fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA16078081 = fjcyFgXfkBqmxfhOywdEkDLOaZpFcxdcvWiUZmuRSwZlOvHtzDfUXsscYOYfqSCmXOrawNpLCMmoeZzeSwFYRJECdGwLOObAtmOA73393500;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void dDTxZUcViZwdDHbgoIZaCkyCtFRmeLkBETIqDaUkLjBxgaQjwRZpQuVtFRDTDpqXovYjykcNooOSiAWSphqMmvkJxPkmWAhWsdIk9066271() {     long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy21365895 = -608347468;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy38426282 = -418519020;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy7586750 = -532274810;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy16794950 = -57470427;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy76314640 = -547611055;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy58628981 = -589139728;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy48525456 = -675025518;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy11658697 = -989163661;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy97225320 = -299570524;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy39818983 = -554392585;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy44554192 = -393540241;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy42079813 = -247142254;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy6609802 = -929019865;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy34137254 = -646509525;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy98414859 = -865899938;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy50685158 = -932547854;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy19346528 = -383733111;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy5538043 = -507248579;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy9322517 = -546422295;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy28570690 = -886930070;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy16691002 = -555036464;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy16461985 = -761004295;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy42378271 = -878003064;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy59430532 = -39105768;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy46398758 = -732930649;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy39824108 = -966665979;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy76655561 = -847201467;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy20754320 = -903833964;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy80384516 = -500874789;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy62381236 = -897371407;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy48988986 = -798543959;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy45725070 = -802830423;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy69546525 = -959690244;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy43947220 = -749139012;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy75843571 = -970784892;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy35091920 = -214185178;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy97905504 = -633469716;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy87925972 = -53313966;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy66643104 = -501124393;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy22960459 = -381153051;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy73505977 = -438914442;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy64825498 = -368679540;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy66690781 = -41559020;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy14771873 = -878315955;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy17727055 = -826123484;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy8138064 = -223374759;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy15524489 = -523737971;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy814155 = -60662901;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy49007447 = -802421781;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy93905921 = -616594724;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy68771489 = -13183644;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy68438073 = -23686920;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy87347179 = -3264041;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy18740120 = -849709307;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy92375269 = -137086272;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy4903910 = -847343173;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy96048011 = -540515957;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy48156217 = -393169042;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy70396192 = -324539778;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy36490533 = -580945076;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy81973420 = -741938262;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy27771136 = -771191554;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy31274180 = -388288872;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy34844084 = -402199118;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy90829997 = -755848627;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy98829121 = -590709819;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy72533287 = -287452010;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy62662581 = -79880853;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy58293683 = -675724634;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy63322940 = -551714761;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy52779653 = -199078139;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy31420555 = -230419145;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy38894938 = 93875814;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy86362057 = -65269245;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy55064713 = -348015628;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy51865504 = -86356925;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy49771204 = -619445276;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy27606398 = -999687109;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy41703478 = -212982285;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy38260695 = -409555891;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy24299619 = -342928008;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy75841407 = -686538567;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy71746873 = -1412183;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy86478595 = -884280065;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy93609747 = -784187763;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy80550913 = -674857040;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy58377891 = -699566383;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy50806405 = -9980938;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy51571951 = -512052741;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy70939661 = -23441719;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy39043908 = -673669221;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy49749288 = -140300674;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy17529780 = -728774188;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy30152571 = -920179317;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy40987038 = -639214790;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy45734841 = -667722888;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy33551319 = -980390668;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy31846697 = -639359903;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy23941875 = -22467329;    long CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy18897933 = -608347468;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy21365895 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy38426282;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy38426282 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy7586750;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy7586750 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy16794950;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy16794950 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy76314640;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy76314640 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy58628981;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy58628981 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy48525456;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy48525456 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy11658697;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy11658697 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy97225320;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy97225320 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy39818983;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy39818983 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy44554192;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy44554192 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy42079813;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy42079813 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy6609802;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy6609802 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy34137254;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy34137254 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy98414859;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy98414859 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy50685158;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy50685158 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy19346528;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy19346528 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy5538043;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy5538043 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy9322517;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy9322517 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy28570690;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy28570690 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy16691002;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy16691002 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy16461985;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy16461985 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy42378271;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy42378271 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy59430532;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy59430532 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy46398758;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy46398758 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy39824108;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy39824108 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy76655561;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy76655561 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy20754320;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy20754320 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy80384516;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy80384516 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy62381236;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy62381236 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy48988986;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy48988986 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy45725070;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy45725070 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy69546525;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy69546525 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy43947220;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy43947220 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy75843571;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy75843571 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy35091920;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy35091920 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy97905504;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy97905504 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy87925972;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy87925972 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy66643104;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy66643104 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy22960459;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy22960459 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy73505977;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy73505977 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy64825498;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy64825498 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy66690781;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy66690781 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy14771873;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy14771873 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy17727055;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy17727055 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy8138064;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy8138064 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy15524489;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy15524489 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy814155;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy814155 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy49007447;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy49007447 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy93905921;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy93905921 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy68771489;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy68771489 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy68438073;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy68438073 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy87347179;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy87347179 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy18740120;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy18740120 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy92375269;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy92375269 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy4903910;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy4903910 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy96048011;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy96048011 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy48156217;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy48156217 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy70396192;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy70396192 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy36490533;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy36490533 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy81973420;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy81973420 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy27771136;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy27771136 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy31274180;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy31274180 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy34844084;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy34844084 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy90829997;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy90829997 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy98829121;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy98829121 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy72533287;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy72533287 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy62662581;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy62662581 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy58293683;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy58293683 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy63322940;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy63322940 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy52779653;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy52779653 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy31420555;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy31420555 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy38894938;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy38894938 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy86362057;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy86362057 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy55064713;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy55064713 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy51865504;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy51865504 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy49771204;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy49771204 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy27606398;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy27606398 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy41703478;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy41703478 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy38260695;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy38260695 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy24299619;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy24299619 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy75841407;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy75841407 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy71746873;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy71746873 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy86478595;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy86478595 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy93609747;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy93609747 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy80550913;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy80550913 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy58377891;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy58377891 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy50806405;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy50806405 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy51571951;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy51571951 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy70939661;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy70939661 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy39043908;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy39043908 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy49749288;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy49749288 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy17529780;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy17529780 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy30152571;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy30152571 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy40987038;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy40987038 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy45734841;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy45734841 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy33551319;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy33551319 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy31846697;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy31846697 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy23941875;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy23941875 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy18897933;     CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy18897933 = CgkMpLBcfDKFuleVzeltKOljzpqnHsZFsmQIxdWatIeuEfslWLWOrOZnuAMzUOUmdFMIUWYVHLzCkrfTxFRvWLNdMdNwQXFfnrQy21365895;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void XiJRlFEbsbqSarTgbdSiREuOOIjHOsqTFJyTzSYYCTimbawSLjwtaWLTovDdWwNAlWMQsAhJEufyWHmoMQvewxtjyffDOvCFFiBF33938357() {     long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ4803729 = -261537290;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ93870687 = -460496333;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ5308469 = -282928651;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ64667062 = -71811093;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ91276066 = -822833867;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ91938814 = -67644193;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ88112110 = -119473298;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ95220189 = -680499008;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ38495186 = -354675945;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ69004149 = -659575569;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ14290735 = -543128186;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ19576889 = 29761137;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ56580491 = -742339581;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ67608365 = -779952934;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ4441432 = -922635772;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ11638867 = 46577170;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ68951171 = 43307292;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ9390022 = -51625077;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ92190928 = -63079432;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ90011895 = -928540514;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ52000813 = -955031869;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ6905569 = -162147949;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ83082374 = -276850103;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ37787199 = 8148313;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ16417406 = -619277262;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ51740333 = -755381623;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ96331883 = -776085522;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ95889470 = -482755836;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ29144058 = 90549754;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ54304448 = 14347368;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ496979 = -823394395;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ93555354 = -9847264;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ98537834 = -782506637;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ86388567 = -217422915;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ36928339 = -327763552;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ90309293 = -81974830;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ78461051 = -611972274;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ98422483 = -857047165;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ50951779 = -298703607;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ44328487 = -843178259;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ95010568 = -98407314;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ1010945 = -36379944;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ50541211 = -51746517;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ65908711 = -191463169;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ93689603 = 72460162;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ88172027 = -55030353;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ19237712 = -37322823;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ95868330 = -668145975;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ98512948 = -951787725;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ49283828 = -410640066;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ55288307 = -912640074;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ38486125 = -464091979;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ76907986 = -107065365;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ53667149 = -42546;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ81937543 = -242471673;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ97898159 = 610658;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ10788313 = -83646230;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ67521269 = -191076965;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ48249657 = -452533832;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ39535733 = 32547755;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ95606931 = -291558671;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ92222639 = -636717462;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ66076132 = -671048762;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ84190738 = -269023313;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ68507170 = -836181174;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ20735380 = -433280922;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ21039054 = -187732226;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ70191923 = -424916666;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ30680027 = -352189382;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ14132139 = -740660943;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ33177816 = -341450557;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ70528688 = -99645543;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ58438242 = -752921471;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ47862442 = -219901174;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ95001326 = -730133200;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ50989868 = -818651925;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ56364358 = -10401433;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ17173664 = 14613066;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ44097595 = 35688151;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ28245378 = -464246909;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ32502622 = -618058801;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ463553 = -7939547;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ97376522 = -530968112;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ79860229 = -498810181;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ99016140 = -73012559;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ62010853 = -259302416;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ16647368 = -902781899;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ44870685 = -682464092;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ4451024 = -974951242;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ39030180 = -228374211;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ79520980 = -998328600;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ10939782 = -320895309;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ50172827 = -304513333;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ11416046 = -231251363;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ48721555 = -451619589;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ2787930 = -461689852;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ34934813 = -365331183;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ66350473 = -782723204;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ97401540 = -355281995;    long NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ72954223 = -261537290;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ4803729 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ93870687;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ93870687 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ5308469;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ5308469 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ64667062;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ64667062 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ91276066;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ91276066 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ91938814;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ91938814 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ88112110;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ88112110 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ95220189;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ95220189 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ38495186;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ38495186 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ69004149;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ69004149 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ14290735;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ14290735 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ19576889;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ19576889 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ56580491;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ56580491 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ67608365;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ67608365 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ4441432;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ4441432 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ11638867;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ11638867 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ68951171;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ68951171 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ9390022;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ9390022 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ92190928;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ92190928 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ90011895;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ90011895 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ52000813;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ52000813 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ6905569;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ6905569 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ83082374;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ83082374 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ37787199;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ37787199 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ16417406;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ16417406 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ51740333;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ51740333 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ96331883;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ96331883 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ95889470;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ95889470 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ29144058;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ29144058 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ54304448;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ54304448 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ496979;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ496979 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ93555354;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ93555354 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ98537834;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ98537834 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ86388567;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ86388567 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ36928339;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ36928339 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ90309293;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ90309293 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ78461051;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ78461051 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ98422483;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ98422483 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ50951779;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ50951779 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ44328487;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ44328487 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ95010568;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ95010568 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ1010945;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ1010945 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ50541211;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ50541211 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ65908711;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ65908711 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ93689603;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ93689603 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ88172027;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ88172027 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ19237712;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ19237712 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ95868330;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ95868330 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ98512948;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ98512948 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ49283828;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ49283828 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ55288307;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ55288307 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ38486125;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ38486125 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ76907986;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ76907986 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ53667149;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ53667149 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ81937543;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ81937543 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ97898159;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ97898159 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ10788313;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ10788313 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ67521269;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ67521269 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ48249657;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ48249657 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ39535733;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ39535733 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ95606931;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ95606931 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ92222639;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ92222639 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ66076132;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ66076132 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ84190738;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ84190738 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ68507170;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ68507170 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ20735380;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ20735380 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ21039054;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ21039054 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ70191923;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ70191923 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ30680027;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ30680027 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ14132139;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ14132139 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ33177816;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ33177816 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ70528688;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ70528688 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ58438242;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ58438242 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ47862442;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ47862442 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ95001326;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ95001326 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ50989868;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ50989868 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ56364358;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ56364358 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ17173664;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ17173664 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ44097595;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ44097595 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ28245378;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ28245378 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ32502622;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ32502622 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ463553;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ463553 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ97376522;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ97376522 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ79860229;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ79860229 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ99016140;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ99016140 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ62010853;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ62010853 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ16647368;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ16647368 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ44870685;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ44870685 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ4451024;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ4451024 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ39030180;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ39030180 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ79520980;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ79520980 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ10939782;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ10939782 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ50172827;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ50172827 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ11416046;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ11416046 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ48721555;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ48721555 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ2787930;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ2787930 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ34934813;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ34934813 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ66350473;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ66350473 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ97401540;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ97401540 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ72954223;     NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ72954223 = NfaLBmogqMwhIOQGAiDnddSNVduIsasnFhagqkcJINslixvzfvEXeNbjNNqYsgnPUJbbojPQFufPkgeXiudwVVJrFrTcuEQcZdRZ4803729;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void nvJeYZwvUwpdfPAvXlKyRjWKruuoJtwYJNbNinCywdkwgQclLyKfOFSDZymdeIlbzRTSonMjiLBnyZdbRJcLiQJDuXojBYUosJwP52923999() {     double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC68766921 = -474120042;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC34863546 = 809225;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC87506407 = -339082492;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC78971886 = -90118326;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC14631079 = -940139585;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC34462006 = -455096701;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC51414221 = 74848685;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC82745500 = 17796295;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC1818418 = -495236056;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC16900105 = -395979377;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC50124620 = -219197902;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC7871029 = -529511129;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC48032436 = -152960494;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC46507658 = -716263668;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC95113652 = -644000667;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC80941474 = 56098477;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC68446462 = -277492193;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC12179783 = -242318479;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC95852730 = -920514076;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC42915562 = -864638953;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC54523976 = -459281321;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC60663336 = -661480273;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC92491869 = -937080366;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC42072305 = -188974136;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC3675254 = -638017617;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC75463175 = -766507977;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC2301656 = -240618359;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC55636471 = -506911418;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC48837089 = 49815127;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC37610675 = -857926536;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC55613564 = -270011972;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC86530185 = -144336849;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC1505465 = -252059480;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC70356246 = -357785344;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC40440810 = -981353331;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC52288920 = -825961620;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC2574515 = -163252135;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC37354200 = -478834228;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC3260726 = -344549412;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC16287671 = -918104057;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC62888771 = -436057788;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC11034920 = -758976204;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC23541760 = -673262470;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC92891908 = -812502166;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC73641794 = -325092632;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC54172833 = -261399197;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC68658847 = -118494974;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC66150258 = -601103092;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC25541249 = -650978292;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC83808815 = -30697949;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC80628925 = -984286579;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC6632574 = -909289928;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC72091995 = -426811737;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC25914422 = -787701999;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC98400022 = -330197717;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC8103586 = -812639770;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC42371677 = -62110409;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC45434102 = -50108356;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC75296633 = -452100709;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC39167904 = -73631609;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC32160350 = -114478343;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC95777750 = -418239898;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC33908412 = 67981167;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC64207742 = -637309520;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC61286540 = -25967406;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC63594434 = 25138946;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC6365564 = -177451650;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC77676190 = -795175151;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC6066849 = -734910338;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC42824733 = -818039047;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC78366959 = -780649388;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC31092262 = -798657966;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC8919057 = -897769068;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC79565059 = 97589981;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC80026790 = -328581165;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC43489057 = -700305118;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC37121576 = -988217803;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC99599960 = -24578200;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC68430511 = -863881505;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC49502421 = -276618420;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC6804328 = -548013003;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC36151398 = -639515267;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC30095223 = -855933127;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC65028273 = -919486924;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC56981749 = -873639958;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC48980991 = -360722044;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC14438191 = -717525112;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC75591043 = -464357481;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC71956223 = 72412372;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC32337224 = -68713561;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC9917244 = -663851211;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC57140412 = -13143780;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC62057567 = 73266481;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC64092822 = -170917804;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC84127321 = -703625715;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC67111021 = 82182109;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC77126508 = -726957371;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC59334017 = 64047050;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC31605368 = -686534761;    double wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC10047361 = -474120042;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC68766921 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC34863546;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC34863546 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC87506407;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC87506407 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC78971886;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC78971886 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC14631079;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC14631079 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC34462006;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC34462006 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC51414221;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC51414221 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC82745500;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC82745500 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC1818418;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC1818418 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC16900105;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC16900105 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC50124620;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC50124620 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC7871029;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC7871029 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC48032436;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC48032436 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC46507658;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC46507658 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC95113652;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC95113652 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC80941474;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC80941474 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC68446462;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC68446462 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC12179783;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC12179783 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC95852730;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC95852730 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC42915562;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC42915562 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC54523976;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC54523976 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC60663336;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC60663336 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC92491869;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC92491869 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC42072305;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC42072305 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC3675254;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC3675254 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC75463175;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC75463175 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC2301656;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC2301656 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC55636471;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC55636471 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC48837089;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC48837089 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC37610675;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC37610675 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC55613564;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC55613564 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC86530185;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC86530185 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC1505465;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC1505465 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC70356246;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC70356246 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC40440810;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC40440810 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC52288920;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC52288920 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC2574515;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC2574515 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC37354200;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC37354200 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC3260726;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC3260726 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC16287671;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC16287671 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC62888771;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC62888771 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC11034920;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC11034920 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC23541760;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC23541760 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC92891908;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC92891908 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC73641794;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC73641794 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC54172833;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC54172833 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC68658847;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC68658847 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC66150258;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC66150258 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC25541249;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC25541249 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC83808815;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC83808815 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC80628925;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC80628925 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC6632574;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC6632574 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC72091995;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC72091995 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC25914422;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC25914422 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC98400022;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC98400022 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC8103586;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC8103586 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC42371677;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC42371677 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC45434102;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC45434102 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC75296633;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC75296633 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC39167904;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC39167904 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC32160350;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC32160350 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC95777750;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC95777750 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC33908412;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC33908412 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC64207742;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC64207742 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC61286540;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC61286540 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC63594434;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC63594434 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC6365564;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC6365564 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC77676190;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC77676190 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC6066849;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC6066849 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC42824733;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC42824733 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC78366959;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC78366959 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC31092262;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC31092262 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC8919057;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC8919057 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC79565059;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC79565059 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC80026790;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC80026790 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC43489057;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC43489057 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC37121576;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC37121576 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC99599960;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC99599960 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC68430511;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC68430511 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC49502421;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC49502421 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC6804328;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC6804328 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC36151398;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC36151398 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC30095223;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC30095223 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC65028273;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC65028273 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC56981749;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC56981749 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC48980991;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC48980991 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC14438191;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC14438191 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC75591043;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC75591043 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC71956223;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC71956223 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC32337224;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC32337224 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC9917244;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC9917244 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC57140412;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC57140412 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC62057567;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC62057567 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC64092822;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC64092822 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC84127321;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC84127321 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC67111021;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC67111021 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC77126508;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC77126508 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC59334017;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC59334017 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC31605368;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC31605368 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC10047361;     wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC10047361 = wKwXmawDDeEzVLMVfasjHQXRyciKqNvpcOeHMFZITGAQSoOpMyJSRNIgmfYJrmMqiPWGgEJaBkPAPHzKNsxrEdYPGyVggPQhtKhC68766921;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void nUiMTXjvzyORQmFhPyJYxXIQOYCcfzLCwfMMfVQzghyCfwRLeaaxubjQMRxuUGPAnLUlZsCuhKrQYFkRHryFZOIVwwANXWfaRyPT86516605() {     double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm3667465 = -901684169;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm86683611 = -271203054;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm40537664 = -955184641;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm82483838 = -289105212;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm24036323 = -963577030;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm45014644 = -68946565;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm30389137 = -306671400;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm86891780 = -559844732;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm19956367 = -271520772;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm4682945 = -551336569;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm1360159 = -720866861;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm54426434 = -510589851;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm20394930 = -602630994;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm70156016 = -669842066;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm66658578 = -384077036;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm93633041 = -407616478;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm12055347 = 6602926;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm45488328 = -488653153;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm39330384 = -557128762;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm67272376 = -917786169;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm44212593 = -146681031;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm86375354 = -904109535;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm32316934 = -595701471;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm2992977 = -664984226;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm21640936 = -524646810;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm70860129 = -430170160;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm18447832 = -615320339;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm90629746 = -143710383;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm23831208 = -300898159;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm3714114 = -572920661;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm47658578 = -759310820;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm979482 = -849303978;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm72551846 = -510971629;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm12972265 = -997490202;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm28496073 = -759201836;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm88905664 = -659781783;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm89687507 = -244963942;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm68963685 = -80144415;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm10410006 = -854103521;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm1171242 = 4740111;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm34103186 = -48274396;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm42696166 = -89286799;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm94458984 = -923749233;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm22442088 = -671312653;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm1572745 = -55889543;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm90812390 = -9481294;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm73676932 = -823797980;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm45734066 = -140829327;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm42379712 = -986351035;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm7256713 = -771734804;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm27732883 = -968821178;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm98108195 = -635355325;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm78173257 = -582487192;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm99394574 = -362618898;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm42304012 = -740206219;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm17292110 = -997574635;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm54366678 = -675501584;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm37544687 = -190200415;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm60842903 = -764458402;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm53176193 = -433406870;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm26566813 = -453626227;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm39759391 = -62961018;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm63060572 = -158946574;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm16242254 = -698600112;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm57024367 = -792025749;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm380677 = -871562884;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm81874587 = -999618223;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm7422665 = -605140792;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm41659943 = -910640230;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm77752913 = -724295253;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm3945534 = -62652536;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm43091661 = -913252659;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm35078323 = -634549632;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm38159142 = -461868874;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm33169190 = -769511774;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm1516428 = 42605768;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm91916370 = -980360302;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm9874846 = -924388819;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm1420232 = -509094684;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm30828545 = -415165517;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm97183196 = -606372180;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm72713765 = -374491012;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm48250034 = -157359349;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm16574496 = -529163355;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm75981231 = -604099483;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm49550382 = -23955495;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm22806225 = -166816786;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm73157272 = -48352731;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm70668252 = -157283984;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm11203964 = -761627202;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm34538987 = -984280200;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm52142821 = 45236473;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm8120783 = -315686014;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm57233812 = -320696651;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm74604429 = -541633662;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm94343795 = -985313379;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm79635593 = -930340225;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm78216731 = -125149122;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm65417721 = -879286904;    double bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm1192069 = -901684169;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm3667465 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm86683611;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm86683611 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm40537664;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm40537664 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm82483838;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm82483838 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm24036323;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm24036323 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm45014644;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm45014644 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm30389137;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm30389137 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm86891780;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm86891780 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm19956367;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm19956367 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm4682945;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm4682945 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm1360159;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm1360159 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm54426434;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm54426434 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm20394930;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm20394930 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm70156016;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm70156016 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm66658578;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm66658578 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm93633041;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm93633041 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm12055347;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm12055347 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm45488328;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm45488328 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm39330384;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm39330384 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm67272376;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm67272376 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm44212593;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm44212593 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm86375354;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm86375354 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm32316934;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm32316934 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm2992977;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm2992977 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm21640936;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm21640936 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm70860129;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm70860129 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm18447832;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm18447832 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm90629746;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm90629746 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm23831208;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm23831208 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm3714114;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm3714114 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm47658578;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm47658578 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm979482;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm979482 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm72551846;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm72551846 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm12972265;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm12972265 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm28496073;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm28496073 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm88905664;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm88905664 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm89687507;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm89687507 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm68963685;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm68963685 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm10410006;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm10410006 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm1171242;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm1171242 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm34103186;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm34103186 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm42696166;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm42696166 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm94458984;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm94458984 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm22442088;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm22442088 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm1572745;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm1572745 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm90812390;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm90812390 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm73676932;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm73676932 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm45734066;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm45734066 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm42379712;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm42379712 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm7256713;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm7256713 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm27732883;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm27732883 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm98108195;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm98108195 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm78173257;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm78173257 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm99394574;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm99394574 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm42304012;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm42304012 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm17292110;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm17292110 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm54366678;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm54366678 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm37544687;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm37544687 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm60842903;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm60842903 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm53176193;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm53176193 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm26566813;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm26566813 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm39759391;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm39759391 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm63060572;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm63060572 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm16242254;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm16242254 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm57024367;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm57024367 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm380677;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm380677 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm81874587;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm81874587 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm7422665;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm7422665 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm41659943;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm41659943 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm77752913;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm77752913 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm3945534;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm3945534 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm43091661;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm43091661 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm35078323;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm35078323 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm38159142;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm38159142 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm33169190;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm33169190 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm1516428;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm1516428 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm91916370;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm91916370 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm9874846;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm9874846 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm1420232;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm1420232 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm30828545;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm30828545 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm97183196;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm97183196 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm72713765;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm72713765 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm48250034;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm48250034 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm16574496;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm16574496 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm75981231;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm75981231 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm49550382;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm49550382 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm22806225;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm22806225 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm73157272;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm73157272 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm70668252;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm70668252 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm11203964;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm11203964 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm34538987;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm34538987 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm52142821;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm52142821 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm8120783;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm8120783 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm57233812;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm57233812 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm74604429;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm74604429 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm94343795;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm94343795 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm79635593;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm79635593 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm78216731;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm78216731 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm65417721;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm65417721 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm1192069;     bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm1192069 = bgllXfPjjSrGggndHKaZDzodrpWLfRzapQqaFNhtyYldEugriBSGXqQbrIuhIEESTRQPpMXudgaeOLYwJfcdzsCzAOjxSzktUTKm3667465;}
// Junk Finished
