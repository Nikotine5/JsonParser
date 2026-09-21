JsonParser - TODO / Code Review Checklist
=========================================

A hand-written recursive-descent JSON parser in C++23.

    Main.cpp            entry point + ad-hoc tests
    h/, src/
      Tokenizer         byte stream -> tokens
      JsonParser        tokens -> JsonValue tree
      JsonValue         variant-based DOM node
      Buffer            chunked reader over a file descriptor

Build:  make        Run:  ./Parser

Status as of the 2026-09-14 review: the tokenizer is in good shape. Five of the
six bugs in the old section 1 are fixed and were re-verified by running them;
the sixth (locale-dependent stod) was replaced with a malformed from_chars call
that does not compile. Two things block the build. Three new correctness bugs
were found by round-tripping parse() -> toString() -> parse(). Still no tests.

Review method: everything marked "VERIFIED" below was reproduced by compiling
and running the parser against ~50 inputs, not by reading the code.


-------------------------------------------------------------------------------
1. BUILD BLOCKERS  (fix these first - the project does not compile)
-------------------------------------------------------------------------------

[x] from_chars is called with the wrong signature.
    src/JsonParser.cpp:44 reads
        value = std::from_chars(m_current.m_value);
    from_chars takes (const char* first, const char* last, T& value) and
    returns a from_chars_result - it does not return the parsed number. Correct
    form:
        auto [ptr, ec] = std::from_chars(
            m_current.m_value.data(),
            m_current.m_value.data() + m_current.m_value.size(),
            value);
    The ec != std::errc{} case should call error(). This is a real API misuse,
    not a compiler-version problem - it fails on every compiler.

[ ] Main.cpp constructs JsonParser from an ifstream.
    Main.cpp:27 reads  JsonParser parser(file);  where file is a std::ifstream.
    JsonParser's only constructor takes std::string_view, so there is no
    matching overload. See section 2 - the right fix is an fd constructor, not
    an ifstream one.


-------------------------------------------------------------------------------
2. CONFIRMED CORRECTNESS BUGS  (all VERIFIED by running)
-------------------------------------------------------------------------------

[x] m_depth is never decremented, so valid JSON is rejected.
    src/JsonParser.cpp:71-86 increments m_depth on '{' and '[' but never
    decrements on the way back out. The counter therefore measures TOTAL
    CONTAINERS IN THE DOCUMENT, not nesting depth.
    VERIFIED: an array of 1500 empty sibling objects - true nesting depth 2 -
    throws "Maximum nesting depth exceeded". A genuinely 50-deep nest parses
    fine. Any real document with more than 1000 objects/arrays in total is
    refused.
    Fix: decrement after ParseObj()/ParseArray() returns. An RAII guard is
    tidier, but since every error path throws all the way out, a plain
    m_depth-- after the recursive call is enough.

[ ] toString() does not escape strings, so it emits invalid JSON.
    src/JsonValue.cpp:8 reads
        return "\"" + str + "\"";
    which wraps raw bytes in quotes with no re-escaping. It is therefore not
    the inverse of readString(): the tokenizer correctly decodes \" into a
    literal '"', and toString() writes that '"' straight back out bare.
    VERIFIED round-trip results (parse -> toString -> parse):
        "a\"b"      -> "a"b"        REPARSE FAILED: Unexpected character 'b'
        "a\nb"      -> "a<LF>b"     REPARSE FAILED: Unescaped control character
        "a\tb"      -> "a<TAB>b"    REPARSE FAILED: Unescaped control character
        "ab"  -> "a<0x01>b"   REPARSE FAILED: Unescaped control character
        "a\\b"      -> "a\b"        reparses as 'a' + backspace - SILENT
                                    DATA CORRUPTION, no error raised
    Five of six cases fail. The backslash case is the worst because it does not
    throw, it just changes the data.
    Fix: escape '"' and '\', and every byte below 0x20 - using \n \t \r \b \f
    where they exist and \u00XX otherwise. ('/' does not need escaping.)

