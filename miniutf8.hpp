/*
Copyright (c) 2023 [Marco4413](https://github.com/Marco4413/miniutf8)

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

// Put this line in a .cpp file
// #define MINIUTF8_IMPLEMENTATION

// MINIUTF8_NO_GUARDS tells miniutf8 to ignore include guards.
#if defined(MINIUTF8_NO_GUARDS) || !defined(_MINIUTF8_HPP)

#ifdef MINIUTF8_NO_GUARDS
#undef MINIUTF8_NO_GUARDS
#else // MINIUTF8_NO_GUARDS
#define _MINIUTF8_HPP
#endif // MINIUTF8_NO_GUARDS

#ifndef MINIUTF8_EXT_INCLUDE
#include <cinttypes>
#include <cstring>
#include <string>
#include <string_view>
#endif // MINIUTF8_EXT_INCLUDE

namespace UTF8
{
    constexpr uint8_t BYTE1 = 0x00;
    constexpr uint8_t BYTE1_MASK = 0x80;
    constexpr uint8_t BYTE2 = 0xC0;
    constexpr uint8_t BYTE2_MASK = 0xE0;
    constexpr uint8_t BYTE3 = 0xE0;
    constexpr uint8_t BYTE3_MASK = 0xF0;
    constexpr uint8_t BYTE4 = 0xF0;
    constexpr uint8_t BYTE4_MASK = 0xF8;

    constexpr uint8_t BYTE = 0x80;
    constexpr uint8_t BYTE_MASK = 0xC0;

    constexpr char32_t BOM = 0xFEFF;
    constexpr char32_t NOT_A_CHARACTER = 0x10FFFF;

    std::string Encode(char32_t ch);
    std::string Encode(std::u32string_view str);

    // A class which enables Decoding of an UTF-8 encoded string.
    // Also useful to iterate over character when parsing files.
    class StringDecoder
    {
    public:
        StringDecoder(const char* begin, const char* end)
            : m_Cursor(begin > end ? end : begin), m_End(end) { }

        StringDecoder(std::string_view str, size_t offset)
            : StringDecoder(str.data()+offset, str.data()+str.length()) { }

        StringDecoder(std::string_view str)
            : StringDecoder(str, 0) { }
        
        const char* GetCursor() const { return m_Cursor; }
        const char* GetEnd() const { return m_End; }

        /// @brief Parses the next character in the buffer.
        /// @return Either the parsed character or UTF8::NOT_A_CHARACTER if invalid or at the end of stream.
        char32_t Next();
        /// @brief The boolean value of StringDecoder indicates whether or not it is at the End of File.
        operator bool() const { return m_Cursor < m_End; }

    private:
        const char* m_Cursor;
        const char* const m_End;
    };

    std::u32string Decode(std::string_view str);

    size_t Length(std::string_view str);

    // A view on an UTF-8 encoded std::string.
    // This class allows for string manipulation on encoded strings,
    //  which means that characters that span more than one byte are treated accordingly.
    // NOTE: This class is useful only if you are planning on doing bulk/one-time operations on an encoded string.
    //       Otherwise, Decoding and Encoding might be faster while occupying more memory.
    class StringView
    {
    public:
        explicit StringView(std::string& str)
            : m_Ref(str) { }

        StringView(const StringView& other)
            : StringView(other.m_Ref) { }

        size_t Length() const { return UTF8::Length(m_Ref); }
        std::u32string Decode() const { return UTF8::Decode(m_Ref); }

        const std::string& GetRef() const { return m_Ref; }
        std::string& GetRef() { return m_Ref; }

        StringView& operator=(const StringView& other) { m_Ref = other.m_Ref; return *this; }

        StringView& Erase(size_t pos, size_t len);

        StringView& Insert(size_t pos, const std::string& str) { return Insert(pos, str.c_str(), str.length()); }
        StringView& Insert(size_t pos, const StringView& str) { return Insert(pos, str.m_Ref); }
        StringView& Insert(size_t pos, const std::string& str, size_t subpos, size_t sublen = -1);
        StringView& Insert(size_t pos, const StringView& str, size_t subpos, size_t sublen = -1) { return Insert(pos, str.m_Ref, subpos, sublen); }
        StringView& Insert(size_t pos, const char* s) { return Insert(pos, s, std::strlen(s)); }
        StringView& Insert(size_t pos, const char* s, size_t n);
        StringView& Insert(size_t pos, size_t n, char c) { return Insert(pos, std::string(n, c)); }

        StringView& Replace(size_t pos, size_t len, const std::string& str) { return Replace(pos, len, str, 0); }
        StringView& Replace(size_t pos, size_t len, const StringView& str) { return Replace(pos, len, str.m_Ref); }
        StringView& Replace(size_t pos, size_t len, const std::string& str, size_t subpos, size_t sublen = -1);
        StringView& Replace(size_t pos, size_t len, const char* s) { return Replace(pos, len, s, std::strlen(s)); }
        StringView& Replace(size_t pos, size_t len, const char* s, size_t n);
        StringView& Replace(size_t pos, size_t len, size_t n, char c) { return Replace(pos, len, std::string(n, c)); }

    private:
        std::string& m_Ref;
    };
}

#endif // _MINIUTF8_HPP

#ifdef MINIUTF8_IMPLEMENTATION

// https://en.wikipedia.org/wiki/UTF-8
std::string UTF8::Encode(char32_t ch)
{
    std::string encoded;
    encoded.reserve(4);
    if (ch > UTF8::NOT_A_CHARACTER)
        ch = UTF8::NOT_A_CHARACTER;

    if (ch <= 0x7F) {
        encoded += (char)ch;
    } else if (ch <= 0x07FF) {
        encoded += (char)(ch >> 6) | UTF8::BYTE2;
        encoded += (char)(ch & ~UTF8::BYTE_MASK) | UTF8::BYTE;
    } else if (ch <= 0xFFFF) {
        encoded += (char)(ch >> 12) | UTF8::BYTE3;
        encoded += (char)((ch >> 6) & ~UTF8::BYTE_MASK) | UTF8::BYTE;
        encoded += (char)( ch       & ~UTF8::BYTE_MASK) | UTF8::BYTE;
    } else if (ch <= 0x10FFFF) {
        encoded += (char)(ch >> 18) | UTF8::BYTE4;
        encoded += (char)((ch >> 12) & ~UTF8::BYTE_MASK) | UTF8::BYTE;
        encoded += (char)((ch >> 6 ) & ~UTF8::BYTE_MASK) | UTF8::BYTE;
        encoded += (char)( ch        & ~UTF8::BYTE_MASK) | UTF8::BYTE;
    }

    return encoded;
}

std::string UTF8::Encode(std::u32string_view str)
{
    std::string encoded;
    encoded.reserve(str.length());
    for (size_t i = 0; i < str.length(); i++)
        encoded += Encode(str[i]);
    return encoded;
}

char32_t UTF8::StringDecoder::Next()
{
    if (!(*this))
        return UTF8::NOT_A_CHARACTER;

    char32_t codepoint = 0;
    size_t byteCount = 0;

    uint8_t ch = *(m_Cursor++);
    if ((ch & UTF8::BYTE1_MASK) == UTF8::BYTE1) {
        return ch;
    } else if ((ch & UTF8::BYTE2_MASK) == UTF8::BYTE2) {
        codepoint = (ch & ~UTF8::BYTE2_MASK);
        byteCount = 1;
    } else if ((ch & UTF8::BYTE3_MASK) == UTF8::BYTE3) {
        codepoint = (ch & ~UTF8::BYTE3_MASK);
        byteCount = 2;
    } else if ((ch & UTF8::BYTE4_MASK) == UTF8::BYTE4) {
        codepoint = (ch & ~UTF8::BYTE4_MASK);
        byteCount = 3;
    } else
        return UTF8::NOT_A_CHARACTER;

    for (size_t i = 0; i < byteCount; ++i) {
        if ((m_Cursor + i) >= m_End) {
            m_Cursor = m_End;
            return UTF8::NOT_A_CHARACTER;
        } else if ((m_Cursor[i] & UTF8::BYTE_MASK) != UTF8::BYTE) {
            m_Cursor += i+1;
            return UTF8::NOT_A_CHARACTER;
        }
        codepoint = (codepoint << 6) | (uint8_t)(m_Cursor[i] & ~UTF8::BYTE_MASK);
    }

    m_Cursor += byteCount;
    return codepoint;
}

std::u32string UTF8::Decode(std::string_view str)
{
    std::u32string decoded;
    StringDecoder decoder(str);
    while (decoder)
        decoded += decoder.Next();
    return decoded;
}

size_t UTF8::Length(std::string_view str)
{
    size_t len = 0;
    StringDecoder decoder(str);
    for (; decoder; decoder.Next(), ++len);
    return len;
}

UTF8::StringView& UTF8::StringView::Erase(size_t pos, size_t len)
{
    StringDecoder decoder(m_Ref);

    for (size_t i = 0; i < pos && decoder; decoder.Next(), ++i);
    const char* eraseStart = decoder.GetCursor();

    for (size_t i = 0; i < len && decoder; decoder.Next(), ++i);
    const char* eraseEnd = decoder.GetCursor();

    size_t erasepos = eraseStart - m_Ref.c_str();
    size_t eraselen = eraseEnd - eraseStart;
    m_Ref.erase(erasepos, eraselen);
    return *this;
}

UTF8::StringView& UTF8::StringView::Insert(size_t pos, const std::string& str, size_t subpos, size_t sublen)
{
    StringDecoder decoder(str);

    const char* substart;
    for (size_t i = 0; i < subpos && decoder; decoder.Next(), ++i);
    substart = decoder.GetCursor();

    const char* subend;
    for (size_t i = 0; i < sublen && decoder; decoder.Next(), ++i);
    subend = decoder.GetCursor();

    return Insert(pos, substart, subend-substart);
}

UTF8::StringView& UTF8::StringView::Insert(size_t pos, const char* s, size_t n)
{
    StringDecoder decoder(m_Ref);
    for (size_t i = 0; i < pos && decoder; decoder.Next(), ++i);

    const char* insertCursor = decoder.GetCursor();
    size_t insertAt = insertCursor-m_Ref.c_str();
    m_Ref.insert(insertAt, s, n);
    return *this;
}

UTF8::StringView& UTF8::StringView::Replace(size_t pos, size_t len, const std::string& str, size_t subpos, size_t sublen)
{
    return Erase(pos, len).Insert(pos, str, subpos, sublen);
}

UTF8::StringView& UTF8::StringView::Replace(size_t pos, size_t len, const char* s, size_t n)
{
    return Erase(pos, len).Insert(pos, s, n);
}

#endif // MINIUTF8_IMPLEMENTATION
