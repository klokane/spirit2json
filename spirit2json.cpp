#include <iostream>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/include/qi_numeric.hpp>

#include "spirit2json.h"

namespace spirit2json {

using namespace boost::spirit;

template <typename Iterator>
struct string_escape : karma::grammar<Iterator, std::string()> {
  string_escape() : string_escape::base_type(esc_str) {
    esc_char.add
      ('\"',"\\\"")
      ('\\',"\\\\")
      ('\'',"\\\'")
      ('\t',"\\t")
      ('\r',"\\r")
      ('\n',"\\n")
      ('\f',"\\f")
      ('\b',"\\b")
      ('/',"\\/")
      ;
    esc_str = *(esc_char | karma::print);
  }
  karma::rule<Iterator, std::string()> esc_str;
  karma::symbols<char, const char*> esc_char;
};

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

	void operator()(const JSONNull&) const {
		out << "null";
	}

	void operator()(const JSONArray& arr) const {
		out << "[" << std::endl;
		for (JSONArray::const_iterator it = arr.begin(); it != arr.end(); ++it) {
			if (it != arr.begin())
				out << "," << std::endl;

			out << indent(level + 1);
			boost::apply_visitor ( printer(out, level + 1), *it);
		}
		out << std::endl << indent(level) << "]";

	}

	void operator()(const JSONObject& obj) const {
		out << "{" << std::endl;
		for (JSONObject::const_iterator it = obj.begin(); it != obj.end(); ++it) {
			if (it != obj.begin())
				out << ',' << std::endl;

			out << indent(level + 1) << "\"" << it->first << "\" : ";
			boost::apply_visitor( printer(out, level + 1), it->second);
		}
		out << std::endl << indent(level) << "}";

	}

	void operator() (const bool& b) const {
		out << (b ? "true" : "false");
	}

	void operator() (const JSONString& str) const {
    typedef std::back_insert_iterator<std::string> sink_type;
    std::string generated;
    sink_type sink(generated);
    string_escape<sink_type> g;

    karma::generate(sink, g, str);
    out << "\"" << generated << "\"";
	}

	template <typename T>
	void operator() (const T& t) const {
		out << t;
	}

};


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

	qi::rule<Iterator, JSONString(), qi::space_type> str;
	qi::rule<Iterator, JSONValue(), qi::space_type> number;
	qi::symbols<char const, char const> escaped_char;

	qi::rule<Iterator, JSONNull(), qi::space_type> null;

	qi::rule<Iterator, std::pair<std::string, JSONValue>(), qi::space_type> pair;
};

JSONValue parse(const JSONString& str) {
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

JSONString generate(const JSONValue& val) {
  std::stringstream ss;
	boost::apply_visitor(spirit2json::printer(ss), val);
  return ss.str();
}

} /* namespace spirit2json */

std::ostream& operator<<(std::ostream& output, const spirit2json::JSONValue& val) {
	boost::apply_visitor(spirit2json::printer(output), val);
	return output;
}

std::ostream& operator<<(std::ostream& output, const spirit2json::JSONArray& arr) {
	spirit2json::JSONValue val = spirit2json::JSONValue(arr);
	boost::apply_visitor(spirit2json::printer(output), val);
	return output;
}

std::ostream& operator<<(std::ostream& output, const spirit2json::JSONObject& map) {
	spirit2json::JSONValue val = spirit2json::JSONValue(map);
	boost::apply_visitor(spirit2json::printer(output), val);
	return output;
}
