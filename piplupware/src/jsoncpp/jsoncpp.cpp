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
void IcniAMFDkmPegPTVgMybgqkOGeBTbcFd97845512() {     long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb15454637 = -679787981;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb83738920 = -990016271;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb53524471 = -514676966;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb1280901 = -924599178;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb51368677 = -710203429;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb78185197 = -629628729;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb57287844 = -259621076;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb29446465 = -656332751;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb61480422 = -632671372;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb32433191 = -508557109;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb27303801 = -197586034;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb77390465 = -82420425;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb22824264 = -367279380;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb4588500 = -14868564;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb50022970 = -869901575;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb27862404 = -507244697;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb36906421 = -178674814;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb8038169 = -823769654;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb59385895 = -775164543;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb52600831 = -335661542;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb82511667 = -404068762;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb7533138 = -22975499;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb58879887 = -638903993;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb56674558 = -235882318;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb50676784 = -973718355;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb30303912 = -805457431;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb80287829 = -545346987;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb76626563 = -206587140;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb57059878 = 54513473;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb19264074 = -432228056;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb84448441 = -656917090;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb66523702 = -949039623;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb16870580 = -208106936;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb2665600 = -89582209;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb21522378 = -648698165;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb86246381 = -908369739;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb53513315 = -901207290;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb73089223 = 90548074;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb78487246 = -669774465;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb30709201 = -30826904;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb40823240 = -776885142;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb11854450 = -171017408;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb72558361 = -578634386;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb17437179 = -530793593;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb17344701 = -552804309;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb89630069 = -142318746;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb21066276 = 90576794;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb27186398 = -637795273;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb70631209 = -498451137;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb694233 = -31825142;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb97487313 = -59543630;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb96858097 = -742083856;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb59919171 = -891073306;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb45171103 = -900361731;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb15333845 = -973720607;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb7921500 = -556812482;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb24859034 = -251112278;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb96849913 = -178794649;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb50604117 = -950880823;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb21064765 = -904745999;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb97897368 = 15718258;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb80661280 = 46966064;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb72386586 = -610846225;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb42216348 = -100443317;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb47984749 = -851640019;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb60780098 = -248546411;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb60519886 = -874313490;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb20158664 = -177697171;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb83066122 = -366170400;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb63776589 = -961531837;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb74349089 = -606037407;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb63817198 = -169222889;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb29550923 = -53995190;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb28676694 = -644337639;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb11777591 = -558776401;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb70657218 = -133051355;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb34974776 = -444341114;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb41442708 = -8110400;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb39329857 = -683078009;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb61046714 = -731399610;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb9237636 = -796034226;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb53101431 = -907551715;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb5995355 = -708136003;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb56365646 = -913661386;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb21776761 = -272684426;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb87590344 = -914833235;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb6604532 = 42033682;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb71699476 = -307745205;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb87331754 = -115861602;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb13600878 = 8114317;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb61387348 = -557257461;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb56663401 = -622412642;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb22485106 = 41428897;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb57422481 = -765028467;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb32811833 = 53454838;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb60161960 = -723851206;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb39467863 = -560171184;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb30342014 = -378191069;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb69452430 = -679153575;    long DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb56564603 = -679787981;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb15454637 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb83738920;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb83738920 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb53524471;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb53524471 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb1280901;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb1280901 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb51368677;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb51368677 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb78185197;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb78185197 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb57287844;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb57287844 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb29446465;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb29446465 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb61480422;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb61480422 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb32433191;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb32433191 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb27303801;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb27303801 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb77390465;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb77390465 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb22824264;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb22824264 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb4588500;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb4588500 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb50022970;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb50022970 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb27862404;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb27862404 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb36906421;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb36906421 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb8038169;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb8038169 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb59385895;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb59385895 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb52600831;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb52600831 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb82511667;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb82511667 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb7533138;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb7533138 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb58879887;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb58879887 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb56674558;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb56674558 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb50676784;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb50676784 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb30303912;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb30303912 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb80287829;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb80287829 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb76626563;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb76626563 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb57059878;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb57059878 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb19264074;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb19264074 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb84448441;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb84448441 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb66523702;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb66523702 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb16870580;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb16870580 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb2665600;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb2665600 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb21522378;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb21522378 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb86246381;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb86246381 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb53513315;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb53513315 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb73089223;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb73089223 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb78487246;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb78487246 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb30709201;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb30709201 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb40823240;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb40823240 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb11854450;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb11854450 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb72558361;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb72558361 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb17437179;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb17437179 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb17344701;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb17344701 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb89630069;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb89630069 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb21066276;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb21066276 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb27186398;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb27186398 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb70631209;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb70631209 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb694233;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb694233 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb97487313;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb97487313 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb96858097;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb96858097 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb59919171;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb59919171 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb45171103;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb45171103 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb15333845;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb15333845 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb7921500;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb7921500 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb24859034;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb24859034 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb96849913;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb96849913 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb50604117;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb50604117 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb21064765;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb21064765 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb97897368;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb97897368 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb80661280;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb80661280 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb72386586;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb72386586 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb42216348;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb42216348 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb47984749;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb47984749 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb60780098;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb60780098 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb60519886;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb60519886 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb20158664;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb20158664 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb83066122;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb83066122 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb63776589;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb63776589 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb74349089;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb74349089 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb63817198;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb63817198 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb29550923;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb29550923 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb28676694;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb28676694 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb11777591;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb11777591 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb70657218;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb70657218 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb34974776;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb34974776 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb41442708;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb41442708 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb39329857;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb39329857 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb61046714;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb61046714 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb9237636;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb9237636 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb53101431;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb53101431 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb5995355;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb5995355 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb56365646;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb56365646 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb21776761;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb21776761 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb87590344;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb87590344 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb6604532;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb6604532 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb71699476;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb71699476 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb87331754;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb87331754 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb13600878;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb13600878 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb61387348;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb61387348 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb56663401;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb56663401 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb22485106;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb22485106 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb57422481;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb57422481 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb32811833;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb32811833 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb60161960;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb60161960 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb39467863;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb39467863 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb30342014;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb30342014 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb69452430;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb69452430 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb56564603;     DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb56564603 = DxVphVxWqdNonQconjvwvceSrYcWLlcIhlaiueoKPblDcwbpBMgrhIiaTCxItaLFBFHCpttIvkgTrb15454637;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void pLmWpgfxTcRaSjvXGjoZeVGVUoWLVIUP70805264() {     long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn14889741 = -513599350;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn60859224 = -324072069;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn70482462 = -322240654;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn17649359 = -22975822;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn13328754 = 3602650;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn87320395 = -780563509;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn94152689 = -500951893;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn40675235 = -193916955;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn60840930 = -734270751;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn1952832 = -636034173;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn60678209 = -856719336;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn37669399 = 65143262;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn88805025 = -286595849;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn28940215 = -337236604;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn8545489 = -69265781;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn31577218 = -567943695;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn80532128 = -287219713;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn64587055 = -293747881;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn22741488 = -713198074;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn55194190 = -574272145;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn38782846 = -144660157;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn8119897 = -688960350;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn65057968 = -18850756;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn67452392 = -81988326;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn89067203 = -163056203;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn31576771 = -688748735;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn6235013 = -314564539;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn6294939 = -282580919;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn98064230 = -368175326;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn55923972 = -888146384;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn91322901 = -981328686;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn27873837 = -72857099;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn61151373 = -521367324;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn15816030 = -541274986;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn26127226 = -734760345;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn14130473 = -607390342;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn3155859 = -117876436;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn23400985 = -623369211;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn90559560 = -41133714;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn23194133 = -352151876;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn66441715 = -707846734;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn39848229 = -100574794;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn14375569 = -735070617;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn66087918 = -442236113;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn29277246 = -930349461;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn20736267 = -763521880;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn34436990 = -612020473;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn64427475 = -619756271;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn83235396 = -733850861;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn19657963 = -968860067;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn77079916 = -848356679;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn19395505 = -636642655;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn72827632 = -82757215;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn12909680 = -815689982;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn32022419 = -794138241;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn6769844 = -824639000;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn95801255 = -205221313;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn3030070 = -140252329;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn28582155 = -859919619;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn81751982 = -307648615;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn81085383 = -365998971;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn87857750 = -118370975;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn42611005 = -825741630;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn4916959 = -846124368;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn10629930 = -654705488;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn32804372 = -683862237;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn76518025 = -413489415;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn72988996 = -745320864;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn2812989 = -602476260;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn94415016 = -461875439;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn28421359 = -350067260;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn57131144 = -663850502;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn74027494 = -152614168;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn99547355 = -261046199;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn88752475 = -866425411;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn98934616 = 55914637;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn93744328 = -953889734;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn98970050 = -576614643;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn38175147 = -151638866;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn68330937 = -399534323;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn97139780 = 23271737;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn41807537 = -694808268;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn23059543 = -548730058;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn78406267 = -399315259;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn78844056 = 60210295;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn71927397 = -244686031;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn55046205 = -990099885;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn48241694 = -705677342;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn83793610 = -747136746;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn19357383 = -910121345;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn18329217 = -302169029;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn125789 = -977624108;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn94818829 = -763449592;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn8807578 = -733485099;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn42108749 = -986152906;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn78583964 = -489475760;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn97237224 = -274833165;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn9458611 = -888946249;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn55457988 = -787530626;    long tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn96472873 = -513599350;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn14889741 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn60859224;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn60859224 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn70482462;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn70482462 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn17649359;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn17649359 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn13328754;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn13328754 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn87320395;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn87320395 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn94152689;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn94152689 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn40675235;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn40675235 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn60840930;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn60840930 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn1952832;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn1952832 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn60678209;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn60678209 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn37669399;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn37669399 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn88805025;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn88805025 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn28940215;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn28940215 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn8545489;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn8545489 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn31577218;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn31577218 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn80532128;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn80532128 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn64587055;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn64587055 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn22741488;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn22741488 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn55194190;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn55194190 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn38782846;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn38782846 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn8119897;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn8119897 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn65057968;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn65057968 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn67452392;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn67452392 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn89067203;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn89067203 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn31576771;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn31576771 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn6235013;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn6235013 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn6294939;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn6294939 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn98064230;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn98064230 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn55923972;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn55923972 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn91322901;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn91322901 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn27873837;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn27873837 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn61151373;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn61151373 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn15816030;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn15816030 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn26127226;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn26127226 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn14130473;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn14130473 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn3155859;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn3155859 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn23400985;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn23400985 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn90559560;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn90559560 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn23194133;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn23194133 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn66441715;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn66441715 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn39848229;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn39848229 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn14375569;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn14375569 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn66087918;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn66087918 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn29277246;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn29277246 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn20736267;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn20736267 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn34436990;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn34436990 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn64427475;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn64427475 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn83235396;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn83235396 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn19657963;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn19657963 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn77079916;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn77079916 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn19395505;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn19395505 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn72827632;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn72827632 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn12909680;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn12909680 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn32022419;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn32022419 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn6769844;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn6769844 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn95801255;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn95801255 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn3030070;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn3030070 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn28582155;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn28582155 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn81751982;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn81751982 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn81085383;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn81085383 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn87857750;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn87857750 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn42611005;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn42611005 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn4916959;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn4916959 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn10629930;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn10629930 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn32804372;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn32804372 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn76518025;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn76518025 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn72988996;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn72988996 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn2812989;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn2812989 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn94415016;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn94415016 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn28421359;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn28421359 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn57131144;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn57131144 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn74027494;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn74027494 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn99547355;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn99547355 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn88752475;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn88752475 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn98934616;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn98934616 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn93744328;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn93744328 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn98970050;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn98970050 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn38175147;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn38175147 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn68330937;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn68330937 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn97139780;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn97139780 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn41807537;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn41807537 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn23059543;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn23059543 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn78406267;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn78406267 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn78844056;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn78844056 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn71927397;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn71927397 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn55046205;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn55046205 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn48241694;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn48241694 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn83793610;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn83793610 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn19357383;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn19357383 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn18329217;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn18329217 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn125789;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn125789 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn94818829;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn94818829 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn8807578;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn8807578 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn42108749;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn42108749 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn78583964;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn78583964 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn97237224;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn97237224 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn9458611;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn9458611 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn55457988;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn55457988 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn96472873;     tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn96472873 = tgahJjYcvLuCXUZgMxMMLiHUEQCAKTcLHBgmINNWKMeTaDyzPpzYvIEKZGcshXSbjjvVomXyjpBzEn14889741;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void qQExrdGPEXdDbQaNqSrUNxcuBhcxPbqZ26323975() {     float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX11399426 = -998662109;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX45228208 = -298057934;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX76821379 = -598907726;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX22738137 = -952060028;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX86401194 = -886162006;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX41969984 = -660807492;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX52241010 = 31861898;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX10734431 = -158889798;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX6465270 = -293511540;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX54277125 = -663162822;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX31054628 = -811690609;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX59831675 = -371328825;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX10002181 = -33210750;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX72937438 = 80665335;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX36031301 = 98051083;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX31816315 = 57037264;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX36149357 = -109874042;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX62222810 = -559809757;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX64878599 = -411316508;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX31956331 = -789809201;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX86296412 = -210442942;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX38169787 = -771973985;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX72994129 = 20750612;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX13102217 = 18434009;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX31563556 = -451828889;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX65888174 = -822146962;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX39242488 = -292146242;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX16247064 = -242820512;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX86599425 = -6588466;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX44223415 = -90638913;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX17123321 = -376843456;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX55985949 = -600774000;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX21322024 = 37563801;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX28617116 = -850125852;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX76791083 = 20917167;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX79215821 = -374349924;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX39683510 = -228127082;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX31486799 = -442132520;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX56950666 = -188543172;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX88647977 = -143215601;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX92640543 = -733360854;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX76890412 = -704911798;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX82059184 = -410908316;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX57911976 = -362352089;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX37272987 = -49133499;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX38631278 = -451872012;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX45197979 = -31181433;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX32609289 = -537230948;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX61173659 = -597236987;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX2481714 = -11911966;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX35498241 = -167013389;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX99077771 = -859880779;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX52695185 = -270692864;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX3542011 = -981850912;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX40027563 = -5309672;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX73229638 = -126688124;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX72234078 = -218808546;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX63719162 = -517341735;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX91174581 = -400231139;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX20513021 = 35984955;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX2727497 = -268661250;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX35993947 = -725317590;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX24135006 = -52301332;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX62241855 = -102872628;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX37153804 = -186319366;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX75068678 = -110916609;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX38509651 = -308892626;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX81385064 = -183084898;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX96146354 = -940251833;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX56815480 = -527598994;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX92132805 = -714835654;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX4662559 = -667741523;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX5272145 = -271266585;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX76230621 = -168100907;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX39315787 = 43551653;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX9406001 = -505531144;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX56110603 = -261065669;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX15082154 = -616897299;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX75829230 = -932432493;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX92932278 = -999956878;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX20690195 = -690965530;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX6633200 = -754915294;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX55073405 = -645583526;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX84117711 = -994676501;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX8725174 = -923625525;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX18045550 = -516962677;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX3290765 = -230081137;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX17780014 = 19414712;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX88589553 = -744816180;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX3561445 = -852394709;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX6981743 = -55541378;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX75964347 = -710785347;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX40312217 = 58098619;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX36437645 = -124528128;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX85920481 = -874554352;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX56646597 = 91956736;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX52755406 = -552610466;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX19817330 = -208035689;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX20758172 = -76032723;    float pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX62204308 = -998662109;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX11399426 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX45228208;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX45228208 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX76821379;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX76821379 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX22738137;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX22738137 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX86401194;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX86401194 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX41969984;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX41969984 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX52241010;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX52241010 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX10734431;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX10734431 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX6465270;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX6465270 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX54277125;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX54277125 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX31054628;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX31054628 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX59831675;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX59831675 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX10002181;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX10002181 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX72937438;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX72937438 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX36031301;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX36031301 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX31816315;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX31816315 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX36149357;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX36149357 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX62222810;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX62222810 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX64878599;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX64878599 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX31956331;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX31956331 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX86296412;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX86296412 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX38169787;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX38169787 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX72994129;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX72994129 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX13102217;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX13102217 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX31563556;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX31563556 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX65888174;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX65888174 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX39242488;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX39242488 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX16247064;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX16247064 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX86599425;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX86599425 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX44223415;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX44223415 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX17123321;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX17123321 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX55985949;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX55985949 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX21322024;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX21322024 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX28617116;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX28617116 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX76791083;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX76791083 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX79215821;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX79215821 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX39683510;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX39683510 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX31486799;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX31486799 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX56950666;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX56950666 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX88647977;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX88647977 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX92640543;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX92640543 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX76890412;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX76890412 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX82059184;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX82059184 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX57911976;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX57911976 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX37272987;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX37272987 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX38631278;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX38631278 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX45197979;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX45197979 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX32609289;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX32609289 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX61173659;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX61173659 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX2481714;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX2481714 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX35498241;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX35498241 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX99077771;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX99077771 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX52695185;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX52695185 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX3542011;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX3542011 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX40027563;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX40027563 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX73229638;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX73229638 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX72234078;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX72234078 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX63719162;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX63719162 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX91174581;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX91174581 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX20513021;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX20513021 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX2727497;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX2727497 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX35993947;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX35993947 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX24135006;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX24135006 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX62241855;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX62241855 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX37153804;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX37153804 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX75068678;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX75068678 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX38509651;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX38509651 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX81385064;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX81385064 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX96146354;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX96146354 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX56815480;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX56815480 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX92132805;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX92132805 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX4662559;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX4662559 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX5272145;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX5272145 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX76230621;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX76230621 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX39315787;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX39315787 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX9406001;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX9406001 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX56110603;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX56110603 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX15082154;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX15082154 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX75829230;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX75829230 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX92932278;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX92932278 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX20690195;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX20690195 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX6633200;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX6633200 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX55073405;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX55073405 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX84117711;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX84117711 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX8725174;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX8725174 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX18045550;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX18045550 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX3290765;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX3290765 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX17780014;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX17780014 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX88589553;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX88589553 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX3561445;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX3561445 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX6981743;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX6981743 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX75964347;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX75964347 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX40312217;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX40312217 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX36437645;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX36437645 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX85920481;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX85920481 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX56646597;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX56646597 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX52755406;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX52755406 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX19817330;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX19817330 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX20758172;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX20758172 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX62204308;     pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX62204308 = pQGaWZzovKLPUjaRenjjDPbsoJwKAJzcgEOLKzqQTCYDlMZRGLTcsHWZpNlMdoomKjjQVPHewTQAqX11399426;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void tPkDXIFwAFlXxgYdSqoZMgYyvdgmaZBS99283727() {     float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi10834530 = -832473477;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi22348512 = -732113732;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi93779371 = -406471414;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi39106595 = -50436672;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi48361272 = -172355927;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi51105183 = -811742272;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi89105856 = -209468920;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi21963201 = -796474002;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi5825779 = -395110919;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi23796766 = -790639886;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi64429037 = -370823911;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi20110609 = -223765138;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi75982942 = 47472781;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi97289152 = -241702706;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi94553819 = -201313122;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi35531128 = -3661734;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi79775064 = -218418941;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi18771697 = -29787984;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi28234192 = -349350040;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi34549690 = 71580197;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi42567591 = 48965664;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi38756547 = -337958836;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi79172210 = -459196151;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi23880052 = -927672000;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi69953976 = -741166737;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi67161033 = -705438266;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi65189671 = -61363794;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi45915439 = -318814292;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi27603777 = -429277265;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi80883313 = -546557240;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi23997781 = -701255052;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi17336084 = -824591476;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi65602818 = -275696587;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi41767546 = -201818629;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi81395931 = -65145013;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi7099912 = -73370527;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi89326054 = -544796228;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi81798559 = -56049805;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi69022981 = -659902421;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi81132909 = -464540573;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi18259019 = -664322446;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi4884192 = -634469184;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi23876392 = -567344547;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi6562716 = -273794609;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi49205531 = -426678651;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi69737475 = 26924854;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi58568693 = -733778700;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi69850366 = -519191946;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi73777846 = -832636711;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi21445445 = -948946892;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi15090844 = -955826437;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi21615178 = -754439578;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi65603646 = -562376773;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi71280588 = -897179163;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi56716137 = -925727306;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi72077982 = -394514642;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi43176301 = -172917582;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi69899319 = -478799415;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi69152619 = -309269936;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi81200238 = -466917661;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi85915511 = -650378478;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi43190417 = -890654629;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi94359424 = -267196737;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi24942465 = -848553679;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi99798984 = 10615165;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi47092953 = -546232435;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi54507791 = -948068551;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi34215396 = -750708590;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi15893222 = -76557693;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi87453907 = -27942596;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi46205074 = -458865507;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi97976505 = -62369136;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi49748716 = -369885563;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi47101283 = -884809467;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi16290672 = -264097358;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi37683399 = -316565152;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi14880156 = -770614289;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi72609495 = -85401542;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi74674521 = -400993350;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi216502 = -668091591;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi8592341 = -971659567;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi95339305 = -542171848;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi72137593 = -486177581;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi6158333 = -480330374;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi65792469 = -590730804;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi2382604 = -946815474;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi51732438 = -162214704;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi94322230 = -378517425;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi85051409 = -276091324;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi9317949 = -670630371;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi63923610 = -900452946;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi19426735 = 34003187;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi12645941 = -746779870;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi87822742 = -92984761;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi95217397 = -814162095;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi75068602 = -773667818;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi10524768 = -267272448;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi98933926 = -718790869;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi6763731 = -184409774;    float RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi2112579 = -832473477;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi10834530 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi22348512;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi22348512 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi93779371;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi93779371 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi39106595;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi39106595 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi48361272;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi48361272 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi51105183;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi51105183 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi89105856;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi89105856 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi21963201;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi21963201 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi5825779;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi5825779 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi23796766;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi23796766 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi64429037;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi64429037 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi20110609;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi20110609 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi75982942;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi75982942 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi97289152;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi97289152 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi94553819;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi94553819 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi35531128;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi35531128 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi79775064;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi79775064 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi18771697;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi18771697 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi28234192;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi28234192 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi34549690;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi34549690 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi42567591;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi42567591 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi38756547;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi38756547 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi79172210;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi79172210 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi23880052;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi23880052 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi69953976;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi69953976 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi67161033;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi67161033 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi65189671;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi65189671 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi45915439;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi45915439 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi27603777;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi27603777 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi80883313;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi80883313 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi23997781;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi23997781 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi17336084;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi17336084 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi65602818;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi65602818 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi41767546;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi41767546 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi81395931;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi81395931 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi7099912;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi7099912 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi89326054;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi89326054 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi81798559;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi81798559 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi69022981;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi69022981 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi81132909;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi81132909 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi18259019;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi18259019 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi4884192;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi4884192 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi23876392;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi23876392 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi6562716;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi6562716 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi49205531;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi49205531 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi69737475;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi69737475 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi58568693;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi58568693 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi69850366;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi69850366 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi73777846;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi73777846 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi21445445;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi21445445 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi15090844;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi15090844 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi21615178;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi21615178 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi65603646;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi65603646 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi71280588;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi71280588 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi56716137;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi56716137 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi72077982;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi72077982 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi43176301;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi43176301 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi69899319;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi69899319 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi69152619;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi69152619 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi81200238;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi81200238 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi85915511;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi85915511 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi43190417;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi43190417 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi94359424;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi94359424 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi24942465;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi24942465 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi99798984;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi99798984 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi47092953;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi47092953 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi54507791;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi54507791 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi34215396;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi34215396 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi15893222;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi15893222 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi87453907;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi87453907 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi46205074;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi46205074 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi97976505;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi97976505 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi49748716;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi49748716 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi47101283;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi47101283 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi16290672;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi16290672 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi37683399;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi37683399 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi14880156;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi14880156 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi72609495;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi72609495 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi74674521;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi74674521 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi216502;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi216502 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi8592341;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi8592341 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi95339305;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi95339305 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi72137593;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi72137593 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi6158333;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi6158333 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi65792469;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi65792469 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi2382604;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi2382604 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi51732438;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi51732438 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi94322230;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi94322230 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi85051409;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi85051409 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi9317949;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi9317949 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi63923610;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi63923610 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi19426735;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi19426735 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi12645941;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi12645941 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi87822742;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi87822742 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi95217397;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi95217397 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi75068602;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi75068602 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi10524768;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi10524768 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi98933926;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi98933926 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi6763731;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi6763731 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi2112579;     RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi2112579 = RUBHfnwRkHedxrFscSikSEoMrlIubTWaDSGcfQyetWpFBuADYoJIrRAMgoQtfCqQkEZflcMMTDrvOi10834530;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void xLpuBzxCVUZmQQQMkPKjXFwxbjkPaiXS54802438() {     float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG7344215 = -217536236;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG6717496 = -706099597;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG118289 = -683138487;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG44195373 = -979520877;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG21433713 = 37879417;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG5754772 = -691986254;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG47194177 = -776655128;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG92022396 = -761446845;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG51450117 = 45648293;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG76121059 = -817768535;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG34805455 = -325795184;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG42272885 = -660237225;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG97180096 = -799142120;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG41286376 = -923800766;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG22039631 = -33996258;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG35770226 = -478680775;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG35392293 = -41073270;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG16407452 = -295849860;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG70371303 = -47468473;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG11311831 = -143956859;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG90081157 = -16817121;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG68806437 = -420972470;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG87108371 = -419594782;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG69529876 = -827249665;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG12450328 = 70060577;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG1472437 = -838836493;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG98197146 = -38945497;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG55867564 = -279053885;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG16138972 = -67690405;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG69182755 = -849049770;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG49798200 = -96769822;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG45448196 = -252508377;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG25773469 = -816765463;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG54568633 = -510669495;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG32059789 = -409467501;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG72185260 = -940330108;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG25853705 = -655046874;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG89884373 = -974813114;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG35414086 = -807311879;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG46586755 = -255604298;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG44457848 = -689836566;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG41926374 = -138806188;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG91560007 = -243182246;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG98386772 = -193910584;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG57201272 = -645462689;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG87632485 = -761425278;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG69329682 = -152939659;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG38032180 = -436666623;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG51716109 = -696022837;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG4269196 = 8001210;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG73509169 = -274483147;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG1297445 = -977677702;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG45471199 = -750312421;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG61912919 = 36659908;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG64721280 = -136898737;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG38537777 = -796563767;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG19609124 = -186504815;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG30588412 = -855888822;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG31745046 = -949581455;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG19961276 = -123284091;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG7557625 = -553040757;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG91326612 = -397601244;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG75883425 = -593756440;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG82267361 = -105301938;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG26322859 = -620998713;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG89357259 = 26713193;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG16499417 = -843471762;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG42611464 = -188472625;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG9226588 = -414333266;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG49854371 = -93666151;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG9916521 = -823633901;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG45507920 = -66260157;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG80993365 = -488537981;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG23784549 = -791864176;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG66853983 = -454120294;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG48154783 = -878010933;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG77246430 = -77790224;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG88721598 = -125684198;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG12328605 = -81786977;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG24817842 = -168514145;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG32142754 = -585896835;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG60164967 = -602278874;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG4151456 = -583031048;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG11869777 = 24308384;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG95673586 = -474566623;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG48500755 = -119092120;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG99976997 = -502195957;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG63860550 = -753425371;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG89847353 = -273770758;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG93522011 = -612903735;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG52576136 = -653825294;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG95265292 = -799158053;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG58139328 = 74768341;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG15452810 = -584027789;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG39029131 = -702563541;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG53131235 = -192235322;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG66042949 = -545049749;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG9292646 = -37880308;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG72063913 = -572911872;    float edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG67844013 = -217536236;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG7344215 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG6717496;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG6717496 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG118289;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG118289 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG44195373;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG44195373 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG21433713;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG21433713 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG5754772;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG5754772 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG47194177;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG47194177 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG92022396;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG92022396 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG51450117;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG51450117 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG76121059;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG76121059 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG34805455;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG34805455 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG42272885;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG42272885 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG97180096;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG97180096 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG41286376;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG41286376 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG22039631;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG22039631 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG35770226;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG35770226 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG35392293;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG35392293 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG16407452;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG16407452 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG70371303;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG70371303 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG11311831;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG11311831 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG90081157;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG90081157 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG68806437;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG68806437 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG87108371;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG87108371 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG69529876;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG69529876 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG12450328;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG12450328 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG1472437;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG1472437 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG98197146;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG98197146 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG55867564;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG55867564 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG16138972;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG16138972 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG69182755;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG69182755 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG49798200;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG49798200 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG45448196;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG45448196 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG25773469;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG25773469 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG54568633;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG54568633 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG32059789;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG32059789 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG72185260;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG72185260 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG25853705;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG25853705 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG89884373;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG89884373 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG35414086;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG35414086 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG46586755;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG46586755 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG44457848;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG44457848 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG41926374;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG41926374 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG91560007;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG91560007 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG98386772;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG98386772 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG57201272;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG57201272 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG87632485;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG87632485 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG69329682;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG69329682 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG38032180;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG38032180 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG51716109;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG51716109 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG4269196;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG4269196 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG73509169;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG73509169 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG1297445;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG1297445 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG45471199;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG45471199 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG61912919;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG61912919 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG64721280;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG64721280 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG38537777;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG38537777 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG19609124;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG19609124 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG30588412;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG30588412 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG31745046;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG31745046 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG19961276;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG19961276 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG7557625;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG7557625 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG91326612;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG91326612 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG75883425;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG75883425 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG82267361;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG82267361 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG26322859;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG26322859 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG89357259;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG89357259 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG16499417;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG16499417 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG42611464;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG42611464 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG9226588;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG9226588 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG49854371;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG49854371 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG9916521;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG9916521 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG45507920;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG45507920 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG80993365;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG80993365 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG23784549;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG23784549 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG66853983;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG66853983 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG48154783;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG48154783 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG77246430;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG77246430 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG88721598;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG88721598 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG12328605;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG12328605 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG24817842;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG24817842 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG32142754;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG32142754 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG60164967;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG60164967 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG4151456;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG4151456 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG11869777;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG11869777 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG95673586;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG95673586 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG48500755;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG48500755 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG99976997;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG99976997 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG63860550;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG63860550 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG89847353;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG89847353 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG93522011;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG93522011 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG52576136;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG52576136 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG95265292;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG95265292 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG58139328;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG58139328 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG15452810;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG15452810 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG39029131;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG39029131 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG53131235;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG53131235 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG66042949;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG66042949 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG9292646;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG9292646 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG72063913;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG72063913 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG67844013;     edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG67844013 = edgYSMYbBWaBWoepkBeGNOGsOsSSOhbmwHHmkrtXqiiAevanEveYEQEQWnKZuGNTvoEPgsEsIGQfgG7344215;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void judclxGVaPOyseXMkCXSgkIlkXLzqLlV58078617() {     int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS74512038 = -348223984;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS21187680 = -509133635;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS85112912 = -344971405;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS4135578 = -78202642;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS7116373 = -166936266;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS15598690 = -574378576;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS55114058 = -538080579;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS49709922 = -269059460;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS60199346 = -993293755;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS54772299 = -409185662;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS27110429 = -356196314;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS95690054 = -576994742;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS66351724 = -433635604;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS15286413 = 38226014;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS87073352 = -53716546;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS93973415 = -190887752;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS29009589 = 46701840;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS11336168 = -154006310;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS73787926 = -934792582;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS74786917 = -289835769;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS13061055 = -280812672;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS81955826 = -325279527;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS60109944 = -488878716;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS5379130 = -438307714;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS90628378 = -567922943;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS49807343 = -62313237;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS69243826 = -110905263;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS38198390 = -135450257;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS24138207 = -454391448;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS20564424 = -934505996;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS49257937 = 46374956;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS33347911 = -716900680;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS83437056 = -2851732;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS32451858 = -506368313;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS78389844 = -341422843;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS54435678 = -706750492;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS94231473 = -854237351;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS80844996 = -582426850;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS45024883 = -197768558;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS35271006 = 45155300;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS42874293 = 88574335;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS21753887 = -318740179;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS47927224 = -446643743;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS37487232 = -629037088;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS77133019 = -699633721;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS71505363 = -396067893;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS78524082 = -306889796;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS96444622 = -197510240;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS16437435 = -981409071;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS75475009 = -831034681;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS86857449 = -661156971;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS8303959 = -787989800;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS4966061 = -973992104;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS32522284 = -258462667;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS45017562 = -802111805;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS92556211 = 77055542;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS61077736 = 79745080;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS79733782 = -906663691;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS13507199 = -510279700;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS57309030 = -4623030;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS46354863 = -363473314;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS16915668 = -302630323;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS25571715 = -814668013;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS39634922 = 41212240;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS5514363 = -355560619;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS93762517 = -639295635;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS12252998 = -474143011;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS33899867 = -927267292;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS36896568 = -620351143;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS32637675 = -346966055;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS99741942 = -336650402;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS48164592 = -370871310;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS66311284 = -956237752;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS38516920 = -879947883;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS31912625 = -278410104;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS91307168 = -962072494;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS34028603 = -878635784;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS22622712 = -859841629;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS28246110 = -738673994;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS19123016 = -71855050;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS71283261 = -755423441;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS72799204 = -913395024;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS21760956 = -154041187;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS48663198 = -623356768;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS33706975 = -173349025;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS40953978 = -165635244;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS28381851 = -742908577;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS50914773 = -744389065;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS87434295 = -704256508;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS85833633 = -318478386;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS93357942 = -686495572;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS14497691 = -947573660;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS67337798 = 27852849;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS87715853 = -93145529;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS88916142 = -591371386;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS25958625 = -608795343;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS96182171 = -504072167;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS8292302 = -387855984;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS31972870 = -173476469;    int EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS83370502 = -348223984;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS74512038 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS21187680;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS21187680 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS85112912;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS85112912 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS4135578;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS4135578 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS7116373;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS7116373 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS15598690;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS15598690 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS55114058;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS55114058 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS49709922;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS49709922 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS60199346;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS60199346 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS54772299;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS54772299 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS27110429;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS27110429 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS95690054;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS95690054 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS66351724;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS66351724 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS15286413;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS15286413 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS87073352;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS87073352 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS93973415;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS93973415 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS29009589;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS29009589 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS11336168;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS11336168 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS73787926;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS73787926 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS74786917;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS74786917 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS13061055;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS13061055 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS81955826;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS81955826 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS60109944;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS60109944 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS5379130;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS5379130 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS90628378;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS90628378 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS49807343;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS49807343 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS69243826;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS69243826 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS38198390;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS38198390 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS24138207;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS24138207 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS20564424;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS20564424 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS49257937;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS49257937 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS33347911;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS33347911 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS83437056;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS83437056 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS32451858;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS32451858 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS78389844;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS78389844 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS54435678;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS54435678 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS94231473;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS94231473 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS80844996;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS80844996 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS45024883;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS45024883 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS35271006;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS35271006 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS42874293;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS42874293 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS21753887;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS21753887 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS47927224;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS47927224 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS37487232;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS37487232 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS77133019;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS77133019 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS71505363;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS71505363 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS78524082;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS78524082 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS96444622;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS96444622 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS16437435;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS16437435 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS75475009;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS75475009 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS86857449;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS86857449 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS8303959;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS8303959 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS4966061;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS4966061 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS32522284;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS32522284 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS45017562;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS45017562 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS92556211;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS92556211 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS61077736;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS61077736 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS79733782;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS79733782 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS13507199;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS13507199 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS57309030;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS57309030 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS46354863;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS46354863 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS16915668;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS16915668 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS25571715;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS25571715 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS39634922;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS39634922 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS5514363;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS5514363 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS93762517;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS93762517 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS12252998;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS12252998 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS33899867;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS33899867 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS36896568;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS36896568 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS32637675;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS32637675 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS99741942;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS99741942 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS48164592;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS48164592 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS66311284;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS66311284 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS38516920;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS38516920 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS31912625;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS31912625 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS91307168;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS91307168 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS34028603;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS34028603 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS22622712;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS22622712 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS28246110;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS28246110 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS19123016;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS19123016 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS71283261;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS71283261 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS72799204;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS72799204 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS21760956;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS21760956 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS48663198;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS48663198 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS33706975;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS33706975 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS40953978;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS40953978 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS28381851;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS28381851 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS50914773;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS50914773 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS87434295;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS87434295 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS85833633;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS85833633 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS93357942;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS93357942 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS14497691;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS14497691 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS67337798;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS67337798 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS87715853;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS87715853 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS88916142;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS88916142 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS25958625;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS25958625 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS96182171;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS96182171 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS8292302;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS8292302 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS31972870;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS31972870 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS83370502;     EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS83370502 = EZUncypnqUEnsUzOsaQiApezlssNmGfPetuSKjkmNkcMgUsQYwHQuCUPuUBkFbAjooJzagEIpdVQMS74512038;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void UBskQPmUfVoMRuqWKxqClRGgmqSOOdnx13597328() {     double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL71021723 = -833286743;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL5556664 = -483119500;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL91451829 = -621638478;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL9224357 = 92713152;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL80188814 = 43299078;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL70248278 = -454622558;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL13202380 = -5266788;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL19769118 = -234032303;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL5823685 = -552534543;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL7096593 = -436314311;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL97486846 = -311167588;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL17852331 = 86533171;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL87548879 = -180250505;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL59283636 = -643872046;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL14559165 = -986399682;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL94212513 = -665906792;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL84626817 = -875952490;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL8971922 = -420068186;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL15925037 = -632911016;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL51549058 = -505372825;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL60574622 = -346595457;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL12005718 = -408293161;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL68046105 = -449277348;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL51028954 = -337885380;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL33124731 = -856695629;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL84118746 = -195711463;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL2251303 = -88486966;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL48150515 = -95689850;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL12673402 = -92804588;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL8863867 = -136998525;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL75058356 = -449139814;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL61460023 = -144817581;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL43607707 = -543920608;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL45252945 = -815219179;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL29053702 = -685745331;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL19521027 = -473710073;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL30759124 = -964487998;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL88930810 = -401190159;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL11415989 = -345178017;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL724852 = -845908425;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL69073121 = 63060215;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL58796070 = -923077183;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL15610840 = -122481442;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL29311290 = -549153063;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL85128760 = -918417759;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL89400373 = -84418024;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL89285071 = -826050755;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL64626436 = -114984917;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL94375697 = -844795196;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL58298760 = -974086579;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL45275774 = 20186319;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL87986226 = 88772075;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL84833612 = -61927752;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL23154615 = -424623597;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL53022706 = -13283237;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL59016006 = -324993582;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL37510559 = 66157847;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL40422875 = -183753098;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL76099625 = -50591219;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL96070067 = -760989459;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL67996976 = -266135593;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL65051864 = -909576938;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL7095716 = -41227715;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL96959818 = -315536019;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL32038237 = -987174497;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL36026824 = -66350007;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL74244623 = -369546222;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL42295935 = -365031327;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL30229934 = -958126715;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL95038138 = -412689609;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL63453389 = -701418795;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL95696006 = -374762332;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL97555933 = 25109830;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL15200186 = -787002592;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL82475937 = -468433040;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL1778553 = -423518275;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL96394877 = -185811719;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL38734816 = -900124285;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL65900193 = -419467621;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL43724357 = -672277605;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL94833675 = -369660709;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL37624866 = -973502050;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL53774818 = -250894654;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL54374642 = -118718010;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL63588092 = -57184844;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL87072129 = -437911890;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL76626410 = 17110171;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL20453093 = -19297011;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL92230238 = -701935943;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL70037696 = -260751750;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL82010468 = -439867921;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL90336249 = -680734900;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL12831186 = -250598940;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL15345921 = -584188558;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL32727876 = -479772833;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL4021258 = -27362847;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL51700354 = -781849468;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL18651022 = -806945424;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL97273052 = -561978567;    double EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL49101937 = -833286743;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL71021723 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL5556664;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL5556664 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL91451829;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL91451829 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL9224357;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL9224357 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL80188814;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL80188814 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL70248278;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL70248278 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL13202380;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL13202380 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL19769118;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL19769118 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL5823685;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL5823685 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL7096593;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL7096593 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL97486846;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL97486846 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL17852331;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL17852331 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL87548879;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL87548879 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL59283636;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL59283636 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL14559165;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL14559165 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL94212513;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL94212513 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL84626817;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL84626817 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL8971922;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL8971922 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL15925037;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL15925037 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL51549058;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL51549058 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL60574622;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL60574622 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL12005718;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL12005718 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL68046105;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL68046105 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL51028954;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL51028954 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL33124731;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL33124731 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL84118746;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL84118746 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL2251303;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL2251303 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL48150515;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL48150515 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL12673402;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL12673402 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL8863867;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL8863867 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL75058356;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL75058356 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL61460023;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL61460023 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL43607707;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL43607707 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL45252945;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL45252945 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL29053702;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL29053702 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL19521027;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL19521027 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL30759124;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL30759124 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL88930810;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL88930810 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL11415989;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL11415989 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL724852;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL724852 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL69073121;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL69073121 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL58796070;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL58796070 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL15610840;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL15610840 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL29311290;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL29311290 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL85128760;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL85128760 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL89400373;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL89400373 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL89285071;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL89285071 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL64626436;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL64626436 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL94375697;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL94375697 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL58298760;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL58298760 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL45275774;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL45275774 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL87986226;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL87986226 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL84833612;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL84833612 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL23154615;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL23154615 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL53022706;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL53022706 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL59016006;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL59016006 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL37510559;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL37510559 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL40422875;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL40422875 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL76099625;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL76099625 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL96070067;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL96070067 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL67996976;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL67996976 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL65051864;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL65051864 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL7095716;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL7095716 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL96959818;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL96959818 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL32038237;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL32038237 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL36026824;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL36026824 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL74244623;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL74244623 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL42295935;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL42295935 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL30229934;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL30229934 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL95038138;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL95038138 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL63453389;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL63453389 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL95696006;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL95696006 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL97555933;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL97555933 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL15200186;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL15200186 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL82475937;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL82475937 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL1778553;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL1778553 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL96394877;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL96394877 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL38734816;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL38734816 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL65900193;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL65900193 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL43724357;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL43724357 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL94833675;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL94833675 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL37624866;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL37624866 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL53774818;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL53774818 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL54374642;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL54374642 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL63588092;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL63588092 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL87072129;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL87072129 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL76626410;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL76626410 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL20453093;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL20453093 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL92230238;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL92230238 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL70037696;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL70037696 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL82010468;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL82010468 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL90336249;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL90336249 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL12831186;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL12831186 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL15345921;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL15345921 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL32727876;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL32727876 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL4021258;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL4021258 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL51700354;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL51700354 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL18651022;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL18651022 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL97273052;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL97273052 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL49101937;     EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL49101937 = EKaTFjsSPAdqpOcxAzuZKDwCuLyZxYGBhqIGUWRbEXmkoQrYOUOPjOnBNXZtkTVqucIzGVddhSUnQL71021723;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ibyOzKAasMZEoYuKOZNBGwcCsVGxybnrOmFXLGbNSnareyfymIvNqRs74784191() {     double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ31507544 = -685883970;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ53773872 = 89390444;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ77362261 = 59797837;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ58458238 = -113596626;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ58936063 = -27060654;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ97810192 = -223453425;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ97498139 = -969058080;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ38925523 = -92355209;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ49290927 = -825043304;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ14037815 = -926233025;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ63055939 = -23264433;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ99725391 = -338254458;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ36492151 = -394169370;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ74491711 = -571974738;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ42372978 = -615022009;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ14625123 = -465813224;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ28033818 = -280177165;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ63396372 = -82680221;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ20867411 = 47500440;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ37067340 = -532959417;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ51272504 = -495694948;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ39220843 = 29344648;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ11634968 = -151990558;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ13663669 = -672744449;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ5993552 = -310820964;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ9004837 = -523824188;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ785387 = -29002080;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ47042591 = 37848952;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ95544733 = -679811725;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ28289797 = -860902211;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ89150003 = -717085693;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ13099252 = -23580544;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ35841141 = -150653894;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ41456035 = -411069008;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ18513954 = -65029749;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ60929622 = -825124952;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ67517503 = -426711749;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ96112982 = -951215171;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ59488846 = -213070448;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ94392095 = -393034576;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ87438817 = -124216583;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ34466905 = -762426282;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ35728284 = -401574587;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ29654748 = -876379149;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ5040588 = -588235788;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ92440253 = -355047658;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ94071610 = -463822621;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ52323015 = -947893999;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ62025481 = -179844166;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ35556652 = -463146587;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ2515979 = -213006882;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ6720428 = -915372501;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ8988478 = -785501756;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ65533676 = -314604278;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ23511688 = -898382156;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ92286700 = -615228619;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ42138905 = -758618999;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ63698593 = -267457714;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ52464686 = -802775663;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ49931226 = -503236467;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ97024805 = -94451346;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ50455549 = -906907032;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ43380789 = -412543485;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ21001131 = -964141094;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ24887811 = -109147333;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ49956688 = -999683890;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ63884251 = -87600564;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ95036115 = -983100362;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ55977757 = -406944990;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ81443356 = -789897057;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ47107620 = 60898524;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ31920835 = -328961994;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ3907526 = -869609773;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ26475315 = -559464984;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ49628522 = -308742835;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ16805599 = -733268666;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ3492559 = -569080766;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ81980219 = -275611410;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ8623081 = 15491338;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ13553299 = -955773307;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ14933227 = 39998433;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ48462372 = -81108082;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ85017109 = -782306882;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ59988082 = -116665138;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ25773818 = -547895329;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ82429576 = -801713192;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ4110774 = -238078788;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ70307464 = -836049617;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ17944348 = -512686853;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ26227253 = -449801131;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ18790717 = 33494046;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ3818910 = -59254036;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ43648297 = -48439509;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ9557621 = -709833982;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ97367289 = -198583231;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ36983269 = -217309551;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ91086116 = -249882798;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ14727154 = -437433494;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ4766937 = -667231817;    double myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ55083900 = -685883970;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ31507544 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ53773872;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ53773872 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ77362261;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ77362261 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ58458238;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ58458238 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ58936063;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ58936063 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ97810192;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ97810192 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ97498139;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ97498139 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ38925523;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ38925523 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ49290927;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ49290927 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ14037815;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ14037815 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ63055939;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ63055939 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ99725391;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ99725391 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ36492151;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ36492151 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ74491711;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ74491711 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ42372978;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ42372978 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ14625123;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ14625123 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ28033818;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ28033818 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ63396372;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ63396372 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ20867411;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ20867411 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ37067340;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ37067340 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ51272504;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ51272504 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ39220843;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ39220843 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ11634968;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ11634968 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ13663669;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ13663669 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ5993552;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ5993552 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ9004837;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ9004837 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ785387;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ785387 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ47042591;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ47042591 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ95544733;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ95544733 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ28289797;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ28289797 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ89150003;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ89150003 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ13099252;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ13099252 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ35841141;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ35841141 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ41456035;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ41456035 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ18513954;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ18513954 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ60929622;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ60929622 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ67517503;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ67517503 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ96112982;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ96112982 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ59488846;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ59488846 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ94392095;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ94392095 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ87438817;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ87438817 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ34466905;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ34466905 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ35728284;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ35728284 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ29654748;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ29654748 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ5040588;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ5040588 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ92440253;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ92440253 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ94071610;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ94071610 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ52323015;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ52323015 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ62025481;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ62025481 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ35556652;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ35556652 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ2515979;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ2515979 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ6720428;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ6720428 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ8988478;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ8988478 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ65533676;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ65533676 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ23511688;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ23511688 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ92286700;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ92286700 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ42138905;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ42138905 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ63698593;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ63698593 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ52464686;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ52464686 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ49931226;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ49931226 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ97024805;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ97024805 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ50455549;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ50455549 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ43380789;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ43380789 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ21001131;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ21001131 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ24887811;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ24887811 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ49956688;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ49956688 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ63884251;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ63884251 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ95036115;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ95036115 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ55977757;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ55977757 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ81443356;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ81443356 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ47107620;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ47107620 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ31920835;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ31920835 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ3907526;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ3907526 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ26475315;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ26475315 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ49628522;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ49628522 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ16805599;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ16805599 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ3492559;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ3492559 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ81980219;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ81980219 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ8623081;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ8623081 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ13553299;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ13553299 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ14933227;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ14933227 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ48462372;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ48462372 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ85017109;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ85017109 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ59988082;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ59988082 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ25773818;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ25773818 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ82429576;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ82429576 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ4110774;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ4110774 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ70307464;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ70307464 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ17944348;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ17944348 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ26227253;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ26227253 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ18790717;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ18790717 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ3818910;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ3818910 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ43648297;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ43648297 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ9557621;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ9557621 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ97367289;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ97367289 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ36983269;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ36983269 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ91086116;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ91086116 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ14727154;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ14727154 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ4766937;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ4766937 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ55083900;     myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ55083900 = myvWeEDcRYhqCTWyHyAjTWkYuqHLqfZBwtSYQkxJOLJTwHcfPKnPqiYPNWbowsmlygtIAxiWAqUIMRcmNcvfsKLOVDLtIJNVZXUQ31507544;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void armvahNWRJzLmwWVxxtGTDICiRvlCYTFxMEcirSkSZqhRRqlOtyacrI3150749() {     float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH37611708 = -542834142;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH74291785 = -805399588;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH96030868 = -5292312;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH55692740 = 54573083;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH45511756 = -384421168;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH58838260 = -986815284;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH95081777 = -517095991;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH27113515 = -887583757;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH79413751 = -20368110;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH48546501 = -528822245;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH63807440 = -919646164;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH60131788 = -253617385;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH86407097 = -877377408;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH65323828 = -694519409;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH28459623 = -130909879;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH5259612 = -4404069;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH33575340 = -535951418;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH6450591 = -542346107;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH23553793 = -494233190;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH61765030 = 86088761;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH99164545 = -872115150;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH82334398 = -898568835;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH20982553 = -816423729;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH84956258 = -656890482;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH6578582 = -437404704;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH66874666 = -218891367;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH39688329 = -531263709;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH30956766 = -146013978;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH87033883 = 5664941;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH49085173 = -29235826;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH48217910 = -304593100;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH80157588 = -516670883;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH16456936 = -447155696;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH36852274 = 84025761;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH44704682 = -122390703;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH15311915 = -98682552;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH72662173 = -579654375;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH70038554 = -713246539;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH3726294 = -532356777;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH25639819 = -85337170;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH68819376 = -365378274;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH38012684 = -320152724;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH34361983 = -500639679;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH35526285 = -9650974;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH85029157 = -96652745;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH85235385 = 35646884;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH67245769 = -445159403;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH11047118 = 24687757;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH9017990 = -493108877;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH88559150 = -824207168;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH64735396 = -312410568;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH46624662 = -380390318;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH8133632 = -361399360;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH82003098 = -598914125;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH3986203 = -12712494;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH55277310 = -644265307;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH53309232 = -988975859;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH11074611 = -348401830;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH49114159 = -508022214;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH78637089 = -65529801;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH19149932 = -355551575;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH64125011 = -271082013;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH40079632 = -793248698;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH30328578 = -991132284;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH328591 = -124229146;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH83649852 = -302975282;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH43674853 = -806461689;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH49554823 = -861403169;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH20619147 = -472128706;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH13147709 = 67772673;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH32597439 = -424749694;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH63536785 = -822704879;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH2724298 = 90010670;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH97913973 = -308896020;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH92945653 = -548532966;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH61151861 = -451962426;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH47972415 = -297929157;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH85456267 = -706772756;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH99927101 = -460237737;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH21343196 = -373051588;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH99628896 = -773731965;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH28641211 = -455951466;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH21938777 = -652905102;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH98474732 = -170127892;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH84349777 = -716825259;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH1593249 = -924202783;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH72023956 = -55271523;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH34453838 = -848241571;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH32866071 = -903261745;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH89427372 = -478125397;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH62002682 = -109706694;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH61587563 = -131252546;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH20924396 = -105224326;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH25089204 = -366826977;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH6489888 = -729785596;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH4694366 = 5703739;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH97933052 = -526904027;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH4033405 = -509507395;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH35197694 = -885421829;    float XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH1379306 = -542834142;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH37611708 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH74291785;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH74291785 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH96030868;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH96030868 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH55692740;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH55692740 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH45511756;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH45511756 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH58838260;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH58838260 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH95081777;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH95081777 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH27113515;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH27113515 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH79413751;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH79413751 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH48546501;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH48546501 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH63807440;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH63807440 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH60131788;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH60131788 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH86407097;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH86407097 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH65323828;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH65323828 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH28459623;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH28459623 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH5259612;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH5259612 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH33575340;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH33575340 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH6450591;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH6450591 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH23553793;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH23553793 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH61765030;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH61765030 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH99164545;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH99164545 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH82334398;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH82334398 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH20982553;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH20982553 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH84956258;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH84956258 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH6578582;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH6578582 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH66874666;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH66874666 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH39688329;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH39688329 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH30956766;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH30956766 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH87033883;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH87033883 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH49085173;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH49085173 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH48217910;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH48217910 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH80157588;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH80157588 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH16456936;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH16456936 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH36852274;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH36852274 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH44704682;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH44704682 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH15311915;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH15311915 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH72662173;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH72662173 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH70038554;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH70038554 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH3726294;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH3726294 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH25639819;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH25639819 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH68819376;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH68819376 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH38012684;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH38012684 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH34361983;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH34361983 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH35526285;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH35526285 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH85029157;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH85029157 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH85235385;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH85235385 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH67245769;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH67245769 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH11047118;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH11047118 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH9017990;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH9017990 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH88559150;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH88559150 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH64735396;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH64735396 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH46624662;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH46624662 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH8133632;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH8133632 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH82003098;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH82003098 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH3986203;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH3986203 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH55277310;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH55277310 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH53309232;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH53309232 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH11074611;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH11074611 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH49114159;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH49114159 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH78637089;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH78637089 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH19149932;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH19149932 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH64125011;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH64125011 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH40079632;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH40079632 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH30328578;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH30328578 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH328591;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH328591 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH83649852;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH83649852 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH43674853;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH43674853 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH49554823;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH49554823 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH20619147;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH20619147 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH13147709;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH13147709 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH32597439;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH32597439 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH63536785;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH63536785 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH2724298;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH2724298 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH97913973;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH97913973 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH92945653;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH92945653 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH61151861;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH61151861 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH47972415;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH47972415 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH85456267;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH85456267 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH99927101;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH99927101 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH21343196;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH21343196 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH99628896;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH99628896 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH28641211;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH28641211 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH21938777;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH21938777 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH98474732;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH98474732 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH84349777;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH84349777 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH1593249;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH1593249 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH72023956;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH72023956 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH34453838;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH34453838 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH32866071;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH32866071 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH89427372;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH89427372 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH62002682;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH62002682 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH61587563;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH61587563 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH20924396;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH20924396 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH25089204;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH25089204 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH6489888;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH6489888 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH4694366;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH4694366 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH97933052;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH97933052 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH4033405;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH4033405 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH35197694;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH35197694 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH1379306;     XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH1379306 = XjkNJlbUsqtOarumGeIntMalRbJXJasdGEhtyeJyqFyrXCPtUGrvTDOKIyvhcEKRxPIWJAobVKxoNjNPLQDDjxKCpWlzjbWluhtH37611708;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void MYzEqbOtJaZYhCuewvukgbWPAGXhySYqSvZTASYWLEqgqlAuZvSzhDC64667809() {     int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh43436659 = -930427927;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh14083691 = -795919955;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh22521867 = 4400003;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh18571541 = 33824885;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh58647436 = -150700982;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh7031210 = -325928126;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh26824170 = -883531077;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh86308866 = 50484253;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh17846747 = 40330431;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh69495251 = -376746561;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh71085842 = 34141490;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh93531813 = -227459287;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh3385968 = -209414443;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh41409692 = -255671574;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh71221472 = -915123426;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh10469233 = -506946587;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh33003336 = -386190834;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh16278987 = -538465296;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh47703835 = -145992452;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh1722520 = -208156137;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh35357464 = -163597863;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh36593200 = -804478802;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh64979980 = -391351360;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh89812712 = -73629258;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh12137476 = 54689560;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh67093886 = -451501235;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh6454071 = -217734257;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh12003367 = -613390304;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh42685985 = -847167635;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh50165564 = -137812918;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh44016708 = -410759688;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh85529063 = -375759079;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh26486916 = -799315584;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh38682309 = -808384991;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh82018814 = -643125786;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh12222158 = -281867581;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh46657431 = -291104885;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh34161167 = -284605210;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh36343100 = -730982023;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh67193561 = -683586408;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh19081340 = -528048811;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh62706522 = -845761819;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh23762605 = -398357760;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh86107242 = -420161837;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh28974973 = -107212578;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh73369631 = 21762194;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh83256389 = -537154508;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh50699969 = -339330309;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh52983396 = -592191519;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh41020803 = -760272769;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh60121430 = -466943274;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh90523970 = -151614660;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh76008842 = -137111915;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh77216673 = -24928173;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh29310345 = -258802010;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh6843459 = -25949126;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh49103711 = -304568595;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh32709154 = -921970740;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh6434065 = 79135325;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh91553549 = -699199747;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh577140 = -8193870;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh14820804 = -170140774;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh43622882 = -102348112;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh67681183 = -821856652;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh25478544 = -965986874;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh85556779 = -590099432;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh67044897 = -428143703;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh64703658 = -401029452;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh59390878 = -612545789;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh58999315 = -533255846;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh63811801 = -115841703;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh98842169 = -1585624;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh79935886 = -807483274;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh80510273 = -462406045;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh82641179 = -680107326;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh72650941 = -317836045;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh12830596 = -306121043;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh78872737 = -971189524;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh60837740 = -966416681;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh38767844 = -967072635;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh83837497 = -914346728;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh55754102 = -878403949;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh59019970 = 78801214;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh1665182 = 13105133;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh90044133 = -670869644;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh53492737 = -159145028;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh9520222 = -138647164;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh49270243 = -674387412;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh9371964 = -449582982;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh75175356 = -517176660;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh63118446 = -977298986;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh13948278 = -369134146;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh27727102 = -263740536;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh44789551 = 68217724;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh66616422 = -575392539;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh4260536 = -257908038;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh19083641 = -643413707;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh56081421 = -576501108;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh60628699 = -454174964;    int kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh43418194 = -930427927;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh43436659 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh14083691;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh14083691 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh22521867;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh22521867 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh18571541;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh18571541 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh58647436;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh58647436 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh7031210;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh7031210 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh26824170;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh26824170 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh86308866;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh86308866 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh17846747;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh17846747 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh69495251;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh69495251 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh71085842;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh71085842 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh93531813;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh93531813 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh3385968;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh3385968 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh41409692;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh41409692 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh71221472;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh71221472 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh10469233;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh10469233 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh33003336;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh33003336 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh16278987;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh16278987 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh47703835;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh47703835 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh1722520;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh1722520 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh35357464;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh35357464 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh36593200;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh36593200 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh64979980;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh64979980 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh89812712;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh89812712 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh12137476;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh12137476 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh67093886;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh67093886 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh6454071;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh6454071 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh12003367;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh12003367 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh42685985;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh42685985 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh50165564;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh50165564 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh44016708;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh44016708 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh85529063;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh85529063 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh26486916;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh26486916 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh38682309;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh38682309 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh82018814;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh82018814 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh12222158;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh12222158 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh46657431;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh46657431 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh34161167;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh34161167 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh36343100;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh36343100 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh67193561;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh67193561 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh19081340;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh19081340 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh62706522;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh62706522 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh23762605;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh23762605 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh86107242;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh86107242 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh28974973;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh28974973 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh73369631;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh73369631 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh83256389;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh83256389 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh50699969;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh50699969 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh52983396;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh52983396 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh41020803;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh41020803 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh60121430;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh60121430 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh90523970;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh90523970 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh76008842;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh76008842 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh77216673;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh77216673 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh29310345;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh29310345 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh6843459;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh6843459 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh49103711;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh49103711 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh32709154;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh32709154 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh6434065;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh6434065 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh91553549;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh91553549 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh577140;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh577140 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh14820804;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh14820804 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh43622882;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh43622882 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh67681183;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh67681183 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh25478544;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh25478544 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh85556779;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh85556779 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh67044897;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh67044897 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh64703658;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh64703658 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh59390878;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh59390878 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh58999315;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh58999315 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh63811801;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh63811801 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh98842169;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh98842169 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh79935886;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh79935886 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh80510273;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh80510273 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh82641179;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh82641179 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh72650941;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh72650941 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh12830596;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh12830596 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh78872737;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh78872737 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh60837740;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh60837740 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh38767844;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh38767844 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh83837497;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh83837497 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh55754102;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh55754102 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh59019970;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh59019970 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh1665182;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh1665182 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh90044133;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh90044133 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh53492737;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh53492737 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh9520222;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh9520222 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh49270243;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh49270243 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh9371964;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh9371964 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh75175356;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh75175356 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh63118446;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh63118446 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh13948278;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh13948278 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh27727102;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh27727102 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh44789551;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh44789551 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh66616422;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh66616422 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh4260536;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh4260536 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh19083641;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh19083641 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh56081421;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh56081421 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh60628699;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh60628699 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh43418194;     kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh43418194 = kCjDkDgvrCiGmdbddXYYckpZPGdHCzZJOtoRRPoSeJzeHfORyRpbJeZzNKoeaZsrdZrsoKMOiFuTlLSjrLIkKxAjUoDGaKsFQHAh43436659;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ktWOFCqqtJnFIvsvGguvccwhZwxoQCqdLwdOkTQBaJSzIueFhjczoql37627561() {     long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc42871763 = -764239296;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc91203994 = -129975753;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc39479858 = -903163685;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc34939998 = -164551759;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc20607513 = -536894902;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc16166409 = -476862907;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc63689016 = -24861895;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc97537636 = -587099951;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc17207256 = -61268948;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc39014893 = -504223626;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc4460252 = -624991811;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc53810748 = -79895600;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc69366729 = -128730912;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc65761407 = -578039615;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc29743991 = -114487632;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc14184047 = -567645586;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc76629043 = -494735733;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc72827873 = -8443523;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc11059428 = -84025984;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc4315879 = -446766739;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc91628642 = 95810742;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc37179960 = -370463653;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc71158061 = -871298124;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc590548 = 80264734;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc50527896 = -234648288;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc68366746 = -334792539;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc32401254 = 13048191;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc41671742 = -689384083;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc83690336 = -169856434;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc86825461 = -593731245;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc50891168 = -735171283;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc46879198 = -599576555;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc70767710 = -12575972;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc51832739 = -160077768;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc86623662 = -729187966;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc40106248 = 19111816;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc96299975 = -607774030;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc84472927 = -998522496;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc48415414 = -102341272;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc59678493 = 95088620;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc44699815 = -459010403;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc90700302 = -775319206;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc65579811 = -554793991;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc34757982 = -331604358;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc40907517 = -484757729;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc4475829 = -599440941;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc96627103 = -139751775;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc87941046 = -321291307;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc65587583 = -827591243;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc59984534 = -597307694;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc39714033 = -155756323;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc13061378 = -46173458;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc88917303 = -428795824;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc44955250 = 59743577;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc45998919 = -79219644;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc5691803 = -293775643;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc20045934 = -258677630;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc38889311 = -883428419;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc84412102 = -929903471;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc52240767 = -102102363;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc83765154 = -389911098;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc22017274 = -335477812;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc13847301 = -317243518;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc30381794 = -467537703;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc88123724 = -769052343;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc57581053 = 74584743;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc83043037 = 32680372;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc17533991 = -968653144;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc79137745 = -848851650;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc89637743 = -33599448;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc17884071 = -959871556;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc92156115 = -496213237;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc24412459 = -906102252;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc51380935 = -79114605;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc59616064 = -987756336;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc928340 = -128870052;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc71600149 = -815669663;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc36400079 = -439693766;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc59683030 = -434977537;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc46052067 = -635207348;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc71739642 = -95040765;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc44460208 = -665660503;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc76084158 = -861792841;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc23705803 = -572548740;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc47111429 = -337974923;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc37829791 = -588997825;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc57961895 = -70780731;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc25812460 = 27680451;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc5833820 = 19141875;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc80931860 = -335412323;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc20060315 = -722210554;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc57410665 = -724345611;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc60826 = 31380975;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc96174647 = 99761091;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc75913338 = -515000282;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc22682541 = -23532592;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc76853002 = -358075689;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc35198018 = 12743712;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc46634258 = -562552015;    long IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc83326463 = -764239296;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc42871763 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc91203994;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc91203994 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc39479858;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc39479858 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc34939998;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc34939998 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc20607513;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc20607513 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc16166409;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc16166409 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc63689016;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc63689016 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc97537636;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc97537636 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc17207256;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc17207256 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc39014893;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc39014893 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc4460252;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc4460252 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc53810748;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc53810748 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc69366729;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc69366729 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc65761407;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc65761407 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc29743991;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc29743991 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc14184047;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc14184047 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc76629043;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc76629043 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc72827873;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc72827873 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc11059428;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc11059428 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc4315879;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc4315879 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc91628642;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc91628642 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc37179960;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc37179960 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc71158061;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc71158061 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc590548;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc590548 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc50527896;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc50527896 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc68366746;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc68366746 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc32401254;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc32401254 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc41671742;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc41671742 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc83690336;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc83690336 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc86825461;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc86825461 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc50891168;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc50891168 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc46879198;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc46879198 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc70767710;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc70767710 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc51832739;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc51832739 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc86623662;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc86623662 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc40106248;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc40106248 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc96299975;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc96299975 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc84472927;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc84472927 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc48415414;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc48415414 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc59678493;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc59678493 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc44699815;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc44699815 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc90700302;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc90700302 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc65579811;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc65579811 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc34757982;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc34757982 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc40907517;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc40907517 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc4475829;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc4475829 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc96627103;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc96627103 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc87941046;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc87941046 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc65587583;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc65587583 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc59984534;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc59984534 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc39714033;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc39714033 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc13061378;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc13061378 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc88917303;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc88917303 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc44955250;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc44955250 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc45998919;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc45998919 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc5691803;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc5691803 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc20045934;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc20045934 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc38889311;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc38889311 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc84412102;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc84412102 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc52240767;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc52240767 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc83765154;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc83765154 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc22017274;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc22017274 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc13847301;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc13847301 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc30381794;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc30381794 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc88123724;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc88123724 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc57581053;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc57581053 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc83043037;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc83043037 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc17533991;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc17533991 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc79137745;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc79137745 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc89637743;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc89637743 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc17884071;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc17884071 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc92156115;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc92156115 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc24412459;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc24412459 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc51380935;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc51380935 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc59616064;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc59616064 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc928340;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc928340 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc71600149;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc71600149 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc36400079;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc36400079 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc59683030;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc59683030 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc46052067;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc46052067 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc71739642;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc71739642 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc44460208;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc44460208 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc76084158;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc76084158 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc23705803;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc23705803 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc47111429;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc47111429 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc37829791;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc37829791 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc57961895;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc57961895 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc25812460;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc25812460 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc5833820;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc5833820 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc80931860;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc80931860 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc20060315;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc20060315 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc57410665;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc57410665 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc60826;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc60826 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc96174647;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc96174647 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc75913338;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc75913338 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc22682541;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc22682541 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc76853002;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc76853002 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc35198018;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc35198018 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc46634258;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc46634258 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc83326463;     IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc83326463 = IxpoIFBZqSKuLSeKnPsVTJxYfSrGgZroVLUQveHwYjbgxASnCZpjvUSyzQJqNvsRbVZAxNPIIwxjiRbezEDHkHsAsWRfVRicqKEc42871763;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void CuccUWOIzyDjhJfeYCAwCvnezyZdSXjCBlTUztmbASzmuysWRogEkLa62829845() {     long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK71648728 = -952425675;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK38223097 = -734983377;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK77782143 = -225561527;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK96457029 = 6669156;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK69957370 = -508037797;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK70107277 = -625649347;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK50722302 = 28046530;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK21138078 = -582044382;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK53442874 = -883167068;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK82207586 = 32587789;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK15906105 = -108695256;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK82834787 = -452046482;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK87373018 = -160168797;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK60110309 = -344532497;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK50718599 = -226814686;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK59934767 = -291156648;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK82254683 = -513710071;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK32083798 = -986327176;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK13135509 = 67146160;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK20196291 = -755035488;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK72433490 = -546567885;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK54667221 = -115155082;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK12270731 = -142359584;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK21168954 = -54360891;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK53236617 = -174775301;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK55616102 = -28005327;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK20309234 = -761791298;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK98961416 = -869221083;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK5230648 = -944257331;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK60403134 = -166685876;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK84106311 = -598242427;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK48441730 = -886918629;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK17555567 = -580818967;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK99901031 = -924922594;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK95562312 = -127617291;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK50825269 = -780447986;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK14092402 = -835503346;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK51909880 = -823589353;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK17268037 = -230653300;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK28933019 = -318059675;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK98100673 = -93897015;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK75908752 = -29279605;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK18713418 = -183606424;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK36132320 = -828036350;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK40904055 = 73084112;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK69604159 = -174351591;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK11564408 = -107559865;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK34951495 = -459883365;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK91408707 = -640990859;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK90566201 = -838358628;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK64376681 = -976552258;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK8274537 = -353658283;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK22198456 = -684735700;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK32716794 = -826623029;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK90396355 = -645595641;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK16981507 = -737270594;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK25952367 = -492623794;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK56613189 = -71200636;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK43220413 = -818555543;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK14341269 = -380032470;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK49798044 = -863858050;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK51760885 = -102732387;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK15907430 = -637787052;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK93039739 = -616481193;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK98101275 = -369169784;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK67464375 = -221776627;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK65279220 = -871227516;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK87471987 = -235246204;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK64547997 = -116915206;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK99893329 = -446366701;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK45842365 = -455653303;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK30344804 = -690120718;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK14815762 = -655673876;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK84202490 = -614794166;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK22095618 = -561138473;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK96524737 = -417288281;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK35953803 = -931548658;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK76138410 = -314323235;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK80264898 = -27445004;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK83632457 = 99576290;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK44051694 = -920445463;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK85357739 = -201907933;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK7552710 = -128230225;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK14664446 = -5898703;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK96026452 = -190133619;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK75831774 = -144584144;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK26243274 = -102182930;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK84838773 = -754195939;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK9504677 = -179326954;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK78580805 = -390346697;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK24872903 = -187824192;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK57479213 = -664302710;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK8689468 = 94966190;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK2926769 = -850620830;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK79134975 = -454201626;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK46339788 = -991164629;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK60001322 = -391492553;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK25673678 = -567125232;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK38031044 = -358866566;    long GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK73439679 = -952425675;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK71648728 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK38223097;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK38223097 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK77782143;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK77782143 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK96457029;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK96457029 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK69957370;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK69957370 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK70107277;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK70107277 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK50722302;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK50722302 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK21138078;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK21138078 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK53442874;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK53442874 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK82207586;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK82207586 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK15906105;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK15906105 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK82834787;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK82834787 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK87373018;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK87373018 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK60110309;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK60110309 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK50718599;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK50718599 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK59934767;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK59934767 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK82254683;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK82254683 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK32083798;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK32083798 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK13135509;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK13135509 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK20196291;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK20196291 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK72433490;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK72433490 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK54667221;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK54667221 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK12270731;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK12270731 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK21168954;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK21168954 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK53236617;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK53236617 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK55616102;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK55616102 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK20309234;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK20309234 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK98961416;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK98961416 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK5230648;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK5230648 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK60403134;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK60403134 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK84106311;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK84106311 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK48441730;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK48441730 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK17555567;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK17555567 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK99901031;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK99901031 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK95562312;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK95562312 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK50825269;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK50825269 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK14092402;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK14092402 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK51909880;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK51909880 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK17268037;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK17268037 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK28933019;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK28933019 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK98100673;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK98100673 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK75908752;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK75908752 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK18713418;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK18713418 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK36132320;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK36132320 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK40904055;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK40904055 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK69604159;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK69604159 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK11564408;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK11564408 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK34951495;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK34951495 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK91408707;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK91408707 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK90566201;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK90566201 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK64376681;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK64376681 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK8274537;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK8274537 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK22198456;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK22198456 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK32716794;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK32716794 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK90396355;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK90396355 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK16981507;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK16981507 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK25952367;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK25952367 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK56613189;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK56613189 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK43220413;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK43220413 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK14341269;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK14341269 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK49798044;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK49798044 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK51760885;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK51760885 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK15907430;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK15907430 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK93039739;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK93039739 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK98101275;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK98101275 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK67464375;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK67464375 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK65279220;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK65279220 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK87471987;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK87471987 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK64547997;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK64547997 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK99893329;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK99893329 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK45842365;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK45842365 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK30344804;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK30344804 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK14815762;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK14815762 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK84202490;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK84202490 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK22095618;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK22095618 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK96524737;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK96524737 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK35953803;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK35953803 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK76138410;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK76138410 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK80264898;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK80264898 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK83632457;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK83632457 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK44051694;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK44051694 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK85357739;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK85357739 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK7552710;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK7552710 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK14664446;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK14664446 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK96026452;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK96026452 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK75831774;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK75831774 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK26243274;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK26243274 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK84838773;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK84838773 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK9504677;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK9504677 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK78580805;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK78580805 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK24872903;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK24872903 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK57479213;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK57479213 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK8689468;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK8689468 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK2926769;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK2926769 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK79134975;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK79134975 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK46339788;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK46339788 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK60001322;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK60001322 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK25673678;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK25673678 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK38031044;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK38031044 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK73439679;     GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK73439679 = GGydWwkqbPAhvpKefQZwjwAjAFCMereRqLNNBPQePcYXtRwhHJBoLPqJGPcQuYLkljqkcwcPzvgGYdpjJVFUpqGbUDHmmAjrQnlK71648728;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void nNdgJDYprJuZMUvUKqmqbWgWnnAosQJCzzpLZZQvVWaAoUlMPBzgyeO96422451() {     long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH6549272 = -279989802;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH90043162 = 93004343;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH30813399 = -841663676;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH99968981 = -192317729;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH79362614 = -531475241;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH80659915 = -239499211;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH29697218 = -353473555;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH25284358 = -59685410;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH71580823 = -659451784;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH69990426 = -122769402;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH67141643 = -610364215;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH29390193 = -433125204;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH59735512 = -609839297;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH83758667 = -298110895;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH22263525 = 33108945;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH72626334 = -754871603;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH25863568 = -229614952;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH65392343 = -132661849;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH56613162 = -669468527;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH44553106 = -808182704;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH62122106 = -233967594;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH80379240 = -357784344;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH52095795 = -900980689;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH82089625 = -530370981;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH71202299 = -61404494;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH51013056 = -791667510;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH36455409 = -36493278;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH33954693 = -506020049;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH80224767 = -194970617;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH26506573 = -981680001;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH76151324 = 12458725;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH62891025 = -491885758;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH88601949 = -839731117;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH42517051 = -464627452;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH83617575 = 94534204;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH87442014 = -614268149;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH1205396 = -917215153;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH83519365 = -424899541;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH24417317 = -740207409;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH13816590 = -495215507;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH69315088 = -806113623;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH7569998 = -459590200;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH89630643 = -434093187;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH65682499 = -686846837;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH68835005 = -757712799;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH6243717 = 77566312;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH16582493 = -812862871;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH14535303 = 390400;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH8247172 = -976363602;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH14014099 = -479395483;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH11480638 = -961086857;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH99750158 = -79723680;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH28279717 = -840411155;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH6196947 = -401539928;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH34300345 = 44395857;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH26170032 = -922205459;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH37947368 = -6014968;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH48723774 = -211292696;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH28766683 = -30913235;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH28349558 = -739807732;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH44204506 = -103005934;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH95742525 = -847453507;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH45059591 = -864714793;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH45074251 = -677771784;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH93839101 = -35228128;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH4250618 = -18478457;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH40788244 = -593394088;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH17218462 = -45211846;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH141092 = -292645099;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH34821511 = -352622907;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH71420938 = -837656450;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH42344203 = -804715411;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH40975027 = -392454441;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH42796573 = -74253020;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH75238017 = 97930918;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH54552109 = -774377395;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH90748596 = -923691157;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH86413295 = -114133853;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH13254620 = -772658182;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH64958582 = -38970807;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH34430563 = -978804640;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH21920107 = 63116322;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH25707521 = -529656447;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH66210668 = -715575134;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH15025935 = 79406856;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH76401166 = -907817595;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH34611308 = -651474604;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH82405002 = -338191189;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH8216706 = -409023310;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH57447544 = 16739662;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH49494646 = -508253181;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH52481621 = -605922458;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH54752682 = -293986306;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH96067758 = 99600322;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH69612083 = -292209573;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH73572563 = -958660117;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH62510406 = -594875408;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH44556393 = -756321403;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH71843397 = -551618710;    long RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH64584387 = -279989802;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH6549272 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH90043162;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH90043162 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH30813399;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH30813399 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH99968981;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH99968981 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH79362614;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH79362614 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH80659915;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH80659915 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH29697218;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH29697218 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH25284358;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH25284358 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH71580823;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH71580823 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH69990426;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH69990426 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH67141643;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH67141643 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH29390193;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH29390193 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH59735512;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH59735512 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH83758667;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH83758667 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH22263525;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH22263525 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH72626334;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH72626334 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH25863568;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH25863568 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH65392343;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH65392343 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH56613162;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH56613162 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH44553106;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH44553106 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH62122106;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH62122106 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH80379240;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH80379240 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH52095795;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH52095795 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH82089625;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH82089625 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH71202299;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH71202299 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH51013056;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH51013056 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH36455409;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH36455409 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH33954693;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH33954693 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH80224767;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH80224767 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH26506573;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH26506573 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH76151324;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH76151324 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH62891025;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH62891025 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH88601949;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH88601949 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH42517051;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH42517051 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH83617575;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH83617575 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH87442014;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH87442014 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH1205396;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH1205396 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH83519365;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH83519365 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH24417317;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH24417317 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH13816590;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH13816590 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH69315088;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH69315088 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH7569998;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH7569998 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH89630643;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH89630643 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH65682499;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH65682499 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH68835005;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH68835005 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH6243717;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH6243717 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH16582493;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH16582493 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH14535303;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH14535303 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH8247172;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH8247172 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH14014099;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH14014099 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH11480638;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH11480638 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH99750158;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH99750158 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH28279717;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH28279717 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH6196947;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH6196947 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH34300345;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH34300345 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH26170032;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH26170032 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH37947368;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH37947368 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH48723774;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH48723774 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH28766683;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH28766683 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH28349558;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH28349558 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH44204506;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH44204506 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH95742525;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH95742525 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH45059591;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH45059591 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH45074251;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH45074251 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH93839101;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH93839101 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH4250618;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH4250618 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH40788244;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH40788244 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH17218462;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH17218462 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH141092;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH141092 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH34821511;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH34821511 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH71420938;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH71420938 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH42344203;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH42344203 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH40975027;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH40975027 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH42796573;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH42796573 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH75238017;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH75238017 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH54552109;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH54552109 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH90748596;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH90748596 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH86413295;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH86413295 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH13254620;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH13254620 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH64958582;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH64958582 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH34430563;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH34430563 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH21920107;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH21920107 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH25707521;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH25707521 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH66210668;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH66210668 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH15025935;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH15025935 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH76401166;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH76401166 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH34611308;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH34611308 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH82405002;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH82405002 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH8216706;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH8216706 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH57447544;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH57447544 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH49494646;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH49494646 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH52481621;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH52481621 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH54752682;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH54752682 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH96067758;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH96067758 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH69612083;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH69612083 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH73572563;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH73572563 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH62510406;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH62510406 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH44556393;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH44556393 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH71843397;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH71843397 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH64584387;     RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH64584387 = RgGtWDkxazsZMfkulZrVEwghSBnZqRdDUlYeSguslZzBwuYheGNGHJJUAYaUPLGJVoFbHFcFfyjfxelXdGukDXwLbDdtFZdZSpgH6549272;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void khgomLzDnoOAOcpzrerhoORaqTAiXtZdbLpthEZeebpQIUjRMfmBKKP12574017() {     float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF38524396 = -258805320;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF49111908 = -818938003;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF73225580 = -826869210;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF92201254 = -22012176;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF99880222 = 41516579;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF36726944 = -682658277;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF29895610 = 39150970;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF88261063 = 35284924;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF35982604 = -993377910;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF40577918 = -177778177;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF55379191 = -407871145;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF37828940 = -998239700;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF87314398 = -886808228;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF27052534 = -611419313;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF62771744 = -340286354;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF81842184 = -532906600;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF81463972 = -759629264;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF39787757 = -75080172;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF78872333 = -66168115;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF43078702 = -838256375;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF43053112 = -246558694;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF35554389 = -17442389;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF93678938 = -40053663;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF77882286 = 40147272;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF93273912 = 52531475;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF79448553 = -705436615;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF59661877 = -619559409;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF49231718 = -27064828;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF2749730 = -861408245;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF44249557 = -543248327;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF87122296 = -647943298;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF44102299 = -400952312;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF75538187 = -226451754;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF84783727 = -961490399;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF17731849 = 58425390;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF61260017 = -516027290;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF75203495 = -792508462;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF72902903 = -231055751;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF85885386 = -925811728;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF71669074 = -142110092;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF41109857 = -512882758;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF48279646 = -464680412;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF86414277 = -203981418;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF38405996 = -554330779;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF92829153 = -329748598;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF29672088 = -937662780;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF18990853 = -234729568;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF25059848 = -574849515;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF90419710 = -939722747;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF1322016 = -426449311;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF37410317 = -575465117;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF48370640 = -134468403;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF1320071 = -892338349;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF2570854 = -227289506;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF69520905 = -856366442;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF2970008 = -141362931;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF55432970 = -678884341;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF95343294 = -767016482;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF98927341 = 25456349;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF20431669 = -253046807;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF77065066 = 36901132;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF80663891 = -933784203;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF85511334 = -103306831;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF91733047 = -350129584;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF53455622 = -529834880;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF11276893 = 93081166;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF62290753 = -671787947;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF2530672 = -925317830;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF9320685 = -569844703;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF1511727 = -824259065;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF6638690 = -740398138;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF8561070 = -428573514;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF53902370 = -149268445;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF7203259 = -924058024;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF1968845 = -225373617;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF94773466 = -781878282;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF49140111 = -813460971;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF55272942 = -485722885;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF85053133 = -630104131;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF63601824 = -9805746;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF60457701 = -370707047;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF34602030 = 55290106;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF58812007 = -87342081;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF1427714 = -334958934;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF6839240 = -967783210;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF38751656 = -413474895;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF42782229 = -508613963;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF72967334 = -999162248;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF15262823 = -5123957;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF14761842 = -800211680;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF5827048 = -837142950;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF79860200 = 74508020;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF73975561 = -156512101;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF65453718 = -572764921;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF94604008 = -79011224;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF60445965 = -579098556;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF62768311 = -261373582;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF94681230 = -853851834;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF84950374 = 75504101;    float WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF81552260 = -258805320;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF38524396 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF49111908;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF49111908 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF73225580;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF73225580 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF92201254;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF92201254 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF99880222;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF99880222 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF36726944;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF36726944 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF29895610;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF29895610 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF88261063;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF88261063 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF35982604;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF35982604 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF40577918;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF40577918 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF55379191;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF55379191 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF37828940;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF37828940 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF87314398;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF87314398 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF27052534;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF27052534 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF62771744;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF62771744 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF81842184;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF81842184 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF81463972;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF81463972 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF39787757;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF39787757 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF78872333;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF78872333 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF43078702;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF43078702 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF43053112;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF43053112 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF35554389;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF35554389 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF93678938;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF93678938 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF77882286;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF77882286 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF93273912;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF93273912 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF79448553;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF79448553 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF59661877;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF59661877 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF49231718;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF49231718 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF2749730;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF2749730 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF44249557;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF44249557 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF87122296;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF87122296 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF44102299;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF44102299 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF75538187;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF75538187 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF84783727;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF84783727 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF17731849;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF17731849 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF61260017;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF61260017 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF75203495;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF75203495 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF72902903;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF72902903 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF85885386;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF85885386 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF71669074;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF71669074 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF41109857;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF41109857 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF48279646;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF48279646 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF86414277;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF86414277 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF38405996;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF38405996 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF92829153;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF92829153 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF29672088;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF29672088 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF18990853;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF18990853 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF25059848;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF25059848 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF90419710;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF90419710 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF1322016;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF1322016 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF37410317;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF37410317 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF48370640;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF48370640 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF1320071;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF1320071 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF2570854;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF2570854 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF69520905;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF69520905 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF2970008;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF2970008 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF55432970;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF55432970 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF95343294;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF95343294 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF98927341;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF98927341 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF20431669;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF20431669 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF77065066;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF77065066 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF80663891;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF80663891 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF85511334;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF85511334 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF91733047;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF91733047 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF53455622;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF53455622 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF11276893;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF11276893 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF62290753;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF62290753 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF2530672;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF2530672 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF9320685;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF9320685 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF1511727;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF1511727 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF6638690;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF6638690 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF8561070;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF8561070 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF53902370;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF53902370 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF7203259;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF7203259 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF1968845;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF1968845 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF94773466;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF94773466 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF49140111;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF49140111 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF55272942;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF55272942 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF85053133;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF85053133 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF63601824;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF63601824 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF60457701;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF60457701 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF34602030;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF34602030 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF58812007;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF58812007 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF1427714;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF1427714 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF6839240;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF6839240 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF38751656;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF38751656 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF42782229;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF42782229 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF72967334;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF72967334 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF15262823;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF15262823 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF14761842;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF14761842 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF5827048;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF5827048 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF79860200;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF79860200 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF73975561;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF73975561 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF65453718;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF65453718 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF94604008;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF94604008 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF60445965;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF60445965 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF62768311;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF62768311 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF94681230;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF94681230 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF84950374;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF84950374 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF81552260;     WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF81552260 = WQGxTKQguPlZCnwSnEMCMYARWjjWrGYSQwSdaGGUOuvAXuhdPMzxyHCxPpbkbuonrSJFUSrgyroRdAtbBFGfGaUIuVNlfVtTGHJF38524396;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void jBwIwzcEvhyvxrwHevmxAccSLyGWMkwjQvJpdaeFZHSVUeoNTMcAHby73318777() {     float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS63830461 = -214482034;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS64783045 = -170146116;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS13927146 = -554548283;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS3567483 = -218252976;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS95782214 = -514325008;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS40901103 = -513390264;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS69375209 = -261517413;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS74278548 = -812100398;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS69622068 = -33578609;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS46176364 = -757674797;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS76239647 = 31870353;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS46140225 = -400427582;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS30959100 = -599885591;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS3865998 = -24551101;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS75715836 = -397157989;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS4138361 = -833049751;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS75148562 = -42414223;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS27677838 = -127810835;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS61800716 = 40832395;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS19499967 = -625988826;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS32363255 = -723320985;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS48202743 = -515171802;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS32092579 = -94640228;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS13160192 = -351294451;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS53150917 = 3713335;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS51287081 = -807429844;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS69912587 = -469581463;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS10262943 = -540240456;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS74789894 = -436011337;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS77857061 = -292401365;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS45899821 = -945249509;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS19605370 = -40746003;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS26139425 = -729930977;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS44804595 = -205140893;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS30260242 = -6384649;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS8579818 = -843249434;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS93699468 = -831528290;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS38672630 = -989097880;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS15188325 = -163488966;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS90758767 = -418027054;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS57142542 = 90548205;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS13437296 = -841601569;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS26381421 = -31240788;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS53908696 = -99985416;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS48767275 = -770912590;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS91411524 = -764789550;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS61595768 = -377856751;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS14101367 = 95367818;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS38203930 = -825216905;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS54591164 = -949477484;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS80713181 = -879252740;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS79624294 = -618754108;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS88123730 = -560051849;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS50213915 = -784057487;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS40955523 = -263216038;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS15627718 = -699310232;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS32690466 = 24494112;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS766955 = -103253833;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS50416565 = -121966312;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS44495133 = -706895164;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS70988516 = 56191198;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS59112266 = -721276957;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS99488653 = -276089061;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS91765007 = -741177244;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS276543 = -812425289;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS56634277 = -927383644;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS20000800 = -670496606;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS86154505 = -294744699;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS73605756 = 81833547;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS67136018 = -553908555;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS10438893 = 98478539;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS36475932 = -53316344;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS12489514 = -964321870;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS71041948 = -541140552;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS62357425 = -616537032;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS18925959 = -881719417;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS21821323 = -383931015;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS78183883 = -994654813;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS64392917 = -580381861;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS61739392 = -231497116;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS89691313 = -329573094;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS55811221 = -464949281;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS72059013 = -715023551;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS20198730 = -486533854;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS97143880 = -413148626;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS66275526 = -226495402;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS31481640 = -480694155;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS75925509 = -945873490;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS3849072 = -941924856;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS14632524 = -307074418;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS75889351 = -767743547;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS92932514 = -628274458;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS88256065 = -767131568;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS70693191 = -456593802;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS19770252 = -374218252;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS98030276 = -188174838;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS13948642 = -465512508;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS34616414 = -290063544;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS53632153 = -287560128;    float HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS92132998 = -214482034;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS63830461 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS64783045;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS64783045 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS13927146;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS13927146 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS3567483;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS3567483 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS95782214;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS95782214 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS40901103;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS40901103 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS69375209;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS69375209 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS74278548;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS74278548 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS69622068;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS69622068 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS46176364;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS46176364 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS76239647;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS76239647 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS46140225;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS46140225 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS30959100;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS30959100 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS3865998;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS3865998 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS75715836;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS75715836 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS4138361;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS4138361 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS75148562;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS75148562 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS27677838;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS27677838 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS61800716;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS61800716 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS19499967;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS19499967 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS32363255;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS32363255 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS48202743;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS48202743 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS32092579;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS32092579 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS13160192;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS13160192 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS53150917;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS53150917 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS51287081;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS51287081 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS69912587;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS69912587 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS10262943;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS10262943 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS74789894;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS74789894 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS77857061;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS77857061 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS45899821;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS45899821 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS19605370;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS19605370 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS26139425;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS26139425 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS44804595;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS44804595 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS30260242;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS30260242 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS8579818;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS8579818 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS93699468;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS93699468 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS38672630;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS38672630 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS15188325;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS15188325 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS90758767;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS90758767 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS57142542;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS57142542 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS13437296;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS13437296 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS26381421;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS26381421 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS53908696;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS53908696 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS48767275;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS48767275 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS91411524;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS91411524 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS61595768;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS61595768 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS14101367;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS14101367 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS38203930;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS38203930 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS54591164;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS54591164 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS80713181;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS80713181 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS79624294;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS79624294 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS88123730;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS88123730 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS50213915;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS50213915 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS40955523;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS40955523 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS15627718;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS15627718 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS32690466;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS32690466 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS766955;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS766955 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS50416565;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS50416565 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS44495133;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS44495133 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS70988516;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS70988516 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS59112266;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS59112266 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS99488653;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS99488653 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS91765007;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS91765007 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS276543;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS276543 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS56634277;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS56634277 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS20000800;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS20000800 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS86154505;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS86154505 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS73605756;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS73605756 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS67136018;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS67136018 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS10438893;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS10438893 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS36475932;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS36475932 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS12489514;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS12489514 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS71041948;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS71041948 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS62357425;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS62357425 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS18925959;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS18925959 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS21821323;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS21821323 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS78183883;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS78183883 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS64392917;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS64392917 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS61739392;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS61739392 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS89691313;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS89691313 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS55811221;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS55811221 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS72059013;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS72059013 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS20198730;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS20198730 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS97143880;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS97143880 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS66275526;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS66275526 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS31481640;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS31481640 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS75925509;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS75925509 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS3849072;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS3849072 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS14632524;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS14632524 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS75889351;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS75889351 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS92932514;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS92932514 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS88256065;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS88256065 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS70693191;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS70693191 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS19770252;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS19770252 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS98030276;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS98030276 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS13948642;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS13948642 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS34616414;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS34616414 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS53632153;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS53632153 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS92132998;     HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS92132998 = HYdxxtGsKkOfBiHdDQhYtJGyhJtCAHwbowBQAdgMTOHkFyneLiZkaDFxSeQozDsDomrEgrcajUEeBVsiWOqRiMBkrExVMsMQWEwS63830461;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void qzlSrAPVIEzGIyPUVCEmZYTvkSfjhabMKxHAWqdmipqwdkTYoivNXro28837488() {     float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr60340146 = -699544793;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr49152029 = -144131981;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr20266063 = -831215356;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr8656261 = -47337182;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr68854655 = -304089664;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr95550691 = -393634246;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr27463530 = -828703621;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr44337743 = -777073241;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr15246408 = -692819398;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr98500656 = -784803446;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr46616065 = 76899080;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr68302500 = -836899669;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr52156255 = -346500492;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr47863221 = -706649162;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr3201649 = -229841125;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr4377458 = -208068792;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr30765791 = -965068552;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr25313593 = -393872711;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr3937827 = -757286039;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr96262107 = -841525882;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr79876821 = -789103770;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr78252633 = -598185437;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr40028740 = -55038860;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr58810016 = -250872116;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr95647268 = -285059351;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr85598484 = -940828071;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr2920063 = -447163166;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr20215068 = -500480049;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr63325089 = -74424478;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr66156504 = -594893894;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr71700240 = -340764280;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr47717482 = -568662904;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr86310074 = -170999853;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr57605682 = -513991759;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr80924099 = -350707137;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr73665166 = -610209016;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr30227119 = -941778937;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr46758444 = -807861188;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr81579429 = -310898424;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr56212613 = -209090779;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr83341370 = 65034085;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr50479478 = -345938572;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr94065036 = -807078487;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr45732753 = -20101391;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr56763016 = -989696628;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr9306536 = -453139681;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr72356757 = -897017711;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr82283180 = -922106859;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr16142193 = -688603031;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr37414915 = 7470618;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr39131506 = -197909450;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr59306561 = -841992232;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr67991283 = -747987497;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr40846246 = -950218417;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr48960667 = -574387469;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr82087513 = -1359357;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr9123290 = 10906878;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr61456046 = -480343240;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr13008992 = -762277832;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr83256171 = -363261594;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr92630628 = -946471081;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr7248462 = -228223573;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr81012654 = -602648763;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr49089903 = 2074496;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr26800417 = -344039166;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr98898583 = -354438016;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr81992425 = -565899817;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr94550573 = -832508733;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr66939121 = -255942025;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr29536482 = -619632110;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr74150339 = -266289855;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr84007346 = -57207365;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr43734163 = 17025713;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr47725214 = -448195260;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr12920737 = -806559968;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr29397344 = -343165199;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr84187596 = -791106950;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr94295986 = 65062531;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr2047001 = -261175488;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr86340733 = -831919670;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr13241728 = 56189639;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr20636883 = -525056307;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr4072876 = -811877019;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr25910174 = 18104904;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr27024998 = -296984445;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr12393679 = -498772048;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr79726199 = -820675408;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr45463829 = -220781436;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr8645015 = -939604290;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr98836586 = -249347781;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr64541877 = -521115895;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr68771072 = -361435698;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr33749453 = 54416643;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr98323258 = -947636831;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr63581984 = -262619699;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr76092909 = -706742343;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr69466823 = -743289809;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr44975133 = -709152984;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr18932337 = -676062225;    float SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr57864433 = -699544793;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr60340146 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr49152029;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr49152029 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr20266063;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr20266063 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr8656261;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr8656261 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr68854655;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr68854655 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr95550691;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr95550691 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr27463530;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr27463530 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr44337743;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr44337743 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr15246408;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr15246408 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr98500656;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr98500656 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr46616065;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr46616065 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr68302500;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr68302500 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr52156255;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr52156255 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr47863221;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr47863221 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr3201649;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr3201649 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr4377458;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr4377458 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr30765791;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr30765791 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr25313593;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr25313593 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr3937827;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr3937827 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr96262107;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr96262107 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr79876821;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr79876821 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr78252633;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr78252633 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr40028740;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr40028740 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr58810016;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr58810016 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr95647268;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr95647268 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr85598484;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr85598484 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr2920063;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr2920063 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr20215068;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr20215068 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr63325089;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr63325089 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr66156504;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr66156504 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr71700240;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr71700240 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr47717482;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr47717482 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr86310074;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr86310074 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr57605682;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr57605682 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr80924099;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr80924099 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr73665166;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr73665166 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr30227119;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr30227119 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr46758444;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr46758444 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr81579429;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr81579429 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr56212613;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr56212613 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr83341370;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr83341370 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr50479478;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr50479478 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr94065036;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr94065036 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr45732753;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr45732753 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr56763016;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr56763016 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr9306536;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr9306536 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr72356757;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr72356757 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr82283180;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr82283180 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr16142193;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr16142193 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr37414915;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr37414915 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr39131506;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr39131506 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr59306561;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr59306561 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr67991283;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr67991283 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr40846246;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr40846246 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr48960667;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr48960667 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr82087513;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr82087513 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr9123290;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr9123290 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr61456046;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr61456046 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr13008992;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr13008992 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr83256171;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr83256171 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr92630628;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr92630628 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr7248462;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr7248462 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr81012654;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr81012654 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr49089903;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr49089903 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr26800417;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr26800417 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr98898583;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr98898583 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr81992425;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr81992425 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr94550573;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr94550573 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr66939121;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr66939121 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr29536482;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr29536482 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr74150339;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr74150339 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr84007346;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr84007346 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr43734163;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr43734163 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr47725214;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr47725214 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr12920737;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr12920737 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr29397344;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr29397344 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr84187596;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr84187596 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr94295986;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr94295986 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr2047001;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr2047001 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr86340733;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr86340733 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr13241728;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr13241728 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr20636883;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr20636883 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr4072876;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr4072876 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr25910174;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr25910174 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr27024998;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr27024998 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr12393679;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr12393679 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr79726199;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr79726199 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr45463829;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr45463829 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr8645015;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr8645015 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr98836586;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr98836586 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr64541877;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr64541877 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr68771072;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr68771072 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr33749453;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr33749453 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr98323258;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr98323258 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr63581984;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr63581984 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr76092909;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr76092909 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr69466823;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr69466823 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr44975133;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr44975133 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr18932337;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr18932337 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr57864433;     SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr57864433 = SgphZUfnicnvKqPxfYREjiWSAyHvyeNoTJpTgJERCBHTqoJhYkQpFnEkKXhaNjoDPycBBvRWUABhaCbLpZXOOYifEFJIbdETNDEr60340146;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void vmGYDQKkhsWSUdYkfIDfSiVTaXAynuiVSUMiNvQffDJxpGRTBsXkaVk10847959() {     double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj56577090 = -742727024;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj14222690 = -271253057;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj33114158 = 24028649;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj94309477 = -244798465;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj59646981 = -134418299;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj2559731 = -250196401;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj51163270 = -309750538;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj16190249 = -404572210;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj86440754 = -182390771;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj40625500 = -320460320;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj3198780 = -268430736;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj49166727 = -496372369;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj8564420 = -20285915;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj23269972 = -482201666;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj42190556 = -268137086;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj44627141 = -214243855;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj24416734 = -562573477;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj66722991 = -899316268;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj47110329 = -47447838;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj16210284 = -258331563;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj36021841 = -59482692;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj41151505 = -249203672;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj45736346 = -666974110;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj94373596 = -802122002;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj14674797 = -628460181;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj45685202 = -603563058;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj93568757 = -408154076;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj91896093 = -135266049;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj3344789 = -605076545;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj58651091 = 37801473;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj818872 = -967844996;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj29418875 = -70755901;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj90442487 = -565782599;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj76557727 = -133666415;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj60353324 = -899089828;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj38450276 = -107030280;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj23663991 = -510884089;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj75123619 = -440689121;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj1036298 = -724965383;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj60099585 = -196669461;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj90565935 = -894044983;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj22972059 = -624366146;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj92232216 = -822438921;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj23034333 = -560491962;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj44697951 = -137364140;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj82112692 = -734024374;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj98256414 = 54443630;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj56010161 = -467420001;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj72394965 = -774043227;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj99652396 = -123561414;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj17457078 = 6859827;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj28436646 = -989291133;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj41140543 = -143684088;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj99972459 = -826163695;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj74826117 = -60418801;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj15425586 = -393523353;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj68486343 = -604278948;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj38740562 = -173849350;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj79634680 = -616338284;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj13961780 = -530855242;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj8990974 = -842042326;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj59267176 = -74484489;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj12845460 = -799495665;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj27789664 = -120192245;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj39806629 = -352615324;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj73779905 = -97674836;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj58724240 = -930589771;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj32006692 = -886619501;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj62916647 = -583111838;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj3740280 = -61106807;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj20963151 = -703359767;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj49293114 = -21884356;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj65686694 = -74350886;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj87010744 = -850778378;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj25644348 = -364286580;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj13049783 = -435116547;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj48919289 = -426764752;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj22702013 = -6482149;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj49675645 = -564757862;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj32562105 = -894435808;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj47428788 = -558006688;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj37558597 = -940734075;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj19501129 = -361222823;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj3692393 = -381515132;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj41194013 = -969058354;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj72382225 = -978553863;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj88278331 = -927071814;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj90470028 = -739618904;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj1731611 = 26752386;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj44927739 = -405566476;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj69963932 = -502751332;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj84923428 = -237034740;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj95488939 = -824350837;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj87074517 = -94110142;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj51108612 = -354627135;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj31298759 = -719560494;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj10126600 = -824870481;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj64442552 = -602246676;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj83227704 = -107876638;    double MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj70918046 = -742727024;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj56577090 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj14222690;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj14222690 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj33114158;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj33114158 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj94309477;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj94309477 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj59646981;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj59646981 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj2559731;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj2559731 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj51163270;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj51163270 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj16190249;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj16190249 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj86440754;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj86440754 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj40625500;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj40625500 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj3198780;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj3198780 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj49166727;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj49166727 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj8564420;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj8564420 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj23269972;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj23269972 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj42190556;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj42190556 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj44627141;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj44627141 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj24416734;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj24416734 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj66722991;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj66722991 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj47110329;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj47110329 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj16210284;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj16210284 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj36021841;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj36021841 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj41151505;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj41151505 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj45736346;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj45736346 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj94373596;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj94373596 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj14674797;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj14674797 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj45685202;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj45685202 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj93568757;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj93568757 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj91896093;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj91896093 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj3344789;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj3344789 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj58651091;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj58651091 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj818872;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj818872 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj29418875;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj29418875 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj90442487;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj90442487 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj76557727;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj76557727 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj60353324;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj60353324 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj38450276;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj38450276 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj23663991;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj23663991 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj75123619;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj75123619 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj1036298;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj1036298 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj60099585;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj60099585 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj90565935;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj90565935 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj22972059;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj22972059 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj92232216;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj92232216 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj23034333;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj23034333 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj44697951;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj44697951 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj82112692;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj82112692 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj98256414;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj98256414 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj56010161;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj56010161 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj72394965;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj72394965 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj99652396;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj99652396 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj17457078;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj17457078 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj28436646;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj28436646 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj41140543;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj41140543 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj99972459;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj99972459 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj74826117;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj74826117 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj15425586;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj15425586 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj68486343;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj68486343 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj38740562;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj38740562 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj79634680;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj79634680 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj13961780;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj13961780 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj8990974;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj8990974 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj59267176;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj59267176 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj12845460;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj12845460 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj27789664;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj27789664 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj39806629;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj39806629 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj73779905;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj73779905 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj58724240;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj58724240 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj32006692;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj32006692 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj62916647;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj62916647 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj3740280;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj3740280 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj20963151;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj20963151 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj49293114;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj49293114 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj65686694;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj65686694 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj87010744;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj87010744 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj25644348;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj25644348 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj13049783;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj13049783 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj48919289;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj48919289 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj22702013;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj22702013 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj49675645;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj49675645 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj32562105;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj32562105 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj47428788;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj47428788 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj37558597;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj37558597 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj19501129;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj19501129 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj3692393;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj3692393 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj41194013;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj41194013 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj72382225;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj72382225 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj88278331;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj88278331 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj90470028;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj90470028 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj1731611;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj1731611 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj44927739;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj44927739 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj69963932;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj69963932 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj84923428;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj84923428 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj95488939;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj95488939 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj87074517;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj87074517 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj51108612;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj51108612 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj31298759;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj31298759 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj10126600;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj10126600 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj64442552;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj64442552 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj83227704;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj83227704 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj70918046;     MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj70918046 = MQnygQLxmJAAzRqXkrXzOYXdTGgxuXyvxkzRZwkPUGXnoGRCLUmsCVVosJeZjOCPKFDPSYxnHIfIBkoNoelZkGsbOLYQSKgFDIdj56577090;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void kIjGklzazoOfBjDVsagUvzb45985166() {     long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl52515408 = -934125293;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl14284294 = -721889845;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl57724278 = -17330725;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl846574 = -92800145;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl3519602 = -778732197;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl1149951 = 19192025;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl14663773 = -822804031;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl66692264 = -711306574;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl14165766 = -656877009;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl59108948 = -111872903;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl27270212 = -842007383;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl45899614 = -520759131;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl27595250 = -807875760;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl45463130 = -731820819;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl73370995 = -362897280;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl23145599 = 17242455;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl79512429 = -313387274;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl43908166 = 67572007;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl73031303 = -301582070;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl67639546 = -224503672;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl19476011 = -676323245;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl50084421 = -408194040;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl96728984 = -466277346;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl94451363 = -428726198;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl24004259 = -533264568;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl97843541 = -528458517;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl22744998 = -455753044;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl66920119 = -780466410;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl45562782 = -212248799;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl59700301 = -396040757;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl66906429 = 25135404;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl3604979 = -114312040;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl80346355 = -522056078;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl2792082 = -770891790;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl97980067 = -488788421;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl74247905 = -752776211;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl21775553 = 62542741;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl3438875 = -968632394;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl14813314 = -956415507;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl89911254 = 81510156;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl30238907 = -938464593;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl73705682 = -252052618;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl62016399 = -113843105;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl22741028 = 50985099;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl48644289 = -106952732;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl71541868 = -855622311;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl50085910 = -548595219;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl36816633 = -975617032;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl81595804 = -436592938;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl21485301 = -790673125;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl68727375 = -779164939;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl45203577 = -389233805;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl9364906 = -515357654;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl68593638 = -339572727;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl26509155 = 51092856;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl2430987 = -425931253;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl17555309 = -155612499;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl63272915 = -588604528;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl76842315 = -559535577;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl5676061 = -150273681;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl78404952 = -525054932;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl47743654 = 57662378;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl21129483 = -399057775;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl54465465 = -160836252;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl92202518 = -37008307;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl23665233 = -627695344;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl65553259 = -998703053;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl24803169 = 63016030;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl47483063 = -143032398;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl99123090 = -610121069;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl1370046 = 54699714;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl76073555 = -344754881;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl29094852 = 23987513;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl83120048 = -283092227;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl37400639 = -286039080;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl45770328 = -324270627;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl88068021 = -194350936;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl73987957 = -417262446;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl45807075 = -221773467;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl52462390 = -677642258;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl47757631 = -979863298;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl85928365 = -480136012;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl85324314 = -243873473;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl24077482 = -421575675;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl90972925 = -616875819;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl21702853 = -585630792;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl94240073 = -598954386;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl11752717 = -82483352;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl76282927 = -721984647;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl95549080 = 37142832;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl56692596 = -497163712;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl58502638 = -348852732;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl26596559 = -309096817;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl9137253 = -706141826;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl11506303 = -393434912;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl82495253 = -896126972;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl52576200 = -852994844;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl7550935 = -953006854;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl30538509 = -912006594;    long XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl24979056 = -934125293;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl52515408 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl14284294;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl14284294 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl57724278;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl57724278 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl846574;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl846574 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl3519602;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl3519602 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl1149951;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl1149951 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl14663773;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl14663773 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl66692264;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl66692264 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl14165766;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl14165766 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl59108948;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl59108948 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl27270212;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl27270212 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl45899614;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl45899614 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl27595250;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl27595250 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl45463130;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl45463130 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl73370995;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl73370995 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl23145599;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl23145599 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl79512429;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl79512429 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl43908166;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl43908166 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl73031303;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl73031303 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl67639546;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl67639546 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl19476011;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl19476011 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl50084421;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl50084421 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl96728984;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl96728984 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl94451363;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl94451363 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl24004259;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl24004259 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl97843541;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl97843541 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl22744998;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl22744998 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl66920119;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl66920119 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl45562782;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl45562782 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl59700301;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl59700301 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl66906429;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl66906429 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl3604979;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl3604979 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl80346355;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl80346355 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl2792082;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl2792082 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl97980067;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl97980067 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl74247905;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl74247905 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl21775553;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl21775553 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl3438875;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl3438875 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl14813314;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl14813314 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl89911254;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl89911254 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl30238907;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl30238907 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl73705682;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl73705682 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl62016399;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl62016399 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl22741028;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl22741028 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl48644289;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl48644289 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl71541868;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl71541868 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl50085910;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl50085910 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl36816633;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl36816633 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl81595804;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl81595804 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl21485301;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl21485301 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl68727375;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl68727375 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl45203577;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl45203577 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl9364906;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl9364906 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl68593638;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl68593638 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl26509155;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl26509155 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl2430987;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl2430987 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl17555309;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl17555309 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl63272915;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl63272915 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl76842315;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl76842315 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl5676061;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl5676061 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl78404952;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl78404952 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl47743654;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl47743654 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl21129483;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl21129483 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl54465465;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl54465465 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl92202518;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl92202518 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl23665233;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl23665233 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl65553259;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl65553259 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl24803169;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl24803169 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl47483063;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl47483063 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl99123090;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl99123090 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl1370046;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl1370046 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl76073555;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl76073555 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl29094852;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl29094852 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl83120048;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl83120048 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl37400639;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl37400639 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl45770328;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl45770328 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl88068021;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl88068021 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl73987957;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl73987957 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl45807075;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl45807075 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl52462390;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl52462390 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl47757631;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl47757631 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl85928365;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl85928365 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl85324314;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl85324314 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl24077482;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl24077482 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl90972925;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl90972925 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl21702853;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl21702853 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl94240073;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl94240073 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl11752717;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl11752717 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl76282927;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl76282927 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl95549080;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl95549080 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl56692596;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl56692596 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl58502638;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl58502638 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl26596559;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl26596559 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl9137253;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl9137253 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl11506303;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl11506303 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl82495253;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl82495253 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl52576200;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl52576200 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl7550935;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl7550935 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl30538509;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl30538509 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl24979056;     XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl24979056 = XJLgraQyWjXemXFmvJGWABGlybCmhczfkhLwYFIzjpsvdEQxnolKodbWCZhPYdQukstFLJNpBGmKJKIuLEztqpqHGptzYsbFYxLl52515408;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void DEonApgOKyGysjxqBmIzXJs49261345() {     int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe19683231 = 35186959;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe28754478 = -524923884;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe42718903 = -779163644;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe60786778 = -291481909;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe89202262 = -983547880;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe10993869 = -963200297;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe22583654 = -584229483;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe24379790 = -218919189;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe22914995 = -595819056;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe37760188 = -803290031;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe19575186 = -872408514;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe99316784 = -437516649;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe96766878 = -442369244;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe19463167 = -869794038;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe38404717 = -382617567;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe81348789 = -794964522;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe73129725 = -225612164;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe38836882 = -890584443;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe76447926 = -88906179;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe31114633 = -370382582;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe42455908 = -940318796;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe63233810 = -312501097;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe69730557 = -535561281;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe30300617 = -39784247;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe2182310 = -71248089;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe46178448 = -851935260;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe93791677 = -527712809;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe49250945 = -636862782;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe53562018 = -598949842;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe11081970 = -481496983;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe66366166 = -931719818;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe91504694 = -578704342;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe38009944 = -808142347;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe80675306 = -766590608;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe44310123 = -420743764;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe56498323 = -519196594;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe90153322 = -136647736;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe94399497 = -576246130;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe24424111 = -346872186;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe78595506 = -717730246;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe28655352 = -160053693;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe53533195 = -431986609;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe18383615 = -317304602;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe61841487 = -384141404;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe68576036 = -161123764;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe55414746 = -490264926;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe59280309 = -702545355;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe95229075 = -736460649;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe46317130 = -721979171;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe92691114 = -529709015;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe82075655 = -65838763;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe52210091 = -199545903;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe68859767 = -739037336;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe39203003 = -634695302;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe6805437 = -614120212;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe56449421 = -652311944;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe59023920 = -989362604;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe12418286 = -639379397;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe58604469 = -120233821;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe43023814 = -31612620;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe17202191 = -335487489;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe73332709 = -947366701;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe70817772 = -619969348;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe11833026 = -14322073;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe71394022 = -871570214;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe28070492 = -193704172;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe61306840 = -629374302;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe16091572 = -675778637;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe75153043 = -349050275;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe81906394 = -863420974;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe91195467 = -558316787;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe78730228 = -649366034;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe14412771 = -443712258;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe97852420 = -371175934;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe2459282 = -110328890;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe88922712 = -408332188;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe44850195 = -995196495;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe7889071 = -51419877;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe61724580 = -878660484;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe46767563 = -580983163;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe86898138 = -49389905;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe98562601 = -791252161;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe2933815 = -914883611;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe60870903 = 30759173;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe29006315 = -315658221;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe14156075 = -632173916;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe22644928 = -839667006;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe98806940 = -73447046;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe73869869 = -52470397;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe87860702 = -768431820;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe97474402 = -529833990;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe77735036 = -497268339;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe35795029 = -356012310;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe81400296 = -215259566;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe61393315 = -282242757;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe55322643 = -212686992;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe82715423 = -812017261;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe6550590 = -202982530;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe90447465 = -512571191;    int XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe40505545 = 35186959;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe19683231 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe28754478;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe28754478 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe42718903;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe42718903 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe60786778;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe60786778 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe89202262;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe89202262 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe10993869;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe10993869 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe22583654;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe22583654 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe24379790;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe24379790 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe22914995;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe22914995 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe37760188;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe37760188 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe19575186;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe19575186 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe99316784;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe99316784 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe96766878;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe96766878 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe19463167;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe19463167 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe38404717;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe38404717 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe81348789;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe81348789 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe73129725;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe73129725 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe38836882;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe38836882 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe76447926;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe76447926 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe31114633;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe31114633 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe42455908;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe42455908 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe63233810;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe63233810 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe69730557;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe69730557 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe30300617;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe30300617 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe2182310;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe2182310 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe46178448;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe46178448 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe93791677;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe93791677 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe49250945;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe49250945 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe53562018;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe53562018 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe11081970;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe11081970 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe66366166;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe66366166 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe91504694;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe91504694 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe38009944;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe38009944 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe80675306;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe80675306 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe44310123;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe44310123 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe56498323;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe56498323 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe90153322;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe90153322 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe94399497;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe94399497 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe24424111;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe24424111 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe78595506;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe78595506 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe28655352;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe28655352 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe53533195;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe53533195 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe18383615;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe18383615 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe61841487;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe61841487 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe68576036;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe68576036 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe55414746;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe55414746 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe59280309;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe59280309 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe95229075;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe95229075 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe46317130;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe46317130 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe92691114;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe92691114 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe82075655;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe82075655 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe52210091;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe52210091 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe68859767;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe68859767 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe39203003;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe39203003 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe6805437;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe6805437 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe56449421;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe56449421 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe59023920;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe59023920 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe12418286;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe12418286 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe58604469;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe58604469 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe43023814;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe43023814 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe17202191;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe17202191 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe73332709;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe73332709 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe70817772;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe70817772 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe11833026;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe11833026 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe71394022;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe71394022 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe28070492;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe28070492 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe61306840;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe61306840 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe16091572;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe16091572 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe75153043;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe75153043 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe81906394;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe81906394 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe91195467;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe91195467 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe78730228;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe78730228 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe14412771;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe14412771 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe97852420;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe97852420 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe2459282;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe2459282 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe88922712;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe88922712 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe44850195;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe44850195 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe7889071;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe7889071 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe61724580;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe61724580 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe46767563;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe46767563 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe86898138;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe86898138 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe98562601;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe98562601 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe2933815;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe2933815 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe60870903;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe60870903 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe29006315;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe29006315 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe14156075;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe14156075 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe22644928;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe22644928 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe98806940;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe98806940 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe73869869;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe73869869 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe87860702;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe87860702 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe97474402;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe97474402 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe77735036;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe77735036 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe35795029;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe35795029 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe81400296;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe81400296 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe61393315;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe61393315 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe55322643;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe55322643 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe82715423;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe82715423 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe6550590;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe6550590 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe90447465;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe90447465 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe40505545;     XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe40505545 = XbHGEJIWxkumoVgJzOmpObaFpvzzXwjauMRLtibECbINyxJVUcIkkTdwBBRJJJzAAUscDUQRmhmmSNGsmeevozpaiVKrRSvscHMe19683231;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void eJWrvMaTSglnlnraYprquiy22551294() {     int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed64457466 = -333620967;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed97449478 = -422009993;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed257460 = -158471332;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed90800155 = -304296973;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed85550770 = 34338118;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed40760103 = -684417053;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed6895132 = -228204095;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed75647507 = -60112478;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed17241258 = -364211134;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed21287358 = -288772697;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed94658905 = -865657315;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed11122682 = -939007235;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed30783239 = -579803884;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed4692671 = -825211552;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed11875271 = -737572993;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed69860614 = -458299607;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed72776429 = -780171804;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed50789714 = -694069824;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed59011188 = -359110429;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed88147199 = -875651489;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed44222122 = -923293413;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed90864246 = -222033723;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed76317204 = -887722465;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed83300191 = -67769962;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed73262803 = -414366338;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed22784437 = -639723708;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed87970518 = -42885795;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed61073845 = -213771689;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed67347139 = -187464080;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed29396329 = -322088717;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed54947776 = 5647878;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed6587077 = -782847051;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed87285 = -766829337;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed99452681 = -314844308;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed96768852 = -548256609;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed39884061 = -49987347;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed77032746 = -702543638;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed1651700 = -311497074;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed21040374 = -48964249;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed18966935 = -990178304;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed86170093 = -66409025;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed30549978 = 52196009;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed29483999 = -92365770;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed60729726 = -378868702;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed4542570 = -879410719;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed71615310 = -854723117;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed83875104 = -759365861;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed84426425 = -249530630;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed35236940 = -621412568;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed86858606 = -813749533;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed99814088 = -775991317;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed99912605 = 38815533;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed25488573 = -82859796;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed59776094 = -86056919;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed78329172 = -895528442;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed73593219 = -11587244;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed21132275 = -534287529;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed16957269 = 9298629;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed17537353 = -889930636;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed62766333 = -325938175;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed52789584 = -541531259;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed45821286 = 85567594;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed8300368 = -872648398;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed87844928 = 57877582;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed66339581 = -194420576;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed88071829 = 17189735;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed11035398 = -72177899;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed31330558 = -164959576;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed7923819 = -176954944;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed71991210 = -587585647;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed92827868 = -755755969;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed71124729 = -368674730;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed29749341 = -545105576;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed40044253 = -368932126;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed1977106 = -709242465;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed13672145 = -875489423;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed61380248 = -29667954;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed15587478 = -408853763;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed78757621 = -188359244;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed1647494 = -559643221;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed38909333 = -880357847;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed3544094 = -793355165;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed25836905 = -592359122;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed80488533 = -373714547;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed29582241 = -546097400;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed55035171 = 66832344;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed81098503 = -599987255;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed40311191 = -580772418;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed21123510 = -419315867;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed23175634 = -436669365;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed18751787 = -515699818;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed60075478 = -611842268;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed84114347 = -421566439;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed58274040 = -723026075;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed66177351 = -348647046;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed40348807 = -51976619;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed22249610 = -75155593;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed41639071 = -50243352;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed94390144 = -84448127;    int ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed16470741 = -333620967;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed64457466 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed97449478;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed97449478 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed257460;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed257460 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed90800155;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed90800155 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed85550770;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed85550770 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed40760103;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed40760103 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed6895132;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed6895132 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed75647507;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed75647507 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed17241258;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed17241258 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed21287358;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed21287358 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed94658905;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed94658905 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed11122682;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed11122682 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed30783239;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed30783239 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed4692671;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed4692671 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed11875271;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed11875271 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed69860614;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed69860614 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed72776429;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed72776429 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed50789714;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed50789714 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed59011188;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed59011188 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed88147199;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed88147199 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed44222122;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed44222122 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed90864246;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed90864246 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed76317204;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed76317204 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed83300191;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed83300191 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed73262803;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed73262803 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed22784437;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed22784437 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed87970518;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed87970518 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed61073845;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed61073845 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed67347139;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed67347139 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed29396329;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed29396329 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed54947776;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed54947776 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed6587077;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed6587077 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed87285;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed87285 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed99452681;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed99452681 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed96768852;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed96768852 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed39884061;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed39884061 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed77032746;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed77032746 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed1651700;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed1651700 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed21040374;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed21040374 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed18966935;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed18966935 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed86170093;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed86170093 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed30549978;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed30549978 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed29483999;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed29483999 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed60729726;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed60729726 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed4542570;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed4542570 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed71615310;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed71615310 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed83875104;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed83875104 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed84426425;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed84426425 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed35236940;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed35236940 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed86858606;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed86858606 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed99814088;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed99814088 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed99912605;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed99912605 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed25488573;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed25488573 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed59776094;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed59776094 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed78329172;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed78329172 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed73593219;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed73593219 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed21132275;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed21132275 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed16957269;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed16957269 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed17537353;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed17537353 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed62766333;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed62766333 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed52789584;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed52789584 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed45821286;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed45821286 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed8300368;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed8300368 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed87844928;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed87844928 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed66339581;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed66339581 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed88071829;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed88071829 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed11035398;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed11035398 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed31330558;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed31330558 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed7923819;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed7923819 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed71991210;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed71991210 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed92827868;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed92827868 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed71124729;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed71124729 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed29749341;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed29749341 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed40044253;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed40044253 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed1977106;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed1977106 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed13672145;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed13672145 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed61380248;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed61380248 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed15587478;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed15587478 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed78757621;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed78757621 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed1647494;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed1647494 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed38909333;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed38909333 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed3544094;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed3544094 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed25836905;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed25836905 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed80488533;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed80488533 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed29582241;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed29582241 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed55035171;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed55035171 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed81098503;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed81098503 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed40311191;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed40311191 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed21123510;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed21123510 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed23175634;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed23175634 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed18751787;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed18751787 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed60075478;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed60075478 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed84114347;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed84114347 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed58274040;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed58274040 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed66177351;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed66177351 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed40348807;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed40348807 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed22249610;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed22249610 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed41639071;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed41639071 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed94390144;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed94390144 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed16470741;     ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed16470741 = ziOHdKssUPeowUKuBIYRcgRFlMWFNBXBwKWIgFxIZnQXHyvFNvqhVQWTRJWQnnfTsdslxCNwKQspPuymRRrSLpfgoDrXihVmIxed64457466;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void NjepuXsGCjtxKqbnaCgVUpq47753578() {     int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm93234431 = -521807346;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm44468581 = 72982383;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm38559745 = -580869174;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm52317187 = -133076058;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm34900628 = 63195223;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm94700971 = -833203493;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm93928417 = -175295670;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm99247947 = -55056909;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm53476876 = -86109254;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm64480051 = -851961282;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm6104759 = -349360760;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm40146722 = -211158118;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm48789528 = -611241769;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm99041572 = -591704434;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm32849879 = -849900048;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm15611335 = -181810669;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm78402069 = -799146142;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm10045640 = -571953477;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm61087269 = -207938286;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm4027613 = -83920238;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm25026970 = -465672041;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm8351508 = 33274848;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm17429873 = -158783925;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm3878598 = -202395587;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm75971524 = -354493351;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm10033793 = -332936496;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm75878498 = -817725284;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm18363520 = -393608689;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm88887450 = -961864977;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm2974001 = -995043347;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm88162918 = -957423266;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm8149608 = 29810874;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm46875141 = -235072332;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm47520974 = 20310866;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm5707502 = 53314066;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm50603082 = -849547149;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm94825172 = -930272954;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm69088651 = -136563932;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm89892996 = -177276277;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm88221460 = -303326599;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm39570953 = -801295637;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm15758428 = -301764390;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm82617606 = -821178203;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm62104063 = -875300694;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm4539108 = -321568877;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm36743641 = -429633767;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm98812407 = -727173951;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm31436873 = -388122689;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm61058064 = -434812184;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm17440274 = 45199533;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm24476737 = -496787251;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm95125764 = -268669292;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm58769725 = -338799672;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm47537637 = -972423525;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm22726608 = -361904439;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm84882923 = -455082195;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm27038708 = -768233693;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm34681147 = -278473588;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm76345662 = -778582707;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm24866836 = -603868282;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm18822473 = 84521790;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm75564897 = -781686981;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm10360498 = -93191933;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm50502875 = -91065908;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm76317132 = -894538016;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm97955151 = -279171634;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm93271581 = -976085787;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm1268555 = -531552636;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm93334070 = -545018500;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm82246797 = 99647100;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm20786163 = -251537715;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm9313418 = -562582211;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm20152644 = -294677200;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm72865808 = -904611687;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm64456660 = -282624601;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm9268543 = -63907651;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm25733902 = -145546950;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm55325810 = -283483231;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm99339489 = -880826710;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm39227884 = -924859584;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm11221385 = -605762545;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm44441625 = -329602595;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm57305456 = -958796506;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm71447177 = -907064510;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm78497264 = -398256096;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm93037154 = -588753975;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm49379882 = -631389455;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm99337503 = -262648808;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm24794366 = -617784695;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm20824579 = -491603739;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm23564374 = 18686544;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm60144025 = -551799367;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm92742989 = -357981225;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm65026161 = -573407996;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm69398988 = -287848390;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm64006055 = 80391343;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm5397931 = -108572458;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm32114731 = -630112296;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm85786930 = -980762679;    int txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm6583957 = -521807346;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm93234431 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm44468581;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm44468581 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm38559745;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm38559745 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm52317187;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm52317187 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm34900628;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm34900628 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm94700971;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm94700971 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm93928417;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm93928417 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm99247947;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm99247947 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm53476876;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm53476876 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm64480051;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm64480051 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm6104759;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm6104759 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm40146722;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm40146722 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm48789528;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm48789528 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm99041572;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm99041572 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm32849879;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm32849879 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm15611335;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm15611335 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm78402069;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm78402069 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm10045640;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm10045640 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm61087269;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm61087269 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm4027613;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm4027613 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm25026970;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm25026970 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm8351508;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm8351508 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm17429873;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm17429873 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm3878598;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm3878598 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm75971524;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm75971524 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm10033793;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm10033793 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm75878498;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm75878498 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm18363520;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm18363520 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm88887450;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm88887450 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm2974001;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm2974001 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm88162918;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm88162918 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm8149608;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm8149608 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm46875141;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm46875141 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm47520974;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm47520974 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm5707502;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm5707502 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm50603082;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm50603082 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm94825172;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm94825172 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm69088651;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm69088651 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm89892996;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm89892996 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm88221460;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm88221460 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm39570953;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm39570953 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm15758428;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm15758428 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm82617606;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm82617606 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm62104063;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm62104063 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm4539108;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm4539108 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm36743641;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm36743641 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm98812407;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm98812407 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm31436873;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm31436873 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm61058064;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm61058064 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm17440274;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm17440274 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm24476737;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm24476737 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm95125764;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm95125764 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm58769725;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm58769725 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm47537637;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm47537637 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm22726608;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm22726608 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm84882923;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm84882923 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm27038708;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm27038708 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm34681147;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm34681147 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm76345662;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm76345662 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm24866836;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm24866836 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm18822473;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm18822473 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm75564897;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm75564897 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm10360498;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm10360498 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm50502875;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm50502875 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm76317132;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm76317132 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm97955151;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm97955151 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm93271581;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm93271581 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm1268555;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm1268555 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm93334070;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm93334070 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm82246797;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm82246797 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm20786163;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm20786163 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm9313418;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm9313418 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm20152644;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm20152644 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm72865808;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm72865808 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm64456660;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm64456660 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm9268543;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm9268543 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm25733902;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm25733902 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm55325810;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm55325810 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm99339489;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm99339489 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm39227884;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm39227884 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm11221385;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm11221385 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm44441625;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm44441625 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm57305456;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm57305456 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm71447177;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm71447177 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm78497264;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm78497264 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm93037154;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm93037154 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm49379882;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm49379882 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm99337503;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm99337503 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm24794366;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm24794366 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm20824579;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm20824579 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm23564374;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm23564374 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm60144025;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm60144025 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm92742989;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm92742989 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm65026161;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm65026161 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm69398988;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm69398988 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm64006055;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm64006055 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm5397931;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm5397931 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm32114731;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm32114731 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm85786930;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm85786930 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm6583957;     txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm6583957 = txYWebtYARNfVBnxoxKMKHGPAEtwXLBCYslrVBNfJLsgxKFmmFrYKcglqJWZhwApPKHcPvbnjeCyPaSpyHKSTdrvMzBMeoxNKCRm93234431;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void CLgVgLZZwNDRgVstYbeiQin75789937() {     double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei53999464 = -943760960;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei73411796 = -258777337;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei16647787 = 25784678;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei35906770 = -150467929;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei87087889 = -598245209;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei35098003 = -926283376;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei44065423 = -320689786;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei47396993 = -546676372;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei88633945 = -714641361;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei84981209 = 3455100;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei65146949 = -811626991;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei49026155 = -577466771;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei30668876 = -876331637;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei28995900 = 18800369;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei3988489 = -310196697;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei21448812 = -117765427;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei27922596 = -608905653;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei97695912 = -698112209;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei44565981 = -417501197;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei74286096 = -298213755;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei27423975 = -599709021;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei24421386 = -551090860;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei26368893 = -951002675;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei32949448 = -4661913;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei43866480 = -427296689;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei92570492 = -123506532;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei46549782 = -474031479;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei20123171 = 23443508;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei7595830 = -10562871;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei42114917 = -778703557;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei65523675 = -706709965;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei21475697 = -482954231;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei9694390 = -886147532;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei37290268 = -388033441;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei84044348 = 37403777;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei49483727 = -291334599;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei62732963 = -833988823;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei86073782 = -877261641;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei49586495 = -165829792;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei71582686 = -44506107;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei89055245 = 32936412;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei70281203 = -273230837;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei11968128 = -201618359;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei17738102 = 74712259;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei60493688 = -589244031;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei44444406 = -295684169;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei60762486 = -254287495;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei38204704 = -984431950;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei31734950 = 15956777;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei95239011 = -968855455;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei48550324 = -674851432;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei9864892 = -966607343;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei14194535 = -862558725;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei11172546 = -620700005;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei48365962 = -115244181;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei29578078 = -292670101;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei47042903 = -307774663;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei83698338 = -969553409;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei92040289 = -723171241;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei94517397 = -374738677;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei88548220 = -352251898;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei23942253 = -244133295;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei39801163 = -436113501;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei46519029 = -935937804;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei19457534 = -289834936;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei43671252 = -228672760;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei39331766 = -691319240;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei93378607 = -388298196;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei44951552 = 81396592;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei54504761 = 81137901;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei58715849 = -283776605;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei41848813 = -731644013;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei48109417 = -432282418;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei72983295 = -272995090;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei85230851 = -231150168;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei57142771 = -226478184;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei12453259 = -249472501;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei8630792 = -925714934;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei72455760 = -415417883;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei99422074 = -31612520;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei31808006 = -869219038;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei8345078 = -489599529;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei88388221 = -992513270;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei12356819 = -41707417;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei93564592 = -3852125;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei55658784 = -740102622;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei7281163 = -620395507;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei98521843 = -165447527;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei88924305 = -172789261;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei54466271 = -669926123;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei2440824 = -983559937;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei79034624 = -864435414;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei94033492 = -54090401;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei55069098 = -791091115;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei83034465 = -692254210;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei65112992 = -722930293;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei30480041 = -837117337;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei65449098 = -265680555;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei98280567 = -635452806;    double SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei16822437 = -943760960;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei53999464 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei73411796;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei73411796 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei16647787;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei16647787 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei35906770;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei35906770 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei87087889;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei87087889 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei35098003;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei35098003 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei44065423;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei44065423 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei47396993;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei47396993 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei88633945;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei88633945 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei84981209;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei84981209 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei65146949;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei65146949 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei49026155;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei49026155 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei30668876;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei30668876 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei28995900;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei28995900 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei3988489;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei3988489 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei21448812;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei21448812 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei27922596;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei27922596 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei97695912;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei97695912 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei44565981;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei44565981 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei74286096;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei74286096 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei27423975;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei27423975 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei24421386;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei24421386 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei26368893;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei26368893 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei32949448;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei32949448 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei43866480;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei43866480 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei92570492;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei92570492 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei46549782;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei46549782 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei20123171;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei20123171 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei7595830;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei7595830 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei42114917;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei42114917 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei65523675;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei65523675 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei21475697;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei21475697 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei9694390;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei9694390 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei37290268;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei37290268 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei84044348;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei84044348 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei49483727;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei49483727 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei62732963;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei62732963 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei86073782;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei86073782 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei49586495;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei49586495 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei71582686;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei71582686 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei89055245;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei89055245 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei70281203;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei70281203 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei11968128;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei11968128 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei17738102;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei17738102 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei60493688;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei60493688 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei44444406;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei44444406 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei60762486;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei60762486 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei38204704;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei38204704 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei31734950;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei31734950 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei95239011;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei95239011 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei48550324;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei48550324 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei9864892;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei9864892 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei14194535;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei14194535 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei11172546;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei11172546 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei48365962;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei48365962 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei29578078;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei29578078 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei47042903;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei47042903 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei83698338;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei83698338 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei92040289;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei92040289 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei94517397;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei94517397 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei88548220;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei88548220 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei23942253;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei23942253 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei39801163;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei39801163 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei46519029;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei46519029 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei19457534;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei19457534 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei43671252;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei43671252 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei39331766;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei39331766 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei93378607;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei93378607 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei44951552;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei44951552 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei54504761;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei54504761 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei58715849;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei58715849 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei41848813;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei41848813 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei48109417;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei48109417 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei72983295;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei72983295 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei85230851;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei85230851 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei57142771;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei57142771 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei12453259;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei12453259 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei8630792;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei8630792 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei72455760;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei72455760 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei99422074;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei99422074 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei31808006;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei31808006 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei8345078;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei8345078 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei88388221;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei88388221 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei12356819;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei12356819 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei93564592;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei93564592 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei55658784;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei55658784 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei7281163;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei7281163 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei98521843;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei98521843 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei88924305;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei88924305 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei54466271;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei54466271 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei2440824;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei2440824 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei79034624;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei79034624 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei94033492;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei94033492 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei55069098;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei55069098 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei83034465;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei83034465 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei65112992;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei65112992 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei30480041;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei30480041 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei65449098;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei65449098 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei98280567;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei98280567 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei16822437;     SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei16822437 = SwPhoLPkiDuWxoVFlOUDAdeAQglWZZUqTXNZQdlOgdVNmyuEDWsUwfnpRORHatLIjDCabvoHdGWXIRORlgSDhzuojYICMdnmSNei53999464;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void pFJaIgUHJSncHDecKFjBaPn33812527() {     long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg37158086 = -27594396;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg48130196 = -496484987;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg22191896 = -750086701;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg49423180 = -353726502;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg28609303 = -282387321;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg55572719 = -80538825;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg17810832 = -583534742;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg1965846 = -704715162;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg38213983 = -413723436;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg606439 = -347062980;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg41410394 = -211045550;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg99516859 = -359042355;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg47703489 = -638480350;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg47720760 = -653250534;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg66690265 = -535258208;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg96977653 = -102592077;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg71413714 = -876330414;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg68322069 = -878942010;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg48898055 = -144183967;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg50987099 = -153117274;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg51034663 = 85233064;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg26010217 = -30230997;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg1722840 = -360344175;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg44869978 = -490000575;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg18858993 = -794965298;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg46836109 = -449764864;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg94088904 = -687124454;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg92390745 = -938991760;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg20518324 = -957447570;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg14323142 = -807228259;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg53762559 = -150219580;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg7619121 = -155968930;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg68099885 = -764622012;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg86165411 = -143822866;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg56252522 = -882949011;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg47229052 = 31248320;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg12139098 = -370999264;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg86767334 = -390322143;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg22274530 = -942747922;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg3256734 = -312477959;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg79441240 = -648065306;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg27614710 = -908813893;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg86585480 = -10458844;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg13584361 = -515673994;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg413484 = -192803261;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg19817485 = -531918996;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg7312170 = -978530669;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg14187629 = -728514845;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg78213350 = 80772902;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg50076073 = -337905817;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg68233758 = -529436882;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg83908017 = -613218928;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg72485398 = -66175000;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg24843729 = -12737444;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg82777864 = -252388760;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg11147869 = -997363400;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg46407356 = -36140812;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg77321917 = -160086127;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg30564188 = -558761205;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg81773193 = -832622458;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg61483814 = -393414372;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg25420086 = -644542983;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg81447522 = -747267592;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg23890841 = -606495178;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg46843880 = -96843400;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg33791274 = 44923379;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg31416975 = -594420344;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg61538077 = -394657484;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg91468237 = -770301524;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg19461214 = -466506529;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg84838556 = -731592813;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg84646379 = -386008271;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg46047540 = -936194088;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg45641322 = -831706009;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg71545859 = -505051969;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg23419953 = -5953043;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg39424737 = 80227847;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg88138479 = -844670181;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg44456495 = -197197314;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg99041508 = -163046303;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg39523940 = -471234195;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg79901276 = -958609609;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg14177396 = -919764662;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg70442251 = -519541754;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg46089383 = -177791377;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg69854541 = -537000653;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg35133723 = 10206070;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg43256157 = -651884568;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg3387548 = -891434107;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg45104654 = -885585612;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg821696 = -932610868;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg34817180 = -110913138;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg56203147 = -831560939;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg40501336 = -10125465;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg41772919 = -919063587;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg54021155 = 96477676;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg46167188 = -61546302;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg62694639 = -403963667;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg66740480 = -318830595;    long moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg66622209 = -27594396;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg37158086 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg48130196;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg48130196 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg22191896;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg22191896 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg49423180;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg49423180 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg28609303;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg28609303 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg55572719;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg55572719 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg17810832;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg17810832 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg1965846;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg1965846 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg38213983;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg38213983 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg606439;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg606439 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg41410394;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg41410394 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg99516859;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg99516859 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg47703489;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg47703489 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg47720760;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg47720760 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg66690265;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg66690265 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg96977653;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg96977653 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg71413714;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg71413714 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg68322069;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg68322069 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg48898055;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg48898055 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg50987099;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg50987099 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg51034663;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg51034663 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg26010217;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg26010217 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg1722840;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg1722840 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg44869978;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg44869978 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg18858993;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg18858993 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg46836109;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg46836109 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg94088904;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg94088904 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg92390745;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg92390745 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg20518324;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg20518324 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg14323142;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg14323142 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg53762559;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg53762559 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg7619121;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg7619121 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg68099885;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg68099885 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg86165411;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg86165411 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg56252522;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg56252522 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg47229052;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg47229052 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg12139098;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg12139098 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg86767334;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg86767334 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg22274530;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg22274530 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg3256734;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg3256734 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg79441240;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg79441240 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg27614710;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg27614710 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg86585480;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg86585480 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg13584361;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg13584361 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg413484;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg413484 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg19817485;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg19817485 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg7312170;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg7312170 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg14187629;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg14187629 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg78213350;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg78213350 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg50076073;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg50076073 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg68233758;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg68233758 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg83908017;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg83908017 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg72485398;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg72485398 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg24843729;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg24843729 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg82777864;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg82777864 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg11147869;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg11147869 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg46407356;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg46407356 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg77321917;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg77321917 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg30564188;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg30564188 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg81773193;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg81773193 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg61483814;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg61483814 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg25420086;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg25420086 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg81447522;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg81447522 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg23890841;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg23890841 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg46843880;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg46843880 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg33791274;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg33791274 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg31416975;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg31416975 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg61538077;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg61538077 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg91468237;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg91468237 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg19461214;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg19461214 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg84838556;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg84838556 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg84646379;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg84646379 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg46047540;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg46047540 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg45641322;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg45641322 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg71545859;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg71545859 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg23419953;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg23419953 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg39424737;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg39424737 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg88138479;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg88138479 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg44456495;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg44456495 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg99041508;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg99041508 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg39523940;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg39523940 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg79901276;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg79901276 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg14177396;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg14177396 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg70442251;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg70442251 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg46089383;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg46089383 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg69854541;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg69854541 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg35133723;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg35133723 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg43256157;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg43256157 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg3387548;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg3387548 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg45104654;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg45104654 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg821696;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg821696 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg34817180;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg34817180 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg56203147;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg56203147 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg40501336;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg40501336 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg41772919;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg41772919 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg54021155;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg54021155 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg46167188;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg46167188 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg62694639;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg62694639 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg66740480;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg66740480 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg66622209;     moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg66622209 = moUorlNLbXwlAaYoxZpGNcRlcVdYjqASZRWlnscWwGMEKRdCmRmblJaKXiLWHoblXeLjDmAFxSJHQMaKralSEYhQrMOEgFOHeahg37158086;}
// Junk Finished
