#include "StdAfx.h"
#define BOOST_TEST_MODULE spirit2json test
#include <spirit2json.h>
#include <boost/test/included/unit_test.hpp>
#include <string>

using namespace std;
using namespace spirit2json;
using boost::get;
/*
	Tests for Number
*/
BOOST_AUTO_TEST_CASE(number_basic_usage) {
	BOOST_CHECK_EQUAL(get<double>(JSONValue(parse("42.0"))), 42.0);
	BOOST_CHECK_EQUAL(get<double>(JSONValue(parse("-5"))), -5.);
	BOOST_CHECK_EQUAL(get<double>(JSONValue(parse("42"))), 42.);
	BOOST_CHECK_EQUAL(get<double>(JSONValue(parse("42"))), 42);
	BOOST_CHECK_EQUAL(get<double>(JSONValue(parse("1234567890.09876"))), 1234567890.09876);
	BOOST_CHECK_EQUAL(get<double>(JSONValue(parse("-1234567890.12345"))), -1234567890.12345);

	BOOST_CHECK_THROW(parse("123.123.123"), ParsingFailed);
}


BOOST_AUTO_TEST_CASE(number_notations) {
	//TODO: Test whether Numbers can be entered in all notations specified by the spec
	//BOOST_FAIL("Not implemented");
}

BOOST_AUTO_TEST_CASE(number_precision) {
	//TODO: Make sure we meet the precision requirements outlined in the JSON spec
	//BOOST_FAIL("Not implemented");
}

/*
	Tests for String
*/
BOOST_AUTO_TEST_CASE(string_basic_usage) {
	BOOST_CHECK_THROW(parse("aa"), ParsingFailed); // not quoting
	BOOST_CHECK(get<string>(JSONValue(parse("\"aa\""))) == "aa");
	BOOST_CHECK(get<string>(JSONValue(parse("\"a a\""))) == "a a");
	BOOST_CHECK(get<string>(JSONValue(parse("  \"a a\"  "))) == "a a");
}


BOOST_AUTO_TEST_CASE(string_escape_characters) {
	// Normal escapes
	BOOST_CHECK(get<string>(JSONValue(parse("\"\\t \\r \\n \\f \\b \\/ \\\\ \\\"\"")))
			== "\t \r \n \f \b / \\ \"");
	BOOST_CHECK(get<string>(JSONValue(parse("\"testing\\tescapes\\\"in\\rtext \"")))
			== "testing\tescapes\"in\rtext ");

	// Make sure we do not ignore unknown escapes
	BOOST_CHECK_THROW(parse("\"\\x\""), ParsingFailed);
	BOOST_CHECK_THROW(parse("\"\\U\""), ParsingFailed);

	// Make sure we only accept 4 character hex numbers
	BOOST_CHECK(get<string>(JSONValue(parse("\"\\u002C\\u0060\""))) == "\x2C\x60");
	BOOST_CHECK(get<string>(JSONValue(parse("\"\\u0060A\""))) == "\x60""A");
	//BOOST_CHECK(get<string>(JSONValue(parse("\"\\uABBA\""))) == L"\xabba");
	BOOST_CHECK_THROW(parse("\"\\uAB\""), ParsingFailed);
	BOOST_CHECK_THROW(parse("\"\\u\""), ParsingFailed);
}

BOOST_AUTO_TEST_CASE(string_unicode) {

	// Make sure we do not accept any characters between U+0000 thru U+001F
	BOOST_CHECK_THROW(parse("\"\x0000\""), ParsingFailed);
	BOOST_CHECK_THROW(parse("\"\x000F\""), ParsingFailed);
	BOOST_CHECK_THROW(parse("\"\x001F\""), ParsingFailed);
}


/*
	Tests for Bool
*/

BOOST_AUTO_TEST_CASE(boolean_basic_usage) {
	BOOST_CHECK_EQUAL(get<bool>(JSONValue(parse("true"))), true);
	BOOST_CHECK_EQUAL(get<bool>(JSONValue(parse("false"))), false);

	// Make sure we parse case sensitive
	BOOST_CHECK_THROW(parse("False"), ParsingFailed);
	BOOST_CHECK_THROW(parse("FALSE"), ParsingFailed);
	BOOST_CHECK_THROW(parse("True"), ParsingFailed);
	BOOST_CHECK_THROW(parse("TRUE"), ParsingFailed);
}



