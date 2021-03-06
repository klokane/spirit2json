#ifndef SPIRIT2JSON_H
#define SPIRIT2JSON_H

#include <string>
#include <stdexcept>
#include <ostream>
#include <vector>
#include <map>
#include <cstddef>
#include <boost/variant.hpp>
#include <boost/config.hpp>

struct boost::recursive_variant_ {};

namespace spirit2json {

enum JSONValueTypes
{
  JSON_STRING, //!< JSONValue(JSONString()).which() value
  JSON_LONG,   //!< JSONValue(JSONLong(0)).which() value
  JSON_DOUBLE, //!< JSONValue(JSONDouble(0)).which() value
  JSON_BOOL,   //!< JSONValue(JSONBool(false)).which() value
  JSON_NULL,   //!< JSONValue(JSONNull()).which() value
  JSON_ARRAY,  //!< JSONValue(JSONArray()).which() value
  JSON_OBJECT  //!< JSONValue(JSONObject()).which() value
};

#if defined(BOOST_NO_NULLPTR)
struct JSONNull {
	bool operator==(const JSONNull&) const {
		return true;
	}

	operator bool() const {
		return false;
	}
};

static JSONNull nullptr = JSONNull();

#else
typedef std::nullptr_t JSONNull;
#endif

typedef std::string JSONString;
typedef long JSONLong;
typedef double JSONDouble;
typedef bool JSONBool;

typedef boost::make_recursive_variant<
	JSONString,
	JSONLong,
	JSONDouble,
	JSONBool,
	JSONNull,
	std::vector<boost::recursive_variant_ >,
	std::map<std::string, boost::recursive_variant_ > >::type JSONValue;

typedef std::vector<JSONValue> JSONArray;
typedef std::map<std::string, JSONValue> JSONObject;
typedef JSONObject::value_type JSONPair;

class Exception : public std::exception {
	virtual const char* what() const throw() {
		return "spirit2json: Exception";
	}
};

class ParsingFailed : public Exception {
	virtual const char* what() const throw() {
		return "spirit2json: Failed to parse given json";
	}
};

JSONValue parse(const JSONString& str);
JSONString generate(const JSONValue& val);

}

std::ostream& operator<<(std::ostream& output, const spirit2json::JSONValue& val);
std::ostream& operator<<(std::ostream& output, const spirit2json::JSONArray& arr);
std::ostream& operator<<(std::ostream& output, const spirit2json::JSONObject& obj);

#endif
