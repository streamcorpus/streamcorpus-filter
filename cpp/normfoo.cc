#include <assert.h>
#include <iostream>

// icu
#include <unicode/normalizer2.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>

using icu::Normalizer2;
using icu::UnicodeString;
//using icu::UErrorCode;

using std::cin;
using std::cout;
using std::cerr;
using std::string;
using std::endl;

#define HAVE_ICU 1
#include "normalize.h"

int main(int argc, const char** argv) {

    string utf8line;
    UErrorCode uerr = U_ZERO_ERROR;
    //Normalizer2* norm = icu::Normalizer2::getNFKCCasefoldInstance(uerr);
    const Normalizer2* norm = icu::Normalizer2::getInstance(NULL, "nfkc", UNORM2_DECOMPOSE, uerr);
    if ((norm == NULL) || U_FAILURE(uerr)) {
	cerr << u_errorName(uerr) << endl;
	return 1;
    }

    cin >> utf8line;
    while (!utf8line.empty()) {
#if 1
	string utf8out;
	lesserUTF8String(utf8line, &utf8out, NULL);
#else
	UnicodeString line = UnicodeString::fromUTF8(utf8line);
	UnicodeString decomped;
	
	int32_t len = line.countChar32();
	for (int32_t i = 0; i < len; i++) {
	    UChar32 c = line.char32At(i);
	    if (norm->isInert(c)) {
		decomped += u_tolower(c);
	    } else {
		UnicodeString texpand;
		norm->getDecomposition(c, texpand);
		int32_t exlen = texpand.countChar32();
		for (int32_t xi = 0; xi < exlen; xi++) {
		    UChar32 xc = texpand.char32At(xi);
		    if (u_getCombiningClass(xc) == 0) {
			decomped += u_tolower(xc);
		    }
		}
	    }
	}
	string utf8out;
	decomped.toUTF8String(utf8out);
#endif
	cout << utf8out << endl;
	if (cin.eof()) break;
	cin >> utf8line;
    }
    return 0;
}
