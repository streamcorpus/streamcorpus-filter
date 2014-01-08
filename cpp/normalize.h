#ifndef NORMALIZE_H
#define NORMALIZE_H

#if HAVE_ICU

#include <vector>
#include <unicode/unistr.h>

int lesserUnicodeString(const icu::UnicodeString& in, icu::UnicodeString* out, std::vector<size_t>* offsets);
int lesserUTF8String(const std::string& in, std::string* out, std::vector<size_t>* offsets);

#endif /* HAVE_ICU */

// normalize.cc
size_t normalize(const char* orig, size_t len, size_t* offsets, char* out);


#endif /* NORMALIZE_H */
