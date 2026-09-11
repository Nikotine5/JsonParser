JsonParser - TODO / Code Review Checklist
=========================================

A hand-written recursive-descent JSON parser in C++23.

    Main.cpp            entry point + ad-hoc tests
    h/, src/
      Tokenizer         byte stream -> tokens
      JsonParser        tokens -> JsonValue tree
      JsonValue         variant-based DOM node
      Buffer            chunked reader over a file descriptor (not yet wired in)

Build:  make        Run:  ./Parser

Status as of the last review: builds clean, ./Parser runs, but there are five
confirmed correctness bugs in the Unicode/escape path and no test suite.


-------------------------------------------------------------------------------
1. CONFIRMED CORRECTNESS BUGS  (fix these first)
-------------------------------------------------------------------------------

All of the following were reproduced by running the parser, not just by reading.

[X] \n escape produces the letter 'n', not a newline.
    src/Tokenizer.cpp:193 reads  value += 'n';  and should be  value += '\n';
    Every other escape in that switch is correct, so it is a one-character typo.
    Verified: "a\nb" parses to bytes 61 6E 62.

[X] All surrogate pairs are rejected.
    src/Tokenizer.cpp:126 reads
        if (second >= 0xDC00 && second <= 0xDFFF) { error(...); }
    which fires when 'second' IS a valid low surrogate. The condition needs to
    be negated.
    Verified: 😀 throws "Invalid Unicode surrogate pair".
    No emoji, and nothing above U+FFFF, can currently be parsed.

[X] appendUtf8 three-byte bound is wrong.
    src/Tokenizer.cpp:91 says  codepoint <= 0x7FFF;  the three-byte UTF-8 range
    ends at 0xFFFF. Codepoints U+8000..U+FFFF fall through into the four-byte
    branch and come out as invalid UTF-8.
    Verified: 耀 gives F0 88 80 80, should be E8 80 80.
    This silently corrupts a large slice of the BMP.

[X] appendUtf8 four-byte bound is wrong too.
    src/Tokenizer.cpp:96 says 0x7FFFF; Unicode's maximum is 0x10FFFF. Once the
    surrogate bug above is fixed, codepoints above U+7FFFF will incorrectly hit
    the "Invalid Unicode codepoint" error.

[X] Deeply nested input segfaults.
    parseValue -> ParseArray -> parseValue recurses with no depth cap, so
    "[[[[..." overflows the stack.
    Verified: 200,000 open brackets crashes with SIGSEGV, not an exception.
    Add an m_depth counter with a limit (a few hundred is plenty) and call
    error() past it. Standard hardening for any parser that may see untrusted
    input.

[X] std::stod is locale-dependent.
    src/JsonParser.cpp:45 - stod honors the global C locale's decimal
    separator, so under e.g. a German locale "1.5" parses as 1. JSON's grammar
    is locale-independent. Replace with std::from_chars, which is also faster
    and removes the need for the try/catch.


-------------------------------------------------------------------------------
2. BUFFER INTEGRATION  (work in flight)
-------------------------------------------------------------------------------

Buffer is a 64K sliding-window reader over a file descriptor. It compiles and
is in the Makefile, but nothing calls it yet.

API contract: peek(), peekNext(), advance() all return int, with -1 as the ONLY
end-of-input signal (0..255 are all real data). Bytes are cast through
unsigned char so a UTF-8 byte above 0x7F cannot be mistaken for EOF.

The one rule: never save a position and come back to it. There is no seeking
backwards and no substr. Build each token by appending bytes to a std::string
as you consume them.

[ ] Replace  std::string input + position  in Tokenizer with a Buffer member.
[ ] Change Tokenizer's peek/peekNext/advance to return int, -1 for EOF instead
    of '\0'. peek() can no longer be const (it may trigger a read).
