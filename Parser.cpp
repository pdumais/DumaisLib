#include "Parser.h"

#include "Object.h"
#include "List.h"
#include "Value.h"
#include <algorithm>

using namespace Dumais::JSON;

//TODO: This could be considerably optimized. 

Parser::Parser(){
}

Parser::~Parser(){
}

std::string Parser::removeSpaces(std::string str)
{
    // According to the RFC, /r /n /t and spaces are allowed in the document.
    // We will remove them all to make the parsing easier. 
    std::string ret="";
    ret.reserve(str.size());

    size_t i=0;
    while (i<str.size())
    {
        char c = str[i];
        if (c=='\"')
        {
            // a Quote was found, so it is the start of a string. We don't wanna remove spaces located inside strings.
            // We find the end of the string by find the the closing quote char and we increase the buffer pointer.
            size_t l = findClosingQuote(str.substr(i));
            if (l==std::string::npos) return ""; // what the hell?
            // Copy the string over.
            while (l)
            {
                ret+=str[i];
                i++;
                l--;
            }
        } else if (c=='\t' || c==' ' || c=='\r' || c=='\n') {
            // A space was found, skip over it and don't copy over.
            i++;
        } else {
            // a non-space value was found, copy it over.
            i++;
            ret+=c;
        }
    }

    return ret;
}


// Function used to retrieve a key/value pair.
Pair Parser::getPair(std::string str)
{
    const char *buf = str.c_str();
    Pair p;
    p.size=0;
    
    // parse the string, This will be the key.
    std::string key = parseString(str);
    if (key=="")
    {
        p.key  = "";
        p.size=0;
        p.val.type = JSON_Invalid;
        return p;
    }

    p.key = key;
    p.size+=key.size()+2;
    buf += key.size()+2;

    // after a key should come a ":". If not, then we don't know how to handle this.
    if (*buf!=':')
    {
        p.val.type = JSON_Invalid;
        return p;
    }
    buf++;
    p.size++;

    // After the semicolumn, a value should be there. It could be either an object, a list, a string or a literal name or number.
    JSONProperty val = getValue(buf);

    p.size+=val.size;
    p.val = val;
    return p;
}

// This will return the whole content inside a string. Escaped chars will not be un-escaped
JSONProperty Parser::getValue(std::string str)
{
    
    JSONProperty val;
    val.size=0;
	if (str.size() < 1 )
   {
      val.type = JSON_Invalid;
      return val;
   }
   const char *buf = str.c_str();
    if (*buf=='{'){
        // The value is an object
        size_t i = findClosingCurly(str);
        if (i!=std::string::npos){
            val.value = str.substr(0,i);
            val.size +=i;
            val.type=JSON_Object;
        } else {
            val.type = JSON_Invalid;
        }
    } else if (*buf=='['){
        // The value is a list.
        size_t i = findClosingBracket(str);
        if (i!=std::string::npos){
            val.value = str.substr(0,i);
            val.size +=i;
            val.type=JSON_List;
        } else {
            val.type = JSON_Invalid;
        }
    } else if (*buf=='\"'){ 
        // The value is a string
        val.value=parseString(str);
        val.type=JSON_Value;
        val.size += val.value.size()+2;
        val.subType = String;
    } else if ((*buf >= 0x30 && *buf<=0x39) || (*buf>='a' && *buf<='z') || (*buf>='A' && *buf<='Z') || *buf=='.' || *buf=='-'){
        // The value is a literal name or a number
        std::string tmp = parseNakedValue(str);
        ValueSubType nrv = validateNakedValue(tmp);
        switch (nrv)
        {
            case String:
            case Double:
            case Int:
            case UInt:
            case Bool:
            {
                val.value+=tmp;
                val.size+=tmp.size();
                val.subType = nrv;
                val.type=JSON_Value;
            }
            break;
            case Illegal:
            {
                val.value="{illegal}";
                val.size+=tmp.size();
                val.subType = Illegal;
                val.type=JSON_Value;
            }
            break;
        }
    } else {
        val.type = JSON_Invalid;
    }

    return val;

}


std::string Parser::parseString(std::string str)
{
   if (str.size() < 1) return "";
   const char *buf = str.c_str();

    if (buf[0]!='\"') return "";

   size_t i = findClosingQuote(str);
   if (i < 3 || i == std::string::npos) return "";

   return str.substr(1,i-2);
}

