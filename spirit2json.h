#ifndef SPIRIT2JSON_H
#define SPIRIT2JSON_H

#include "StdAfx.h"
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

typedef boost::make_recursive_variant<
	std::string,
	long,
	double,
	bool,
	JSONNull,
	std::vector<boost::recursive_variant_ >,
	std::map<std::string, boost::recursive_variant_ > >::type JSONValue;

typedef std::vector<JSONValue> JSONArray;
typedef std::map<std::string, JSONValue> JSONObject;

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

JSONValue parse(std::string str);

void get_stats(unsigned int &accumulated, 
			unsigned int &strings,
			unsigned int &objects,
			unsigned int &arrays,
			unsigned int &bools,
			unsigned int &nulls,
			unsigned int &doubles,
			unsigned int &longs,
			JSONValue &val);


}

std::ostream& operator<<(std::ostream& output, spirit2json::JSONValue& val);
std::ostream& operator<<(std::ostream& output, spirit2json::JSONArray& arr);
std::ostream& operator<<(std::ostream& output, spirit2json::JSONObject& obj);

#endif
