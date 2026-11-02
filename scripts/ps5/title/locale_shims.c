// PS5 OpenGL - OpenGL implementation for PlayStation 5.
// Copyright (C) 2026 BlackBearReloaded
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Vendored for the wiliwili PS5 native title, unchanged: this is the set of
// libc entry points a statically linked C++ program needs and the console does
// not export. The one below that wiliwili relies on as well is __wrap_fopen,
// the linker is told to route fopen() through it (see build_title.sh) so that
// the relative paths the application libraries open resolve under /app0.

#include <_ctype.h>
#include <errno.h>
#include <execinfo.h>
#include <locale.h>
#include <nl_types.h>
#include <pthread.h>
#include <runetype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <wchar.h>
#include <wctype.h>
#include <xlocale.h>

struct ps5_thread_destructor {
  void (*function)(void *);
  void *object;
  struct ps5_thread_destructor *next;
};

static pthread_key_t ps5_thread_destructor_key;
static pthread_once_t ps5_thread_destructor_once = PTHREAD_ONCE_INIT;
static int ps5_thread_destructor_error;

static void ps5_run_thread_destructors(void *opaque) {
  struct ps5_thread_destructor *entry = opaque;
  pthread_setspecific(ps5_thread_destructor_key, NULL);
  while (entry != NULL) {
    struct ps5_thread_destructor *next = entry->next;
    entry->function(entry->object);
    free(entry);
    entry = next;
  }
}

static void ps5_create_thread_destructor_key(void) {
  ps5_thread_destructor_error = pthread_key_create(
      &ps5_thread_destructor_key, ps5_run_thread_destructors);
}

int __cxa_thread_atexit_impl(void (*function)(void *), void *object,
                             void *dso_handle) {
  (void)dso_handle; /* The native title does not unload its executable DSO. */
  pthread_once(&ps5_thread_destructor_once, ps5_create_thread_destructor_key);
  if (ps5_thread_destructor_error != 0)
    return -1;

  struct ps5_thread_destructor *entry = malloc(sizeof(*entry));
  if (entry == NULL)
    return -1;
  entry->function = function;
  entry->object = object;
  entry->next = pthread_getspecific(ps5_thread_destructor_key);
  if (pthread_setspecific(ps5_thread_destructor_key, entry) != 0) {
    free(entry);
    return -1;
  }
  return 0;
}

static _RuneLocale ps5_cts_runes;
static int ps5_cts_runes_ready;

static void ps5_cts_init_runes(void) {
  if (ps5_cts_runes_ready)
    return;

  memcpy(ps5_cts_runes.__magic, _RUNE_MAGIC_1, 8);
  memcpy(ps5_cts_runes.__encoding, "NONE", 5);
  for (int value = 0; value < _CACHED_RUNES; ++value) {
    ps5_cts_runes.__maplower[value] = value;
    ps5_cts_runes.__mapupper[value] = value;
    if (value < 32 || value == 127)
      ps5_cts_runes.__runetype[value] = _CTYPE_C;
    else if (value >= 33 && value <= 126)
      ps5_cts_runes.__runetype[value] =
          _CTYPE_G | _CTYPE_R | _CTYPE_P | _CTYPE_SW1;
  }

  ps5_cts_runes.__runetype[' '] = _CTYPE_S | _CTYPE_B | _CTYPE_R | _CTYPE_SW1;
  ps5_cts_runes.__runetype['\t'] |= _CTYPE_S | _CTYPE_B;
  for (int value = '\n'; value <= '\r'; ++value)
    ps5_cts_runes.__runetype[value] |= _CTYPE_S;

  for (int value = '0'; value <= '9'; ++value)
    ps5_cts_runes.__runetype[value] = _CTYPE_D | _CTYPE_N | _CTYPE_G |
                                      _CTYPE_R | _CTYPE_SW1 |
                                      (unsigned long)(value - '0');
  for (int value = 'A'; value <= 'Z'; ++value) {
    ps5_cts_runes.__runetype[value] =
        _CTYPE_A | _CTYPE_U | _CTYPE_G | _CTYPE_R | _CTYPE_SW1;
    ps5_cts_runes.__maplower[value] = value - 'A' + 'a';
  }
  for (int value = 'a'; value <= 'z'; ++value) {
    ps5_cts_runes.__runetype[value] =
        _CTYPE_A | _CTYPE_L | _CTYPE_G | _CTYPE_R | _CTYPE_SW1;
    ps5_cts_runes.__mapupper[value] = value - 'a' + 'A';
  }
  for (int value = 'A'; value <= 'F'; ++value)
    ps5_cts_runes.__runetype[value] |= _CTYPE_X;
  for (int value = 'a'; value <= 'f'; ++value)
    ps5_cts_runes.__runetype[value] |= _CTYPE_X;
  for (int value = '0'; value <= '9'; ++value)
    ps5_cts_runes.__runetype[value] |= _CTYPE_X;

  ps5_cts_runes_ready = 1;
}

