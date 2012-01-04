#include <iostream>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/include/qi_numeric.hpp>

#include "spirit2json.h"

namespace spirit2json {

/**
 * Printer class used in implementation of << operator overloads
 */
class printer : public boost::static_visitor<> {
	const unsigned int level;
	std::ostream& out;

public:
	printer(std::ostream& out, unsigned int level = 0) : level(level), out(out) {}

	std::string indent(unsigned int l) const {
		return std::string(l * 4, ' ');
	}

	void operator()(JSONNull&) const {
		out << "null";
	}

	void operator()(JSONArray &arr) const {
		out << "[" << std::endl;
		for (JSONArray::iterator it = arr.begin(); it != arr.end(); ++it) {
			if (it != arr.begin())
				out << "," << std::endl;

			out << indent(level + 1);
			boost::apply_visitor ( printer(out, level + 1), *it);
		}
		out << std::endl << indent(level) << "]";

	}

	void operator()(JSONObject &obj) const {
		out << "{" << std::endl;
		for (JSONObject::iterator it = obj.begin(); it != obj.end(); ++it) {
			if (it != obj.begin())
				out << ',' << std::endl;

			out << indent(level + 1) << "\"" << it->first << "\" : ";
			boost::apply_visitor( printer(out, level + 1), it->second);
		}
		out << std::endl << indent(level) << "}";

	}

	void operator() (bool &b) const {
		out << (b ? "true" : "false");
	}

	void operator() (std::string &str) const {
		out << "\"" << str << "\"";
	}

	template <typename T>
	void operator() (T &t) const {
		out << t;
	}

};


using namespace boost::spirit;
using namespace boost;

template <typename Iterator>
struct json_grammar : qi::grammar<Iterator, JSONValue(), qi::space_type> {
	json_grammar() : json_grammar::base_type(val) {
		using qi::lit;
		using qi::lexeme;
		using qi::_val;
		using qi::uint_parser;
		using qi::real_parser;
		using qi::strict_real_policies;
		using standard_wide::char_;
		using namespace qi::labels;

		str %= lexeme['"' > 
			*(
				("\\u" > uint_parser<unsigned, 16, 4, 4>() ) |
				('\\' > escaped_char) |
				(char_ - '"' - char_(L'\x0000', L'\x001F'))
			 )
			> '"'];

		number %= real_parser<double,strict_real_policies<double> >() |
		       long_;	

		val %= str | bool_ | number | null | arr | obj;
		arr %= '[' > -(val % ',') > ']';
		obj %= '{' > -(pair % ',') > '}';
		null = lit("null")	[_val = nullptr];
		pair %= str > ':' > val;

		escaped_char.add("\"", '"')
				("\\", '\\')
				("/", '/')
				("b", '\b')
				("f", '\f')
				("n", '\n')
				("r", '\r')
				("t", '\t');
		//escaped_char.name("Escape character");

		val.name("Value");
		arr.name("Array");
		obj.name("Object");
		str.name("String");
		null.name("null");
		pair.name("Pair");

		qi::on_error<qi::fail>(
			val,
			std::cerr
			    << phoenix::val("Error! Expecting: ")
			    << _4
			    << phoenix::val(" instead found: \"")
				<< phoenix::construct<std::string>(_3, _2)
				<< phoenix::val("\"")
				<< std::endl
		);
	}
	
	qi::rule<Iterator, JSONValue(), qi::space_type> val;
	qi::rule<Iterator, JSONArray(), qi::space_type> arr;
	qi::rule<Iterator, JSONObject(), qi::space_type> obj;

	qi::rule<Iterator, std::string(), qi::space_type> str;
	qi::rule<Iterator, JSONValue(), qi::space_type> number;
	qi::symbols<char const, char const> escaped_char;

	qi::rule<Iterator, JSONNull(), qi::space_type> null;

	qi::rule<Iterator, std::pair<std::string, JSONValue>(), qi::space_type> pair;
};

JSONValue parse(std::string str) {
	JSONValue result;
	std::string::const_iterator iter = str.begin();
	std::string::const_iterator end = str.end();

	bool r = qi::phrase_parse(iter, end, json_grammar<std::string::const_iterator>(), qi::space, result);
	//TODO: Implement this right
	if (!r || iter != str.end()) {
		throw ParsingFailed();
	}

	return result;
}

}

std::ostream& operator<<(std::ostream& output, spirit2json::JSONValue& val) {
	boost::apply_visitor(spirit2json::printer(output), val);
	return output;
}

std::ostream& operator<<(std::ostream& output, spirit2json::JSONArray& arr) {
	spirit2json::JSONValue val = spirit2json::JSONValue(arr);
	boost::apply_visitor(spirit2json::printer(output), val);
	return output;
}

std::ostream& operator<<(std::ostream& output, spirit2json::JSONObject& map) {
	spirit2json::JSONValue val = spirit2json::JSONValue(map);
	boost::apply_visitor(spirit2json::printer(output), val);
	return output;
}