/*
	Tests for null
*/
BOOST_AUTO_TEST_CASE(null_basic_usage) {
#ifndef _WIN32 // Doesn't work with vs2010 sp1
	BOOST_CHECK(get<JSONNull>(JSONValue(parse("null"))) == nullptr);
#else
	JSONValue val(parse("null"));
	//TODO: Figure out how to do this in a nice way
	unsigned int accumulated = 0;
	unsigned int strings = 0;
	unsigned int objects = 0;
	unsigned int arrays = 0;
	unsigned int bools = 0;
	unsigned int nulls = 0;
	unsigned int doubles = 0;
	get_stats(accumulated, strings, objects, arrays, bools, nulls, doubles, val);
	BOOST_CHECK((nulls == 1) && (accumulated == 1));
	
#endif

	// Make sure we don't accept anything used in other languages
	BOOST_CHECK_THROW(parse("nil"), ParsingFailed);
	BOOST_CHECK_THROW(parse("None"), ParsingFailed);

	// Make sure we parse case sensitive
	BOOST_CHECK_THROW(parse("Null"), ParsingFailed);
	BOOST_CHECK_THROW(parse("NULL"), ParsingFailed);
}



/*
	Tests for Array
*/
BOOST_AUTO_TEST_CASE(array_basic_usage) {
	JSONArray arr;
	BOOST_CHECK(JSONValue(arr) == parse("[]"));

	arr.push_back(0.5);
	arr.push_back(nullptr);
	arr.push_back(string("testing"));
	arr.push_back(false);

	BOOST_CHECK(JSONValue(arr) == parse("[ 0.5, null,\"testing\",\n false] "));
	BOOST_CHECK(JSONValue(arr) == parse("[ 0.5, null,\t\"testing\",\n false] "));
}

/*
	Tests for Object
*/
BOOST_AUTO_TEST_CASE(object_basic_usage) {
	JSONObject obj;
	BOOST_CHECK(JSONValue(obj) == parse("{}"));

	{
		JSONObject o;
		o.insert(JSONObject::value_type("NULL", nullptr));
		BOOST_CHECK(JSONValue(o) == parse("{\"NULL\":null}"));
	}

	{
		JSONObject o;
		o.insert(JSONObject::value_type("other", JSONArray()));
		BOOST_CHECK(JSONValue(o) == parse("{\"other\":[]}"));
	}

	obj.insert(JSONObject::value_type("test", 0.5));
	BOOST_CHECK(JSONValue(obj) == parse("{\"test\":0.5}"));

	obj.insert(JSONObject::value_type("other", JSONArray()));
	BOOST_CHECK(JSONValue(obj) == parse("{\"test\":0.5,\"other\":[]}"));

	obj.insert(JSONObject::value_type("NULL", nullptr));
	BOOST_CHECK(JSONValue(obj) == parse(" { \"test\" :0.5,\n\t\t\"other\"  \t\n:[],  \"NULL\":null}"));

	BOOST_CHECK_THROW(parse("{:}"), ParsingFailed);
}

BOOST_AUTO_TEST_CASE(use_operators) {
	JSONObject obj;
	BOOST_CHECK(JSONValue(obj) == parse("{}"));

	obj["test"] = 0.5;
	BOOST_CHECK(JSONValue(obj) == parse("{\"test\":0.5}"));
	obj["arr"] = JSONArray();
	BOOST_CHECK(JSONValue(obj) == parse("{\"test\":0.5, \"arr\" : []}"));
}


/*
	Tests for miscellaneous stuff
*/
BOOST_AUTO_TEST_CASE(misc) {
	BOOST_CHECK_THROW(parse(""), ParsingFailed);
	BOOST_CHECK_THROW(parse("["), ParsingFailed);
	BOOST_CHECK_THROW(parse("]"), ParsingFailed);
	BOOST_CHECK_THROW(parse(","), ParsingFailed);
}

