#ifndef JSONPARSER_H
#define JSONPARSER_H
#include <string>

//#include "ParserState.h"
namespace Dumais
{
    namespace JSON
    {
        enum ValueSubType
        {
            Illegal,
            Double,
            Int,
            UInt,
            String,
            Bool
        };

    	enum JSONType
	    {
    	    JSON_Invalid,
    	    JSON_Object,
    	    JSON_List,
    	    JSON_Value
    	};

    	struct JSONProperty
    	{
    	    std::string value;
            ValueSubType subType;
    	    JSONType type;
    	    size_t size;
    	};
    
    	struct Pair
    	{
	        std::string key;
    	    JSONProperty val;
    	    size_t size;
    	};


	    //struct Value

    	class Parser{
    	private:
    	    std::string parseString(std::string str);
	        std::string parseNakedValue(std::string str);
    	    ValueSubType validateNakedValue(std::string str);
    	public:
    		Parser();
    		~Parser();
    
    	    std::string removeSpaces(std::string);
	        Pair getPair(std::string);
    	    JSONProperty getValue(std::string);
    
    	    size_t findClosingCurly(std::string str);
    	    size_t findClosingBracket(std::string str);
	        size_t findClosingQuote(std::string str);
    
    	};
    }
}
#endif

