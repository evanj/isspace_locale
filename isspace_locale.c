#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>

static const char *const LOCALES[] = {
    "C", "en_US.UTF-8", "en_US.US-ASCII", "en_US", "de_DE.UTF-8",
};

static const size_t NUM_LOCALES = sizeof(LOCALES) / sizeof(*LOCALES);

int main() {
  char *current_locale = setlocale(LC_CTYPE, NULL);

  printf("default locale at startup LC_TYPE=%s\n", current_locale);
  printf("sizeof(int)=%zd (%zd bits; %zd hex chars)\n", sizeof(int),
         sizeof(int) * 8, sizeof(int) * 2);
  printf("EOF=%d = 0x%08x\n", EOF, EOF);

  // test the valid locales on this system
  const char *valid_locales[NUM_LOCALES];
  size_t num_valid_locales = 0;
  for (size_t locale_index = 0; locale_index < NUM_LOCALES; locale_index++) {
    char *result = setlocale(LC_CTYPE, LOCALES[locale_index]);
    if (result == NULL) {
      printf("WARNING: setlocale(LC_CTYPE, \"%s\")=NULL; skipping locale\n",
             LOCALES[locale_index]);
    } else {
      valid_locales[num_valid_locales] = LOCALES[locale_index];
      num_valid_locales += 1;
    }
  }

  // defines the INCLUSIVE range where the C standard defines values for
  // isspace() this assumes EOF is -1.
  assert(EOF == -1);
  static const int c_standard_defined_start = EOF;
  static const int c_standard_defined_end = UCHAR_MAX;

  static const int start = -128;
  static const int end = 0x2010;
  printf("testing isspace() in inclusive range [%d, %d] with locales:\n", start,
         end);
  printf("  NOTE: values outside [%d, %d] are undefined in the C standard\n",
         c_standard_defined_start, c_standard_defined_end);
  for (size_t locale_index = 0; locale_index < num_valid_locales;
       locale_index++) {
    printf("    %s\n", valid_locales[locale_index]);
  }
  printf("\n");

  for (int c = start; c <= end; c++) {
    bool printed = false;
    for (size_t locale_index = 0; locale_index < num_valid_locales;
         locale_index++) {
      char *result = setlocale(LC_CTYPE, valid_locales[locale_index]);
      assert(result != NULL);

      if (isspace(c) != 0) {
        if (!printed) {
          const char *c_undefined_value = "";
          if (!(c_standard_defined_start <= c && c <= c_standard_defined_end)) {
            c_undefined_value = " (undefined result in C standard)";
          }
          printf("c: %d = 0x%08x ((char) %d = 0x%02hhx)%s\n", c, c, (char)c,
                 (char)c, c_undefined_value);
          printed = true;
        }
        printf("    %s: isspace(%d) = %d\n", valid_locales[locale_index], c,
               isspace(c));
        printed = true;
      }
    }

    if (printed) {
      printf("\n");
    }
  }
}
