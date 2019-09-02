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
void swdDxKKnlAqijNAUMIZilSpOMOB96824991() {     int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo57814775 = -126499824;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo38407815 = -378132044;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo13300797 = -972656197;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo5437793 = -752175842;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo28858049 = -135654426;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo26455558 = -124290372;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo52506854 = -528689191;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo5781946 = -808359169;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo48115737 = -36403034;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo83632049 = -838650674;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo457954 = -912014514;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo60999808 = -501139150;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo7405978 = -32125041;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo20319218 = -25701398;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo39226799 = -898610613;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo50842913 = -467629064;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo25332883 = -382943893;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo24703928 = -605534041;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo94177918 = -199192716;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo19506304 = -572321695;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo58727457 = -351099366;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo22861069 = -820024617;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo36254668 = -908476548;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo61521162 = -307355960;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo69147856 = 56309371;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo32494331 = -828212374;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo77355288 = -284500566;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo92745600 = -283720713;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo77919221 = -781907117;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo65984608 = -431385112;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo67163447 = -824064955;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo70670509 = -855018792;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo18978395 = 32000814;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo18591600 = 54331145;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo46152642 = -248609480;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo72675910 = -663256444;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo43502745 = 80388227;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo48288708 = -107506477;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo77975673 = -852321568;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo11725781 = -673074493;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo13787648 = -617834493;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo35298043 = -412879572;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo95906044 = -581336807;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo79452507 = -275914614;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo97730160 = -964797189;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo21430520 = -723110453;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo45347829 = -958920740;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo76652376 = -742312117;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo89080357 = -203657880;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo85917038 = -843490313;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo56654141 = 77234223;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo89574691 = -212677453;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo99874030 = -549283220;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo93318380 = -734855350;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo38097535 = -901530761;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo34953706 = -306475208;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo2153148 = -469655496;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo51779634 = -565300238;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo36289936 = -708485214;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo96363718 = -307442053;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo49100270 = -839789806;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo59761253 = -144968478;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo27862724 = 73547947;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo82131129 = -605017922;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo16468602 = 85414280;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo29787445 = 43004278;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo42021413 = -433139965;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo88814378 = 13543814;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo74166576 = -777091919;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo66550888 = -135354170;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo7340168 = -448017292;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo77044174 = -175437417;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo46728254 = -753212473;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo82452137 = -526118223;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo5718657 = -954487202;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo23429414 = -938219795;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo26955025 = -138687810;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo56802161 = -532561934;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo63791002 = -342558772;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo47717336 = -220580176;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo87146501 = -869291635;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo702912 = -542188450;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo3665244 = 19937166;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo92002182 = -938416804;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo9330467 = -408619336;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo77588756 = -511387503;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo70796478 = -205735573;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo25660015 = -233143837;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo80494064 = -44138094;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo11198937 = -942134273;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo70522763 = -93600948;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo91723111 = -354311536;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo11998772 = -399021263;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo81611955 = -444879516;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo62625511 = -833284687;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo54026394 = -372866016;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo7435319 = -386427519;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo13774915 = -976318885;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo62983905 = -261328895;    int ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo67942716 = -126499824;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo57814775 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo38407815;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo38407815 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo13300797;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo13300797 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo5437793;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo5437793 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo28858049;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo28858049 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo26455558;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo26455558 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo52506854;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo52506854 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo5781946;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo5781946 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo48115737;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo48115737 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo83632049;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo83632049 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo457954;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo457954 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo60999808;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo60999808 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo7405978;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo7405978 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo20319218;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo20319218 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo39226799;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo39226799 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo50842913;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo50842913 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo25332883;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo25332883 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo24703928;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo24703928 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo94177918;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo94177918 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo19506304;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo19506304 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo58727457;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo58727457 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo22861069;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo22861069 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo36254668;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo36254668 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo61521162;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo61521162 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo69147856;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo69147856 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo32494331;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo32494331 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo77355288;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo77355288 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo92745600;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo92745600 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo77919221;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo77919221 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo65984608;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo65984608 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo67163447;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo67163447 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo70670509;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo70670509 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo18978395;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo18978395 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo18591600;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo18591600 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo46152642;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo46152642 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo72675910;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo72675910 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo43502745;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo43502745 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo48288708;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo48288708 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo77975673;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo77975673 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo11725781;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo11725781 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo13787648;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo13787648 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo35298043;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo35298043 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo95906044;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo95906044 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo79452507;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo79452507 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo97730160;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo97730160 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo21430520;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo21430520 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo45347829;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo45347829 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo76652376;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo76652376 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo89080357;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo89080357 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo85917038;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo85917038 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo56654141;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo56654141 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo89574691;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo89574691 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo99874030;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo99874030 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo93318380;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo93318380 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo38097535;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo38097535 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo34953706;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo34953706 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo2153148;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo2153148 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo51779634;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo51779634 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo36289936;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo36289936 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo96363718;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo96363718 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo49100270;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo49100270 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo59761253;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo59761253 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo27862724;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo27862724 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo82131129;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo82131129 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo16468602;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo16468602 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo29787445;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo29787445 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo42021413;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo42021413 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo88814378;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo88814378 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo74166576;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo74166576 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo66550888;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo66550888 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo7340168;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo7340168 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo77044174;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo77044174 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo46728254;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo46728254 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo82452137;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo82452137 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo5718657;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo5718657 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo23429414;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo23429414 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo26955025;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo26955025 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo56802161;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo56802161 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo63791002;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo63791002 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo47717336;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo47717336 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo87146501;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo87146501 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo702912;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo702912 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo3665244;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo3665244 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo92002182;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo92002182 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo9330467;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo9330467 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo77588756;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo77588756 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo70796478;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo70796478 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo25660015;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo25660015 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo80494064;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo80494064 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo11198937;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo11198937 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo70522763;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo70522763 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo91723111;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo91723111 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo11998772;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo11998772 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo81611955;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo81611955 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo62625511;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo62625511 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo54026394;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo54026394 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo7435319;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo7435319 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo13774915;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo13774915 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo62983905;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo62983905 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo67942716;     ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo67942716 = ZTkQLlhbVMHxrrbegDKRaNFNALNLaHPqiIlUAkfxBmfHELiqJnIKBwbLOXPAXVBOxQPjGHewYNPo57814775;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void yntUlOiHssNiybRGSoOWslBlhKU10254387() {     float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp47642457 = -333472063;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp56523823 = 23426074;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp20555522 = -906054036;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp99820247 = -588888062;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp94995078 = -890963131;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp98823142 = -990972899;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp86971055 = -98241240;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp37310020 = -24042303;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp28458089 = 70789464;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp64246324 = -664280909;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp44098491 = -548681502;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp11617976 = -345641349;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp8374777 = -358165322;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp5524480 = -497928932;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp29492703 = -340195788;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp13291431 = 69652440;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp30739815 = -797598008;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp81835416 = -676051501;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp37840780 = -529575584;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp18311639 = -669566434;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp73959009 = -301986090;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp66976695 = -561093386;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp14778119 = -502304456;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp33956448 = -930734646;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp6334979 = -148605129;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp43356918 = -966246581;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp37850169 = -130637618;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp19258976 = -254025132;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp41326512 = -620626351;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp22328312 = -272325101;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp7595778 = -630670383;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp62522134 = -797306353;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp13718892 = -929715080;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp49712552 = -954670733;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp39946695 = -40261043;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp96919436 = 84789479;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp48411006 = -392895695;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp72596070 = -868681061;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp82828839 = -377166779;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp82162620 = -312023967;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp59935727 = -509036311;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp68183548 = -676631684;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp27339889 = -332806154;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp32519564 = -88130172;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp5705981 = -799228224;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp58492533 = 52552397;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp51700958 = -961903429;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp74118326 = -631852259;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp69947078 = -216706742;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp74792867 = -736566329;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp58964390 = -187941864;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp80984645 = -529748056;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp44401587 = -137113190;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp55720408 = -495874385;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp36295378 = -332588044;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp80665761 = -772378678;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp41745705 = -474269470;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp86599074 = -975319391;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp93485269 = -340282933;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp51638161 = -924716551;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp60972973 = -760335282;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp67712079 = -844216108;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp95983508 = -403415952;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp6129778 = -656885435;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp56650547 = 66389473;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp81576356 = -751375149;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp97899084 = -415926269;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp58662224 = -403494589;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp65577784 = -357667890;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp32573266 = -324985268;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp64880424 = -537451865;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp58143745 = -928916948;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp99006576 = -198884722;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp55678160 = -117551617;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp58375911 = -60530124;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp5775461 = -625354406;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp39636807 = -128287232;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp82258554 = -314174285;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp28250467 = -31506422;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp47842445 = -101157527;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp91655959 = 95656847;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp63731843 = -498785359;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp49311898 = 62681609;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp66533644 = -884060023;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp63363921 = 15616762;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp26611132 = -922327;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp18120547 = -560193164;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp57998483 = -333840695;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp13417175 = -522082689;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp59280934 = -267882366;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp55173732 = -440941051;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp61811932 = -417576305;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp79110801 = -428398128;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp31190679 = -452450229;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp21189647 = -551688686;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp92223648 = -664820203;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp72200040 = -173215732;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp21210112 = -675920719;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp75869016 = -54519645;    float hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp24129625 = -333472063;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp47642457 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp56523823;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp56523823 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp20555522;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp20555522 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp99820247;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp99820247 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp94995078;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp94995078 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp98823142;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp98823142 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp86971055;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp86971055 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp37310020;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp37310020 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp28458089;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp28458089 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp64246324;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp64246324 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp44098491;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp44098491 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp11617976;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp11617976 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp8374777;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp8374777 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp5524480;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp5524480 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp29492703;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp29492703 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp13291431;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp13291431 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp30739815;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp30739815 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp81835416;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp81835416 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp37840780;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp37840780 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp18311639;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp18311639 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp73959009;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp73959009 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp66976695;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp66976695 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp14778119;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp14778119 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp33956448;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp33956448 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp6334979;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp6334979 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp43356918;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp43356918 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp37850169;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp37850169 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp19258976;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp19258976 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp41326512;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp41326512 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp22328312;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp22328312 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp7595778;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp7595778 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp62522134;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp62522134 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp13718892;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp13718892 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp49712552;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp49712552 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp39946695;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp39946695 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp96919436;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp96919436 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp48411006;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp48411006 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp72596070;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp72596070 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp82828839;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp82828839 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp82162620;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp82162620 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp59935727;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp59935727 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp68183548;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp68183548 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp27339889;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp27339889 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp32519564;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp32519564 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp5705981;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp5705981 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp58492533;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp58492533 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp51700958;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp51700958 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp74118326;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp74118326 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp69947078;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp69947078 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp74792867;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp74792867 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp58964390;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp58964390 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp80984645;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp80984645 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp44401587;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp44401587 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp55720408;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp55720408 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp36295378;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp36295378 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp80665761;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp80665761 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp41745705;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp41745705 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp86599074;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp86599074 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp93485269;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp93485269 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp51638161;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp51638161 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp60972973;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp60972973 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp67712079;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp67712079 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp95983508;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp95983508 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp6129778;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp6129778 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp56650547;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp56650547 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp81576356;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp81576356 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp97899084;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp97899084 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp58662224;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp58662224 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp65577784;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp65577784 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp32573266;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp32573266 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp64880424;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp64880424 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp58143745;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp58143745 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp99006576;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp99006576 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp55678160;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp55678160 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp58375911;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp58375911 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp5775461;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp5775461 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp39636807;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp39636807 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp82258554;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp82258554 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp28250467;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp28250467 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp47842445;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp47842445 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp91655959;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp91655959 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp63731843;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp63731843 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp49311898;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp49311898 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp66533644;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp66533644 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp63363921;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp63363921 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp26611132;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp26611132 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp18120547;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp18120547 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp57998483;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp57998483 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp13417175;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp13417175 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp59280934;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp59280934 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp55173732;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp55173732 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp61811932;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp61811932 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp79110801;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp79110801 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp31190679;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp31190679 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp21189647;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp21189647 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp92223648;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp92223648 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp72200040;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp72200040 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp21210112;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp21210112 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp75869016;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp75869016 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp24129625;     hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp24129625 = hlHTVDGNJemRzMcLPhYtDbOUNkWONlInatBWlYserNHbgWPKPalDJRRFWmGkgsAYVyqGKGZqptBp47642457;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void RdOwEgwzCrVNXgfmMNHnabZDvJZ53339814() {     float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC14524597 = -867327566;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC28840318 = -17933427;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC14685748 = -450233106;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC10484612 = -797028564;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC16077828 = -973053435;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC30637377 = -248549017;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC97597026 = -382600334;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC35218956 = -802535679;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC28257655 = -325775307;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC25977142 = -137840005;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC63250971 = -888385320;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC52320451 = -56356204;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC76463242 = 36853722;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC18622484 = -419662697;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC96373738 = -490954604;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC60634301 = -939301862;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC74096344 = -123902632;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC66538842 = -467732875;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC83149334 = -44907592;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC69120286 = -140762870;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC64909207 = -291510525;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC69567598 = 46611191;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC59307930 = 58959307;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC47019673 = -955305960;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC17929584 = -594604500;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC50615292 = -635471941;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC6981232 = -787606016;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC34125752 = 97098112;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC26167147 = -991706951;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC30084864 = -973456178;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC77199083 = -293278019;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC73458845 = 80481725;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC86249088 = -373403650;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC34312411 = -14556805;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC79758194 = -694904438;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC64525995 = -671024079;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC97580730 = -250247433;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC23671414 = -280884780;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC16132593 = -359643790;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC53025783 = -526642698;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC15089244 = -840078156;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC54856782 = -918240409;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC34757389 = -894050893;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC75561342 = -257460157;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC73613027 = -728801533;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC78132493 = -898714121;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC31429611 = -607792510;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC88843098 = -138057052;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC50299692 = -951674768;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC65503258 = -737632126;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC18738657 = -208299716;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC6533492 = 71587573;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC48074853 = -452661831;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC15324197 = -464621011;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC88430607 = -786459568;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC44956999 = -813938757;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC69532388 = 23107266;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC67666075 = -494927146;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC92555027 = -102424064;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC65462535 = -237581494;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC23656146 = -460943002;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC63471275 = -379698446;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC9051810 = -810828729;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC98172790 = -352319130;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC48778058 = -844561986;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC89792126 = -868867046;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC66071363 = -682952554;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC42150832 = -948589473;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC38864290 = -724758260;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC31847744 = -819930526;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC63053570 = -589054429;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC50424931 = -843017853;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC50406249 = -8089086;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC30123551 = -518264895;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC54031043 = -300684715;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC10052426 = -373270117;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC34810210 = -59337916;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC83746588 = -683580536;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC73406645 = -126504428;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC39797091 = -695890380;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC19185682 = 72320568;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC18138133 = -549548964;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC83826059 = 48772879;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC60663889 = -154074825;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC11346208 = -665156463;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC70665592 = -264865592;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC25383992 = -466856444;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC70924891 = -908782640;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC45881804 = -228097238;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC34801195 = -880965681;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC94993607 = -594131345;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC29914656 = -755320288;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC31116387 = -78460717;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC50670057 = -22062297;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC29369638 = 34300303;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC51617968 = -360379710;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC45804973 = -7411681;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC36584598 = -441731764;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC26783284 = -412898171;    float QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC83820900 = -867327566;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC14524597 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC28840318;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC28840318 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC14685748;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC14685748 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC10484612;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC10484612 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC16077828;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC16077828 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC30637377;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC30637377 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC97597026;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC97597026 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC35218956;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC35218956 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC28257655;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC28257655 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC25977142;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC25977142 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC63250971;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC63250971 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC52320451;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC52320451 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC76463242;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC76463242 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC18622484;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC18622484 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC96373738;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC96373738 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC60634301;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC60634301 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC74096344;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC74096344 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC66538842;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC66538842 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC83149334;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC83149334 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC69120286;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC69120286 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC64909207;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC64909207 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC69567598;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC69567598 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC59307930;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC59307930 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC47019673;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC47019673 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC17929584;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC17929584 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC50615292;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC50615292 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC6981232;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC6981232 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC34125752;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC34125752 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC26167147;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC26167147 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC30084864;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC30084864 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC77199083;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC77199083 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC73458845;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC73458845 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC86249088;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC86249088 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC34312411;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC34312411 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC79758194;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC79758194 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC64525995;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC64525995 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC97580730;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC97580730 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC23671414;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC23671414 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC16132593;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC16132593 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC53025783;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC53025783 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC15089244;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC15089244 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC54856782;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC54856782 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC34757389;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC34757389 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC75561342;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC75561342 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC73613027;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC73613027 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC78132493;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC78132493 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC31429611;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC31429611 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC88843098;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC88843098 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC50299692;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC50299692 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC65503258;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC65503258 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC18738657;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC18738657 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC6533492;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC6533492 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC48074853;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC48074853 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC15324197;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC15324197 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC88430607;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC88430607 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC44956999;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC44956999 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC69532388;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC69532388 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC67666075;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC67666075 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC92555027;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC92555027 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC65462535;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC65462535 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC23656146;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC23656146 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC63471275;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC63471275 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC9051810;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC9051810 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC98172790;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC98172790 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC48778058;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC48778058 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC89792126;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC89792126 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC66071363;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC66071363 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC42150832;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC42150832 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC38864290;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC38864290 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC31847744;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC31847744 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC63053570;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC63053570 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC50424931;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC50424931 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC50406249;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC50406249 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC30123551;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC30123551 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC54031043;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC54031043 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC10052426;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC10052426 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC34810210;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC34810210 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC83746588;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC83746588 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC73406645;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC73406645 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC39797091;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC39797091 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC19185682;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC19185682 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC18138133;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC18138133 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC83826059;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC83826059 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC60663889;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC60663889 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC11346208;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC11346208 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC70665592;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC70665592 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC25383992;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC25383992 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC70924891;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC70924891 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC45881804;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC45881804 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC34801195;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC34801195 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC94993607;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC94993607 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC29914656;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC29914656 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC31116387;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC31116387 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC50670057;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC50670057 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC29369638;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC29369638 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC51617968;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC51617968 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC45804973;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC45804973 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC36584598;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC36584598 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC26783284;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC26783284 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC83820900;     QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC83820900 = QACmjQuhowhvjrCAOIHZByGcJEJOqELAvGgyAFmxdyqqfRXvYbLvojYRuZcfGfrklgzdKDAJVJwC14524597;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void jQEKMiAzFAiXHOSFrHQzTwlCRpl45173302() {     float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz88082267 = -451797730;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz5982105 = -477432035;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz9213378 = -294810022;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz16935159 = -818081882;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz52936092 = -557955010;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz79539047 = -419119402;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz394455 = -269130054;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz40873063 = -934496081;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz76079371 = -102419436;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz56057491 = -549704385;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz29459939 = -405865494;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz78858711 = -94519310;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz96632978 = -110360329;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz44356670 = -896420041;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz45646791 = -995524233;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz20332300 = 6647642;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz23515929 = -877822040;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz14747067 = -852030288;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz47360407 = -645957432;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz69959503 = -342276075;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz67810845 = -106397396;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz36389030 = -197620982;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz70128849 = -205305495;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz76947544 = -136996777;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz63276109 = -451155910;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz97896560 = -208267248;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz18846470 = -776818778;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz67834802 = -150680807;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz48814132 = -708551771;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz45887025 = -711571168;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz65583158 = 68111767;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz5379902 = -19181297;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz9661863 = -698389419;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz875241 = -450973599;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz58797535 = 38467317;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz15802565 = -921608887;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz90311213 = -944219274;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz28442888 = -945939902;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz46287881 = -577366466;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz90778845 = -502807366;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz38149176 = -293376201;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz31384353 = -594226108;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz38708019 = -838794240;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz16592020 = -91655004;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz25558046 = -415987245;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz19033420 = 73961709;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz43263917 = -151140483;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz49667315 = -280957737;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz46382237 = -743920;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz70206994 = -575698691;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz47880368 = 39306802;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz34901908 = -715390068;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz62536464 = -160370159;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz13408560 = -270429383;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz77362457 = -777344518;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz51693238 = -154176749;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz35853256 = -172126540;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz32265833 = -57813245;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz53659050 = -266925973;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz55039531 = -249687763;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz60692577 = -642300624;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz32559653 = -18449247;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz92058930 = -125944311;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz30192346 = -390848268;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz90474333 = -517816152;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz24080038 = -286684197;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz69196849 = -396129892;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz95757738 = -659386730;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz85559135 = -834887359;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz29844227 = 26084654;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz30021086 = -49133085;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz95073040 = -931882139;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz68459185 = -174663822;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz56581562 = -43150067;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz31810327 = 51100126;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz36426492 = -512171288;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz97681010 = -358826742;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz53536829 = -13650492;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz51389499 = -721009533;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz44242690 = -425117619;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz54632643 = 42873235;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz69179155 = -395861042;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz21452565 = -49936888;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz78607138 = -32853080;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz98006657 = -650877971;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz30681250 = -216498165;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz42843437 = -858811139;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz96253302 = -327960037;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz23512783 = -673629081;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz7104297 = -807355935;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz79949309 = -649482348;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz58045381 = -786406029;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz74783838 = -579013930;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz91248349 = -227678704;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz30086269 = -860506742;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz5589524 = -174926954;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz39325422 = -368281797;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz8515674 = -347945972;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz26117686 = -573838852;    float ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz1478008 = -451797730;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz88082267 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz5982105;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz5982105 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz9213378;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz9213378 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz16935159;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz16935159 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz52936092;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz52936092 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz79539047;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz79539047 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz394455;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz394455 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz40873063;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz40873063 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz76079371;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz76079371 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz56057491;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz56057491 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz29459939;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz29459939 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz78858711;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz78858711 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz96632978;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz96632978 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz44356670;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz44356670 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz45646791;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz45646791 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz20332300;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz20332300 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz23515929;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz23515929 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz14747067;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz14747067 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz47360407;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz47360407 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz69959503;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz69959503 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz67810845;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz67810845 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz36389030;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz36389030 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz70128849;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz70128849 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz76947544;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz76947544 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz63276109;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz63276109 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz97896560;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz97896560 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz18846470;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz18846470 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz67834802;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz67834802 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz48814132;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz48814132 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz45887025;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz45887025 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz65583158;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz65583158 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz5379902;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz5379902 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz9661863;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz9661863 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz875241;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz875241 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz58797535;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz58797535 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz15802565;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz15802565 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz90311213;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz90311213 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz28442888;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz28442888 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz46287881;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz46287881 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz90778845;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz90778845 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz38149176;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz38149176 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz31384353;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz31384353 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz38708019;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz38708019 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz16592020;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz16592020 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz25558046;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz25558046 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz19033420;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz19033420 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz43263917;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz43263917 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz49667315;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz49667315 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz46382237;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz46382237 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz70206994;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz70206994 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz47880368;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz47880368 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz34901908;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz34901908 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz62536464;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz62536464 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz13408560;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz13408560 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz77362457;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz77362457 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz51693238;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz51693238 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz35853256;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz35853256 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz32265833;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz32265833 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz53659050;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz53659050 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz55039531;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz55039531 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz60692577;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz60692577 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz32559653;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz32559653 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz92058930;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz92058930 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz30192346;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz30192346 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz90474333;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz90474333 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz24080038;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz24080038 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz69196849;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz69196849 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz95757738;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz95757738 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz85559135;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz85559135 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz29844227;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz29844227 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz30021086;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz30021086 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz95073040;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz95073040 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz68459185;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz68459185 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz56581562;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz56581562 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz31810327;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz31810327 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz36426492;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz36426492 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz97681010;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz97681010 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz53536829;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz53536829 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz51389499;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz51389499 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz44242690;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz44242690 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz54632643;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz54632643 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz69179155;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz69179155 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz21452565;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz21452565 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz78607138;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz78607138 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz98006657;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz98006657 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz30681250;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz30681250 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz42843437;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz42843437 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz96253302;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz96253302 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz23512783;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz23512783 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz7104297;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz7104297 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz79949309;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz79949309 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz58045381;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz58045381 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz74783838;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz74783838 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz91248349;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz91248349 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz30086269;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz30086269 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz5589524;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz5589524 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz39325422;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz39325422 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz8515674;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz8515674 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz26117686;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz26117686 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz1478008;     ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz1478008 = ZQQWteFAKlPDvnqaqoWjMLMkWbkomeBsXwaioeptCNHTcDULdWvOvFYODgASuQNgjWgzNHlLzdhz88082267;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void lnUgrSGRxsiJLkHTckpQppGctiu4298390() {     double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj97098907 = -502544794;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj96395970 = -817482250;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj41127484 = -905054013;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj95609061 = -660286271;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj46079625 = -248455431;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj64663589 = -852037682;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj13849289 = -385508;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj8658732 = -710690624;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj25418693 = -367394971;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj1040554 = -626255762;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj33850641 = -825353397;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj5965121 = 3196811;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj55037360 = -809586884;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj23231719 = -249540795;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj53114361 = -903518877;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj63571598 = -883214462;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj28771449 = 41283999;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj62715483 = -209755768;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj12121811 = -463570694;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj64635938 = -970350346;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj83799346 = -678558955;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj6631987 = -528489448;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj51475148 = -107202482;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj668363 = -929512198;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj16640586 = -331692517;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj55876000 = -569639362;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj91132282 = -572315681;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj42272278 = -568231900;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj18129333 = -999491393;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj67222597 = -484193329;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj72550463 = -122478934;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj75123975 = -991815734;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj45292647 = -70971166;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj97186496 = -952084205;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj3645330 = -279261179;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj18639979 = -286759002;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj32453513 = -402887155;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj84429766 = -493650605;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj6833732 = -445965418;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj92803439 = 55765421;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj94660717 = -615873161;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj97277051 = -964757098;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj32042029 = -336718373;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj97754035 = -530182261;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj77519523 = 70315881;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj5895675 = -92286094;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj74443386 = -178474818;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj28217843 = -590385014;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj65357448 = -913549951;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj99440318 = -904792072;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj57792803 = -687363237;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj46755797 = -616020056;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj45619223 = -724124040;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj27484770 = -267746254;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj20499044 = -14719614;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj90466921 = -974055347;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj44920823 = -610279768;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj40459122 = -975541816;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj78968476 = -228593755;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj90203624 = -678816070;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj73531306 = -179722001;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj71577011 = -432153608;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj90529398 = -711199232;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj58196095 = -883201643;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj28490090 = -403776829;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj58726666 = -833537664;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj60672474 = -925832024;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj57850863 = -857502679;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj19586390 = -970279616;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj34474382 = -516759876;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj31118085 = -380327308;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj44341683 = -465065396;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj55881752 = -763790351;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj19318371 = -419336115;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj69975221 = -254477185;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj86522295 = -713801858;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj74589957 = -91771076;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj53721112 = -577020222;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj23148839 = -899828080;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj10744912 = -139406423;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj81432613 = -291164545;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj62914439 = -981930667;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj76914829 = -654681949;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj18689014 = 5300678;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj9429795 = -796830093;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj25794667 = -506458879;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj29504753 = -167691694;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj17807878 = -803224912;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj76687452 = -837364592;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj13178408 = -305205833;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj73719156 = -676479234;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj91994390 = -427345339;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj5461291 = -165056851;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj16630107 = -767149348;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj19272134 = -764512578;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj23083706 = -83719553;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj6747653 = -153557867;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj73845933 = -453516730;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj69263945 = -26405432;    double RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj18792858 = -502544794;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj97098907 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj96395970;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj96395970 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj41127484;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj41127484 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj95609061;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj95609061 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj46079625;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj46079625 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj64663589;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj64663589 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj13849289;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj13849289 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj8658732;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj8658732 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj25418693;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj25418693 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj1040554;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj1040554 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj33850641;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj33850641 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj5965121;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj5965121 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj55037360;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj55037360 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj23231719;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj23231719 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj53114361;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj53114361 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj63571598;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj63571598 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj28771449;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj28771449 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj62715483;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj62715483 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj12121811;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj12121811 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj64635938;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj64635938 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj83799346;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj83799346 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj6631987;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj6631987 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj51475148;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj51475148 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj668363;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj668363 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj16640586;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj16640586 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj55876000;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj55876000 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj91132282;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj91132282 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj42272278;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj42272278 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj18129333;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj18129333 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj67222597;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj67222597 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj72550463;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj72550463 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj75123975;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj75123975 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj45292647;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj45292647 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj97186496;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj97186496 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj3645330;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj3645330 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj18639979;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj18639979 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj32453513;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj32453513 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj84429766;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj84429766 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj6833732;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj6833732 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj92803439;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj92803439 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj94660717;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj94660717 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj97277051;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj97277051 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj32042029;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj32042029 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj97754035;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj97754035 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj77519523;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj77519523 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj5895675;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj5895675 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj74443386;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj74443386 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj28217843;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj28217843 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj65357448;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj65357448 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj99440318;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj99440318 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj57792803;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj57792803 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj46755797;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj46755797 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj45619223;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj45619223 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj27484770;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj27484770 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj20499044;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj20499044 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj90466921;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj90466921 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj44920823;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj44920823 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj40459122;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj40459122 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj78968476;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj78968476 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj90203624;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj90203624 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj73531306;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj73531306 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj71577011;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj71577011 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj90529398;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj90529398 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj58196095;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj58196095 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj28490090;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj28490090 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj58726666;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj58726666 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj60672474;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj60672474 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj57850863;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj57850863 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj19586390;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj19586390 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj34474382;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj34474382 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj31118085;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj31118085 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj44341683;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj44341683 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj55881752;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj55881752 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj19318371;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj19318371 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj69975221;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj69975221 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj86522295;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj86522295 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj74589957;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj74589957 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj53721112;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj53721112 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj23148839;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj23148839 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj10744912;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj10744912 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj81432613;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj81432613 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj62914439;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj62914439 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj76914829;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj76914829 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj18689014;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj18689014 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj9429795;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj9429795 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj25794667;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj25794667 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj29504753;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj29504753 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj17807878;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj17807878 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj76687452;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj76687452 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj13178408;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj13178408 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj73719156;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj73719156 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj91994390;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj91994390 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj5461291;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj5461291 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj16630107;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj16630107 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj19272134;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj19272134 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj23083706;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj23083706 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj6747653;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj6747653 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj73845933;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj73845933 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj69263945;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj69263945 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj18792858;     RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj18792858 = RBkQTksRcNdUiZZjtEQKCulPKzhyvhrnJYpPNcOYSSfFzQGrPGMXcXsWPUGUDuWuVpKKjJVRqrsj97098907;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void nObikZWfblTzDIMvASBqoecRClM96131877() {     double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs70656579 = -87014959;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs73537757 = -176980858;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs35655114 = -749630929;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs2059610 = -681339590;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs82937888 = -933357007;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs13565259 = 77391934;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs16646717 = -986915228;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs14312839 = -842651027;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs73240409 = -144039100;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs31120903 = 61879858;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs59609 = -342833571;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs32503382 = -34966295;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs75207096 = -956800934;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs48965905 = -726298139;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs2387414 = -308088506;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs23269597 = 62735042;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs78191033 = -712635409;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs10923709 = -594053181;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs76332883 = 35379466;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs65475155 = -71863550;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs86700983 = -493445826;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs73453418 = -772721620;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs62296066 = -371467284;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs30596234 = -111203014;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs61987111 = -188243926;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs3157268 = -142434669;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs2997522 = -561528443;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs75981328 = -816010819;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs40776318 = -716336213;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs83024758 = -222308319;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs60934537 = -861089147;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs7045032 = 8521244;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs68705421 = -395956934;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs63749326 = -288500998;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs82684670 = -645889424;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs69916549 = -537343810;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs25183996 = 3141005;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs89201240 = -58705727;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs36989020 = -663688094;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs30556502 = 79600753;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs17720650 = -69171207;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs73804622 = -640742797;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs35992660 = -281461719;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs38784713 = -364377107;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs29464542 = -716869831;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs46796600 = -219610265;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs86277692 = -821822791;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs89042059 = -733285698;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs61439993 = 37380897;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs4144055 = -742858637;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs86934514 = -439756718;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs75124213 = -302997697;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs60080833 = -431832368;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs25569133 = -73554626;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs9430895 = -5604564;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs97203161 = -314293339;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs11241691 = -805513574;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs5058880 = -538427916;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs40072498 = -393095664;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs79780620 = -690922339;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs10567738 = -361079624;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs40665389 = -70904410;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs73536520 = -26314814;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs90215650 = -921730781;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs70186365 = -77030995;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs93014577 = -251354816;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs63797960 = -639009361;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs11457770 = -568299936;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs66281235 = 19591285;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs32470865 = -770744696;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs98085600 = -940405964;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs88989792 = -553929682;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs73934688 = -930365088;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs45776381 = 55778713;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs47754505 = 97307656;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs12896362 = -852703029;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs37460759 = -391259901;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs23511354 = 92909822;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs1131693 = -394333184;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs15190511 = -968633662;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs16879576 = -320611878;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs13955462 = -828242745;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs14541335 = -753391717;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs36632264 = -973477576;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs96090244 = -782551602;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs85810324 = -458091451;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs46964198 = -559646389;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs43136289 = -222402309;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs54318432 = -182896435;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs85481509 = -231596086;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs58674858 = -731830236;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs20125116 = -458431080;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs49128742 = -665610064;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs57208399 = -972765755;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs19988764 = -559319624;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs77055261 = -998266798;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs268103 = -514427984;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs45777009 = -359730939;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs68598348 = -187346113;    double ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs36449965 = -87014959;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs70656579 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs73537757;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs73537757 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs35655114;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs35655114 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs2059610;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs2059610 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs82937888;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs82937888 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs13565259;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs13565259 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs16646717;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs16646717 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs14312839;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs14312839 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs73240409;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs73240409 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs31120903;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs31120903 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs59609;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs59609 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs32503382;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs32503382 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs75207096;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs75207096 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs48965905;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs48965905 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs2387414;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs2387414 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs23269597;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs23269597 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs78191033;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs78191033 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs10923709;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs10923709 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs76332883;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs76332883 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs65475155;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs65475155 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs86700983;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs86700983 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs73453418;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs73453418 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs62296066;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs62296066 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs30596234;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs30596234 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs61987111;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs61987111 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs3157268;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs3157268 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs2997522;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs2997522 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs75981328;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs75981328 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs40776318;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs40776318 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs83024758;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs83024758 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs60934537;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs60934537 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs7045032;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs7045032 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs68705421;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs68705421 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs63749326;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs63749326 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs82684670;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs82684670 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs69916549;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs69916549 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs25183996;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs25183996 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs89201240;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs89201240 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs36989020;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs36989020 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs30556502;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs30556502 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs17720650;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs17720650 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs73804622;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs73804622 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs35992660;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs35992660 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs38784713;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs38784713 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs29464542;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs29464542 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs46796600;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs46796600 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs86277692;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs86277692 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs89042059;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs89042059 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs61439993;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs61439993 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs4144055;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs4144055 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs86934514;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs86934514 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs75124213;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs75124213 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs60080833;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs60080833 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs25569133;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs25569133 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs9430895;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs9430895 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs97203161;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs97203161 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs11241691;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs11241691 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs5058880;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs5058880 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs40072498;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs40072498 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs79780620;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs79780620 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs10567738;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs10567738 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs40665389;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs40665389 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs73536520;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs73536520 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs90215650;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs90215650 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs70186365;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs70186365 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs93014577;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs93014577 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs63797960;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs63797960 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs11457770;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs11457770 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs66281235;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs66281235 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs32470865;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs32470865 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs98085600;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs98085600 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs88989792;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs88989792 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs73934688;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs73934688 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs45776381;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs45776381 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs47754505;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs47754505 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs12896362;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs12896362 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs37460759;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs37460759 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs23511354;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs23511354 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs1131693;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs1131693 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs15190511;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs15190511 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs16879576;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs16879576 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs13955462;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs13955462 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs14541335;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs14541335 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs36632264;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs36632264 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs96090244;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs96090244 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs85810324;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs85810324 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs46964198;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs46964198 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs43136289;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs43136289 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs54318432;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs54318432 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs85481509;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs85481509 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs58674858;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs58674858 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs20125116;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs20125116 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs49128742;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs49128742 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs57208399;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs57208399 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs19988764;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs19988764 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs77055261;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs77055261 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs268103;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs268103 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs45777009;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs45777009 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs68598348;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs68598348 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs36449965;     ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs36449965 = ZoCDOCqosuWobvukBDwhNBzTGlSvJmJfyNwqZiFwNUTBeeZvttaOFEyXpjxckveGkqpOyELSyATs70656579;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void xPgSwVZJKFJDyQoxCGWeirpVjxt33661059() {     long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY63403208 = -615259948;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY22977402 = -278087799;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY54842126 = -171053998;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY92801604 = -707885078;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY46802656 = -553450298;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY75223886 = -759414203;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY98434778 = 64851647;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY56224539 = -435122838;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY90059094 = -292851262;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY25570039 = -600905664;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY27018742 = -643134661;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY35529885 = -130911082;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY52812415 = -377201258;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY68369879 = -83948703;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY68862133 = -179067603;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY63758377 = -418459063;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY27459205 = -132794663;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY49968862 = -265558614;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY61642497 = -52900767;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY62185471 = -804206287;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY90359570 = -929607533;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY66402180 = -506753489;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY75939834 = -943801166;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY11809639 = -562030566;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY23510991 = -820417442;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY97555387 = 61432118;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY26653692 = -500101056;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY57614479 = -411036412;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY69331213 = -885401421;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY63818788 = -992105481;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY15853588 = -883684634;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY16858537 = -21488654;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY33008485 = -231808556;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY95502459 = -217026520;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY12777752 = -438594603;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY99787006 = -901124655;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY55148518 = -776214794;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY25652230 = -610296968;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY22836993 = -125164511;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY99897319 = -799041654;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY51144044 = 46235605;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY83339386 = -423507374;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY1843455 = 27340148;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY7910350 = -824883653;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY25395218 = -83321381;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY37497768 = -188845089;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY22938338 = -389522410;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY30950854 = -196073518;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY95631028 = 88554575;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY49205287 = 83057433;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY23678411 = -653644151;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY23936565 = -673534722;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY13097647 = -15464607;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY75327677 = -115660834;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY43301488 = -902807327;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY97001028 = -8506460;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY47037568 = -334286633;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY43032488 = -609023433;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY69290613 = -887467636;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY49247268 = -514882416;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY48570195 = -159313148;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY40820299 = -524111942;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY86893325 = -549721418;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY26240307 = -300745781;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY9716452 = -717221031;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY10160205 = -521646007;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY2521400 = -899102526;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY57309956 = -60174739;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY55592127 = -645354101;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY69075126 = -277942948;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY8609860 = -642244269;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY1806975 = -522497695;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY27131870 = -40394103;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY61745177 = -253859113;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY11041428 = -750441893;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY7020185 = -406100159;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY64558725 = -434093638;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY68029484 = -18917513;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY86414420 = -378709185;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY86013222 = -531572354;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY74617049 = -549045472;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY95702837 = -204027539;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY61983450 = -399590988;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY20125927 = -868458854;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY40140377 = -238461330;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY91917023 = -110149913;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY3760890 = 93975953;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY57680808 = -16147723;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY52200971 = -314219194;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY15776724 = -330088144;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY52749439 = -466838022;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY12116031 = -67191362;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY56361616 = -722829333;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY73589725 = -610282095;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY51327125 = -539728507;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY10323745 = -429652454;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY96446060 = -873785957;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY75603148 = -671914071;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY98193898 = -7662623;    long heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY15235013 = -615259948;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY63403208 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY22977402;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY22977402 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY54842126;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY54842126 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY92801604;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY92801604 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY46802656;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY46802656 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY75223886;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY75223886 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY98434778;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY98434778 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY56224539;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY56224539 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY90059094;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY90059094 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY25570039;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY25570039 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY27018742;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY27018742 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY35529885;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY35529885 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY52812415;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY52812415 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY68369879;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY68369879 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY68862133;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY68862133 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY63758377;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY63758377 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY27459205;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY27459205 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY49968862;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY49968862 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY61642497;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY61642497 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY62185471;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY62185471 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY90359570;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY90359570 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY66402180;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY66402180 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY75939834;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY75939834 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY11809639;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY11809639 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY23510991;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY23510991 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY97555387;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY97555387 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY26653692;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY26653692 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY57614479;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY57614479 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY69331213;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY69331213 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY63818788;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY63818788 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY15853588;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY15853588 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY16858537;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY16858537 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY33008485;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY33008485 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY95502459;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY95502459 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY12777752;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY12777752 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY99787006;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY99787006 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY55148518;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY55148518 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY25652230;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY25652230 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY22836993;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY22836993 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY99897319;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY99897319 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY51144044;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY51144044 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY83339386;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY83339386 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY1843455;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY1843455 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY7910350;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY7910350 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY25395218;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY25395218 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY37497768;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY37497768 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY22938338;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY22938338 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY30950854;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY30950854 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY95631028;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY95631028 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY49205287;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY49205287 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY23678411;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY23678411 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY23936565;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY23936565 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY13097647;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY13097647 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY75327677;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY75327677 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY43301488;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY43301488 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY97001028;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY97001028 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY47037568;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY47037568 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY43032488;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY43032488 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY69290613;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY69290613 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY49247268;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY49247268 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY48570195;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY48570195 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY40820299;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY40820299 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY86893325;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY86893325 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY26240307;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY26240307 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY9716452;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY9716452 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY10160205;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY10160205 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY2521400;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY2521400 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY57309956;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY57309956 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY55592127;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY55592127 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY69075126;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY69075126 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY8609860;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY8609860 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY1806975;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY1806975 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY27131870;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY27131870 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY61745177;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY61745177 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY11041428;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY11041428 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY7020185;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY7020185 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY64558725;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY64558725 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY68029484;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY68029484 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY86414420;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY86414420 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY86013222;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY86013222 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY74617049;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY74617049 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY95702837;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY95702837 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY61983450;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY61983450 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY20125927;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY20125927 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY40140377;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY40140377 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY91917023;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY91917023 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY3760890;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY3760890 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY57680808;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY57680808 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY52200971;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY52200971 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY15776724;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY15776724 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY52749439;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY52749439 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY12116031;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY12116031 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY56361616;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY56361616 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY73589725;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY73589725 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY51327125;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY51327125 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY10323745;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY10323745 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY96446060;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY96446060 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY75603148;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY75603148 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY98193898;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY98193898 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY15235013;     heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY15235013 = heRyBZNzbQzdIjQjkceGpBqcNIMkFZKizbrfsHIdlQNhDcNvRIFsEqEfopksMdtBFQdgrlqdUBVY63403208;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void OqdQNLGdSZAhtimJnZkgaCRaKoK25494547() {     long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy36960880 = -199730113;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy119189 = -737586407;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy49369756 = -15630914;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy99252151 = -728938396;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy83660919 = -138351874;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy24125557 = -929984588;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy1232206 = -921678073;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy61878646 = -567083241;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy37880811 = -69495390;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy55650389 = 87229956;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy93227709 = -160614835;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy62068145 = -169074189;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy72982151 = -524415309;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy94104064 = -560706047;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy18135186 = -683637231;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy23456375 = -572509559;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy76878789 = -886714071;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy98177086 = -649856026;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy25853570 = -653950607;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy63024688 = 94280509;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy93261208 = -744494403;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy33223612 = -750985661;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy86760752 = -108065968;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy41737510 = -843721382;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy68857516 = -676968851;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy44836656 = -611363189;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy38518930 = -489313818;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy91323529 = -658815331;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy91978198 = -602246241;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy79620949 = -730220471;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy4237662 = -522294848;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy48779592 = -121151677;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy56421259 = -556794325;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy62065289 = -653443313;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy91817092 = -805222849;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy51063577 = -51709464;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy47879001 = -370186635;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy30423704 = -175352090;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy52992281 = -342887186;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy37650382 = -775206321;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy74203976 = -507062440;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy59866957 = -99493074;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy5794086 = 82596802;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy48941027 = -659078500;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy77340237 = -870507094;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy78398694 = -316169259;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy34772644 = 67129616;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy91775070 = -338974202;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy91713573 = -60514576;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy53909022 = -855009132;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy52820122 = -406037633;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy52304981 = -360512363;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy27559257 = -823172935;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy73412039 = 78530795;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy32233338 = -893692277;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy3737269 = -448744452;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy13358436 = -529520439;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy7632246 = -171909532;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy30394636 = 48030455;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy38824264 = -526988685;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy85606626 = -340670770;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy9908677 = -162862743;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy69900447 = -964837001;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy58259862 = -339274920;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy51412727 = -390475197;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy44448117 = 60536841;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy5646887 = -612279864;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy10916863 = -870971996;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy2286972 = -755483199;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy67071609 = -531927768;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy75577374 = -102322925;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy46455085 = -611361981;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy45184806 = -206968840;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy88203188 = -878744286;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy88820711 = -398657052;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy33394251 = -545001330;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy27429526 = -733582463;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy37819726 = -448987469;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy64397273 = -973214289;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy90458821 = -260799593;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy10064012 = -578492805;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy46743859 = -50339617;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy99609955 = -498300755;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy38069177 = -747237109;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy26800827 = -224182839;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy51932681 = -61782485;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy21220335 = -297978742;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy83009219 = -535325120;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy29831951 = -759751037;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy88079824 = -256478397;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy37705141 = -522189025;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy40246755 = -98277103;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy29069 = -123382546;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy14168018 = -815898502;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy52043755 = -334535552;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy64295300 = -244199698;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy89966510 = -134656073;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy47534224 = -578128279;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy97528300 = -168603304;    long HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy32892120 = -199730113;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy36960880 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy119189;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy119189 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy49369756;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy49369756 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy99252151;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy99252151 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy83660919;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy83660919 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy24125557;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy24125557 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy1232206;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy1232206 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy61878646;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy61878646 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy37880811;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy37880811 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy55650389;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy55650389 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy93227709;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy93227709 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy62068145;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy62068145 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy72982151;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy72982151 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy94104064;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy94104064 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy18135186;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy18135186 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy23456375;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy23456375 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy76878789;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy76878789 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy98177086;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy98177086 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy25853570;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy25853570 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy63024688;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy63024688 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy93261208;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy93261208 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy33223612;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy33223612 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy86760752;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy86760752 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy41737510;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy41737510 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy68857516;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy68857516 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy44836656;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy44836656 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy38518930;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy38518930 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy91323529;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy91323529 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy91978198;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy91978198 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy79620949;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy79620949 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy4237662;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy4237662 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy48779592;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy48779592 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy56421259;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy56421259 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy62065289;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy62065289 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy91817092;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy91817092 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy51063577;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy51063577 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy47879001;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy47879001 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy30423704;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy30423704 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy52992281;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy52992281 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy37650382;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy37650382 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy74203976;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy74203976 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy59866957;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy59866957 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy5794086;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy5794086 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy48941027;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy48941027 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy77340237;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy77340237 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy78398694;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy78398694 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy34772644;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy34772644 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy91775070;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy91775070 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy91713573;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy91713573 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy53909022;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy53909022 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy52820122;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy52820122 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy52304981;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy52304981 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy27559257;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy27559257 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy73412039;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy73412039 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy32233338;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy32233338 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy3737269;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy3737269 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy13358436;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy13358436 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy7632246;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy7632246 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy30394636;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy30394636 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy38824264;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy38824264 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy85606626;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy85606626 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy9908677;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy9908677 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy69900447;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy69900447 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy58259862;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy58259862 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy51412727;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy51412727 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy44448117;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy44448117 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy5646887;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy5646887 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy10916863;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy10916863 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy2286972;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy2286972 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy67071609;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy67071609 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy75577374;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy75577374 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy46455085;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy46455085 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy45184806;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy45184806 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy88203188;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy88203188 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy88820711;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy88820711 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy33394251;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy33394251 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy27429526;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy27429526 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy37819726;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy37819726 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy64397273;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy64397273 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy90458821;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy90458821 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy10064012;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy10064012 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy46743859;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy46743859 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy99609955;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy99609955 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy38069177;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy38069177 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy26800827;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy26800827 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy51932681;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy51932681 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy21220335;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy21220335 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy83009219;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy83009219 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy29831951;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy29831951 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy88079824;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy88079824 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy37705141;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy37705141 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy40246755;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy40246755 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy29069;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy29069 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy14168018;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy14168018 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy52043755;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy52043755 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy64295300;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy64295300 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy89966510;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy89966510 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy47534224;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy47534224 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy97528300;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy97528300 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy32892120;     HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy32892120 = HmKOihPtGEidOcOJRQNznrCGoHkMLYCFYBzrIKRAdpfFFwWDLtAKwpWPkDaawDhhBcENdXBGyNVy36960880;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void buWWWkbuwHUzjBtOgwjqLEWFLjz17328035() {     long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc10518552 = -884200277;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc77260975 = -97085015;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc43897386 = -960207830;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc5702700 = -749991714;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc20519183 = -823253449;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc73027226 = -554972;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc4029634 = -808207793;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc67532753 = -699043643;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc85702527 = -946139519;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc85730738 = -324634424;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc59436676 = -778095009;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc88606406 = -207237295;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc93151887 = -671629359;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc19838251 = 62536608;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc67408239 = -88206860;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc83154373 = -726560056;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc26298373 = -540633479;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc46385312 = 65846562;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc90064643 = -155000447;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc63863904 = -107232696;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc96162846 = -559381274;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc45044 = -995217834;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc97581671 = -372330771;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc71665382 = -25412199;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc14204042 = -533520261;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc92117923 = -184158496;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc50384168 = -478526580;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc25032580 = -906594250;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc14625185 = -319091061;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc95423110 = -468335461;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc92621735 = -160905062;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc80700648 = -220814699;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc79834033 = -881780094;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc28628119 = 10139894;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc70856433 = -71851094;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc2340147 = -302294272;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc40609484 = 35841525;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc35195179 = -840407212;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc83147569 = -560609862;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc75403444 = -751370989;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc97263909 = 39639514;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc36394528 = -875478773;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc9744717 = -962146545;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc89971705 = -493273346;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc29285256 = -557692806;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc19299621 = -443493430;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc46606950 = -576218358;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc52599287 = -481874886;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc87796118 = -209583728;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc58612758 = -693075698;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc81961833 = -158431115;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc80673397 = -47490004;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc42020868 = -530881262;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc71496402 = -827277577;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc21165189 = -884577227;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc10473508 = -888982444;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc79679303 = -724754245;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc72232003 = -834795632;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc91498658 = -116471454;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc28401260 = -539094954;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc22643058 = -522028393;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc78997054 = -901613544;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc52907568 = -279952583;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc90279417 = -377804058;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc93109002 = -63729362;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc78736028 = -457280311;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc8772374 = -325457202;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc64523769 = -581769253;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc48981817 = -865612298;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc65068092 = -785912588;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc42544890 = -662401581;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc91103194 = -700226267;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc63237742 = -373543577;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc14661199 = -403629458;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc66599995 = -46872211;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc59768318 = -683902502;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc90300326 = 66928711;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc7609967 = -879057425;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc42380127 = -467719393;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc94904420 = 9973169;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc45510974 = -607940139;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc97784881 = -996651695;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc37236461 = -597010523;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc56012426 = -626015364;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc13461277 = -209904347;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc11948339 = -13415058;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc38679781 = -689933437;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc8337631 = 45497483;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc7462931 = -105282880;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc60382926 = -182868650;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc22660844 = -577540028;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc68377480 = -129362844;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc43696520 = -623935759;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc54746310 = 78485091;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc52760386 = -129342597;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc18266856 = -58746942;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc83486960 = -495526190;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc19465300 = -484342488;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc96862702 = -329543984;    long bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc50549227 = -884200277;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc10518552 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc77260975;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc77260975 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc43897386;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc43897386 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc5702700;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc5702700 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc20519183;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc20519183 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc73027226;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc73027226 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc4029634;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc4029634 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc67532753;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc67532753 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc85702527;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc85702527 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc85730738;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc85730738 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc59436676;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc59436676 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc88606406;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc88606406 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc93151887;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc93151887 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc19838251;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc19838251 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc67408239;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc67408239 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc83154373;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc83154373 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc26298373;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc26298373 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc46385312;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc46385312 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc90064643;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc90064643 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc63863904;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc63863904 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc96162846;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc96162846 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc45044;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc45044 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc97581671;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc97581671 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc71665382;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc71665382 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc14204042;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc14204042 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc92117923;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc92117923 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc50384168;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc50384168 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc25032580;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc25032580 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc14625185;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc14625185 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc95423110;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc95423110 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc92621735;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc92621735 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc80700648;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc80700648 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc79834033;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc79834033 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc28628119;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc28628119 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc70856433;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc70856433 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc2340147;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc2340147 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc40609484;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc40609484 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc35195179;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc35195179 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc83147569;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc83147569 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc75403444;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc75403444 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc97263909;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc97263909 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc36394528;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc36394528 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc9744717;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc9744717 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc89971705;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc89971705 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc29285256;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc29285256 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc19299621;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc19299621 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc46606950;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc46606950 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc52599287;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc52599287 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc87796118;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc87796118 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc58612758;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc58612758 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc81961833;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc81961833 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc80673397;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc80673397 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc42020868;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc42020868 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc71496402;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc71496402 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc21165189;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc21165189 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc10473508;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc10473508 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc79679303;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc79679303 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc72232003;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc72232003 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc91498658;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc91498658 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc28401260;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc28401260 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc22643058;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc22643058 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc78997054;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc78997054 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc52907568;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc52907568 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc90279417;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc90279417 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc93109002;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc93109002 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc78736028;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc78736028 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc8772374;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc8772374 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc64523769;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc64523769 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc48981817;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc48981817 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc65068092;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc65068092 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc42544890;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc42544890 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc91103194;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc91103194 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc63237742;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc63237742 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc14661199;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc14661199 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc66599995;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc66599995 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc59768318;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc59768318 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc90300326;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc90300326 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc7609967;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc7609967 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc42380127;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc42380127 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc94904420;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc94904420 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc45510974;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc45510974 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc97784881;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc97784881 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc37236461;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc37236461 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc56012426;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc56012426 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc13461277;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc13461277 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc11948339;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc11948339 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc38679781;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc38679781 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc8337631;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc8337631 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc7462931;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc7462931 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc60382926;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc60382926 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc22660844;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc22660844 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc68377480;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc68377480 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc43696520;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc43696520 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc54746310;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc54746310 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc52760386;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc52760386 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc18266856;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc18266856 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc83486960;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc83486960 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc19465300;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc19465300 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc96862702;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc96862702 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc50549227;     bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc50549227 = bdpEZpeoXvwqOMIYkDPCpGhfZcOxWYQKnlARGCuJENWTkFVeCflCwgJVwKfwbhRyelCdMrShJFxc10518552;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void fAasaDtOPWWvTvoiRdGDZjvDPGpbSegLheAljpKeNicHkVllptzJwDSvEWdUNRvkqLOovDzbSbpqHoStUEmZeWPdMEgMMqRvmrRZ44389459() {     float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs90469306 = -536190788;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs47788206 = -91750630;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs97301415 = -458964437;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs66123660 = -133146171;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs4133000 = -654063257;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs43583932 = -609587013;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs17430941 = -417573549;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs86546416 = -503263174;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs31226650 = -153662693;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs68920440 = -88542620;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs89389335 = -902918191;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs81119919 = -499066810;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs48393275 = -770734738;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs20384476 = -580428335;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs78334569 = 3730596;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs57064906 = -606056247;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs3593867 = -26440121;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs86372779 = -96056001;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs54856031 = -160784885;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs66030369 = -58367970;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs39151513 = -69926276;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs31390385 = -774379180;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs27782739 = -52097520;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs73846819 = 62724478;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs62497696 = 54707368;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs80058514 = -152169403;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs17957175 = -234592482;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs92418092 = -141272221;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs45580697 = -693044501;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs87600829 = -21393529;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs53223808 = -487723651;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs64281116 = -859837561;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs26269782 = -594693255;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs83729134 = -378654009;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs41426041 = -743495279;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs1228775 = -447160632;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs1721997 = -553084991;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs18818664 = -365272380;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs16190552 = -699257826;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs76288871 = -649744080;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs51039496 = -151395528;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs16773328 = 49379861;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs81137082 = -368606614;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs53316651 = 51945686;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs71234519 = -191263499;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs97014611 = -607476820;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs47232265 = -368323858;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs28143487 = -152451960;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs55910688 = -990453867;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs86791704 = -836660494;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs20267292 = -587266750;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs23130088 = 29939686;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs23725053 = -326591231;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs91930628 = -427811820;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs5237947 = -322684420;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs59078921 = -761811608;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs20005468 = 60346890;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs23454597 = -421688916;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs3625964 = -87853539;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs24074485 = -401893855;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs25626757 = -274994532;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs25012848 = -176301328;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs40965720 = -810218674;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs43625820 = -32269165;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs15696633 = -600818970;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs25108220 = 56919369;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs54850138 = -904373555;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs64664140 = -292080730;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs78958435 = -836933056;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs77105794 = -549108772;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs55342909 = 47028743;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs84775202 = -661167741;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs70182228 = -396798176;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs78567160 = -511040806;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs14990873 = -906972443;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs22378185 = -19306138;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs50253302 = -305772566;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs74466087 = -4043207;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs2612300 = -746012023;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs65483085 = -337815812;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs32826249 = -783845545;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs89813688 = 17859477;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs36507405 = -150818355;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs58788992 = -856384007;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs67333537 = -434126779;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs30093721 = -417663337;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs40556063 = -433246330;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs34339154 = -66881436;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs78491188 = 44030411;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs82347119 = -981683672;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs81223306 = -407507523;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs78267400 = -31396075;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs15192701 = -177418841;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs92116066 = -197363972;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs50662114 = -274749549;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs26026648 = -975094200;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs75807608 = -140401466;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs37511262 = -236337450;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs37620019 = -347235345;    float HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs46126300 = -536190788;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs90469306 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs47788206;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs47788206 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs97301415;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs97301415 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs66123660;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs66123660 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs4133000;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs4133000 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs43583932;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs43583932 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs17430941;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs17430941 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs86546416;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs86546416 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs31226650;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs31226650 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs68920440;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs68920440 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs89389335;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs89389335 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs81119919;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs81119919 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs48393275;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs48393275 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs20384476;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs20384476 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs78334569;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs78334569 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs57064906;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs57064906 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs3593867;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs3593867 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs86372779;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs86372779 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs54856031;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs54856031 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs66030369;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs66030369 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs39151513;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs39151513 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs31390385;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs31390385 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs27782739;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs27782739 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs73846819;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs73846819 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs62497696;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs62497696 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs80058514;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs80058514 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs17957175;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs17957175 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs92418092;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs92418092 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs45580697;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs45580697 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs87600829;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs87600829 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs53223808;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs53223808 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs64281116;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs64281116 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs26269782;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs26269782 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs83729134;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs83729134 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs41426041;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs41426041 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs1228775;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs1228775 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs1721997;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs1721997 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs18818664;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs18818664 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs16190552;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs16190552 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs76288871;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs76288871 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs51039496;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs51039496 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs16773328;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs16773328 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs81137082;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs81137082 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs53316651;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs53316651 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs71234519;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs71234519 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs97014611;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs97014611 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs47232265;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs47232265 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs28143487;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs28143487 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs55910688;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs55910688 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs86791704;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs86791704 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs20267292;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs20267292 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs23130088;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs23130088 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs23725053;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs23725053 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs91930628;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs91930628 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs5237947;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs5237947 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs59078921;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs59078921 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs20005468;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs20005468 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs23454597;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs23454597 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs3625964;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs3625964 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs24074485;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs24074485 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs25626757;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs25626757 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs25012848;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs25012848 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs40965720;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs40965720 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs43625820;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs43625820 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs15696633;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs15696633 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs25108220;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs25108220 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs54850138;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs54850138 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs64664140;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs64664140 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs78958435;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs78958435 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs77105794;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs77105794 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs55342909;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs55342909 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs84775202;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs84775202 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs70182228;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs70182228 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs78567160;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs78567160 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs14990873;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs14990873 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs22378185;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs22378185 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs50253302;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs50253302 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs74466087;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs74466087 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs2612300;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs2612300 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs65483085;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs65483085 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs32826249;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs32826249 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs89813688;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs89813688 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs36507405;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs36507405 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs58788992;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs58788992 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs67333537;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs67333537 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs30093721;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs30093721 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs40556063;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs40556063 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs34339154;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs34339154 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs78491188;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs78491188 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs82347119;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs82347119 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs81223306;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs81223306 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs78267400;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs78267400 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs15192701;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs15192701 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs92116066;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs92116066 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs50662114;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs50662114 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs26026648;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs26026648 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs75807608;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs75807608 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs37511262;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs37511262 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs37620019;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs37620019 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs46126300;     HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs46126300 = HwpluYqUNboQDjzJeWGTvkcqXmOcKuEPWtmYOBfGgvPlQPIcrIQQlydWTzNHJNKTWYuKxaudrznAUraHdllCdoeXPPFPoXwSlvOs90469306;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void OLomAWzlcOCWTeAWfwDylSYdSVdeCePYRpXawbkbiWlcNfIeSQxtOllwnLUesUixzdFdGZZDLcQPgvQnNzMmhVKgDbnlHGiNnndj69261545() {     float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq73907140 = -189380610;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq3232612 = -133727943;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq95023134 = -209618279;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq13995773 = -147486837;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq19094425 = -929286070;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq76893765 = -88091478;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq57017594 = -962021329;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq70107909 = -194598521;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq72496514 = -208768114;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq98105605 = -193725603;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq59125878 = 47493864;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq58616996 = -222163419;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq98363965 = -584054453;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq53855588 = -713871743;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq84361141 = -53005238;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq18018615 = -726931223;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq53198511 = -699399718;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq90224759 = -740432500;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq37724443 = -777442022;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq27471575 = -99978414;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq74461324 = -469921681;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq21833969 = -175522833;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq68486842 = -550944560;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq52203486 = -990021440;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq32516344 = -931639244;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq91974740 = 59114953;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq37633497 = -163476538;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq67553243 = -820194093;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq94340237 = -101619958;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq79524040 = -209674754;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq4731801 = -512574086;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq12111401 = -66854402;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq55261092 = -417509648;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq26170483 = -946937911;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq2510809 = -100473939;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq56446148 = -314950285;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq82277543 = -531587549;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq29315176 = -69005579;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq499227 = -496837040;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq97656899 = -11769288;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq72544088 = -910888400;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq52958774 = -718320543;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq64987512 = -378794111;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq4453490 = -361201529;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq47197069 = -392679854;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq77048576 = -439132415;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq50945488 = -981908710;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq23197663 = -759935035;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq5416190 = -39819810;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq42169611 = -630705836;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq6784110 = -386723180;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq93178139 = -410465374;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq13285861 = -430392556;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq26857658 = -678145058;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq94800221 = -428069821;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq52073171 = 86142223;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq34745769 = -582783384;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq42819649 = -219596839;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq81479428 = -215847593;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq27119685 = -888401024;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq39260268 = -924614941;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq89464351 = -41827236;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq75767672 = 7021437;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq92972474 = -999093361;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq93373805 = -681151518;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq47014478 = -885651734;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq3355904 = -804653771;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq72193482 = -637116543;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq51344779 = -513397805;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq27914993 = -738054954;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq35741072 = -95343674;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq23883335 = -530394139;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq89725532 = -143595460;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq40067544 = -665672735;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq54927486 = -189090015;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq21502550 = -751601139;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq56846456 = -796728723;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq64033353 = -89743032;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq5006418 = -497341587;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq55467768 = -392506830;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq41029252 = 41023662;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq14435834 = -403541504;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq62137053 = -680374283;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq52170626 = -470914123;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq72739930 = -822951575;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq11553661 = -2108713;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq98825539 = -636461846;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq28403434 = -739364591;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq31370261 = -418868091;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq50437637 = -86616163;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq21700379 = -732166901;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq39457894 = -211990710;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq47835748 = -853157987;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq73379541 = -608436017;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq58396631 = -87154347;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq83079736 = -769061164;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq77191102 = -625341980;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq72015038 = -379700751;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq11079685 = -680050012;    float aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq182591 = -189380610;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq73907140 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq3232612;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq3232612 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq95023134;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq95023134 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq13995773;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq13995773 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq19094425;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq19094425 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq76893765;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq76893765 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq57017594;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq57017594 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq70107909;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq70107909 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq72496514;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq72496514 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq98105605;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq98105605 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq59125878;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq59125878 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq58616996;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq58616996 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq98363965;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq98363965 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq53855588;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq53855588 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq84361141;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq84361141 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq18018615;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq18018615 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq53198511;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq53198511 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq90224759;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq90224759 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq37724443;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq37724443 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq27471575;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq27471575 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq74461324;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq74461324 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq21833969;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq21833969 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq68486842;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq68486842 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq52203486;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq52203486 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq32516344;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq32516344 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq91974740;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq91974740 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq37633497;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq37633497 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq67553243;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq67553243 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq94340237;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq94340237 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq79524040;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq79524040 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq4731801;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq4731801 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq12111401;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq12111401 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq55261092;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq55261092 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq26170483;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq26170483 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq2510809;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq2510809 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq56446148;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq56446148 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq82277543;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq82277543 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq29315176;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq29315176 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq499227;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq499227 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq97656899;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq97656899 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq72544088;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq72544088 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq52958774;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq52958774 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq64987512;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq64987512 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq4453490;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq4453490 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq47197069;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq47197069 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq77048576;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq77048576 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq50945488;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq50945488 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq23197663;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq23197663 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq5416190;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq5416190 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq42169611;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq42169611 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq6784110;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq6784110 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq93178139;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq93178139 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq13285861;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq13285861 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq26857658;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq26857658 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq94800221;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq94800221 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq52073171;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq52073171 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq34745769;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq34745769 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq42819649;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq42819649 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq81479428;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq81479428 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq27119685;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq27119685 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq39260268;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq39260268 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq89464351;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq89464351 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq75767672;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq75767672 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq92972474;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq92972474 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq93373805;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq93373805 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq47014478;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq47014478 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq3355904;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq3355904 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq72193482;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq72193482 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq51344779;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq51344779 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq27914993;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq27914993 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq35741072;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq35741072 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq23883335;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq23883335 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq89725532;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq89725532 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq40067544;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq40067544 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq54927486;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq54927486 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq21502550;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq21502550 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq56846456;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq56846456 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq64033353;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq64033353 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq5006418;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq5006418 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq55467768;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq55467768 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq41029252;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq41029252 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq14435834;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq14435834 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq62137053;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq62137053 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq52170626;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq52170626 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq72739930;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq72739930 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq11553661;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq11553661 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq98825539;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq98825539 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq28403434;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq28403434 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq31370261;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq31370261 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq50437637;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq50437637 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq21700379;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq21700379 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq39457894;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq39457894 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq47835748;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq47835748 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq73379541;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq73379541 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq58396631;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq58396631 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq83079736;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq83079736 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq77191102;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq77191102 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq72015038;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq72015038 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq11079685;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq11079685 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq182591;     aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq182591 = aeXsrtimVVPrugRtIwQdTYafmnwEtCctShNjbUunnXKiMvqbppFqXzPBEsFDxdsSsVZNThRnBcqvOrSqwVPExmWSRMejOrqJespq73907140;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void mSTefOxleJMhaiuQfrPNGAovvXZcTtytXHrZEIqEbPBxKWTVQUTPiahleYiHoUSkLTKSqVLWBKmrzqkZEEMzaiKwaGufZiPGxRmr85082913() {     int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP60543134 = -733199569;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP70726660 = -482639978;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP96854750 = -623079812;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP92583125 = -162742865;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP5223602 = -660374168;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP12329758 = -960968568;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP9769354 = -66753010;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP93045668 = -896019102;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP41932541 = -875901541;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP54685569 = -890728777;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP5654116 = -415897566;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP15528780 = -138223641;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP57907252 = -642905215;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP36271665 = -294130689;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP9921325 = -370809317;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP42437454 = -902330134;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP52777920 = -783399289;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP9216227 = -349343668;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP40775946 = -941970892;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP71557963 = -963393780;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP9897294 = -240129558;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP49965441 = -591633103;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP9661422 = -917803112;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP5774408 = -237623481;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP21897885 = -763922874;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP45077108 = 49842992;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP92608307 = 99412765;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP675744 = -840323744;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP44084430 = -502232146;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP15612563 = -386569675;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP33995622 = -234755400;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP39590426 = 4404277;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP24400784 = -158803684;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP62810214 = -147239935;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP88771200 = -95132088;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP74762503 = -384939276;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP19038763 = -157654100;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP61758273 = -853828132;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP77423348 = -351708544;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP7622887 = -807540786;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP12442591 = -642263795;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP44645420 = -37150759;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP92487969 = -530057406;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP26939488 = -145400693;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP47157227 = -723973848;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP15382581 = -611106452;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP42129768 = 50447831;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP81765936 = -704065965;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP11273107 = -339145283;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP54273767 = -130754071;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP94567958 = -79761935;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP16633514 = -598130331;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP42605868 = -330181199;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP70397051 = -967861270;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP75185620 = -867841524;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP10577693 = -41566467;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP61065239 = -564836866;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP91080342 = -285456332;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP70685241 = -398819991;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP60146494 = -610217160;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP19721451 = -960381334;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP9093611 = -226429266;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP48961238 = -293786956;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP26319978 = -389331867;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP20689947 = -555973377;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP66063689 = -320301844;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP91127995 = -979419958;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP95097038 = -395665280;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP47500464 = -98998601;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP35158821 = -985870041;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP23398692 = -644676034;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP91019646 = -929571158;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP31792878 = -997635124;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP33153059 = -34430106;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP59115373 = -221129985;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP65251873 = -102978800;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP57477472 = 38424302;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP82721933 = -672402420;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP58617180 = -513649634;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP6515304 = -52816423;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP2947341 = 99395160;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP10842372 = -196521270;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP89402636 = -401178462;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP89810662 = -271478075;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP21044605 = -206807740;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP17362109 = -636625070;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP96984558 = -665414524;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP54003732 = -190942415;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP87624593 = -279398412;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP78193508 = 46434378;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP13697265 = -820102411;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP27958420 = -872197769;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP91073031 = -355008141;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP17276855 = -741491384;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP87901436 = -847159453;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP3348980 = -315834529;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP95684182 = -743363803;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP66167992 = -40725540;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP6249541 = -589427316;    int MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP81093538 = -733199569;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP60543134 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP70726660;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP70726660 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP96854750;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP96854750 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP92583125;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP92583125 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP5223602;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP5223602 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP12329758;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP12329758 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP9769354;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP9769354 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP93045668;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP93045668 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP41932541;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP41932541 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP54685569;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP54685569 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP5654116;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP5654116 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP15528780;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP15528780 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP57907252;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP57907252 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP36271665;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP36271665 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP9921325;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP9921325 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP42437454;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP42437454 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP52777920;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP52777920 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP9216227;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP9216227 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP40775946;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP40775946 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP71557963;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP71557963 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP9897294;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP9897294 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP49965441;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP49965441 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP9661422;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP9661422 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP5774408;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP5774408 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP21897885;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP21897885 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP45077108;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP45077108 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP92608307;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP92608307 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP675744;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP675744 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP44084430;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP44084430 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP15612563;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP15612563 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP33995622;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP33995622 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP39590426;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP39590426 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP24400784;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP24400784 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP62810214;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP62810214 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP88771200;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP88771200 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP74762503;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP74762503 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP19038763;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP19038763 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP61758273;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP61758273 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP77423348;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP77423348 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP7622887;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP7622887 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP12442591;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP12442591 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP44645420;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP44645420 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP92487969;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP92487969 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP26939488;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP26939488 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP47157227;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP47157227 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP15382581;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP15382581 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP42129768;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP42129768 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP81765936;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP81765936 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP11273107;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP11273107 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP54273767;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP54273767 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP94567958;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP94567958 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP16633514;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP16633514 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP42605868;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP42605868 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP70397051;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP70397051 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP75185620;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP75185620 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP10577693;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP10577693 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP61065239;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP61065239 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP91080342;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP91080342 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP70685241;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP70685241 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP60146494;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP60146494 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP19721451;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP19721451 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP9093611;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP9093611 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP48961238;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP48961238 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP26319978;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP26319978 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP20689947;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP20689947 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP66063689;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP66063689 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP91127995;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP91127995 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP95097038;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP95097038 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP47500464;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP47500464 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP35158821;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP35158821 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP23398692;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP23398692 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP91019646;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP91019646 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP31792878;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP31792878 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP33153059;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP33153059 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP59115373;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP59115373 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP65251873;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP65251873 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP57477472;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP57477472 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP82721933;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP82721933 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP58617180;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP58617180 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP6515304;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP6515304 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP2947341;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP2947341 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP10842372;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP10842372 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP89402636;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP89402636 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP89810662;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP89810662 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP21044605;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP21044605 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP17362109;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP17362109 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP96984558;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP96984558 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP54003732;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP54003732 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP87624593;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP87624593 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP78193508;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP78193508 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP13697265;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP13697265 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP27958420;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP27958420 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP91073031;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP91073031 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP17276855;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP17276855 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP87901436;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP87901436 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP3348980;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP3348980 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP95684182;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP95684182 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP66167992;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP66167992 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP6249541;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP6249541 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP81093538;     MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP81093538 = MBevIwOXopmonJDkHSKcINskagwSufuhEtMDZRqdUmIkUGsPffyeGJizIuUczvcIvEjtwALqbgIVEIQTognPgQbuLNufwOdYKzoP60543134;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void KaNLluWvIMSCXmGamrjpVVJgqWoAJjHzCEaXJbZZuLMwlzSVerTwFMWpQgUOGVlYETATLjQKRJuJJMurDSWSQLtphlGcGKTYansS43435701() {     long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv89040887 = -352029563;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv37019757 = -183377938;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv42979425 = -970695191;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv19744456 = -180439857;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv81133446 = -40436362;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv53435510 = -785505993;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv30961395 = -832241760;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv87653468 = -257666977;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv86478331 = -241776315;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv84318326 = -599252459;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv23626871 = -249431626;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv17546448 = -568853499;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv42977465 = -623172097;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv15874314 = -599231065;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv87571137 = -651462048;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv2763308 = -489792870;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv52290034 = -396838791;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv35246329 = -863680623;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv64315688 = 99175620;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv2698175 = 15044396;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv79003017 = -897570696;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv78597949 = -414321016;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv85423933 = -199359033;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv59916677 = -904841849;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv29580471 = -85371885;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv74675855 = -180912484;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv8379088 = -959635644;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv55097844 = -203674140;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv29787694 = -614942285;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv69475249 = -899767782;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv3941656 = -616485725;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv79466096 = -748935655;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv602826 = -782704765;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv17312303 = -99590283;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv8833256 = 43064459;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv28009475 = -994126506;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv5681778 = 56108700;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv19392266 = -488222292;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv34655330 = -359359489;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv87183431 = 73364276;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv34724853 = -198659254;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv51001929 = -258993811;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv36388500 = 42477173;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv73023246 = -819071724;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv11111012 = -668274882;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv75850025 = -590596334;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv99903531 = -28018581;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv9705133 = 20742155;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv34067130 = 61637170;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv84314588 = 53189975;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv52397223 = -955686890;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv15841748 = -111821682;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv44617077 = -785936025;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv36902748 = -995932075;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv64432683 = -365976699;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv10442938 = -937708547;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv51595824 = -984018906;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv83062748 = 34146657;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv90163984 = 4932027;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv6457592 = -859523879;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv45056422 = -825870350;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv75863551 = -528567621;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv57865775 = -642724692;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv17003082 = -342008533;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv80376671 = -982766734;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv44160774 = -500495972;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv16943622 = -786148735;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv25665163 = -423581815;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv7041059 = -542295525;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv59561662 = -657335542;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv97081530 = -445901571;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv32897768 = -908616500;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv590999 = -404321135;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv77132256 = -974188657;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv67973321 = -786296351;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv28001089 = -538576886;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv42209449 = -356798189;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv12400687 = -380287310;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv48805666 = -136566968;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv53730445 = -494775551;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv74772323 = -52893903;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv98673955 = -880377800;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv21030714 = -165311310;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv45473105 = -568132261;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv17078026 = -944080893;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv88099908 = -404664044;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv34849020 = -962999630;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv63700078 = -786772691;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv52879620 = -733613584;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv98390317 = -19226994;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv76413651 = 89892399;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv22619030 = -978037957;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv29228281 = -393154320;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv28197739 = -499835611;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv42127010 = -100765375;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv58861302 = -670091633;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv93136154 = -616269119;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv19385418 = -615514294;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv92646574 = -836304990;    long zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv66950237 = -352029563;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv89040887 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv37019757;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv37019757 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv42979425;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv42979425 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv19744456;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv19744456 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv81133446;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv81133446 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv53435510;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv53435510 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv30961395;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv30961395 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv87653468;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv87653468 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv86478331;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv86478331 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv84318326;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv84318326 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv23626871;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv23626871 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv17546448;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv17546448 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv42977465;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv42977465 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv15874314;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv15874314 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv87571137;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv87571137 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv2763308;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv2763308 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv52290034;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv52290034 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv35246329;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv35246329 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv64315688;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv64315688 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv2698175;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv2698175 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv79003017;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv79003017 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv78597949;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv78597949 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv85423933;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv85423933 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv59916677;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv59916677 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv29580471;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv29580471 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv74675855;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv74675855 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv8379088;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv8379088 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv55097844;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv55097844 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv29787694;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv29787694 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv69475249;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv69475249 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv3941656;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv3941656 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv79466096;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv79466096 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv602826;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv602826 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv17312303;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv17312303 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv8833256;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv8833256 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv28009475;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv28009475 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv5681778;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv5681778 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv19392266;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv19392266 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv34655330;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv34655330 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv87183431;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv87183431 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv34724853;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv34724853 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv51001929;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv51001929 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv36388500;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv36388500 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv73023246;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv73023246 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv11111012;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv11111012 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv75850025;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv75850025 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv99903531;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv99903531 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv9705133;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv9705133 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv34067130;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv34067130 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv84314588;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv84314588 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv52397223;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv52397223 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv15841748;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv15841748 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv44617077;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv44617077 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv36902748;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv36902748 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv64432683;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv64432683 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv10442938;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv10442938 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv51595824;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv51595824 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv83062748;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv83062748 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv90163984;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv90163984 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv6457592;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv6457592 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv45056422;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv45056422 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv75863551;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv75863551 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv57865775;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv57865775 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv17003082;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv17003082 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv80376671;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv80376671 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv44160774;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv44160774 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv16943622;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv16943622 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv25665163;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv25665163 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv7041059;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv7041059 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv59561662;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv59561662 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv97081530;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv97081530 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv32897768;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv32897768 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv590999;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv590999 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv77132256;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv77132256 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv67973321;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv67973321 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv28001089;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv28001089 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv42209449;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv42209449 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv12400687;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv12400687 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv48805666;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv48805666 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv53730445;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv53730445 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv74772323;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv74772323 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv98673955;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv98673955 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv21030714;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv21030714 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv45473105;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv45473105 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv17078026;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv17078026 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv88099908;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv88099908 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv34849020;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv34849020 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv63700078;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv63700078 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv52879620;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv52879620 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv98390317;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv98390317 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv76413651;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv76413651 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv22619030;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv22619030 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv29228281;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv29228281 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv28197739;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv28197739 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv42127010;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv42127010 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv58861302;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv58861302 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv93136154;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv93136154 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv19385418;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv19385418 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv92646574;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv92646574 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv66950237;     zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv66950237 = zzJuaIevSNfEmdLnzTItZKladnDHqVjVTcrEIbrHgxXUFCqdzfZGfzsoNjRmEXFwdKkJlGeYbAMbnRUaxaILwcpUjDqZJEDJsaqv89040887;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void lXpiSDxGcuYLAbqegdlANyXGaGwRzaIKViTgixYdbfrfchWpuzteeUJnAjHLHfGBmNpRIKwjEIZhGsetkGWzhdgBZggypMWvYqsO50206351() {     float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj78875040 = -686477660;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj16563448 = -839224696;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj48920938 = -946964417;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj29047050 = -196611246;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj38430374 = -327389746;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj90997661 = -852755709;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj96878259 = -697257342;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj49967493 = -869172793;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj84080519 = -420937747;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj68293088 = -788075823;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj46946802 = 73373458;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj53872938 = -677877334;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj12093350 = -927553904;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj47235356 = -726305547;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj32664932 = -130334372;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj90647277 = -719715715;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj1844208 = -991878337;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj69377284 = -537126462;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj87550280 = -713224982;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj29429747 = -570175892;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj14565146 = -37991046;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj44417310 = -745397902;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj27068987 = -434229099;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj88701854 = -547300012;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj38324903 = -963592532;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj68964365 = -410740763;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj98652386 = -504972983;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj46207694 = -665011570;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj80516537 = -907591205;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj49729083 = 34723602;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj10961307 = -35997918;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj86593863 = -299401454;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj9890899 = -442476442;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj48150419 = -131910428;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj20269271 = -589273178;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj9424812 = -166314837;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj98648669 = -317521844;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj73781949 = -154134198;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj4194900 = -271523283;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj85747377 = 43846488;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj93017265 = -1917173;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj98189773 = -328953840;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj7538985 = -249861920;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj66858404 = 25677162;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj35068780 = -29446516;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj72484070 = -2888813;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj78558868 = -649720648;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj31787502 = -360036631;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj96275461 = -387647831;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj53144995 = -252861155;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj41448103 = -542307970;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj92704444 = -46746536;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj13696285 = -481711987;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj89054504 = -225031259;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj35641206 = -40134704;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj34457731 = -941079759;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj89494461 = -304995597;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj60219083 = -299664405;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj90722147 = -233018714;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj69466009 = -916648984;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj92345275 = -247782727;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj50670566 = 67754227;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj69450955 = -961581589;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj34351436 = -355661350;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj57331781 = -652077905;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj60352938 = -627225088;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj43982039 = -135400893;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj63942931 = -695643476;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj26966085 = -37032369;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj23240120 = -964019535;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj91998607 = -302193872;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj28062259 = -737744140;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj65182384 = -165603179;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj1802903 = -657071470;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj36412482 = -468258719;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj16375372 = -709037206;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj36878325 = -395535982;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj60210582 = -359906262;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj53633075 = -417853497;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj65840833 = -860703720;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj90405497 = -761020116;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj66864885 = -44936352;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj49932232 = -177363740;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj27371543 = -554730051;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj8280980 = -422968428;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj18256863 = -989251382;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj72897579 = -817689468;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj20836394 = -117445184;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj12509213 = 8224275;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj85811540 = -648193420;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj19930351 = -861319241;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj38429586 = 82142561;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj83059802 = -921115484;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj34728891 = -354874300;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj93402102 = -708370786;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj42346700 = 30328599;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj28738819 = -367372252;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj73187548 = -894200570;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj9526623 = -322244933;    float sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj74715841 = -686477660;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj78875040 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj16563448;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj16563448 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj48920938;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj48920938 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj29047050;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj29047050 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj38430374;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj38430374 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj90997661;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj90997661 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj96878259;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj96878259 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj49967493;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj49967493 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj84080519;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj84080519 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj68293088;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj68293088 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj46946802;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj46946802 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj53872938;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj53872938 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj12093350;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj12093350 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj47235356;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj47235356 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj32664932;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj32664932 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj90647277;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj90647277 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj1844208;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj1844208 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj69377284;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj69377284 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj87550280;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj87550280 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj29429747;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj29429747 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj14565146;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj14565146 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj44417310;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj44417310 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj27068987;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj27068987 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj88701854;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj88701854 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj38324903;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj38324903 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj68964365;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj68964365 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj98652386;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj98652386 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj46207694;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj46207694 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj80516537;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj80516537 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj49729083;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj49729083 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj10961307;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj10961307 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj86593863;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj86593863 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj9890899;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj9890899 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj48150419;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj48150419 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj20269271;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj20269271 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj9424812;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj9424812 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj98648669;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj98648669 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj73781949;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj73781949 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj4194900;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj4194900 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj85747377;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj85747377 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj93017265;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj93017265 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj98189773;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj98189773 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj7538985;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj7538985 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj66858404;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj66858404 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj35068780;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj35068780 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj72484070;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj72484070 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj78558868;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj78558868 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj31787502;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj31787502 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj96275461;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj96275461 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj53144995;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj53144995 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj41448103;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj41448103 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj92704444;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj92704444 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj13696285;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj13696285 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj89054504;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj89054504 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj35641206;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj35641206 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj34457731;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj34457731 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj89494461;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj89494461 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj60219083;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj60219083 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj90722147;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj90722147 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj69466009;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj69466009 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj92345275;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj92345275 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj50670566;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj50670566 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj69450955;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj69450955 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj34351436;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj34351436 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj57331781;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj57331781 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj60352938;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj60352938 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj43982039;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj43982039 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj63942931;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj63942931 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj26966085;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj26966085 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj23240120;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj23240120 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj91998607;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj91998607 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj28062259;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj28062259 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj65182384;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj65182384 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj1802903;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj1802903 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj36412482;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj36412482 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj16375372;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj16375372 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj36878325;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj36878325 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj60210582;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj60210582 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj53633075;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj53633075 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj65840833;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj65840833 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj90405497;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj90405497 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj66864885;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj66864885 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj49932232;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj49932232 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj27371543;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj27371543 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj8280980;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj8280980 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj18256863;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj18256863 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj72897579;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj72897579 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj20836394;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj20836394 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj12509213;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj12509213 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj85811540;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj85811540 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj19930351;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj19930351 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj38429586;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj38429586 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj83059802;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj83059802 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj34728891;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj34728891 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj93402102;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj93402102 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj42346700;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj42346700 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj28738819;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj28738819 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj73187548;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj73187548 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj9526623;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj9526623 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj74715841;     sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj74715841 = sFhkDKaVvcgHFObAJGTIhoscdxtrldIQzPBfZmwXDKFwzZtalVmRXkhEMFHGQikSTQHqNwbWKvFWivcEDSGPqrtPixSTfqHUandj78875040;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void DVOueFPWCGwYaAsiSbbNnowmUJrszrFQZCRkxhuwMKOKQCFLsLykcqpPZcZUNXWKAfyXjIklhfBcZmVSEgzotVojNCLVIZipoEKk44762010() {     float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr94580154 = -42791103;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr34657973 = -412223768;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr78606024 = -843349028;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr33347416 = -210646792;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr29669216 = -783990797;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr23598775 = -599802632;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr65409878 = -621610489;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr87070231 = -590479728;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr15961664 = -638700499;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr88346654 = -329318743;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr57752780 = -704946658;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr38231779 = -336652739;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr58873174 = 74303396;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr31058146 = 55856224;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr32180301 = -466714124;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr97112609 = -89082713;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr1457264 = -761157942;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr34849434 = -793324736;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr30357662 = -380591542;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr29989224 = -704518028;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr83166237 = 85417707;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr22298265 = -908219350;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr949600 = -243738967;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr41987102 = -735093890;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr68555920 = -501293472;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr33818544 = -859270968;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr73229212 = -131114824;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr68680394 = -463530850;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr62281194 = -352154418;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr26930524 = -524019725;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr69884022 = -528404727;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr7874568 = -365843469;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr25499415 = -292466955;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr25858972 = -56188291;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr39628832 = -100358675;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr10275859 = 33295290;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr60468992 = -413503071;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr43629599 = -964170946;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr90965092 = -50005067;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr10916086 = 59736709;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr41723887 = -370782537;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr82541488 = -846277640;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr76839405 = -213024151;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr27545523 = -963786069;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr3032126 = -554236991;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr99751354 = -821104927;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr86448405 = -711952631;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr5670313 = -88637087;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr93663824 = -487027265;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr56280818 = -144905531;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr94209243 = -743903625;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr78283388 = -571398297;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr56670692 = -653517539;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr21110747 = -95570173;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr61595772 = -400724671;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr72281890 = -134571754;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr33708373 = -68484801;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr36618922 = -8255138;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr64791495 = -709353320;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr95850672 = -924719829;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr50369563 = -368687808;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr96729483 = -58079640;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr24789037 = -138325311;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr89031139 = -14680775;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr18462632 = -800914016;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr49878213 = -239103189;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr12732364 = 55814216;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr33014202 = -869508314;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr91429314 = -843785101;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr21904442 = -400009415;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr36643618 = -675579643;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr57827665 = -796986997;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr43884342 = -643319670;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr19441577 = -340328252;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr88265337 = -233735492;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr624750 = -68304654;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr45458859 = -595195199;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr73404077 = -279952899;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr38954977 = -80856900;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr68804566 = -680188546;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr47370139 = -47318338;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr67558900 = 57522262;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr75016569 = -976503585;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr6000377 = -107248887;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr32721280 = -780116101;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr91600634 = -957006431;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr51203876 = -712325931;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr4388669 = -96896782;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr64263199 = -655463620;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr67346941 = -965786922;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr76567485 = -898219909;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr23850070 = -305247933;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr78838103 = -154817626;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr95114419 = -125285238;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr60546523 = -571575483;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr44994403 = -212702897;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr57752452 = -607952330;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr87808265 = -98343376;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr9082891 = -62872054;    float EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr53153912 = -42791103;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr94580154 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr34657973;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr34657973 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr78606024;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr78606024 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr33347416;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr33347416 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr29669216;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr29669216 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr23598775;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr23598775 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr65409878;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr65409878 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr87070231;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr87070231 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr15961664;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr15961664 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr88346654;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr88346654 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr57752780;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr57752780 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr38231779;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr38231779 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr58873174;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr58873174 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr31058146;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr31058146 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr32180301;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr32180301 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr97112609;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr97112609 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr1457264;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr1457264 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr34849434;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr34849434 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr30357662;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr30357662 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr29989224;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr29989224 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr83166237;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr83166237 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr22298265;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr22298265 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr949600;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr949600 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr41987102;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr41987102 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr68555920;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr68555920 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr33818544;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr33818544 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr73229212;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr73229212 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr68680394;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr68680394 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr62281194;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr62281194 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr26930524;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr26930524 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr69884022;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr69884022 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr7874568;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr7874568 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr25499415;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr25499415 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr25858972;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr25858972 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr39628832;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr39628832 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr10275859;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr10275859 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr60468992;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr60468992 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr43629599;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr43629599 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr90965092;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr90965092 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr10916086;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr10916086 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr41723887;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr41723887 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr82541488;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr82541488 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr76839405;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr76839405 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr27545523;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr27545523 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr3032126;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr3032126 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr99751354;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr99751354 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr86448405;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr86448405 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr5670313;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr5670313 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr93663824;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr93663824 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr56280818;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr56280818 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr94209243;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr94209243 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr78283388;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr78283388 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr56670692;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr56670692 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr21110747;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr21110747 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr61595772;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr61595772 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr72281890;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr72281890 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr33708373;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr33708373 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr36618922;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr36618922 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr64791495;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr64791495 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr95850672;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr95850672 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr50369563;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr50369563 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr96729483;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr96729483 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr24789037;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr24789037 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr89031139;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr89031139 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr18462632;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr18462632 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr49878213;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr49878213 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr12732364;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr12732364 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr33014202;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr33014202 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr91429314;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr91429314 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr21904442;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr21904442 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr36643618;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr36643618 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr57827665;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr57827665 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr43884342;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr43884342 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr19441577;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr19441577 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr88265337;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr88265337 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr624750;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr624750 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr45458859;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr45458859 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr73404077;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr73404077 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr38954977;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr38954977 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr68804566;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr68804566 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr47370139;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr47370139 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr67558900;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr67558900 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr75016569;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr75016569 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr6000377;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr6000377 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr32721280;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr32721280 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr91600634;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr91600634 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr51203876;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr51203876 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr4388669;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr4388669 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr64263199;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr64263199 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr67346941;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr67346941 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr76567485;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr76567485 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr23850070;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr23850070 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr78838103;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr78838103 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr95114419;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr95114419 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr60546523;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr60546523 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr44994403;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr44994403 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr57752452;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr57752452 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr87808265;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr87808265 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr9082891;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr9082891 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr53153912;     EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr53153912 = EhckbNDwpMqnHHQgXiRfrOKgwPdHxLoVbkNXDPMfRrtAMYNAJBSGmetpXvnLkvzeXULWttdHVlNAVKUYBcyvtiixrYzIcAvWrPcr94580154;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void hMbvQyXEadXtAxcHFwQZIYNtHAYdjBMXWKSyWPCNrneHkmjEOBmeqLCJzinPatUZRAWqfekptCiUtFIVRTPhOigLHIyncnnxTFpk78354616() {     float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ29480698 = -470355230;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ86478038 = -684236047;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ31637281 = -359451177;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ36859367 = -409633677;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ39074460 = -807428241;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ34151414 = -213652496;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ44384794 = 96869426;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ91216511 = -68120755;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ34099613 = -414985215;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ76129494 = -484675934;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ8988319 = -106615617;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ84787184 = -317731461;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ31235667 = -375367104;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ54706504 = -997722174;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ3725227 = -206790493;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ9804177 = -552797668;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ45066148 = -477062823;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ68157979 = 60340590;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ73835315 = -17206229;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ54346039 = -757665245;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ72854854 = -701982002;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ48010283 = -50848612;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ40774664 = 97639928;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ2907774 = -111103980;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ86521602 = -387922665;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ29215498 = -522933151;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ89375388 = -505816804;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ3673670 = -100329815;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ37275314 = -702867704;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ93033962 = -239013849;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ61929036 = 82296425;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ22323864 = 29189402;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ96545796 = -551379104;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ68474991 = -695893149;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ27684095 = -978207181;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ46892603 = -900524873;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ47581985 = -495214878;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ75239083 = -565481133;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ98114371 = -559559176;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ95799656 = -117419122;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ12938302 = 17000856;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ14202734 = -176588235;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ47756630 = -463510914;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ57095702 = -822596556;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ30963076 = -285033902;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ36390912 = -569187023;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ91466490 = -317255636;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ85254120 = -728363322;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ10502289 = -822400008;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ79728715 = -885942387;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ41313201 = -728438223;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ69759010 = -297463694;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ62751953 = -809192994;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ94590899 = -770487072;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ5499763 = -810733173;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ81470415 = -319506619;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ45703375 = -681875975;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ28729507 = -148347198;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ50337765 = 78288987;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ9858963 = -184495091;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ44776025 = -707835692;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ40711124 = -802800760;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ53941198 = -365253051;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ41065651 = -75971367;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ14200458 = -466972359;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ86664455 = -35805020;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ88241387 = -766352357;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ62760676 = -679473956;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ27022410 = 80485006;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ56832623 = -306265621;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ62222192 = 42417210;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ69827064 = -911581690;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ70043608 = -380100235;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ78035658 = -899787107;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ41407737 = -674666101;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ58652120 = -425393768;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ253653 = -587337698;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ83678961 = -79763517;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ71944698 = -826070078;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ50130690 = -818735642;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ37749007 = -105677515;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ4121268 = -777453482;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ93171381 = -277929807;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ57546598 = -816925318;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ51720762 = -510575626;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ92170025 = -620239882;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ59571910 = -161617605;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ1954898 = -780892032;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ62975228 = -885159976;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ46213680 = -558700563;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ1189229 = -118648898;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ18852479 = -246867681;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ24901319 = -543770121;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ88255409 = -275064085;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ51023631 = -409583430;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ72227177 = -180198385;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ60261536 = -811335184;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ6690980 = -287539547;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ42895244 = -255624197;    float QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ44298621 = -470355230;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ29480698 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ86478038;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ86478038 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ31637281;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ31637281 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ36859367;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ36859367 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ39074460;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ39074460 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ34151414;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ34151414 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ44384794;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ44384794 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ91216511;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ91216511 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ34099613;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ34099613 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ76129494;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ76129494 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ8988319;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ8988319 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ84787184;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ84787184 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ31235667;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ31235667 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ54706504;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ54706504 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ3725227;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ3725227 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ9804177;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ9804177 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ45066148;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ45066148 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ68157979;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ68157979 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ73835315;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ73835315 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ54346039;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ54346039 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ72854854;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ72854854 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ48010283;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ48010283 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ40774664;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ40774664 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ2907774;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ2907774 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ86521602;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ86521602 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ29215498;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ29215498 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ89375388;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ89375388 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ3673670;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ3673670 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ37275314;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ37275314 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ93033962;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ93033962 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ61929036;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ61929036 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ22323864;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ22323864 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ96545796;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ96545796 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ68474991;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ68474991 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ27684095;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ27684095 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ46892603;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ46892603 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ47581985;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ47581985 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ75239083;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ75239083 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ98114371;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ98114371 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ95799656;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ95799656 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ12938302;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ12938302 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ14202734;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ14202734 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ47756630;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ47756630 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ57095702;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ57095702 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ30963076;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ30963076 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ36390912;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ36390912 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ91466490;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ91466490 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ85254120;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ85254120 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ10502289;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ10502289 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ79728715;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ79728715 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ41313201;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ41313201 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ69759010;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ69759010 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ62751953;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ62751953 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ94590899;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ94590899 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ5499763;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ5499763 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ81470415;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ81470415 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ45703375;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ45703375 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ28729507;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ28729507 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ50337765;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ50337765 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ9858963;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ9858963 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ44776025;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ44776025 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ40711124;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ40711124 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ53941198;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ53941198 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ41065651;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ41065651 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ14200458;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ14200458 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ86664455;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ86664455 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ88241387;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ88241387 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ62760676;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ62760676 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ27022410;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ27022410 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ56832623;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ56832623 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ62222192;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ62222192 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ69827064;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ69827064 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ70043608;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ70043608 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ78035658;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ78035658 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ41407737;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ41407737 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ58652120;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ58652120 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ253653;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ253653 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ83678961;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ83678961 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ71944698;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ71944698 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ50130690;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ50130690 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ37749007;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ37749007 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ4121268;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ4121268 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ93171381;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ93171381 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ57546598;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ57546598 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ51720762;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ51720762 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ92170025;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ92170025 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ59571910;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ59571910 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ1954898;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ1954898 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ62975228;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ62975228 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ46213680;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ46213680 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ1189229;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ1189229 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ18852479;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ18852479 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ24901319;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ24901319 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ88255409;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ88255409 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ51023631;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ51023631 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ72227177;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ72227177 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ60261536;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ60261536 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ6690980;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ6690980 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ42895244;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ42895244 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ44298621;     QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ44298621 = QsZjRwkochQtaRyGenLiTKjWfwNGPkBoPYFZtfQRxsjasFNKogTguHQFJDEUjeYrfbdCMZNyWtlbbIdajDRiRVtIHAPpbwiTTruQ29480698;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void vHugkdXRcbWLGZnPzDYgSmZrJcxkkxOILCqfYkVIJPJIJYYyDAoFENWIiCXHfmgeNOADiIrOhQWCdRilEygjJIDxghxVNNcXjeuv64189755() {     float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW93723103 = -152294368;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW8196903 = -27200153;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW6012830 = -490387480;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW85519892 = -239023003;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW35869485 = -415814660;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW89509721 = -925354020;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW73528150 = 9588584;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW7734463 = -3122010;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW89112672 = -911568673;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW37585386 = 24255354;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW38295302 = -532854719;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW87696 = -818524752;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW55623688 = -937159020;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW48352049 = -395425414;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW37722242 = -859829711;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW64531651 = -679324687;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW50674964 = -103397144;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW4173564 = -593899509;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW56033456 = -664615240;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW91989907 = -880470609;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW87077140 = -191168945;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW90622802 = -472184451;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW15534316 = -552095875;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW73629017 = -875633686;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW68805585 = 74658977;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW10588948 = 3483183;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW67482360 = -786140721;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW66288245 = -940972001;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW92805392 = -305293088;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW96055175 = -171044277;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW80314731 = 54338028;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW76985556 = -739302326;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW70099241 = 34726139;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW46008873 = -548750055;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW20073161 = -68422832;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW66344279 = -734884234;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW2844860 = -487986856;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW23973760 = -377940893;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW62043959 = -726066064;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW57452822 = -386398278;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW11935101 = -399140772;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW3078650 = 68698157;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW29990256 = -186373879;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW39369479 = -166396514;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW46958020 = -180443820;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW7052603 = -370976635;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW98051164 = -287769465;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW74607299 = -424720618;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW40557690 = -735772643;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW14794550 = -930995250;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW33487203 = -744955708;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW33910385 = -436455117;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW89205906 = -929124415;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW88094018 = -216442326;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW77112614 = -866700038;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW3100301 = -680109917;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW92662586 = -475104278;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW32383813 = -614753795;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW16714308 = -213681980;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW25280537 = -319297843;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW22027362 = -39213299;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW7239906 = -49439416;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW14929070 = -697828922;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW93057496 = -640524396;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW57270655 = 69917326;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW61309746 = -793552394;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW29988455 = -753250892;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW9614816 = -288408965;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW28278889 = -227002582;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW71377963 = -24945477;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW61686791 = -91337831;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW26701205 = -725456252;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW42129604 = -867833445;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW98580634 = -178216962;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW80054806 = -381329837;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW83998491 = -159867102;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW60632547 = -185810573;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW76164837 = -285699361;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW26670997 = -595189866;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW61752982 = -554364389;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW12537783 = -708747352;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW92875060 = -261420103;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW25730555 = -105199358;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW78010843 = -374297839;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW62567973 = -426088569;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW46404347 = -509206855;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW87779649 = -810177911;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW82005222 = -748831535;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW68896258 = -682050017;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW16972860 = -388312916;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW73681692 = -159779956;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW70461047 = -873233062;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW7259452 = -64258913;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW36763423 = -306768221;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW35425461 = -247184979;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW4695196 = -249701357;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW88149579 = -233472922;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW36932759 = -545849483;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW82098824 = -136313841;    float YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW85648274 = -152294368;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW93723103 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW8196903;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW8196903 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW6012830;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW6012830 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW85519892;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW85519892 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW35869485;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW35869485 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW89509721;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW89509721 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW73528150;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW73528150 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW7734463;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW7734463 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW89112672;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW89112672 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW37585386;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW37585386 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW38295302;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW38295302 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW87696;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW87696 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW55623688;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW55623688 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW48352049;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW48352049 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW37722242;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW37722242 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW64531651;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW64531651 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW50674964;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW50674964 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW4173564;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW4173564 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW56033456;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW56033456 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW91989907;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW91989907 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW87077140;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW87077140 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW90622802;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW90622802 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW15534316;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW15534316 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW73629017;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW73629017 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW68805585;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW68805585 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW10588948;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW10588948 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW67482360;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW67482360 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW66288245;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW66288245 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW92805392;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW92805392 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW96055175;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW96055175 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW80314731;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW80314731 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW76985556;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW76985556 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW70099241;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW70099241 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW46008873;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW46008873 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW20073161;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW20073161 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW66344279;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW66344279 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW2844860;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW2844860 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW23973760;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW23973760 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW62043959;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW62043959 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW57452822;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW57452822 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW11935101;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW11935101 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW3078650;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW3078650 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW29990256;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW29990256 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW39369479;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW39369479 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW46958020;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW46958020 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW7052603;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW7052603 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW98051164;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW98051164 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW74607299;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW74607299 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW40557690;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW40557690 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW14794550;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW14794550 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW33487203;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW33487203 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW33910385;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW33910385 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW89205906;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW89205906 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW88094018;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW88094018 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW77112614;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW77112614 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW3100301;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW3100301 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW92662586;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW92662586 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW32383813;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW32383813 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW16714308;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW16714308 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW25280537;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW25280537 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW22027362;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW22027362 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW7239906;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW7239906 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW14929070;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW14929070 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW93057496;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW93057496 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW57270655;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW57270655 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW61309746;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW61309746 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW29988455;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW29988455 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW9614816;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW9614816 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW28278889;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW28278889 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW71377963;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW71377963 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW61686791;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW61686791 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW26701205;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW26701205 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW42129604;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW42129604 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW98580634;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW98580634 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW80054806;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW80054806 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW83998491;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW83998491 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW60632547;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW60632547 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW76164837;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW76164837 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW26670997;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW26670997 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW61752982;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW61752982 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW12537783;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW12537783 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW92875060;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW92875060 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW25730555;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW25730555 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW78010843;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW78010843 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW62567973;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW62567973 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW46404347;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW46404347 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW87779649;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW87779649 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW82005222;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW82005222 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW68896258;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW68896258 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW16972860;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW16972860 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW73681692;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW73681692 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW70461047;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW70461047 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW7259452;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW7259452 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW36763423;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW36763423 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW35425461;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW35425461 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW4695196;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW4695196 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW88149579;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW88149579 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW36932759;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW36932759 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW82098824;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW82098824 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW85648274;     YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW85648274 = YVQvJaXorTfQUlDhFwVEKCWALVAUozTyWmMiilajlPGMCgBbJuRmJqkWzPknZDzrAguhZVzUVmSvvKyeUYKthgpgCqjSAmJiPOLW93723103;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void SyQunzcdRBDAaSOpACmijqyDFqLdYMegXugTetIELqVxgDmdaykqPbPcANpxozEFWWdfluLGthXtVAXBDiDZboRvfETRELQMlQbJ49364498() {     float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv67287246 = -964240391;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv46766373 = -444103636;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv99227247 = -377835783;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv6890580 = -439535491;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv63887646 = -632360914;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv3605960 = -296491592;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv7778243 = -172404669;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv44174519 = -430905095;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv54194225 = -974566731;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv71026222 = -750802154;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv84183664 = -90862821;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv12334279 = -21209496;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv43940510 = 37285404;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv20242015 = -527029707;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv41823186 = -301686488;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv49665102 = -500579533;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv44241789 = 62298017;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv29381256 = -581125300;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv99816259 = -647682813;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv20755361 = -469959362;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv10309354 = -295589442;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv79147969 = -206424740;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv89476838 = -357402835;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv59906781 = -176403980;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv85709420 = -455198579;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv41296139 = -761106195;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv9126017 = -474553770;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv64593770 = -579783931;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv2773932 = -476067594;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv35767467 = -233727893;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv35286127 = -297178951;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv24182755 = -447143587;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv8059592 = -88315414;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv12288865 = -8485116;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv16754464 = -175737153;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv74792659 = -905703296;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv83633975 = 17694682;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv58827554 = 52266664;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv56885651 = -231107324;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv23332992 = -753131259;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv47139366 = -864494919;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv93908559 = -513495460;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv73657526 = -671986971;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv21168259 = -443626917;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv14884987 = -494370131;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv7525561 = -686256135;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv82187678 = -449836816;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv60047935 = 41140054;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv17981846 = -221077934;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv99452861 = -82036929;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv49369544 = -918794183;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv47731544 = -841287010;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv28219169 = -744778735;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv75928110 = -590330847;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv39055144 = -685711;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv88139277 = -657815651;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv57289535 = 13299199;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv39320467 = -101431803;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv21181159 = -984336912;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv22591508 = -871254719;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv94479942 = -821937823;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv43184472 = -592620738;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv41400588 = -954837502;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv18426758 = -640838838;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv35740096 = -353623204;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv60000909 = -643719235;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv4274688 = -932894083;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv31651645 = -954229481;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv3487552 = -251292554;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv67030526 = -395983192;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv66031127 = -418274215;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv85414234 = -989968647;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv72495604 = -250017976;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv76483268 = -894551554;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv73615995 = -605464443;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv16400795 = -782093982;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv5490443 = -534437770;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv68308580 = -913775918;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv45021794 = -682033849;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv78183859 = -768942445;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv59108461 = -211269379;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv49078082 = -415693825;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv46611925 = -258705998;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv3321070 = -294030665;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv86397922 = -314933711;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv87554583 = -455891942;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv95963586 = -702364852;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv32131482 = -497984568;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv73233721 = 92200594;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv28615186 = -517921502;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv17503125 = -819002496;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv44313508 = -880873516;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv37646395 = 36603576;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv34294144 = -359852606;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv28853049 = -931193437;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv3954895 = -171874181;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv52507972 = -558657958;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv55230768 = 68851867;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv85428163 = 9996286;    float aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv54884077 = -964240391;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv67287246 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv46766373;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv46766373 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv99227247;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv99227247 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv6890580;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv6890580 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv63887646;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv63887646 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv3605960;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv3605960 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv7778243;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv7778243 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv44174519;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv44174519 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv54194225;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv54194225 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv71026222;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv71026222 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv84183664;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv84183664 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv12334279;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv12334279 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv43940510;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv43940510 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv20242015;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv20242015 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv41823186;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv41823186 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv49665102;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv49665102 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv44241789;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv44241789 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv29381256;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv29381256 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv99816259;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv99816259 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv20755361;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv20755361 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv10309354;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv10309354 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv79147969;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv79147969 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv89476838;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv89476838 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv59906781;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv59906781 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv85709420;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv85709420 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv41296139;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv41296139 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv9126017;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv9126017 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv64593770;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv64593770 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv2773932;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv2773932 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv35767467;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv35767467 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv35286127;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv35286127 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv24182755;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv24182755 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv8059592;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv8059592 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv12288865;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv12288865 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv16754464;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv16754464 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv74792659;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv74792659 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv83633975;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv83633975 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv58827554;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv58827554 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv56885651;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv56885651 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv23332992;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv23332992 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv47139366;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv47139366 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv93908559;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv93908559 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv73657526;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv73657526 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv21168259;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv21168259 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv14884987;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv14884987 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv7525561;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv7525561 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv82187678;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv82187678 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv60047935;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv60047935 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv17981846;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv17981846 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv99452861;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv99452861 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv49369544;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv49369544 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv47731544;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv47731544 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv28219169;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv28219169 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv75928110;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv75928110 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv39055144;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv39055144 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv88139277;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv88139277 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv57289535;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv57289535 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv39320467;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv39320467 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv21181159;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv21181159 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv22591508;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv22591508 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv94479942;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv94479942 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv43184472;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv43184472 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv41400588;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv41400588 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv18426758;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv18426758 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv35740096;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv35740096 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv60000909;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv60000909 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv4274688;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv4274688 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv31651645;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv31651645 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv3487552;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv3487552 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv67030526;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv67030526 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv66031127;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv66031127 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv85414234;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv85414234 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv72495604;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv72495604 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv76483268;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv76483268 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv73615995;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv73615995 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv16400795;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv16400795 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv5490443;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv5490443 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv68308580;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv68308580 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv45021794;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv45021794 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv78183859;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv78183859 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv59108461;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv59108461 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv49078082;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv49078082 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv46611925;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv46611925 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv3321070;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv3321070 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv86397922;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv86397922 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv87554583;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv87554583 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv95963586;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv95963586 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv32131482;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv32131482 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv73233721;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv73233721 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv28615186;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv28615186 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv17503125;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv17503125 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv44313508;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv44313508 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv37646395;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv37646395 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv34294144;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv34294144 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv28853049;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv28853049 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv3954895;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv3954895 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv52507972;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv52507972 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv55230768;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv55230768 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv85428163;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv85428163 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv54884077;     aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv54884077 = aGsfnewhEVGzpntGvediFhAgatBiRKIFZObxpXmwrjrJxUbRRKQGmtDsGLEudOQtNBZbMwQYshqWqqiGRkQGtCTgKiqdTsTdTxwv67287246;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void akXGCYGbtKMYkvLKGsIaTyHwJpyXjbwDrveUwactOCguaBKwfoYpHTOccaxMZjrSOJhoRqOfPOLRSNVHeBruDHvYgCrVdPTfKOfR56135149() {     float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn57121399 = -198688488;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn26310065 = 49607;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn5168761 = -354105008;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn16193175 = -455706881;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn21184574 = -919314298;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn41168112 = -363741308;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn73695107 = -37420251;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn6488544 = 57589089;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn51796413 = -53728163;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn55000983 = -939625519;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn7503596 = -868057738;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn48660769 = -130233332;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn13056395 = -267096403;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn51603056 = -654104189;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn86916980 = -880558811;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn37549072 = -730502378;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn93795962 = -532741528;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn63512211 = -254571138;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn23050852 = -360083415;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn47486933 = 44820350;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn45871482 = -536009791;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn44967330 = -537501626;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn31121892 = -592272901;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn88691957 = -918862144;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn94453853 = -233419227;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn35584649 = -990934475;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn99399316 = -19891109;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn55703621 = 58878638;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn53502775 = -768716513;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn16021301 = -399236509;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn42305778 = -816691145;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn31310522 = 2390614;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn17347665 = -848087091;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn43126981 = -40805261;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn28190479 = -808074790;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn56207996 = -77891627;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn76600868 = -355935863;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn13217238 = -713645241;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn26425221 = -143271118;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn21896938 = -782649047;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn5431779 = -667752838;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn41096404 = -583455490;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn44808011 = -964326063;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn15003417 = -698878032;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn38842755 = -955541765;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn4159606 = -98548614;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn60843014 = 28461117;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn82130304 = -339638732;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn80190177 = -670362934;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn68283267 = -388088058;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn38420424 = -505415263;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn24594241 = -776211865;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn97298376 = -440554697;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn28079867 = -919430031;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn10263667 = -774843716;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn12154070 = -661186863;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn95188172 = -407677492;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn16476803 = -435242865;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn21739322 = -122287654;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn85599924 = -928379824;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn41768796 = -243850200;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn17991487 = 3701110;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn52985768 = -173694399;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn35775112 = -654491655;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn12695206 = -22934374;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn76193073 = -770448352;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn31313105 = -282146241;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn69929413 = -126291142;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn23412578 = -846029399;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn30708985 = -702667184;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn60948204 = -274566516;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn80578724 = -819096287;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn37086991 = -11300021;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn1153914 = -577434368;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn42055155 = -287426812;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn4775078 = -952554302;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn159319 = -573175563;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn16118476 = -893394870;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn49849203 = -963320379;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn90294247 = -34870614;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn74741634 = -919395592;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn17269012 = -680252377;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn75513443 = -270758428;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn85219507 = -280628455;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn77600877 = -893821247;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn17711538 = 59520720;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn34012146 = -557054690;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn89267797 = -928657061;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn32863314 = -265961546;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn16036409 = -46887928;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn61019823 = -670214136;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn60124065 = -920692998;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn91477916 = -491357588;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn40825296 = -214891295;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn80128142 = -438798848;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn87440291 = -571453949;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn88110636 = -309761092;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn9032899 = -209834409;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn2308211 = -575943658;    float UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn62649681 = -198688488;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn57121399 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn26310065;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn26310065 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn5168761;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn5168761 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn16193175;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn16193175 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn21184574;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn21184574 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn41168112;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn41168112 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn73695107;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn73695107 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn6488544;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn6488544 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn51796413;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn51796413 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn55000983;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn55000983 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn7503596;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn7503596 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn48660769;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn48660769 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn13056395;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn13056395 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn51603056;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn51603056 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn86916980;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn86916980 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn37549072;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn37549072 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn93795962;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn93795962 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn63512211;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn63512211 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn23050852;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn23050852 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn47486933;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn47486933 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn45871482;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn45871482 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn44967330;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn44967330 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn31121892;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn31121892 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn88691957;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn88691957 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn94453853;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn94453853 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn35584649;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn35584649 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn99399316;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn99399316 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn55703621;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn55703621 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn53502775;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn53502775 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn16021301;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn16021301 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn42305778;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn42305778 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn31310522;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn31310522 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn17347665;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn17347665 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn43126981;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn43126981 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn28190479;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn28190479 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn56207996;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn56207996 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn76600868;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn76600868 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn13217238;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn13217238 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn26425221;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn26425221 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn21896938;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn21896938 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn5431779;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn5431779 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn41096404;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn41096404 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn44808011;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn44808011 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn15003417;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn15003417 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn38842755;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn38842755 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn4159606;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn4159606 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn60843014;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn60843014 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn82130304;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn82130304 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn80190177;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn80190177 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn68283267;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn68283267 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn38420424;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn38420424 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn24594241;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn24594241 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn97298376;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn97298376 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn28079867;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn28079867 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn10263667;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn10263667 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn12154070;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn12154070 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn95188172;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn95188172 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn16476803;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn16476803 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn21739322;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn21739322 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn85599924;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn85599924 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn41768796;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn41768796 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn17991487;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn17991487 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn52985768;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn52985768 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn35775112;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn35775112 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn12695206;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn12695206 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn76193073;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn76193073 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn31313105;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn31313105 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn69929413;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn69929413 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn23412578;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn23412578 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn30708985;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn30708985 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn60948204;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn60948204 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn80578724;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn80578724 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn37086991;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn37086991 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn1153914;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn1153914 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn42055155;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn42055155 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn4775078;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn4775078 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn159319;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn159319 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn16118476;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn16118476 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn49849203;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn49849203 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn90294247;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn90294247 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn74741634;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn74741634 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn17269012;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn17269012 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn75513443;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn75513443 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn85219507;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn85219507 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn77600877;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn77600877 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn17711538;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn17711538 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn34012146;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn34012146 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn89267797;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn89267797 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn32863314;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn32863314 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn16036409;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn16036409 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn61019823;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn61019823 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn60124065;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn60124065 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn91477916;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn91477916 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn40825296;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn40825296 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn80128142;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn80128142 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn87440291;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn87440291 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn88110636;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn88110636 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn9032899;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn9032899 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn2308211;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn2308211 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn62649681;     UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn62649681 = UWfoasZsHxJOFpiYswOHNJHOztCAvgGEiEmIVCUKhCRjsdPnhvnNzYDLORphqZxarFgACZCpcEZLsYWWdOTAssEJAbtEMmTIEqrn57121399;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void UZuRiOholApHOJhVsAbtSPtQStzhZAWewUondUhGiDHrWaiebYAgbGJEgdypPzPleLKuVtsYSVLOHBurKVGBApCpKjGzVsrioWNK62905799() {     long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc46955553 = -533136585;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc5853756 = -655797150;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc11110274 = -330374234;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc25495769 = -471878270;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc78481500 = -106267682;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc78730264 = -430991024;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc39611972 = 97564167;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc68802567 = -553916728;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc49398601 = -232889595;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc38975745 = -28448883;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc30823527 = -545252654;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc84987259 = -239257167;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc82172278 = -571478210;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc82964097 = -781178670;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc32010775 = -359431135;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc25433041 = -960425223;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc43350136 = -27781073;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc97643166 = 71983023;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc46285444 = -72484016;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc74218505 = -540399937;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc81433609 = -776430141;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc10786691 = -868578512;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc72766945 = -827142967;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc17477135 = -561320307;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc3198286 = -11639875;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc29873159 = -120762754;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc89672615 = -665228448;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc46813471 = -402458792;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc4231620 = 38634567;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc96275134 = -564745125;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc49325429 = -236203338;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc38438290 = -648075186;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc26635738 = -507858769;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc73965096 = -73125407;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc39626494 = -340412428;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc37623332 = -350079958;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc69567760 = -729566407;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc67606921 = -379557146;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc95964790 = -55434913;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc20460885 = -812166835;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc63724190 = -471010757;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc88284248 = -653415519;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc15958496 = -156665156;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc8838575 = -954129146;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc62800523 = -316713400;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc793651 = -610841093;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc39498351 = -593240950;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc4212674 = -720417519;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc42398509 = -19647935;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc37113673 = -694139188;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc27471304 = -92036343;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc1456937 = -711136720;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc66377584 = -136330659;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc80231624 = -148529215;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc81472189 = -449001721;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc36168863 = -664558074;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc33086810 = -828654184;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc93633138 = -769053927;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc22297484 = -360238396;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc48608342 = -985504929;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc89057648 = -765762576;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc92798501 = -499977042;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc64570948 = -492551295;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc53123466 = -668144471;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc89650315 = -792245545;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc92385237 = -897177468;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc58351522 = -731398399;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc8207183 = -398352803;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc43337604 = -340766243;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc94387442 = 90648823;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc55865281 = -130858817;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc75743215 = -648223927;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc1678377 = -872582065;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc25824560 = -260317181;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc10494315 = 30610820;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc93149360 = -23014623;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc94828194 = -611913357;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc63928371 = -873013822;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc54676612 = -144606908;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc2404636 = -400798783;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc90374808 = -527521805;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc85459941 = -944810930;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc4414963 = -282810858;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc67117946 = -267226245;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc68803831 = -372708782;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc47868492 = -525066619;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc72060705 = -411744528;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc46404114 = -259329555;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc92492906 = -624123686;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc3457632 = -675854355;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc4536522 = -521425775;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc75934622 = -960512480;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc45309438 = 80681249;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc47356448 = -69929984;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc31403236 = 53595740;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc70925689 = -971033716;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc23713301 = -60864225;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc62835030 = -488520685;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc19188259 = -61883601;    long PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc70415285 = -533136585;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc46955553 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc5853756;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc5853756 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc11110274;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc11110274 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc25495769;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc25495769 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc78481500;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc78481500 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc78730264;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc78730264 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc39611972;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc39611972 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc68802567;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc68802567 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc49398601;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc49398601 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc38975745;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc38975745 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc30823527;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc30823527 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc84987259;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc84987259 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc82172278;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc82172278 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc82964097;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc82964097 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc32010775;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc32010775 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc25433041;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc25433041 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc43350136;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc43350136 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc97643166;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc97643166 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc46285444;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc46285444 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc74218505;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc74218505 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc81433609;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc81433609 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc10786691;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc10786691 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc72766945;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc72766945 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc17477135;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc17477135 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc3198286;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc3198286 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc29873159;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc29873159 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc89672615;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc89672615 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc46813471;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc46813471 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc4231620;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc4231620 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc96275134;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc96275134 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc49325429;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc49325429 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc38438290;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc38438290 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc26635738;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc26635738 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc73965096;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc73965096 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc39626494;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc39626494 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc37623332;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc37623332 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc69567760;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc69567760 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc67606921;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc67606921 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc95964790;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc95964790 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc20460885;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc20460885 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc63724190;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc63724190 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc88284248;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc88284248 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc15958496;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc15958496 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc8838575;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc8838575 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc62800523;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc62800523 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc793651;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc793651 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc39498351;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc39498351 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc4212674;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc4212674 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc42398509;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc42398509 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc37113673;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc37113673 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc27471304;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc27471304 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc1456937;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc1456937 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc66377584;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc66377584 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc80231624;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc80231624 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc81472189;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc81472189 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc36168863;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc36168863 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc33086810;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc33086810 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc93633138;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc93633138 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc22297484;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc22297484 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc48608342;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc48608342 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc89057648;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc89057648 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc92798501;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc92798501 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc64570948;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc64570948 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc53123466;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc53123466 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc89650315;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc89650315 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc92385237;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc92385237 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc58351522;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc58351522 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc8207183;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc8207183 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc43337604;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc43337604 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc94387442;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc94387442 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc55865281;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc55865281 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc75743215;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc75743215 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc1678377;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc1678377 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc25824560;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc25824560 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc10494315;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc10494315 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc93149360;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc93149360 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc94828194;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc94828194 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc63928371;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc63928371 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc54676612;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc54676612 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc2404636;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc2404636 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc90374808;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc90374808 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc85459941;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc85459941 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc4414963;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc4414963 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc67117946;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc67117946 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc68803831;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc68803831 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc47868492;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc47868492 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc72060705;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc72060705 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc46404114;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc46404114 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc92492906;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc92492906 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc3457632;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc3457632 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc4536522;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc4536522 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc75934622;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc75934622 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc45309438;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc45309438 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc47356448;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc47356448 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc31403236;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc31403236 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc70925689;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc70925689 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc23713301;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc23713301 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc62835030;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc62835030 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc19188259;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc19188259 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc70415285;     PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc70415285 = PoIhgbOGvuwejGeusNloIVjlnsCZdcalnjbWMZVYbwuQnbQOOxqalZCuEXtnYocSTLrAfHhSsMNMaTxLcnLglAzOhgSofhzmTbwc46955553;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void vPxUyaEKrDuFktjjybKQKTNklhLFUaNZrZrszCiDdCzunLzOtMeEsCwbBNiQOyqRqUNLBrjwtHmIUTwaiwLIvIhpdPrlTRoeMhET48740937() {     double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn11197958 = -215075723;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn27572621 = 1238745;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn85485822 = -461310537;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn74156294 = -301267596;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn75276525 = -814654101;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn34088572 = -42692548;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn68755328 = 10283325;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn85320518 = -488917982;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn4411661 = -729473052;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn431637 = -619517595;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn60130510 = -971491756;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn287771 = -740050459;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn6560299 = -33270125;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn76609642 = -178881910;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn66007790 = 87529648;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn80160515 = 13047758;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn48958952 = -754115394;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn33658751 = -582257076;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn28483585 = -719893027;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn11862374 = -663205301;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn95655895 = -265617084;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn53399210 = -189914352;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn47526598 = -376878769;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn88198378 = -225850013;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn85482268 = -649058233;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn11246610 = -694346420;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn67779587 = -945552365;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn9428047 = -143100978;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn59761698 = -663790817;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn99296347 = -496775552;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn67711124 = -264161735;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn93099981 = -316566914;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn189183 = 78246475;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn51498978 = 74017687;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn32015560 = -530628079;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn57075008 = -184439320;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn24830636 = -722338385;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn16341597 = -192016906;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn59894377 = -221941801;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn82114049 = 18854009;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn62720989 = -887152385;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn77160164 = -408129127;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn98192120 = -979528121;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn91112352 = -297929104;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn78795467 = -212123318;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn71455341 = -412630705;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn46083025 = -563754778;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn93565852 = -416774815;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn72453910 = 66979430;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn72179507 = -739192051;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn19645306 = -108553828;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn65608311 = -850128143;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn92831537 = -256262080;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn73734743 = -694484468;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn53085041 = -504968586;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn57798748 = 74838628;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn80046022 = -621882487;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn97287444 = -135460524;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn88674026 = -652209364;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn64029916 = -20307681;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn66308985 = -97140183;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn59327282 = -846615698;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn25558821 = -825127165;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn5115313 = -132697501;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn32720513 = -255355860;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn67030528 = -554924842;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn98589 = -718296934;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn55061321 = -7287813;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn44594083 = -648253831;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn8932783 = -728031033;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn55329880 = -264613858;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn32617356 = -462098489;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn73764373 = -260315275;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn46369536 = -638747037;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn49141384 = -776052916;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn18495732 = -857487957;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn55207089 = -210386231;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn56414246 = 21050335;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn9402911 = 86273304;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn14026927 = -136427529;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn65163584 = -30591642;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn74213734 = -428777551;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn36974136 = -110080409;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn87582190 = -924598766;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn79651042 = -288221725;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn2102813 = -414033592;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn268445 = 39695166;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn26454439 = -227269057;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn98413936 = -421013727;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn74216811 = -505466707;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn77028985 = -562556834;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn27543191 = -486877861;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn27667571 = -539807543;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn95864461 = -101634121;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn15805065 = -884005808;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn3393708 = 59463312;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn51601344 = -583001962;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn93076808 = -746830621;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn58391839 = 57426756;    double ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn11764940 = -215075723;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn11197958 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn27572621;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn27572621 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn85485822;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn85485822 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn74156294;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn74156294 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn75276525;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn75276525 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn34088572;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn34088572 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn68755328;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn68755328 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn85320518;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn85320518 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn4411661;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn4411661 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn431637;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn431637 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn60130510;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn60130510 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn287771;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn287771 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn6560299;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn6560299 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn76609642;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn76609642 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn66007790;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn66007790 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn80160515;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn80160515 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn48958952;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn48958952 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn33658751;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn33658751 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn28483585;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn28483585 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn11862374;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn11862374 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn95655895;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn95655895 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn53399210;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn53399210 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn47526598;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn47526598 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn88198378;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn88198378 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn85482268;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn85482268 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn11246610;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn11246610 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn67779587;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn67779587 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn9428047;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn9428047 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn59761698;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn59761698 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn99296347;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn99296347 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn67711124;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn67711124 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn93099981;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn93099981 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn189183;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn189183 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn51498978;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn51498978 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn32015560;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn32015560 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn57075008;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn57075008 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn24830636;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn24830636 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn16341597;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn16341597 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn59894377;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn59894377 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn82114049;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn82114049 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn62720989;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn62720989 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn77160164;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn77160164 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn98192120;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn98192120 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn91112352;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn91112352 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn78795467;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn78795467 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn71455341;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn71455341 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn46083025;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn46083025 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn93565852;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn93565852 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn72453910;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn72453910 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn72179507;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn72179507 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn19645306;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn19645306 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn65608311;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn65608311 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn92831537;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn92831537 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn73734743;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn73734743 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn53085041;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn53085041 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn57798748;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn57798748 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn80046022;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn80046022 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn97287444;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn97287444 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn88674026;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn88674026 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn64029916;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn64029916 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn66308985;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn66308985 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn59327282;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn59327282 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn25558821;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn25558821 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn5115313;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn5115313 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn32720513;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn32720513 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn67030528;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn67030528 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn98589;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn98589 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn55061321;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn55061321 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn44594083;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn44594083 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn8932783;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn8932783 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn55329880;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn55329880 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn32617356;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn32617356 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn73764373;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn73764373 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn46369536;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn46369536 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn49141384;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn49141384 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn18495732;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn18495732 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn55207089;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn55207089 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn56414246;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn56414246 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn9402911;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn9402911 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn14026927;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn14026927 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn65163584;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn65163584 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn74213734;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn74213734 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn36974136;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn36974136 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn87582190;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn87582190 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn79651042;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn79651042 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn2102813;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn2102813 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn268445;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn268445 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn26454439;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn26454439 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn98413936;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn98413936 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn74216811;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn74216811 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn77028985;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn77028985 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn27543191;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn27543191 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn27667571;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn27667571 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn95864461;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn95864461 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn15805065;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn15805065 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn3393708;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn3393708 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn51601344;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn51601344 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn93076808;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn93076808 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn58391839;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn58391839 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn11764940;     ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn11764940 = ynMCciLkQoHiRIhRmhWhnYimAxcbSHwmOdcukJgNwwYoIJUKkXreLWgZFaBeFkSIrVTngnhdBIlqztdrdSrvPRarxCoJIBHLdQxn11197958;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void IZhJnEdFWchKEetfTeTXaBNkiIGSRoPJFEgiFDvEpSMPlDfYjErfZaZGLwbfxKoiMYFAxKaQiOiHoyzechjNwjjefZXIbPGeeeCM43296596() {     double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax26903072 = -671389166;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax45667145 = -671760327;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax15170910 = -357695148;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax78456659 = -315303142;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax66515368 = -171255151;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax66689686 = -889739471;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax37286947 = 85930178;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax22423257 = -210224917;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax36292805 = -947235805;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax20485204 = -160760514;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax70936488 = -649811872;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax84646611 = -398825863;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax53340123 = -131412826;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax60432433 = -496720139;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax65523159 = -248850105;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax86625847 = -456319240;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax48572009 = -523395000;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax99130900 = -838455351;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax71290967 = -387259587;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax12421852 = -797547437;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax64256988 = -142208331;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax31280165 = -352735800;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax21407211 = -186388638;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax41483626 = -413643891;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax15713286 = -186759173;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax76100788 = -42876625;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax42356413 = -571694207;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax31900747 = 58379742;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax41526355 = -108354031;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax76497788 = 44481121;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax26633841 = -756568544;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax14380686 = -383008929;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax15797699 = -871744038;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax29207531 = -950260175;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax51375120 = -41713576;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax57926054 = 15170808;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax86650957 = -818319612;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax86189246 = 97946346;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax46664570 = -423585;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax7282758 = 34744231;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax11427612 = -156017749;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax61511878 = -925452927;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax67492541 = -942690352;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax51799470 = -187392335;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax46758813 = -736913793;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax98722625 = -130846818;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax53972562 = -625986761;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax67448663 = -145375271;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax69842273 = -32400004;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax75315331 = -631236428;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax72406447 = -310149482;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax51187256 = -274779904;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax35805944 = -428067632;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax5790986 = -565023383;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax79039608 = -865558553;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax95622907 = -218653367;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax24259935 = -385371690;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax73687283 = -944051257;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax62743374 = -28543970;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax90414579 = -28378527;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax24333273 = -218045265;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax5386201 = -972449565;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax80896901 = -1870887;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax59795016 = -891716926;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax93851363 = -404191971;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax56555802 = -166802944;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax68848913 = -527081826;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax24132592 = -181152651;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax9057314 = -355006564;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax7597105 = -164020913;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax99974890 = -637999628;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax62382762 = -521341346;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax52466330 = -738031767;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax64008209 = -322003819;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax994241 = -541529689;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax2745110 = -216755405;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax63787623 = -410045448;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax69607740 = -998996303;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax94724813 = -676730099;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax16990660 = 44087645;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax22128226 = -416889864;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax74907749 = -326318936;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax62058473 = -909220254;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax66211024 = -477117603;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax4091342 = -645369397;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax75446584 = -381788641;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax78574741 = -954941298;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax10006714 = -206720656;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax50167923 = 15298377;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax55752212 = -823060209;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax33666120 = -599457502;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax12963674 = -874268355;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax23445872 = -873509685;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax56249990 = -972045059;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax82949485 = -747210505;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax6041411 = -183568184;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax80614977 = -823582040;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax7697526 = 49026574;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax57948107 = -783200365;    double xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax90203010 = -671389166;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax26903072 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax45667145;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax45667145 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax15170910;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax15170910 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax78456659;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax78456659 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax66515368;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax66515368 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax66689686;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax66689686 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax37286947;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax37286947 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax22423257;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax22423257 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax36292805;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax36292805 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax20485204;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax20485204 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax70936488;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax70936488 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax84646611;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax84646611 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax53340123;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax53340123 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax60432433;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax60432433 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax65523159;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax65523159 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax86625847;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax86625847 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax48572009;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax48572009 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax99130900;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax99130900 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax71290967;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax71290967 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax12421852;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax12421852 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax64256988;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax64256988 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax31280165;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax31280165 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax21407211;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax21407211 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax41483626;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax41483626 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax15713286;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax15713286 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax76100788;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax76100788 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax42356413;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax42356413 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax31900747;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax31900747 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax41526355;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax41526355 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax76497788;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax76497788 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax26633841;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax26633841 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax14380686;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax14380686 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax15797699;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax15797699 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax29207531;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax29207531 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax51375120;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax51375120 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax57926054;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax57926054 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax86650957;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax86650957 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax86189246;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax86189246 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax46664570;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax46664570 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax7282758;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax7282758 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax11427612;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax11427612 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax61511878;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax61511878 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax67492541;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax67492541 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax51799470;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax51799470 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax46758813;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax46758813 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax98722625;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax98722625 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax53972562;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax53972562 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax67448663;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax67448663 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax69842273;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax69842273 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax75315331;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax75315331 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax72406447;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax72406447 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax51187256;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax51187256 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax35805944;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax35805944 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax5790986;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax5790986 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax79039608;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax79039608 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax95622907;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax95622907 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax24259935;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax24259935 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax73687283;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax73687283 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax62743374;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax62743374 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax90414579;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax90414579 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax24333273;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax24333273 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax5386201;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax5386201 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax80896901;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax80896901 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax59795016;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax59795016 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax93851363;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax93851363 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax56555802;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax56555802 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax68848913;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax68848913 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax24132592;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax24132592 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax9057314;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax9057314 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax7597105;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax7597105 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax99974890;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax99974890 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax62382762;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax62382762 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax52466330;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax52466330 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax64008209;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax64008209 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax994241;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax994241 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax2745110;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax2745110 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax63787623;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax63787623 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax69607740;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax69607740 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax94724813;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax94724813 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax16990660;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax16990660 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax22128226;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax22128226 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax74907749;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax74907749 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax62058473;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax62058473 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax66211024;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax66211024 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax4091342;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax4091342 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax75446584;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax75446584 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax78574741;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax78574741 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax10006714;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax10006714 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax50167923;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax50167923 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax55752212;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax55752212 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax33666120;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax33666120 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax12963674;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax12963674 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax23445872;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax23445872 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax56249990;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax56249990 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax82949485;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax82949485 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax6041411;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax6041411 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax80614977;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax80614977 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax7697526;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax7697526 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax57948107;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax57948107 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax90203010;     xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax90203010 = xfaYebxLsRSMxwCQbbldoaVXnSoTHDdJwmgxHLiUmXmNHKKeMqDCsYXXJDkLneDonJQLQFJPsQrQoHQdIJGQAaQXOJnDbWhIUxax26903072;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void MSPjwXNONccRDDThHWdckkSRMipRHcdtFRdqoLLpgpdcVRwGrQdwhvCcAxWNhwHkziLmZcswLnwvGpvumiZrzgLKOBpqpyIjisdn92268467() {     double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY13259854 = -645851740;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY32435187 = -116402699;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY24824915 = -696374218;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY22688312 = -519477076;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY79204532 = -411262549;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY89290561 = -338367545;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY24197461 = -937198679;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY16368375 = -278348942;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY14039004 = -158345886;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY63505230 = -3098785;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY23991628 = -363033917;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY14552024 = -373365061;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY79947334 = -139092584;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY28102258 = -615586579;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY47758547 = -734979860;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY25619820 = -495669824;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY42037892 = -201859735;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY84896544 = 16180178;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY95806130 = -761814090;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY71768038 = -374255879;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY87993834 = 72521282;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY70556884 = -846842553;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY97231631 = -563741651;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY28618411 = -193838675;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY10068691 = -500364800;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY71552547 = -589691274;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY25194024 = -593013823;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY62155672 = -245263304;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY55433500 = -947275461;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY92871325 = 27342724;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY92628552 = -997409038;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY80172850 = -777748107;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY14351576 = -668696159;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY72281059 = -438067721;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY48758917 = -499745852;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY18770361 = -964445612;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY92262765 = -2894046;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY8829385 = -496203509;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY11968052 = -834634005;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY27554764 = -566973910;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY20207517 = -908901991;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY74346583 = -112165795;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY85759922 = -892606635;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY18994890 = -148830538;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY10676218 = -470350662;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY32395745 = -707400087;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY87993302 = -804288543;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY6945685 = -326106022;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY72672089 = -117543408;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY86878641 = -806289684;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY93356912 = -58317258;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY78637704 = -768651386;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY33856008 = -527671225;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY28074532 = 3556206;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY4274634 = -237089434;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY42702969 = -799009187;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY35203556 = -552661049;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY96206504 = -402535544;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY12619621 = 80887723;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY7651985 = -821571275;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY64096537 = -745353722;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY62041789 = -591935375;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY60934875 = -331073482;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY21167678 = -85688610;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY70876677 = -5689747;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY43818777 = -585285811;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY200448 = -704668902;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY7666276 = -701024863;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY79343340 = -15840727;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY28988187 = -770534249;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY33357055 = -392775778;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY33208507 = -705656226;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY72928493 = -149185817;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY68251366 = -94840180;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY51560521 = -465353888;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY13647251 = -815312923;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY84796961 = -954235919;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY78236742 = -314911113;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY17942194 = -723488013;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY77672946 = -792964713;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY83559244 = -785402732;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY18248340 = -166907802;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY89483583 = -27719897;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY68554859 = -40985778;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY99514413 = -914340019;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY13990849 = -128757653;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY46316842 = -150076882;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY86277043 = -572252366;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY68006425 = -100978288;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY6055948 = -700736666;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY83566804 = -311784564;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY96056261 = -600358503;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY96209763 = -477091233;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY4316067 = 86937269;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY63458227 = -821620188;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY58165728 = -216966617;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY13411709 = -781092314;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY64592244 = -706918026;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY48118212 = -43140792;    double FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY66857441 = -645851740;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY13259854 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY32435187;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY32435187 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY24824915;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY24824915 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY22688312;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY22688312 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY79204532;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY79204532 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY89290561;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY89290561 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY24197461;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY24197461 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY16368375;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY16368375 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY14039004;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY14039004 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY63505230;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY63505230 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY23991628;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY23991628 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY14552024;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY14552024 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY79947334;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY79947334 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY28102258;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY28102258 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY47758547;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY47758547 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY25619820;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY25619820 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY42037892;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY42037892 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY84896544;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY84896544 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY95806130;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY95806130 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY71768038;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY71768038 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY87993834;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY87993834 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY70556884;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY70556884 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY97231631;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY97231631 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY28618411;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY28618411 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY10068691;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY10068691 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY71552547;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY71552547 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY25194024;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY25194024 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY62155672;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY62155672 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY55433500;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY55433500 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY92871325;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY92871325 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY92628552;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY92628552 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY80172850;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY80172850 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY14351576;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY14351576 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY72281059;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY72281059 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY48758917;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY48758917 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY18770361;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY18770361 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY92262765;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY92262765 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY8829385;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY8829385 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY11968052;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY11968052 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY27554764;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY27554764 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY20207517;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY20207517 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY74346583;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY74346583 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY85759922;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY85759922 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY18994890;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY18994890 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY10676218;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY10676218 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY32395745;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY32395745 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY87993302;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY87993302 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY6945685;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY6945685 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY72672089;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY72672089 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY86878641;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY86878641 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY93356912;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY93356912 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY78637704;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY78637704 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY33856008;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY33856008 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY28074532;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY28074532 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY4274634;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY4274634 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY42702969;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY42702969 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY35203556;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY35203556 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY96206504;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY96206504 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY12619621;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY12619621 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY7651985;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY7651985 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY64096537;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY64096537 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY62041789;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY62041789 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY60934875;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY60934875 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY21167678;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY21167678 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY70876677;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY70876677 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY43818777;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY43818777 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY200448;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY200448 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY7666276;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY7666276 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY79343340;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY79343340 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY28988187;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY28988187 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY33357055;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY33357055 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY33208507;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY33208507 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY72928493;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY72928493 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY68251366;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY68251366 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY51560521;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY51560521 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY13647251;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY13647251 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY84796961;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY84796961 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY78236742;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY78236742 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY17942194;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY17942194 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY77672946;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY77672946 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY83559244;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY83559244 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY18248340;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY18248340 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY89483583;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY89483583 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY68554859;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY68554859 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY99514413;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY99514413 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY13990849;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY13990849 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY46316842;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY46316842 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY86277043;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY86277043 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY68006425;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY68006425 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY6055948;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY6055948 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY83566804;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY83566804 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY96056261;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY96056261 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY96209763;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY96209763 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY4316067;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY4316067 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY63458227;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY63458227 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY58165728;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY58165728 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY13411709;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY13411709 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY64592244;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY64592244 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY48118212;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY48118212 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY66857441;     FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY66857441 = FJgOXgluyHVLBbbzWQkSWBYYmCGNfMMNMIocBLTTuRLGfeoOXINRIKQElIcVZYOmuHytmyxLwRXEpRsHNspWzuFCrkqtiytWCZLY13259854;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void qEUOufXoYUFimeiVhQkMykkUbGPnpwAZHnqbzsggOzQUiyAONvDFOGGGUAnnQkzcTfbznGWojExzUPlcFAZtsvLAgkHzFsQeznBC60002170() {     long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu83898577 = 90950847;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu78253337 = -73236249;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu7420259 = 47074095;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu32779320 = -350697126;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu18335058 = -31379539;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu48901189 = -538814320;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu79671028 = -345047323;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu11638857 = -33520666;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu25384386 = -778985354;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu79750718 = -677807877;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu6881999 = -316879991;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu88681948 = -160085579;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu23480549 = -91946591;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu19637732 = -6920892;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu20822785 = -810155567;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu7277555 = -731244712;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu47596237 = -850274005;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu51191105 = -767129261;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu18370452 = -504966565;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu74702273 = 59328914;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu2468437 = -357090606;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu88545180 = 1888375;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu72932234 = -949500479;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu49768165 = -648080626;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu31078458 = 70342806;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu35298282 = -504387576;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu73897973 = -489791024;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu40744947 = -868321049;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu12932882 = -333774308;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu84223161 = -981915094;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu66525907 = -420029193;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu94132026 = -789688793;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu68201782 = 80453800;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu38211709 = -854960871;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu91499229 = -865320481;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu64419998 = -103203653;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu59936987 = -390794010;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu1457233 = -270841975;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu61128533 = -15725474;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu66403848 = -403445645;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu55992136 = -368808666;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu74224896 = -269139029;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu55293602 = -897621195;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu43966986 = -434734396;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu74666381 = -625515860;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu19657516 = -89826584;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu69520091 = -782919586;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu23327057 = -895759030;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu15430320 = -330835100;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu35396974 = -263348335;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu88064976 = -961999393;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu49603724 = -402162604;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu39828362 = -239577284;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu38802378 = -621164993;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu57533733 = -961828904;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu95353397 = -910937528;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu5321104 = -123735770;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu57652094 = -304845280;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu1700862 = -321039932;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu83036775 = -526991964;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu75003216 = 50976703;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu38926082 = -476726274;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu98705975 = -699746359;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu41161225 = -797070260;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu13224812 = -157778685;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu12749973 = -527191199;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu20480166 = -140539379;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu85268840 = -236985721;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu28138502 = -141600411;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu56402786 = -606951915;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu47340568 = -240450702;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu46139005 = -479432030;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu90062571 = -651403788;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu51966604 = -1520920;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu18710137 = -571862420;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu28243540 = 12048423;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu33251578 = -100490430;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu28965248 = -414766083;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu75101783 = 77435233;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu11420943 = -839830611;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu65778191 = -721467990;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu50570917 = -594031995;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu25314628 = -437485949;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu77535908 = 29574027;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu96158185 = 80084298;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu16922183 = 82133411;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu54303664 = -450111509;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu29399405 = -298381207;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu80677975 = -893131968;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu96145831 = -954382954;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu59098894 = -979467883;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu2284893 = 14051269;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu99756370 = -949802043;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu78091757 = -488733511;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu91400632 = -354422349;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu17066055 = -892082392;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu75518921 = -569392671;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu14132378 = -550936;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu30742175 = -176955712;    long WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu61916408 = 90950847;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu83898577 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu78253337;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu78253337 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu7420259;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu7420259 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu32779320;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu32779320 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu18335058;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu18335058 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu48901189;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu48901189 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu79671028;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu79671028 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu11638857;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu11638857 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu25384386;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu25384386 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu79750718;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu79750718 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu6881999;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu6881999 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu88681948;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu88681948 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu23480549;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu23480549 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu19637732;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu19637732 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu20822785;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu20822785 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu7277555;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu7277555 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu47596237;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu47596237 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu51191105;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu51191105 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu18370452;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu18370452 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu74702273;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu74702273 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu2468437;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu2468437 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu88545180;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu88545180 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu72932234;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu72932234 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu49768165;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu49768165 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu31078458;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu31078458 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu35298282;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu35298282 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu73897973;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu73897973 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu40744947;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu40744947 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu12932882;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu12932882 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu84223161;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu84223161 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu66525907;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu66525907 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu94132026;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu94132026 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu68201782;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu68201782 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu38211709;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu38211709 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu91499229;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu91499229 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu64419998;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu64419998 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu59936987;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu59936987 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu1457233;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu1457233 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu61128533;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu61128533 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu66403848;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu66403848 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu55992136;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu55992136 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu74224896;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu74224896 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu55293602;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu55293602 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu43966986;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu43966986 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu74666381;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu74666381 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu19657516;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu19657516 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu69520091;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu69520091 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu23327057;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu23327057 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu15430320;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu15430320 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu35396974;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu35396974 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu88064976;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu88064976 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu49603724;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu49603724 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu39828362;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu39828362 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu38802378;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu38802378 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu57533733;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu57533733 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu95353397;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu95353397 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu5321104;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu5321104 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu57652094;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu57652094 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu1700862;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu1700862 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu83036775;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu83036775 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu75003216;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu75003216 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu38926082;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu38926082 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu98705975;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu98705975 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu41161225;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu41161225 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu13224812;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu13224812 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu12749973;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu12749973 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu20480166;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu20480166 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu85268840;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu85268840 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu28138502;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu28138502 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu56402786;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu56402786 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu47340568;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu47340568 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu46139005;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu46139005 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu90062571;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu90062571 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu51966604;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu51966604 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu18710137;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu18710137 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu28243540;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu28243540 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu33251578;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu33251578 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu28965248;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu28965248 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu75101783;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu75101783 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu11420943;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu11420943 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu65778191;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu65778191 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu50570917;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu50570917 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu25314628;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu25314628 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu77535908;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu77535908 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu96158185;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu96158185 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu16922183;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu16922183 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu54303664;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu54303664 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu29399405;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu29399405 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu80677975;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu80677975 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu96145831;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu96145831 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu59098894;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu59098894 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu2284893;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu2284893 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu99756370;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu99756370 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu78091757;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu78091757 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu91400632;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu91400632 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu17066055;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu17066055 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu75518921;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu75518921 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu14132378;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu14132378 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu30742175;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu30742175 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu61916408;     WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu61916408 = WAbGZPVKIGlFOKiPKGkXUaSDzWjrnTzhNiFEZSTdZAsayCluKBUtDdiwiXHDfeXpHErfEdprmHyUhsExlohlyFdjAJWCLPxvdCSu83898577;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void iGdcLYaIMdvHrbSmmAmmxXMkEDMvKukUdDSdTHLZLUvSNZQFufGIvnyKutXHMfxRsbdoIpTfpMadbtWfmGAfocYqFIjZjDNgWeam93594776() {     long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo18799121 = -336613280;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo30073404 = -345248528;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo60451515 = -569028055;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo36291271 = -549684011;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo27740302 = -54816983;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo59453827 = -152664184;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo58645945 = -726567408;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo15785138 = -611161693;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo43522336 = -555270070;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo67533558 = -833165069;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo58117537 = -818548950;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo35237354 = -141164301;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo95843042 = -541617091;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo43286090 = 39500710;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo92367710 = -550231936;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo19969122 = -94959667;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo91205121 = -566178886;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo84499650 = 86536065;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo61848105 = -141581251;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo99059088 = 6181697;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo92157053 = -44490315;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo14257199 = -240740887;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo12757299 = -608121585;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo10688837 = -24090716;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo49044140 = -916286387;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo30695236 = -168049758;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo90044149 = -864493004;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo75738222 = -505120014;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo87927001 = -684487594;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo50326600 = -696909219;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo58570920 = -909328041;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo8581322 = -394655921;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo39248164 = -178458350;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo80827727 = -394665729;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo79554492 = -643168987;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo1036744 = 62976184;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo47049980 = -472505818;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo33066717 = -972152163;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo68277813 = -525279583;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo51287419 = -580601477;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo27206552 = 18974726;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo5886143 = -699449625;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo26210828 = -48107958;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo73517166 = -293544883;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo2597332 = -356312771;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo56297073 = -937908680;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo74538176 = -388222592;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo2910865 = -435485264;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo32268784 = -666207843;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo58844871 = 95614810;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo35168933 = -946533992;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo41079346 = -128228001;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo45909623 = -395252739;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo12282531 = -196081892;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo1437724 = -271837406;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo4541922 = 4127607;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo17316105 = -737126944;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo49762678 = -444937339;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo87247131 = -633397625;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo97045065 = -886767226;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo69409678 = -288171181;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo82907722 = -121447394;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo27858137 = -926674100;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo93195735 = -858360852;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo8962638 = -923837028;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo49536215 = -323893029;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo95989189 = -962705952;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo15015315 = -46951363;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo63731597 = -317330304;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo91330966 = -513208121;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo72919142 = -622453850;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo58138404 = -594026724;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo16221837 = -388184352;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo10560687 = -560979775;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo71852537 = 87206971;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo86270911 = -345040691;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo88046371 = -92632930;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo39240133 = -214576702;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo8091505 = -667777945;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo92747066 = -978377708;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo56157060 = -779827167;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo87133284 = -329007740;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo43469439 = -838912172;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo29082131 = -680102404;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo15157668 = -750375227;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo17491575 = -681100040;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo62671698 = -999403183;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo26965634 = -982376458;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo79390004 = -22828324;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo75012571 = -547296595;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo83720638 = -199896872;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo97287301 = 72431521;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo45819586 = -238754539;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo71232747 = -638512358;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo81877740 = -192430296;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo44298829 = -859577880;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo78028006 = -772775525;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo33015092 = -189747107;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo64554528 = -369707855;    long vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo53061116 = -336613280;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo18799121 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo30073404;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo30073404 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo60451515;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo60451515 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo36291271;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo36291271 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo27740302;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo27740302 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo59453827;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo59453827 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo58645945;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo58645945 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo15785138;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo15785138 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo43522336;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo43522336 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo67533558;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo67533558 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo58117537;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo58117537 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo35237354;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo35237354 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo95843042;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo95843042 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo43286090;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo43286090 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo92367710;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo92367710 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo19969122;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo19969122 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo91205121;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo91205121 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo84499650;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo84499650 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo61848105;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo61848105 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo99059088;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo99059088 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo92157053;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo92157053 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo14257199;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo14257199 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo12757299;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo12757299 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo10688837;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo10688837 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo49044140;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo49044140 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo30695236;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo30695236 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo90044149;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo90044149 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo75738222;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo75738222 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo87927001;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo87927001 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo50326600;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo50326600 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo58570920;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo58570920 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo8581322;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo8581322 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo39248164;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo39248164 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo80827727;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo80827727 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo79554492;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo79554492 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo1036744;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo1036744 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo47049980;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo47049980 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo33066717;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo33066717 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo68277813;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo68277813 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo51287419;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo51287419 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo27206552;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo27206552 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo5886143;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo5886143 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo26210828;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo26210828 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo73517166;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo73517166 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo2597332;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo2597332 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo56297073;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo56297073 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo74538176;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo74538176 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo2910865;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo2910865 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo32268784;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo32268784 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo58844871;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo58844871 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo35168933;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo35168933 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo41079346;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo41079346 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo45909623;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo45909623 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo12282531;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo12282531 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo1437724;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo1437724 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo4541922;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo4541922 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo17316105;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo17316105 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo49762678;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo49762678 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo87247131;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo87247131 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo97045065;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo97045065 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo69409678;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo69409678 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo82907722;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo82907722 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo27858137;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo27858137 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo93195735;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo93195735 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo8962638;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo8962638 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo49536215;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo49536215 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo95989189;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo95989189 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo15015315;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo15015315 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo63731597;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo63731597 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo91330966;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo91330966 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo72919142;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo72919142 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo58138404;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo58138404 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo16221837;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo16221837 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo10560687;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo10560687 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo71852537;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo71852537 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo86270911;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo86270911 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo88046371;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo88046371 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo39240133;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo39240133 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo8091505;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo8091505 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo92747066;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo92747066 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo56157060;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo56157060 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo87133284;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo87133284 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo43469439;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo43469439 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo29082131;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo29082131 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo15157668;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo15157668 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo17491575;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo17491575 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo62671698;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo62671698 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo26965634;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo26965634 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo79390004;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo79390004 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo75012571;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo75012571 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo83720638;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo83720638 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo97287301;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo97287301 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo45819586;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo45819586 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo71232747;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo71232747 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo81877740;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo81877740 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo44298829;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo44298829 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo78028006;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo78028006 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo33015092;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo33015092 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo64554528;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo64554528 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo53061116;     vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo53061116 = vcwEMbTiMUiAWsVFeRdQCnjAGtwmzuGxTkkKKvTMJqekZiDrhIBEAJjHswySfgjROlhlHKvjyQHMJybYqEErtBwJBcROxBhcHMWo18799121;}
// Junk Finished