[ ] std::to_string(double) silently destroys small numbers.
    src/JsonValue.cpp:7 uses std::to_string, which always formats a double with
    exactly six decimal places in fixed notation.
    VERIFIED:
        1        -> 1.000000                                (ugly)
        1e30     -> 1000000000000000019884624838656.000000  (ugly)
        1e-30    -> 0.000000                                DATA DESTROYED
        -2.5e-8  -> -0.000000                               DATA DESTROYED
    Anything smaller than ~1e-7 becomes zero.
    Fix: std::to_chars with no precision argument gives the shortest
    round-trippable form. Also strip a trailing ".000000" so integers print as
    1 rather than 1.000000.


-------------------------------------------------------------------------------
3. BUFFER INTEGRATION
-------------------------------------------------------------------------------

Buffer is a 64K sliding-window reader over a file descriptor.

API contract: peek(), peekNext(), advance() all return int, with -1 as the ONLY
end-of-input signal (0..255 are all real data). Bytes are cast through
unsigned char so a UTF-8 byte above 0x7F cannot be mistaken for EOF.

The one rule: never save a position and come back to it. There is no seeking
backwards and no substr. Build each token by appending bytes to a std::string
as you consume them.

[X] Replace  std::string input + position  in Tokenizer with a Buffer member.
[X] Change Tokenizer's peek/peekNext/advance to return int, -1 for EOF instead
    of '\0'. peek() can no longer be const (it may trigger a read).
[x] Replace all ~10  position >= input.size()  checks with  peek() == -1.
[x] Rewrite readNumber to accumulate into a std::string.
[x] Widen isHexDigit / hexValue from char to int.
[X] Keep a string-backed input path. Buffer(std::string) exists.
[X] Decide fd ownership. Buffer owns and closes the fd in its destructor.

[ ] REOPENED: the local isDigit(int) was never actually written.
    This was marked done, but src/Tokenizer.cpp:372 still reads
        if (current == '-' || std::isdigit(current))
    Harmless in practice - peek() == -1 is handled earlier, so current is
    always 0..255 here - but it is the exact trap the int return exists to
    avoid, and it will bite the moment this line moves.

[ ] NEW: the entire fd path is unreachable. Nothing constructs Tokenizer(int).
    h/Tokenizer.hpp:62 declares  explicit Tokenizer(int fd);  and Buffer's
    chunked read loop is written and working, but no caller exists anywhere in
    the project - JsonParser only exposes the string_view constructor, which
    routes to Tokenizer(std::string). The 64K chunked reader, which is the most
    interesting part of this project, is currently dead code.
    Fix: add  explicit JsonParser(int fd);  delegating to m_tokenizer(fd). That
    also resolves the Main.cpp blocker in section 1 - open the file with
    ::open(path, O_RDONLY) and hand the fd over, rather than using ifstream.

[ ] NEW: the Buffer ownership comment contradicts the code.
    h/Buffer.hpp:9 still says "Does not own the fd." but src/Buffer.cpp:18 is
        ~Buffer() { if (m_fd_ >= 0) { ::close(m_fd_); } }
    It does own it. Anyone who trusts the comment and also closes the fd
    themselves gets a double-close. Update the comment.

[ ] NEW: the string path copies the whole document twice.
    string_view -> std::string (copy 1) -> moved into Tokenizer ->
    Buffer(std::string) copies into vector<char> (copy 2). Minor, but worth a
    note now that Buffer is the only input path.

[ ] NEW: getBuf() returns a 64K copy by value.
    h/Buffer.hpp:39. getBuf/getPos/getEnd are all currently uncalled. If they
    are debug helpers, return const& or delete them.


-------------------------------------------------------------------------------
4. API GAPS IN JsonValue
-------------------------------------------------------------------------------

[x] getType() is declared but never defined. Removed.
[x] getValue is a template defined in the .cpp. Now defined in the header.
[x] Add const overloads for getArray / getObject. Both versions now exist.
[x] JsonValue(JsonObject) is not explicit. Now explicit, like the others.
[X] Deliberate decision to record: std::map sorts keys, so serializing loses
    the original insertion order.
[N/A] No way to build or mutate a value - no setters, no operator[], no array
    push_back.

[/] PARTIAL: getString() now has a const& overload, but the two are asymmetric.
    h/JsonValue.hpp:43-44:
        std::string getString();               // non-const: returns a COPY
        const std::string& getString() const;  // const: returns a reference
    getArray/getObject correctly return T& and const T&. As written, the same
    call copies the whole string on a non-const object but not on a const one,
    and there is no way to mutate the string in place. The non-const overload
    should return std::string&.

