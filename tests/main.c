#include <test.h>

int main(void)
{
    // Lexer
    test_declare(lexer_next_punctuation);

    test_declare(utf8_to_codepoint);

    // String
    test_declare(string_free);
    test_declare(string_append);
    test_declare(string_append_range);
    test_declare(string_append_codepoint);
    test_declare(string_reserve);
    test_declare(string_ref);
    test_declare(string_release);
    test_declare(string_shrink);
}
