#include "Object.h"
#include <string.h>

#include "Value.h"
#include "List.h"
#include <sstream>

using namespace Dumais::JSON;

/*JSON* Object(std::string name)
{
    return new Object();
}*/


Object::Object(std::string str)
{
    parseObject(str);
}

Object::~Object()
{
    for (std::unordered_map<std::string,JSON*>::iterator it = mMembers.begin();it!=mMembers.end();it++)
    {
        delete it->second;
    }
    mMembers.clear();
}

JSON* Object::copy()
{

    Object* obj = new Object("{}");
    for (std::unordered_map<std::string,JSON*>::iterator it = mMembers.begin();it!=mMembers.end();it++)
    {
        JSON *p = it->second->copy();
        obj->mMembers[it->first] = p;
    }
    return obj;
}

JSON* Object::getByIndex(size_t i)
{
   return &mInvalid; 
}

JSON* Object::getByKey(std::string key)
{
    if (mMembers.find(key)!=mMembers.end())
    {
        return mMembers[key];
    }

    return &mInvalid;
}

std::string Object::toString()
{
    return "{object}";
}

bool Object::getIsValid()
{
   return true;
}

std::vector<std::string> Object::getKeys()
{
   std::vector<std::string> list;
   for (std::unordered_map<std::string,JSON*>::iterator it = mMembers.begin(); it != mMembers.end(); it++)
   {
      list.push_back(it->first);
   }

   return list;
}

void Object::parseObject(std::string str)
{
   if (str.size() < 1) return;
   if (str[0] != '{') return;

    std::string tmp = str.substr(1);
    size_t i = 1;
    while (i<str.size())
    {
        Pair p = mParser.getPair(tmp);
        if (p.val.type==JSON_Invalid) break;

        if (p.val.type==JSON_Object){
            mMembers[p.key] = new Object(p.val.value);
        } else if (p.val.type==JSON_List){
            mMembers[p.key]= new List(p.val.value);
        } else if (p.val.type==JSON_Value){
            mMembers[p.key]= new Value(p.val.value, p.val.subType);
        }

        if (tmp[p.size]=='}') break;
        if (tmp[p.size]!=',') return;
        i+=p.size+1; //skip comma
        tmp = str.substr(i);
    
    }

}


std::string Object::stringify(int level)
{
    std::stringstream ss;

    ss <<"{";
    if (level!=-1) ss<<"\r\n";

    std::unordered_map<std::string,JSON*>::iterator it=mMembers.begin();
    while (it!=mMembers.end())
    {
        for (int i=0;i<level;i++) ss<<"\t";
        ss << "\"" << it->first << "\" : ";

        int l2 = level;
        if (l2!=-1) l2++;
        ss << it->second->stringify(l2);
        it++;
        if (it!=mMembers.end()) ss << ",";
        if (level!=-1) ss<<"\r\n";
        
    }

    for (int i=0;i<level-1;i++) ss<<"\t";
    ss <<"}";

    return ss.str();

}

void Object::setStringValue(std::string value)
{
}

void Object::assign(const JSON& json)
{

}

JSON& Object::addObject(const std::string& name)
{
    std::string key = name;
    if (name=="")
    {
        key = generateKey();
    }

    Object *item = new Object("{}");
    mMembers[key]=item;
    return *item;
}

JSON& Object::addList(const std::string& name)
{
    std::string key = name;
    if (name=="")
    {
        key = generateKey();
    }

    List *item = new List("[]");
    mMembers[key] = item;
    return *item;

}

std::string Object::generateKey()
{
    size_t i = mMembers.size();
    std::string ret;
    do
    {
        std::stringstream ss;
        ss << "key" << i;
        ret = ss.str();
        i++;
    } while (mMembers.find(ret) != mMembers.end());

    return ret;
    
}

JSON& Object::addValue(const std::string& val,const std::string& name)
{
   std::string key = name;
   if (name == "")
   {
      key = generateKey();
   }    

   std::unordered_map<std::string,JSON*>::iterator it =  mMembers.find(key);
   if (it != mMembers.end())
   {
      delete it->second;
   }

    Value *item = new Value(val, String);
    mMembers[key]= item;
    return *item;
}

JSON& Object::addValue(unsigned int val,const std::string& name)
{
    std::string key = name;
    if (name=="")
    {
        key = generateKey();
    }

   std::unordered_map<std::string,JSON*>::iterator it =  mMembers.find(key);
   if (it != mMembers.end())
   {
      delete it->second;
   }
   Value *item = new Value(val);
   mMembers[key] = item;
   return *item;
}

JSON& Object::addValue(int val,const std::string& name)
{
    std::string key = name;
    if (name=="")
    {
        key = generateKey();
    }

   std::unordered_map<std::string,JSON*>::iterator it =  mMembers.find(key);
   if (it != mMembers.end())
   {
      delete it->second;
   }
   Value *item = new Value(val);
   mMembers[key] = item;
   return *item;
}

JSON& Object::addValue(double val,const std::string& name)
{
    std::string key = name;
    if (name=="")
    {
        key = generateKey();
    }

   std::unordered_map<std::string,JSON*>::iterator it =  mMembers.find(key);
   if (it != mMembers.end())
   {
      delete it->second;
   }
   Value *item = new Value(val);
   mMembers[key] = item;
   return *item;
}

JSON& Object::addValue(bool val,const std::string& name)
{
   std::string key = name;
   if (name == "")
   {
      key = generateKey();
   }
   std::unordered_map<std::string,JSON*>::iterator it =  mMembers.find(key);
   if (it != mMembers.end())
   {
      delete it->second;
   }
   Value *item = new Value(val);
   mMembers[key] = item;
   return *item;
}



void Object::setBool(bool val)
{
}
void Object::setUInt(unsigned int val)
{
}
void Object::setInt(int val)
{
}
void Object::setDouble(double val)
{
}