[ ] NEW: struct overloaded has external linkage.
    src/JsonValue.cpp:4 defines it at namespace scope in a .cpp. "overloaded"
    is the conventional name for that idiom, so a second definition in another
    translation unit is likely and would be an ODR violation the linker may not
    catch. Wrap it in an anonymous namespace.

[ ] NEW: class/struct mismatch on the forward declaration.
    h/JsonValue.hpp:7 says  struct JsonValue;  and line 22 defines
    class JsonValue. GCC and Clang do not care; MSVC warns (C4099).


-------------------------------------------------------------------------------
5. SERIALIZATION
-------------------------------------------------------------------------------

[x] There is now a serialize/dump on JsonValue: toString(), implemented with
    std::visit over a visitor struct with one operator() per alternative.
    Recursion goes through JsonValue::getValue. The dispatch mechanism is
    correct - see section 2 for the two content bugs in it (escaping and
    number formatting).

[N/A] writeToFile was an empty stub in Main.cpp. It is gone.

[ ] An OUTBOUND buffer would genuinely earn its place here: accumulate
    formatted output, flush at 64K. Right now toString() builds one string by
    repeated += and returns it by value at every level of the recursion, so a
    deep document copies its own sub-strings many times over. (The original
    Buffer class was written this way before it was repurposed for inbound
    reading.)


-------------------------------------------------------------------------------
6. MAIN AND TESTING
-------------------------------------------------------------------------------

[ ] No tests exist. This is still the highest-leverage item on the list, and
    the 2026-09-14 review is the argument for it: the three bugs in section 2
    are all invisible to a reading of the code and all trivially caught by a
    table of {input, expected} pairs. The escaping bug in particular cannot be
    found without feeding output back into input - a round-trip test
    (parse -> toString -> parse, assert stable) catches all five failing cases
    on its own. Add a 'make test' target.

[ ] main() hardcodes test(). Take a file path from argv so the parser is
    actually usable.

[x] The const_cast at Main.cpp:55 is gone.
[x] test2 and readFile are gone.


-------------------------------------------------------------------------------
7. VERIFIED WORKING  (record so it does not get re-investigated)
-------------------------------------------------------------------------------

All six bugs in the old section 1 were re-tested on 2026-09-14. Five are fixed:

    \n escape             produces a real 0A byte
    surrogate pairs       "😀" -> F0 9F 98 80, correct emoji
    appendUtf8 3-byte     "耀" -> E8 80 80, correct
    appendUtf8 4-byte     bound is 0x10FFFF, correct
    deep nesting          throws instead of segfaulting (but see section 2 -
                          the fix introduced the m_depth counter bug)

The sixth, locale-dependent stod, was replaced by the broken from_chars call in
section 1.

Rejection coverage is good. All of these are correctly refused with line and
column information:

    trailing comma in array and object      leading zeros
    bare '+1' and '.5' and '1.'             NaN
    lone high surrogate                     unterminated string
    trailing data after the value           single-quoted strings
    missing colon in object                 truncated literals (tru, truex)
    empty input

Accepted correctly: empty object and array, nested mixes, all simple escapes,
BMP and astral unicode escapes, negative numbers, floats, exponents, bare
top-level scalars, leading/trailing whitespace. Duplicate keys resolve
last-wins, which matches most parsers.


-------------------------------------------------------------------------------
8. REPO
-------------------------------------------------------------------------------

[ ] The Parser binary is committed to git. Add it to .gitignore and run
    'git rm --cached Parser'. The current ignore file only covers *.exe and
    main, so the extensionless binary slips through.

[ ] The Makefile recompiles all five translation units on any change. Object
    file rules plus -MMD -MP would give real header dependency tracking and let
    you drop the manual HEADERS list.

[ ] Consider a debug target with -fsanitize=address,undefined.


-------------------------------------------------------------------------------
SUGGESTED ORDER
-------------------------------------------------------------------------------

1. Section 1 - two small fixes, and nothing else can be tested until it builds.
2. Section 2 - m_depth is one line and it rejects valid documents today; the
   other two silently corrupt data.
3. Tests (section 6), including the round-trip test.
4. The JsonParser fd constructor (section 3), which finally makes Buffer
   reachable and is the point of the whole Buffer exercise.
