#include <qtutil/KeywordTranslator.h>

#include <boost/test/unit_test.hpp>

#define TO_STRING(x) #x

using boost::unit_test_framework::test_suite;

static const char* TS_CONTEXT = "MyClass";
static const char* TS_SOURCE = "@company@, @product@";

static const char* FALLBACK_TS_CONTEXT = "MyFallbackClass";
static const char* FALLBACK_TS_SOURCE = "Some text";
static const char* FALLBACK_TS_FR_TRANSLATION = "Du texte";
void testTranslate() {
	bool ok;

	KeywordTranslator translator(0);
	ok = translator.load("test.qm");
	BOOST_CHECK(ok);

	KeywordTranslator::KeywordHash hash;
	hash["company"] = "ACME inc";
	hash["product"] = "TNT";
	translator.setKeywordHash(hash);

	QString result;

	// Test keywords on a translated string
	result = translator.translate(TS_CONTEXT, TS_SOURCE);
	BOOST_CHECK_EQUAL(result.toStdString(), "ACME inc, TNT");

	// Test keywords on a non translated string
	result = translator.translate(TS_CONTEXT, "Untranslated string with @product@ keyword");
	BOOST_CHECK_EQUAL(result.toStdString(), "Untranslated string with TNT keyword");
}


void testFallback() {
	bool ok;

	KeywordTranslator translator(0);

	QTranslator fallbackTranslator(0);
	ok = fallbackTranslator.load("fallback.qm");
	BOOST_CHECK(ok);

	QString result;

	// First, make sure the fallback translator knows about our translation
	result = fallbackTranslator.translate(FALLBACK_TS_CONTEXT, FALLBACK_TS_SOURCE);
	BOOST_CHECK_EQUAL(result.toStdString(), FALLBACK_TS_FR_TRANSLATION);

	// Then, check that without fallback we get the source string
	result = translator.translate(FALLBACK_TS_CONTEXT, FALLBACK_TS_SOURCE);
	BOOST_CHECK_EQUAL(result.toStdString(), FALLBACK_TS_SOURCE);

	// Install fallback and check we get the translated string
	translator.setFallbackTranslator(&fallbackTranslator);
	result = translator.translate(FALLBACK_TS_CONTEXT, FALLBACK_TS_SOURCE);
	BOOST_CHECK_EQUAL(result.toStdString(), FALLBACK_TS_FR_TRANSLATION);
}


class KeywordTranslatorTestSuite : public test_suite {
public:
	KeywordTranslatorTestSuite() : test_suite("KeywordTranslatorTestSuite") {
		add(BOOST_TEST_CASE(&testTranslate));
		add(BOOST_TEST_CASE(&testFallback));
	}
};