// used to validate that the value is a valid true/false, or a number
ValueSubType Parser::validateNakedValue(std::string ret)
{
   if (ret.size() < 1) return Illegal;
    std::string tmp = ret;
    transform (tmp.begin (), tmp.end (), tmp.begin (), (int(*)(int)) tolower);


    if (tmp == "true" || tmp=="false")
    {
        // it is a boolean.
        return Bool;
    }

    bool dot =false;
    ValueSubType nrv = Int;
    for (size_t i=0;i<ret.size();i++)
    {
        if (ret[i]=='-'){
            // A minus sign can only appear once and only at the begining of the string.
            if (i!=0)
            {
                nrv = Illegal;
                break;
            } else if (nrv!=Double){ // Technically this can't happen since the minus is only allowed to be at the begining.
                nrv = UInt;
            }
        } else if (ret[i]=='.')
        {
            // Only one dot is allowed in the string
            if (dot)
            {
                // there was already a dot.
                nrv = Illegal;
                break;
                //bad = true;
            } else {
                dot = true;
                // if we found a dot, then this is a Double
                nrv = Double;
            }
        } else if (ret[i]<0x30 || ret[i]>0x39){
            // only digits are allowed. The RFC does not support HEX.
            // TODO: According to the RFC though, we should support "e" or "E" for exponents.
            nrv = Illegal;
            break;
        }

    }

    return nrv;
    
}

// retrive a literal value from the string.
std::string Parser::parseNakedValue(std::string str)
{
   if (str.size() < 1) return "";
    const char *buf=str.c_str();

    // According to the RFC, naked values (called Literal Names) could only be: null, true, false. 
    // We also parse numbers in here.

    std::string ret;
    size_t i=0;
    while ((*buf >= 0x30 && *buf<=0x39) || (*buf>='a' && *buf<='z') || (*buf>='A' && *buf<='Z') || *buf=='.' || *buf=='-')
    {
        ret += *buf;
        buf++;
        i++;
        if (i>=str.size()) break;
    }

    return ret;
}

// Find the closing curly brace but ignore the ones located in quoted strings
size_t Parser::findClosingCurly(std::string str)
{
   if (str.size() < 1) return std::string::npos;
    const char *buf=str.c_str();
    const char *b2 = buf;

    if (buf[0]!='{') return std::string::npos;

    int level =1;
    buf++;
    while (*buf!=0)
    {
        if (*buf=='\"')
        {
            size_t i = findClosingQuote(buf);
            if (i==std::string::npos) return std::string::npos;
            buf+=i;
            continue;
        } else if (*buf=='{'){
            level++;
        } else if (*buf=='}'){
            level--;
        }
        buf++;
        if (!level) return (unsigned long)buf - (unsigned long)b2;
    }

    return std::string::npos;


}

// Find the closing bracket but ignore the ones located in quoted strings
size_t Parser::findClosingBracket(std::string str)
{
   if (str.size() < 1) return std::string::npos;
    const char *buf=str.c_str();
    const char *b2 = buf;

    if (buf[0]!='[') return std::string::npos;

    int level =1;
    buf++;
    while (*buf!=0)
    {
        if (*buf=='\"')
        {
            size_t i = findClosingQuote(buf);
            if (i==std::string::npos) return std::string::npos;
            buf+=i;
            continue;
        } else if (*buf=='['){
            level++;
        } else if (*buf==']'){
            level--;
        }
        buf++;
        if (!level) return (unsigned long)buf - (unsigned long)b2;
    }

    return std::string::npos;
}


// Find the closing quote but ignore the ones located in quoted strings
size_t Parser::findClosingQuote(std::string str)
{
   if (str.size() < 1) return std::string::npos;
    const char *buf=str.c_str();
    const char *b2 = buf;
    
    if (buf[0]!='\"') return std::string::npos;

    buf++;
    bool nextEscape = false;
    while (*buf!=0)
    {
        if (nextEscape)
        {
            nextEscape = false;
        }
        else
        {
            if (*buf=='\\') // escape char, skip next one to make sure we don't find a quote
            {
                nextEscape = true;
            } 
            else if (*buf=='\"')
            {
                break;
            }
        }
        buf++;
    }

   return ((unsigned long)buf - (unsigned long)b2 + 1);

}