[ ] Replace all ~10  position >= input.size()  checks with  peek() == -1.
[ ] Rewrite readNumber to accumulate into a std::string.
    src/Tokenizer.cpp:229 saves  size_t start  and calls input.substr at line
    284. A saved offset is meaningless once the window slides. This is the only
    function that structurally cannot survive the swap.
[ ] Replace  std::isdigit(static_cast<unsigned char>(peek()))  with a local
    isDigit(int). The current form casts -1 to 255, which happens to work but
    is exactly the trap the int return exists to avoid.
[ ] Widen isHexDigit / hexValue from char to int.
[ ] Keep a string-backed input path. test() parses an inline literal and tests
    need in-memory input. Either a second Tokenizer constructor or a
    Buffer(std::string) constructor.
[ ] Decide fd ownership. Buffer currently does not close what it is given;
    either wrap the fd in an RAII type or make Buffer own it.


-------------------------------------------------------------------------------
3. API GAPS IN JsonValue
-------------------------------------------------------------------------------

[ ] getType() is declared but never defined.
    h/JsonValue.hpp:43 - a link error the moment anyone calls it. The signature
    is also suspect: returning JsonValue to describe a type should probably be
    an enum.

[ ] getValue is a template defined in the .cpp.
    src/JsonValue.cpp:81 - it can never instantiate from another translation
    unit. Move it into the header.

[ ] Add const overloads for getArray / getObject.
    They are now non-const only, so a const JsonValue cannot be read at all.
    Want both  JsonArray&  and  const JsonArray&  versions.

[ ] getString() returns by value, copying the string on every access.
    Add a  const std::string&  overload.

[ ] JsonValue(JsonObject) is not explicit while every other converting
    constructor is (h/JsonValue.hpp:31). Probably unintentional.

[ ] No way to build or mutate a value - no setters, no operator[], no array
    push_back. These are needed before writeToFile can do anything.

[ ] Deliberate decision to record: std::map sorts keys, so serializing loses
    the original insertion order.


-------------------------------------------------------------------------------
4. SERIALIZATION
-------------------------------------------------------------------------------

[ ] writeToFile (Main.cpp:105) is an empty stub - opens a file, ignores
    'content', returns "success". It is also never called and never forward
    declared. Either implement it or delete it.

[ ] There is no serialize/dump on JsonValue at all. This is where an OUTBOUND
    buffer would genuinely earn its place: accumulate formatted output, flush
    at 64K. (The original Buffer class was written that way before it was
    repurposed for inbound reading.)


-------------------------------------------------------------------------------
5. MAIN AND TESTING
-------------------------------------------------------------------------------

[ ] No tests exist. Given five real bugs in the Unicode path alone, this is the
    highest-leverage item on the whole list. A table of {input, expected} pairs
    run against parse() would have caught all of them. Add a 'make test'
    target.

[ ] main() hardcodes test(). Take a file path from argv so the parser is
    actually usable.

[ ] The const_cast<JsonArray&> at Main.cpp:55 is now unnecessary - getArray()
    already returns a non-const reference.

[ ] test2 and readFile become dead code once Buffer lands.


-------------------------------------------------------------------------------
6. BUILD AND REPO
-------------------------------------------------------------------------------

[ ] The Parser binary is committed to git. Add it to .gitignore and run
    'git rm --cached Parser'. The current ignore file only covers *.exe and
    main.

[ ] The Makefile recompiles all five translation units on any change. Object
    file rules plus -MMD -MP would give real header dependency tracking and let
    you drop the manual HEADERS list.

[ ] Consider a debug target with -fsanitize=address,undefined. It would have
    caught the stack overflow as a clean report rather than a bare segfault.


-------------------------------------------------------------------------------
SUGGESTED ORDER
-------------------------------------------------------------------------------

1. The six items in section 1. All small, all real.
2. Tests (section 5).
3. The Buffer swap (section 2).

Doing tests before the Buffer swap means you will know immediately if the
conversion breaks something.
