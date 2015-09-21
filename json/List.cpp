#include "List.h"

#include "Object.h"
#include "Value.h"
#include <sstream>

using namespace Dumais::JSON;
List::List(std::string json){
    parseList(json);
}

List::~List()
{
    for (std::vector<JSON*>::iterator it = mMembers.begin();it!=mMembers.end();it++)
    {
        delete *it;
    }
    mMembers.clear();
}

JSON* List::copy()
{
    List *list = new List("[]");
    for (std::vector<JSON*>::iterator it = mMembers.begin();it!=mMembers.end();it++)
    {
        JSON* p = (*it)->copy();
        list->mMembers.push_back(p);
    }

    return list;
}

JSON* List::getByIndex(size_t i)
{
    if (i>=0 && i<mMembers.size())
    {
        return mMembers[i];
    }
   return &mInvalid;
}

JSON* List::getByKey(std::string key)
{
    return &mInvalid;
}

std::string List::toString()
{
    return "{list}";
}

bool List::getIsValid()
{
   return true;
}

void List::parseList(std::string str)
{
   if (str.size() < 1) return;
   if (str[0] != '[') return;

    std::string tmp = str.substr(1);
    size_t i = 1;
    while (i<str.size())
    {
        JSONProperty p = mParser.getValue(tmp);
        if (p.type==JSON_Invalid) break;

        if (p.type==JSON_Object){
            mMembers.push_back(new Object(p.value));
        } else if (p.type==JSON_List){
            mMembers.push_back(new List(p.value));
        } else if (p.type==JSON_Value){
            mMembers.push_back(new Value(p.value, p.subType));
        }

        if (tmp[p.size]==']') break;
        if (tmp[p.size]!=',') return;
        i+=p.size+1; //skip comma
        tmp = str.substr(i);

    }

}


std::string List::stringify(int level)
{
    std::stringstream ss;

    ss <<"[";
    if (level!=-1) ss<<"\r\n";

    std::vector<JSON*>::iterator it=mMembers.begin();
    while (it!=mMembers.end())
    {
        for (int i=0;i<level;i++) ss<<"\t";
        int l2 = level;
        if (l2!=-1) l2++;
        ss << (*it)->stringify(l2);
        it++;
        if (it!=mMembers.end()) ss << ",";
        if (level!=-1) ss<<"\r\n";

    }

    for (int i=0;i<level-1;i++) ss<<"\t";
    ss <<"]";

    return ss.str();

}

void List::setStringValue(std::string value)
{
}

void List::assign(const JSON& json)
{

}

JSON& List::addObject(const std::string& name)
{
    // we can ignore the name
    Object *item = new Object("{}");
    mMembers.push_back(item);
    return *item;
}

JSON& List::addList(const std::string& name)
{
    // we can ignore the name
    List *item = new List("[]");
    mMembers.push_back(item);
    return *item;
}

JSON& List::addValue(const std::string& val,const std::string& name)
{
    // we can ignore the name
    Value *item = new Value(val,String);
    mMembers.push_back(item);
    return *item;
}

JSON& List::addValue(unsigned int val,const std::string& name)
{
    // we can ignore the name
    Value *item = new Value(val);
    mMembers.push_back(item);
    return *item;
}

JSON& List::addValue(int val,const std::string& name)
{
    // we can ignore the name
    Value *item = new Value(val);
    mMembers.push_back(item);
    return *item;
}
JSON& List::addValue(double val,const std::string& name)
{
    // we can ignore the name
    Value *item = new Value(val);
    mMembers.push_back(item);
    return *item;
}
JSON& List::addValue(bool val,const std::string& name)
{
    // we can ignore the name
    Value *item = new Value(val);
    mMembers.push_back(item);
    return *item;
}

void List::setBool(bool val)
{
}
void List::setUInt(unsigned int val)
{
}
void List::setInt(int val)
{
}
void List::setDouble(double val)
{
}

size_t List::getSize()
{
    return mMembers.size();
}