_RuneLocale *__runes_for_locale(locale_t locale, int *limit) {
  (void)locale;
  ps5_cts_init_runes();
  if (limit != NULL)
    *limit = _CACHED_RUNES;
  return &ps5_cts_runes;
}

locale_t newlocale(int mask, const char *name, locale_t base) {
  static int c_locale;
  (void)mask;
  (void)name;
  return base != NULL && base != LC_GLOBAL_LOCALE ? base : (locale_t)&c_locale;
}

int freelocale(locale_t locale) {
  (void)locale;
  return 0;
}

int strcoll_l(const char *left, const char *right, locale_t locale) {
  (void)locale;
  return strcoll(left, right);
}

size_t strxfrm_l(char *output, const char *input, size_t size,
                 locale_t locale) {
  (void)locale;
  return strxfrm(output, input, size);
}

int wcscoll_l(const wchar_t *left, const wchar_t *right, locale_t locale) {
  (void)locale;
  return wcscoll(left, right);
}

size_t wcsxfrm_l(wchar_t *restrict output, const wchar_t *restrict input,
                 size_t size, locale_t locale) {
  (void)locale;
  return wcsxfrm(output, input, size);
}

int iswctype_l(wint_t value, wctype_t type, locale_t locale) {
  (void)locale;
  return (___runetype_l((__ct_rune_t)value, locale) & (unsigned long)type) != 0;
}

unsigned long ___runetype_l(__ct_rune_t value, locale_t locale) {
  (void)locale;
  ps5_cts_init_runes();
  return value >= 0 && value < _CACHED_RUNES ? ps5_cts_runes.__runetype[value]
                                             : 0;
}

__ct_rune_t ___toupper_l(__ct_rune_t value, locale_t locale) {
  (void)locale;
  ps5_cts_init_runes();
  return value >= 0 && value < _CACHED_RUNES ? ps5_cts_runes.__mapupper[value]
                                             : value;
}

__ct_rune_t ___tolower_l(__ct_rune_t value, locale_t locale) {
  (void)locale;
  ps5_cts_init_runes();
  return value >= 0 && value < _CACHED_RUNES ? ps5_cts_runes.__maplower[value]
                                             : value;
}

wint_t btowc_l(int value, locale_t locale) {
  (void)locale;
  return btowc(value);
}

int wctob_l(wint_t value, locale_t locale) {
  (void)locale;
  return wctob(value);
}

size_t wcsnrtombs_l(char *restrict output, const wchar_t **restrict source,
                    size_t source_size, size_t output_size,
                    mbstate_t *restrict state, locale_t locale) {
  const wchar_t *input = *source;
  size_t written = 0;
  size_t consumed = 0;
  (void)state;
  (void)locale;
  while (consumed < source_size && input[consumed] != L'\0') {
    if ((unsigned long)input[consumed] > 0x7f) {
      errno = EILSEQ;
      return (size_t)-1;
    }
    if (output != NULL) {
      if (written == output_size)
        break;
      output[written] = (char)input[consumed];
    }
    ++written;
    ++consumed;
  }
  if (output != NULL)
    *source = consumed < source_size && input[consumed] == L'\0'
                  ? NULL
                  : input + consumed;
  return written;
}

size_t wcrtomb_l(char *restrict output, wchar_t value,
                 mbstate_t *restrict state, locale_t locale) {
  (void)locale;
  return wcrtomb(output, value, state);
}

size_t mbsnrtowcs_l(wchar_t *restrict output, const char **restrict source,
                    size_t source_size, size_t output_size,
                    mbstate_t *restrict state, locale_t locale) {
  const unsigned char *input = (const unsigned char *)*source;
  size_t written = 0;
  size_t consumed = 0;
  (void)state;
  (void)locale;
  while (consumed < source_size && input[consumed] != '\0') {
    if (input[consumed] > 0x7f) {
      errno = EILSEQ;
      return (size_t)-1;
    }
    if (output != NULL) {
      if (written == output_size)
        break;
      output[written] = input[consumed];
    }
    ++written;
    ++consumed;
  }
  if (output != NULL)
    *source = consumed < source_size && input[consumed] == '\0'
                  ? NULL
                  : (const char *)input + consumed;
  return written;
}

