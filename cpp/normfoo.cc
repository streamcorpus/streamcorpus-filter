// g++ -Wall -DHAVE_ICU=1 -g normalize.cc normfoo.cc -o normfoo -licuuc
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

    std::getline(cin, utf8line);
    while (!utf8line.empty()) {
	string utf8out;

	lesserUTF8String(utf8line, &utf8out, NULL);

	cout << utf8out << endl;
	if (cin.eof()) break;
	std::getline(cin, utf8line);
    }
    return 0;
}
