#ifndef NORMALIZE_H
#define NORMALIZE_H

#if HAVE_ICU

#include <vector>
#include <unicode/unistr.h>

int lesserUnicodeString(const icu::UnicodeString& in, icu::UnicodeString* out, std::vector<size_t>* offsets, bool squashSpace=true);
int lesserUTF8String(const std::string& in, std::string* out, std::vector<size_t>* offsets);

#endif /* HAVE_ICU */

size_t normalize(const char* orig, size_t len, size_t* offsets, char* out);

int normalize(const std::string& orig, std::string* out, std::vector<size_t>* offsets);

#endif /* NORMALIZE_H */
