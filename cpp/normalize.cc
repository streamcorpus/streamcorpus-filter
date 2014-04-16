
#include <assert.h>
#include <ctype.h>
#include <stddef.h>

#include "normalize.h"

// icu
//#include <normalizer2.h>


#if HAVE_ICU
#include <unicode/normalizer2.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>
#include <vector>
#include <iostream>

using std::vector;
using std::cerr;
using std::endl;

using icu::Normalizer2;
using icu::UnicodeString;

static const Normalizer2* norm = NULL;

// strip accents, lowercase
// offsets could be weird, e.g. there's a 'fi' glyph that will expand into "fi"
int lesserUnicodeString(const icu::UnicodeString& in, icu::UnicodeString* out, vector<size_t>* offsets, bool squashSpace ) {
    if (norm == NULL) {
        UErrorCode uerr = U_ZERO_ERROR;
	norm = icu::Normalizer2::getInstance(NULL, "nfkc", UNORM2_DECOMPOSE, uerr);
        if ((norm == NULL) || U_FAILURE(uerr)) {
            cerr << u_errorName(uerr) << endl;
            return -1;
        }
    }
    bool wasSpace = true;
    int32_t len = in.countChar32();
    for (int32_t i = 0; i < len; i++) {
	UChar32 c = in.char32At(i);
	if (norm->isInert(c)) {
	    if (u_isspace(c)) {
		if (wasSpace) {
		    // drop redundant space
		    continue;
		}
		wasSpace = true;
		// fall through to outputting the first space normally
		c = ' ';
	    } else {
		wasSpace = false;
	    }

	    if (out != NULL) {
		(*out) += u_tolower(c);
	    }
	    if (offsets != NULL) {
		offsets->push_back(i);
	    }
	} else {
	    UnicodeString texpand;
	    norm->getDecomposition(c, texpand);
	    int32_t exlen = texpand.countChar32();
	    for (int32_t xi = 0; xi < exlen; xi++) {
		UChar32 xc = texpand.char32At(xi);
		if (u_getCombiningClass(xc) == 0) {
		    if (u_isspace(xc)) {
			if (wasSpace) {
			    // drop redundant space
			    continue;
			}
			wasSpace = true;
			// fall through to outputting the first space normally
			xc = ' ';
		    } else {
			wasSpace = false;
		    }

		    if (out != NULL) {
			(*out) += u_tolower(xc);
		    }
		    if (offsets != NULL) {
			offsets->push_back(i);
		    }
		}
	    }
	}
    }
    return 0;
}


int lesserUTF8String(const std::string& in, std::string* out, vector<size_t>* offsets) {
    // We reach into the utf8 decoding manually here so that we can
    // track byte offsets.
    int errout = 0;
    UnicodeString raw;
    vector<int32_t> rawOffsets; // maps utf32 pos to utf8 byte offset
    {
	int32_t pos = 0;
	int32_t length = in.size();
	const uint8_t* utf8 = (const uint8_t*)in.data();
	UChar32 c;
	while (pos < length) {
	    uint32_t prevpos = pos;
	    U8_NEXT(utf8, pos, length, c);
	    if (((int32_t)c) < 0) {
		// report that there was an error, but try to process as much data as possible
		errout = -1;
		break;
	    }
	    if (offsets != NULL) {
		rawOffsets.push_back(prevpos);
	    }
	    raw += c;
	}
    }
    vector<size_t> uuoffsets; // maps intermediate utf32 to source utf32
    UnicodeString cooked;
    int decomposeErr = lesserUnicodeString(raw, &cooked, (offsets != NULL) ? &uuoffsets : NULL);
    if (decomposeErr != 0) { errout = decomposeErr; }

    // convert back to UTF8, write out final offset mapping
    {
	uint8_t u8buf[5] = {'\0','\0','\0','\0','\0'};
	int32_t pos;
	int32_t capacity = 5;
	UChar32 c;
	UBool err;
	int32_t cookedlen = cooked.countChar32();
	for (int32_t i = 0; i < cookedlen; i++) {
	    c = cooked.char32At(i);
	    pos = 0;
	    U8_APPEND(u8buf, pos, capacity, c, err);
	    u8buf[pos] = '\0';
	    (*out) += (char*)u8buf;
	    if (offsets != NULL) {
		// map offset all the way back through source
		size_t offset = rawOffsets[uuoffsets[i]];
		for (int32_t oi = 0; oi < pos; oi++) {
		    // every output byte gets a source offset
		    offsets->push_back(offset);
		}
	    }
	}
    }
    
    return errout;
}


#endif /* HAVE_ICU */



enum State {
    initial = 0,
    normal,
    space,
    lastState
};


// orig: original text
// len: length of that text
// offsets: size_t[len] for offsets output
// out: char[len] for normalized text output
size_t normalize(const char* orig, size_t len, size_t* offsets, char* out) {
    size_t outi = 0;
    State state = initial;
    for (size_t i = 0; i < len; i++) {
	char c = orig[i];
	if (isspace(c) || (c == '.')) {
	    switch (state) {
	    case initial:
		// drop leading space, remain in initial state
		break;
	    case normal:
		// transition into a run of space, emit one space
		state = space;
		out[outi] = ' ';
		offsets[outi] = i;
		outi++;
		break;
	    case space:
		// drop redundant space
		break;
	    case lastState:
	    default:
		assert(0); // never get here
	    }
	} else {
	    state = normal;
	    out[outi] = tolower(orig[i]);
	    offsets[outi] = i;
	    outi++;
	}
    }
    return outi;
}

int normalize(const std::string& orig, std::string* out, std::vector<size_t>* offsets) {
#if HAVE_ICU
    return lesserUTF8String(orig, out, offsets);
#else
    // fall back to ye olde C and ASCII way
    size_t origlen = orig.size();
    char* tmpout = new char[origlen];
    size_t* offsetA = new size_t[origlen];
    size_t outlen = normalize(orig.data(), origlen, offsetA, tmpout);
    (*out) = std::string(tmpout, outlen);
    if (offsets != NULL) {
	for (int i = 0; i < outlen; i++) {
	    offsets->push_back(offsetA[i]);
	}
    }
    delete [] tmpout;
    delete [] offsetA;
    return 0;
#endif
}
