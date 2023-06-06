# isspace_locale: weird isspace behaviour on Mac OS X

A C program to print the values where `isspace()` returns true for different locales. This was used to understand a strange corner case bug in Postgres when running on Mac OS X. See [my blog post for more discussion, and details about the Postgres bug](https://www.evanjones.ca/isspace_locale.html). The short version is you should either use your own version for ASCII-only (below), or use [ICU4C](https://unicode-org.github.io/icu/userguide/icu4c/)'s [u_isspace()](https://unicode-org.github.io/icu-docs/apidoc/dev/icu4c/uchar_8h.html#a48dd198b451e691cf81eb41831474ddc) function.

```c
/* Returns true for the 6 ASCII white-space characters: \t \n \v \f \r ' '. */
int isspace_ascii(int c)
{
  return c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r' || c == ' ';
}
```

## ASCII white-space characters

| Description | Acronym | C literal | Hex value |
| --- | --- | --- | --- |
| horizontal tab | TAB | '\t' | 0x09 |
| line feed | LF | '\n' | 0x0a |
| vertical tab | VT | '\v' | 0x0b |
| form feed | FF | '\f' | 0x0c |
| carriage return | CR | '\r' | 0x0d |
| space | SP | ' ' | 0x20 |


## isspace is weird

The `isspace()` function takes an `int` as an argument, not a `char`. The documentation for this function reads: "The value of the argument must be representable as an unsigned char or the value of EOF.". The value of `EOF` is typically `-1`, so this means you should only pass the values in the inclusive range [-1, 255]. This means a "correct" call to `isspace` requires a cast: `isspace((unsigned char) c)`

However, when using a non-default locale, such as `en_US.UTF-8`, different systems may return true for other values.


## Mac OS X weirdness

On Mac OS X in a UTF-8 locale, `isspace()` interprets its argument as a Unicode code point. This means it returns true for [0x85 = NEL = Next Line](https://codepoints.net/U+0085), and [0xA0 = NBSP = No-Break Space](https://codepoints.net/U+00A0), which are in the "valid" range for `isspace()`. It also returns true for other Unicode white-space characters, such as [0x1680 = Ogham Space Mark](https://codepoints.net/U+1680) and [0x2000 = En Quad](https://codepoints.net/U+2000). By default, a process starts in the C locale, so it should only return true for ASCII white-space characters. However, if it calls `setlocale()` for any reason, the behaviour can change.

The man page mentions this behavior may go away:

"The 4.4BSD extension of accepting arguments outside of the range of the unsigned char type in locales with large character sets is considered obsolete and may not be supported in future releases.  The iswspace() function should be used instead."


## Linux weirdness

In UTF-8 locales such as `en_US.UTF-8`, `isspace()` will return true for some values outside the "valid" range declared by the C standard, that don't make any sense to me. For example, `isspace(0x01fe)` is true. I can't figure out why this might be considered a whitespace character.


## Mac OS X Output

As of Ventura 13.4:

```
default locale at startup LC_TYPE=C
sizeof(int)=4 (32 bits; 8 hex chars)
EOF=-1 = 0xffffffff
testing isspace() in inclusive range [-128, 8208] with locales:
  NOTE: values outside [-1, 255] are undefined in the C standard
    C
    en_US.UTF-8
    en_US.US-ASCII
    en_US
    de_DE.UTF-8

c: 9 = 0x00000009 ((char) 9 = 0x09)
    C: isspace(9) = 1
    en_US.UTF-8: isspace(9) = 1
    en_US.US-ASCII: isspace(9) = 1
    en_US: isspace(9) = 1
    de_DE.UTF-8: isspace(9) = 1

c: 10 = 0x0000000a ((char) 10 = 0x0a)
    C: isspace(10) = 1
    en_US.UTF-8: isspace(10) = 1
    en_US.US-ASCII: isspace(10) = 1
    en_US: isspace(10) = 1
    de_DE.UTF-8: isspace(10) = 1

c: 11 = 0x0000000b ((char) 11 = 0x0b)
    C: isspace(11) = 1
    en_US.UTF-8: isspace(11) = 1
    en_US.US-ASCII: isspace(11) = 1
    en_US: isspace(11) = 1
    de_DE.UTF-8: isspace(11) = 1

c: 12 = 0x0000000c ((char) 12 = 0x0c)
    C: isspace(12) = 1
    en_US.UTF-8: isspace(12) = 1
    en_US.US-ASCII: isspace(12) = 1
    en_US: isspace(12) = 1
    de_DE.UTF-8: isspace(12) = 1

c: 13 = 0x0000000d ((char) 13 = 0x0d)
    C: isspace(13) = 1
    en_US.UTF-8: isspace(13) = 1
    en_US.US-ASCII: isspace(13) = 1
    en_US: isspace(13) = 1
    de_DE.UTF-8: isspace(13) = 1

c: 32 = 0x00000020 ((char) 32 = 0x20)
    C: isspace(32) = 1
    en_US.UTF-8: isspace(32) = 1
    en_US.US-ASCII: isspace(32) = 1
    en_US: isspace(32) = 1
    de_DE.UTF-8: isspace(32) = 1

c: 133 = 0x00000085 ((char) -123 = 0x85)
    en_US.UTF-8: isspace(133) = 1
    en_US: isspace(133) = 1
    de_DE.UTF-8: isspace(133) = 1

c: 160 = 0x000000a0 ((char) -96 = 0xa0)
    en_US.UTF-8: isspace(160) = 1
    en_US: isspace(160) = 1
    de_DE.UTF-8: isspace(160) = 1

c: 5760 = 0x00001680 ((char) -128 = 0x80) (undefined result in C standard)
    en_US.UTF-8: isspace(5760) = 1
    en_US: isspace(5760) = 1
    de_DE.UTF-8: isspace(5760) = 1

c: 8192 = 0x00002000 ((char) 0 = 0x00) (undefined result in C standard)
    en_US.UTF-8: isspace(8192) = 1
    en_US: isspace(8192) = 1
    de_DE.UTF-8: isspace(8192) = 1

c: 8193 = 0x00002001 ((char) 1 = 0x01) (undefined result in C standard)
    en_US.UTF-8: isspace(8193) = 1
    en_US: isspace(8193) = 1
    de_DE.UTF-8: isspace(8193) = 1

c: 8194 = 0x00002002 ((char) 2 = 0x02) (undefined result in C standard)
    en_US.UTF-8: isspace(8194) = 1
    en_US: isspace(8194) = 1
    de_DE.UTF-8: isspace(8194) = 1

c: 8195 = 0x00002003 ((char) 3 = 0x03) (undefined result in C standard)
    en_US.UTF-8: isspace(8195) = 1
    en_US: isspace(8195) = 1
    de_DE.UTF-8: isspace(8195) = 1

c: 8196 = 0x00002004 ((char) 4 = 0x04) (undefined result in C standard)
    en_US.UTF-8: isspace(8196) = 1
    en_US: isspace(8196) = 1
    de_DE.UTF-8: isspace(8196) = 1

c: 8197 = 0x00002005 ((char) 5 = 0x05) (undefined result in C standard)
    en_US.UTF-8: isspace(8197) = 1
    en_US: isspace(8197) = 1
    de_DE.UTF-8: isspace(8197) = 1

c: 8198 = 0x00002006 ((char) 6 = 0x06) (undefined result in C standard)
    en_US.UTF-8: isspace(8198) = 1
    en_US: isspace(8198) = 1
    de_DE.UTF-8: isspace(8198) = 1

c: 8199 = 0x00002007 ((char) 7 = 0x07) (undefined result in C standard)
    en_US.UTF-8: isspace(8199) = 1
    en_US: isspace(8199) = 1
    de_DE.UTF-8: isspace(8199) = 1

c: 8200 = 0x00002008 ((char) 8 = 0x08) (undefined result in C standard)
    en_US.UTF-8: isspace(8200) = 1
    en_US: isspace(8200) = 1
    de_DE.UTF-8: isspace(8200) = 1

c: 8201 = 0x00002009 ((char) 9 = 0x09) (undefined result in C standard)
    en_US.UTF-8: isspace(8201) = 1
    en_US: isspace(8201) = 1
    de_DE.UTF-8: isspace(8201) = 1

c: 8202 = 0x0000200a ((char) 10 = 0x0a) (undefined result in C standard)
    en_US.UTF-8: isspace(8202) = 1
    en_US: isspace(8202) = 1
    de_DE.UTF-8: isspace(8202) = 1

c: 8203 = 0x0000200b ((char) 11 = 0x0b) (undefined result in C standard)
    en_US.UTF-8: isspace(8203) = 1
    en_US: isspace(8203) = 1
    de_DE.UTF-8: isspace(8203) = 1
```

Output from `locale`:

```
LANG="en_US.UTF-8"
LC_COLLATE="en_US.UTF-8"
LC_CTYPE="en_US.UTF-8"
LC_MESSAGES="en_US.UTF-8"
LC_MONETARY="en_US.UTF-8"
LC_NUMERIC="en_US.UTF-8"
LC_TIME="en_US.UTF-8"
LC_ALL=
```

## Linux Output

Using Ubuntu 20.04.6 LTS with glibc version `(Ubuntu GLIBC 2.31-0ubuntu9.9) 2.31` (from `ldd --version`), after running `apt install language-pack-en language-pack-de` to get the extra locales (`locale -a` only listed `C`, `C.UTF-8`, `en_US.utf8`, `POSIX`).

```
default locale at startup LC_TYPE=C
sizeof(int)=4 (32 bits; 8 hex chars)
EOF=-1 = 0xffffffff
WARNING: setlocale(LC_CTYPE, "en_US.US-ASCII")=NULL; skipping locale
WARNING: setlocale(LC_CTYPE, "en_US")=NULL; skipping locale
testing isspace() in inclusive range [-128, 8208] with locales:
  NOTE: values outside [-1, 255] are undefined in the C standard
    C
    en_US.UTF-8
    de_DE.UTF-8

c: 9 = 0x00000009 ((char) 9 = 0x09)
    C: isspace(9) = 8192
    en_US.UTF-8: isspace(9) = 8192
    de_DE.UTF-8: isspace(9) = 8192

c: 10 = 0x0000000a ((char) 10 = 0x0a)
    C: isspace(10) = 8192
    en_US.UTF-8: isspace(10) = 8192
    de_DE.UTF-8: isspace(10) = 8192

c: 11 = 0x0000000b ((char) 11 = 0x0b)
    C: isspace(11) = 8192
    en_US.UTF-8: isspace(11) = 8192
    de_DE.UTF-8: isspace(11) = 8192

c: 12 = 0x0000000c ((char) 12 = 0x0c)
    C: isspace(12) = 8192
    en_US.UTF-8: isspace(12) = 8192
    de_DE.UTF-8: isspace(12) = 8192

c: 13 = 0x0000000d ((char) 13 = 0x0d)
    C: isspace(13) = 8192
    en_US.UTF-8: isspace(13) = 8192
    de_DE.UTF-8: isspace(13) = 8192

c: 32 = 0x00000020 ((char) 32 = 0x20)
    C: isspace(32) = 8192
    en_US.UTF-8: isspace(32) = 8192
    de_DE.UTF-8: isspace(32) = 8192

c: 510 = 0x000001fe ((char) -2 = 0xfe) (undefined result in C standard)
    en_US.UTF-8: isspace(510) = 8192
    de_DE.UTF-8: isspace(510) = 8192

c: 511 = 0x000001ff ((char) -1 = 0xff) (undefined result in C standard)
    en_US.UTF-8: isspace(511) = 8192
    de_DE.UTF-8: isspace(511) = 8192

c: 1278 = 0x000004fe ((char) -2 = 0xfe) (undefined result in C standard)
    en_US.UTF-8: isspace(1278) = 8192
    de_DE.UTF-8: isspace(1278) = 8192

c: 1279 = 0x000004ff ((char) -1 = 0xff) (undefined result in C standard)
    en_US.UTF-8: isspace(1279) = 8192
    de_DE.UTF-8: isspace(1279) = 8192

c: 1811 = 0x00000713 ((char) 19 = 0x13) (undefined result in C standard)
    en_US.UTF-8: isspace(1811) = 8192
    de_DE.UTF-8: isspace(1811) = 8192

c: 1813 = 0x00000715 ((char) 21 = 0x15) (undefined result in C standard)
    en_US.UTF-8: isspace(1813) = 8192
    de_DE.UTF-8: isspace(1813) = 8192

c: 1815 = 0x00000717 ((char) 23 = 0x17) (undefined result in C standard)
    en_US.UTF-8: isspace(1815) = 8192
    de_DE.UTF-8: isspace(1815) = 8192

c: 1817 = 0x00000719 ((char) 25 = 0x19) (undefined result in C standard)
    en_US.UTF-8: isspace(1817) = 8192
    de_DE.UTF-8: isspace(1817) = 8192

c: 1819 = 0x0000071b ((char) 27 = 0x1b) (undefined result in C standard)
    en_US.UTF-8: isspace(1819) = 8192
    de_DE.UTF-8: isspace(1819) = 8192

c: 1857 = 0x00000741 ((char) 65 = 0x41) (undefined result in C standard)
    en_US.UTF-8: isspace(1857) = 8192
    de_DE.UTF-8: isspace(1857) = 8192

c: 2304 = 0x00000900 ((char) 0 = 0x00) (undefined result in C standard)
    en_US.UTF-8: isspace(2304) = 8192
    de_DE.UTF-8: isspace(2304) = 8192
... TRUNCATED MANY MORE LINES ...
```

Output from `locale`:

```
LANG=en_US.UTF-8
LANGUAGE=
LC_CTYPE="en_US.UTF-8"
LC_NUMERIC="en_US.UTF-8"
LC_TIME="en_US.UTF-8"
LC_COLLATE="en_US.UTF-8"
LC_MONETARY="en_US.UTF-8"
LC_MESSAGES="en_US.UTF-8"
LC_PAPER="en_US.UTF-8"
LC_NAME="en_US.UTF-8"
LC_ADDRESS="en_US.UTF-8"
LC_TELEPHONE="en_US.UTF-8"
LC_MEASUREMENT="en_US.UTF-8"
LC_IDENTIFICATION="en_US.UTF-8"
LC_ALL=
```