#include "price.h"
#include <pcre.h>
#include <vector>
#include <iostream>
#include <cstdio>
#include <cstring>
#define OVECCOUNT 30

std::vector<Price> Price::PricesInText(const std::string& text) {
  std::vector<Price> prices;
  const char *error;
  int errOffset;

  const char *pattern = "(?:(?<prefix>(?<code>USD|EUR|GBP)|(?<symbol>\\$|€|£)) *)?(?<value>\\d+(?:[., ]+\\d+)*)(?: *(?(prefix)(?(code)(?<symbol_suffix>(?P>symbol))?|(?<code_suffix>(?P>code)?))|(?:(?<code_suffixm>(?P>code))|(?<symbol_suffixm>(?P>symbol)))))";

  pcre *re = pcre_compile(pattern, 0, &error, &errOffset, NULL);
  if (!re) {
    std::cerr << "PCRE compilation failed at offset "<< errOffset
              <<": " << error << std::endl;
    exit(1);
  }

  int ovector[OVECCOUNT];
  int rc;
  bool first = true;
  ovector[1] = 0;

  int nameCount;
  int nameEntrySize;
  unsigned char *nameTable;

  pcre_fullinfo(re, NULL, PCRE_INFO_NAMECOUNT, &nameCount);

  if (nameCount > 0) {
    unsigned char *tabptr;
    pcre_fullinfo(re, NULL, PCRE_INFO_NAMETABLE, &nameTable);
    pcre_fullinfo(re, NULL, PCRE_INFO_NAMEENTRYSIZE, &nameEntrySize);
  }

  for (;;) {
    int options = 0;

    if (!first && ovector[0] == ovector[1]) {
      if (ovector[0] == text.length()) break;
      options = PCRE_NOTEMPTY_ATSTART | PCRE_ANCHORED;
    }
    if (first) first = false;

    rc = pcre_exec(re, NULL, text.c_str(), text.length(),
                   ovector[1], options, ovector, OVECCOUNT);

    if (rc == PCRE_ERROR_NOMATCH && options == 0) {
      break;
    } else if (rc < 0) {
      pcre_free(re);
      return prices;
    } else if (rc == 0) {
      rc = OVECCOUNT/3;
      std::cerr << "WARNING: ovector only has room for "
                << rc - 1 << " captured substrings" << std::endl;
    }

    unsigned char *tabptr = nameTable;
    char* symbol = NULL;
    char* code = NULL;
    char* value = NULL;
    for (int i = 0; i < nameCount; i++) {
      int n = (tabptr[0] << 8) | tabptr[1];

      const char* name = (char*)(tabptr + 2);
      int size = ovector[2*n+1] - ovector[2*n];
      const char* valuePointer = text.c_str() + ovector[2*n];

      if (size > 0) {
        if (!strncmp(name, "symbol", 6)) {
          if (symbol) free(symbol);
          symbol = (char*)calloc(size+1, sizeof(char));
          strncpy(symbol, valuePointer, size);
        } else if (!strncmp(name, "code", 4)) {
          if (code) free(code);
          code = (char*)calloc(size+1, sizeof(char));
          strncpy(code, valuePointer, size);
        } else if (!strcmp(name, "value")) {
          if (value) free(value);
          value = (char*)calloc(size+1, sizeof(char));
          strncpy(value, valuePointer, size);
        }
      }
      tabptr += nameEntrySize;
    }
    prices.push_back(PriceFrom(symbol, code, value));
    if (symbol) free(symbol);
    if (code) free(code);
    if (value) free(value);
  }
  pcre_free(re);
  return prices;
}

Price Price::PriceFrom(char *symbol, char *code, char *value) {
  Price p;
  if (code) {
    p._currencyCode = std::string(code);
  } else {
    if (strcmp(symbol, "€") == 0) p._currencyCode = "EUR";
    else if (strcmp(symbol, "$") == 0) p._currencyCode = "USD";
    else if (strcmp(symbol, "£") == 0) p._currencyCode = "GBP";
  }

  int decimalOffset = -1; // no decimal part
  bool threeDecimals = false;
  bool otherSeparator = false;
  char decimalDelimiter = '\0';
  int delimiters = 0;
  for (int i = strlen(value) - 1; i >= 0; --i) {
    int pos = (strlen(value) - 1) - i;
    if (value[i] < '0' || value[i] > '9') {
      if (value[i] != ' ' && !decimalDelimiter) {
        decimalDelimiter = value[i];
        if (pos == 3) threeDecimals = true;
        decimalOffset = i;
      } else if (value[i] != decimalDelimiter) {
        otherSeparator = true;
        delimiters++;
      } else if (value[i] == decimalDelimiter) {
        // decimal delimiter never appears twice
        decimalDelimiter = '\0';
        decimalOffset = -1;
        delimiters++;
      }
    }
  }
  if (!otherSeparator && threeDecimals && value[0] != '0') {
    decimalDelimiter = '\0';
    decimalOffset = -1;
    delimiters++;
  }

  char* normalized = (char*)calloc(strlen(value)+1-delimiters, sizeof(char));
  char* normalizedPointer = normalized;
  for (int i = 0; i < strlen(value); ++i) {
    if (value[i] >= '0' && value[i] <= '9') {
      *normalizedPointer = value[i];
      normalizedPointer++;
    } else if (i == decimalOffset) {
      *normalizedPointer = '.';
      normalizedPointer++;
    }
  }
  p._amount = std::string(normalized);
  free(normalized);
  return p;
}