size_t mbrtowc_l(wchar_t *restrict output, const char *restrict input,
                 size_t size, mbstate_t *restrict state, locale_t locale) {
  (void)locale;
  return mbrtowc(output, input, size, state);
}

int mbtowc_l(wchar_t *restrict output, const char *restrict input, size_t size,
             locale_t locale) {
  (void)locale;
  return mbtowc(output, input, size);
}

int ___mb_cur_max_l(locale_t locale) {
  (void)locale;
  return 1;
}

size_t mbrlen_l(const char *restrict input, size_t size,
                mbstate_t *restrict state, locale_t locale) {
  (void)locale;
  return mbrlen(input, size, state);
}

struct lconv *localeconv_l(locale_t locale) {
  (void)locale;
  return localeconv();
}

long long strtoll_l(const char *text, char **end, int base, locale_t locale) {
  (void)locale;
  return strtoll(text, end, base);
}

unsigned long long strtoull_l(const char *text, char **end, int base,
                              locale_t locale) {
  (void)locale;
  return strtoull(text, end, base);
}

float strtof_l(const char *text, char **end, locale_t locale) {
  (void)locale;
  return strtof(text, end);
}

double strtod_l(const char *text, char **end, locale_t locale) {
  (void)locale;
  return strtod(text, end);
}

long double strtold_l(const char *text, char **end, locale_t locale) {
  (void)locale;
  return strtold(text, end);
}

int sscanf_l(const char *text, locale_t locale, const char *format, ...) {
  va_list arguments;
  (void)locale;
  va_start(arguments, format);
  int result = vsscanf(text, format, arguments);
  va_end(arguments);
  return result;
}

int snprintf_l(char *output, size_t size, locale_t locale, const char *format,
               ...) {
  va_list arguments;
  (void)locale;
  va_start(arguments, format);
  int result = vsnprintf(output, size, format, arguments);
  va_end(arguments);
  return result;
}

int asprintf_l(char **output, locale_t locale, const char *format, ...) {
  va_list arguments;
  va_list copy;
  (void)locale;
  va_start(arguments, format);
  va_copy(copy, arguments);
  int length = vsnprintf(NULL, 0, format, copy);
  va_end(copy);
  if (length < 0) {
    va_end(arguments);
    return -1;
  }
  *output = malloc((size_t)length + 1);
  if (*output == NULL) {
    va_end(arguments);
    return -1;
  }
  int result = vsnprintf(*output, (size_t)length + 1, format, arguments);
  va_end(arguments);
  return result;
}

size_t strftime_l(char *output, size_t size, const char *format,
                  const struct tm *time_value, locale_t locale) {
  (void)locale;
  return strftime(output, size, format, time_value);
}

size_t mbsrtowcs_l(wchar_t *restrict output, const char **restrict source,
                   size_t size, mbstate_t *restrict state, locale_t locale) {
  (void)locale;
  return mbsrtowcs(output, source, size, state);
}

nl_catd catopen(const char *name, int type) {
  (void)name;
  (void)type;
  return (nl_catd)-1;
}

char *catgets(nl_catd catalog, int set, int message, const char *fallback) {
  (void)catalog;
  (void)set;
  (void)message;
  return (char *)fallback;
}

int catclose(nl_catd catalog) {
  (void)catalog;
  return 0;
}

int utimensat(int directory, const char *path, const struct timespec times[2],
              int flags) {
  (void)directory;
  (void)path;
  (void)times;
  (void)flags;
  errno = ENOSYS;
  return -1;
}

size_t backtrace(void **addresses, size_t size) {
  (void)addresses;
  (void)size;
  return 0;
}

int backtrace_symbols_fd(void *const *addresses, size_t count, int descriptor) {
  (void)addresses;
  (void)count;
  (void)descriptor;
  return 0;
}

// Native titles cannot chdir, but cts-runner names its case-list files relative
// to its data directory. Resolve relative fopen paths against /app0 as if the
// title had started there.
FILE *__real_fopen(const char *path, const char *mode);

FILE *__wrap_fopen(const char *path, const char *mode) {
  char resolved[1024];
  if (path != NULL && path[0] != '\0' && path[0] != '/') {
    int length = snprintf(resolved, sizeof(resolved), "/app0/%s", path);
    if (length > 0 && (size_t)length < sizeof(resolved))
      return __real_fopen(resolved, mode);
  }
  return __real_fopen(path, mode);
}
