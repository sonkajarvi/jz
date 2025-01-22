#include <jz/lexer.h>

bool isbin(const char c)
{
    return c == '0' || c == '1';
}

int bintoi(const char c)
{
    if (isbin(c))
        return c - '0';

    return -1;
}

bool isoct(const char c)
{
    return '0' <= c && c <= '7';
}

int octtoi(const char c)
{
    if (isoct(c))
        return c - '0';

    return -1;
}

bool isdec(const char c)
{
    return '0' <= c && c <= '9';
}

int dectoi(const char c)
{
    if (isdec(c))
        return c - '0';

    return -1;
}

bool ishex(const char c)
{
    switch (c) {
    case '0' ... '9':
    case 'A' ... 'F':
    case 'a' ... 'f':
        return true;
    }

    return false;
}

int hextoi(const char c)
{
    switch (c) {
    case '0' ... '9':
        return c - '0';

    case 'A' ... 'F':
        return c - 'A' + 10;

    case 'a' ... 'f':
        return c - 'a' + 10;
    }

    return -1;
}

