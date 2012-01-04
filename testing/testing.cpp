#define BOOST_TEST_MODULE spirit2json test
#include <spirit2json.h>
#include <boost/test/included/unit_test.hpp>
#include <string>

using namespace std;
using namespace spirit2json;
using boost::get;

BOOST_AUTO_TEST_CASE(check_value_types) {
  BOOST_CHECK_EQUAL(JSON_STRING, JSONValue(JSONString()).which());
  BOOST_CHECK_EQUAL(JSON_LONG,  JSONValue(JSONLong(0)).which() );
  BOOST_CHECK_EQUAL(JSON_DOUBLE, JSONValue(JSONDouble(0)).which());
  BOOST_CHECK_EQUAL(JSON_BOOL,   JSONValue(JSONBool(false)).which());
  BOOST_CHECK_EQUAL(JSON_NULL,   JSONValue(JSONNull()).which());
  BOOST_CHECK_EQUAL(JSON_OBJECT, JSONValue(JSONObject()).which());
};
/*
	Tests for Number
*/
BOOST_AUTO_TEST_CASE(number_basic_usage) {

	BOOST_CHECK_EQUAL(get<double>(JSONValue(parse("42.0"))), 42.0);
	BOOST_CHECK_EQUAL(get<long>(JSONValue(parse("-5"))), -5);
	BOOST_CHECK_EQUAL(get<double>(JSONValue(parse("42."))), 42.);
	BOOST_CHECK_EQUAL(get<long>(JSONValue(parse("42"))), 42);
	BOOST_CHECK_EQUAL(get<double>(JSONValue(parse("1234567890.09876"))), 1234567890.09876);
	BOOST_CHECK_EQUAL(get<double>(JSONValue(parse("-1234567890.12345"))), -1234567890.12345);
	BOOST_CHECK_EQUAL(get<double>(JSONValue(parse("42e1"))), 420);
	BOOST_CHECK_EQUAL(get<double>(JSONValue(parse("4.2e1"))), 42.0);

	BOOST_CHECK_THROW(parse("123.123.123"), ParsingFailed);
	BOOST_CHECK_THROW(get<long>(JSONValue(parse("42."))), boost::bad_get);
	BOOST_CHECK_THROW(get<double>(JSONValue(parse("42e"))), ParsingFailed);
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
	BOOST_CHECK(JSONValue(parse("null")).which() == 4);

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
		o.insert(JSONPair("NULL", nullptr));
		BOOST_CHECK(JSONValue(o) == parse("{\"NULL\":null}"));
	}

	{
		JSONObject o;
		o.insert(JSONPair("other", JSONArray()));
		BOOST_CHECK(JSONValue(o) == parse("{\"other\":[]}"));
	}

	obj.insert(JSONPair("test", 0.5));
	BOOST_CHECK(JSONValue(obj) == parse("{\"test\":0.5}"));


	obj.insert(JSONPair("other", JSONArray()));
	BOOST_CHECK(JSONValue(obj) == parse("{\"test\":0.5,\"other\":[]}"));

	obj.insert(JSONPair("NULL", nullptr));
	BOOST_CHECK(JSONValue(obj) == parse(" { \"test\" :0.5,\n\t\t\"other\"  \t\n:[],  \"NULL\":null}"));

	//obj.insert(JSONObject::value_type("int", 3));
	//BOOST_CHECK(JSONValue(obj) == parse(" { \"test\" :0.5,\n\t\t\"other\"  \t\n:[],  \"NULL\":null, \"int\":3} "));

	BOOST_CHECK_THROW(parse("{:}"), ParsingFailed);
}

BOOST_AUTO_TEST_CASE(use_operators) {
	JSONObject obj;
	BOOST_CHECK(JSONValue(obj) == parse("{}"));

	obj["test"] = 5L;
	BOOST_CHECK(JSONValue(obj) == parse("{\"test\":5}"));

	obj["arr"] = JSONArray();
	BOOST_CHECK(JSONValue(obj) == parse("{\"test\":5, \"arr\" : []}"));
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

BOOST_AUTO_TEST_CASE(array_access) {
	JSONArray a = get<JSONArray>(parse("[ \"a\", 1, 2 ]"));
	BOOST_CHECK_EQUAL(a.size(),3);
	BOOST_CHECK_EQUAL(get<long>(a[1]),1);

	a.push_back((long)3);
	BOOST_CHECK_EQUAL(a.size(),4);
	BOOST_CHECK(a == get<JSONArray>(parse("[ \"a\", 1, 2, 3 ]")));

	a.push_back(JSONObject());
	BOOST_CHECK(a == get<JSONArray>(parse("[ \"a\", 1, 2, 3, {} ]")));

	a[2] = 4L;
	BOOST_CHECK(a == get<JSONArray>(parse("[ \"a\", 1, 4, 3, {} ]")));

	a[2] = std::string("aaa");
	BOOST_CHECK(JSONValue(a) == (parse("[ \"a\", 1, \"aaa\", 3, {} ]")));

	get<JSONObject>(a[4])["test"] = .3;
	BOOST_CHECK(JSONValue(a) == (parse("[ \"a\", 1, \"aaa\", 3, { \"test\" : 0.3 } ]")));
}


