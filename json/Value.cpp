#include "Value.h"
#include <sstream>
#include <stdlib.h>
#include <algorithm>

using namespace Dumais::JSON;

Value::Value()
{
   //WARNING: This sould only be called from the copy() method
   mSubType = String;
   mValue = "";
}
Value::Value(std::string value, ValueSubType subType){
    mSubType = subType;
    
    if (mSubType==Bool)
    {
       std::string tmp = value;
       transform (tmp.begin (), tmp.end (), tmp.begin (), (int(*)(int)) tolower);
       mValue = tmp;
    }
    else
    {
      mValue = unEscapeString(value);
    }
    
}

Value::Value(unsigned int value){
    std::stringstream ss;
    ss << value;
    mValue = ss.str();
    mSubType = UInt;
}

Value::Value(int value){
    std::stringstream ss;
    ss << value;
    mValue = ss.str();
    mSubType = UInt;
}

Value::Value(double value){
    std::stringstream ss;
    ss << value;
    mValue = ss.str();
    mSubType = Double;
}

Value::Value(bool value){
    mValue = value?"true":"false";
    mSubType = Bool;
}

bool Value::getIsValid()
{
   return true;
}

double Value::convertToDouble()
{
    return atof(mValue.c_str());
}

int Value::convertToInt()
{
    return atoi(mValue.c_str());
}

unsigned int Value::convertToUInt()
{
    return atoi(mValue.c_str());
}

bool Value::convertToBool()
{
    std::string tmp = mValue;
    transform (tmp.begin (), tmp.end (), tmp.begin (), (int(*)(int)) tolower);

    return tmp=="true";
}

Value::~Value(){
}

JSON* Value::copy()
{
   Value *pValue = new Value();

   pValue->mValue = toString();
   pValue->mSubType =  mSubType;
   return pValue;
}

JSON* Value::getByIndex(size_t i)
{
    return &mInvalid;
}

JSON* Value::getByKey(std::string key)
{
    return &mInvalid;
}

std::string Value::toString()
{
    
    return mValue;
}

std::string Value::stringify(int level)
{
    std::stringstream ss;

    switch (mSubType)
    {
        case Illegal:
        case String:
        {
            ss<< "\"" << escapeString(mValue) << "\"";
        }
        break;
        case UInt:
        case Int:
        case Double:
        case Bool:
        {
            ss << mValue;
        }
        break;
    }

    return ss.str();
}


void Value::setStringValue(std::string value)
{
    mValue = unEscapeString(value);
    mSubType = String;
}

void Value::assign(const JSON& json)
{
    
}

JSON& Value::addObject(const std::string& name)
{
    return mInvalid;
}

JSON& Value::addList(const std::string& name)
{
    return mInvalid;
}

JSON& Value::addValue(const std::string& val,const std::string& name)
{
    return mInvalid;
}

JSON& Value::addValue(int val,const std::string& name)
{
    return mInvalid;
}
JSON& Value::addValue(unsigned int val,const std::string& name)
{
    return mInvalid;
}
JSON& Value::addValue(double val,const std::string& name)
{
    return mInvalid;
}
JSON& Value::addValue(bool val,const std::string& name)
{
    return mInvalid;
}

void Value::setBool(bool val)
{
    mValue = val?"true":"false";
    mSubType = Bool;
}
void Value::setUInt(unsigned int val)
{
    std::stringstream ss;
    ss << val;
    mValue = ss.str();
    mSubType = UInt;
}
void Value::setInt(int val)
{
    std::stringstream ss;
    ss << val;
    mValue = ss.str();
    mSubType = Int;
}
void Value::setDouble(double val)
{
    std::stringstream ss;
    ss << val;
    mValue = ss.str();
    mSubType = Double;
}



std::string Value::escapeString(const std::string& st)
{
    const char *buf = st.c_str();
    std::string ret;
    for (size_t i=0;i<st.size();i++)
    {
        char c = buf[i];
        switch (c)
        {
            case '\"':
            {
                ret+="\\\"";
            }
            break;
            case '\n':
            {
                ret+="\\n";
            }
            break;
            case '\r':
            {
                ret+="\\r";
            }
            break;
            case '\t':
            {
                ret+="\\t";
            }
            break;
            case '\\':
            {
                ret+="\\\\";
            }
            break;
            default:
            {
                ret+=c;
            }
        }
    }
    return ret;
}

std::string Value::unEscapeString(const std::string& st)
{
   if (st.size() < 1) return "";
    const char *buf = st.c_str();
    std::string ret;
    for (size_t i=0;i<st.size();i++)
    {
        char c = buf[i];
        if (c=='\\' && i<(st.size()-1))
        {
         if ((i + 1) >= st.size()) return "";
            char c2 = buf[i+1];
            i++; // need to skip over next char for next iteration
            switch (c2)
            {
                case '\'':
                {
                    ret+="\'";
                }
                break;
                case '\"':
                {
                    ret+="\"";
                }
                break;
                case 'n':
                {
                    ret+="\n";
                }
                break;
                case 'r':
                {
                    ret+="\r";
                }
                break;
                case 't':
                {
                    ret+="\t";
                }
                break;
                case '\\':
                {
                    ret+="\\";
                }
                break;
                default:
                {
                    i--; // cancel skipping over next char
                    ret+=c;
                }
            }
        } else {
            ret += c;
        }
    }

    return ret;
}
