#include "JSON.h"
#include <string.h>
#include <sstream>
#include <streambuf>
#include <fstream>
#include "Object.h"
#include <regex>

using namespace Dumais::JSON;

JSON JSON::mInvalid;


namespace Dumais
{
    namespace JSON
    {
        //WARNING: regular expressions are not supported yet with GCC
        JSON& JSONPathQuery(JSON& json, std::string query)
        {
            std::regex r("[^\\/]+");
            std::queue<std::string> objects;
            std::smatch match;
            while (std::regex_search(query,match,r))
            {
                objects.push(match[0]);
                query = match.suffix().str();
            }
           return *json.matchNode(objects);
        }
    }
}

/*
    A pure instance of a JSON object (i.e: not a inheriting Object, List or Value) has the sole purpose
    of holding a reference to an inheriting Object,List or Value. If it doesn't hold such a reference,
    it will ignore all operations, and will always return the value "{invalid}".

    When creating an instance of this object and then parsing a string, the mRoot will be a JSON::Object.
    All operations will be forwarded down to the JSON::Object. The Parsing operation will create
    all required entities and for this reason, this instance will be a root instance of a JSON tree. 
    Only the root instance should be allowed to delete what is in the tree

    Note that only the JSON::JSON object can create Object,List and Value objects. Therefore it is safe to 
    assume that deletion will be done from here at all times.

    When creating an instance of this object and using the operator= to assign it a value, the mRoot will reference
    the mRoot of the other JSON::JSON object. In that case, it means that this object is only a reference to another JSON entity
    and we should never delete the reference since this is not the root object.
*/

JSON::JSON(){
    mRoot = this;
}

JSON::~JSON(){
    clean();
}

void JSON::clean()
{
    // if mRoot = this instance, it means that this is a pure JSON instance with no root.
    if (mRoot != this)
    {
        if (mRoot != 0)
        {
            delete mRoot;    
        }
        mRoot = this;
    }
}

JSON::JSON(const JSON& json)
{
    mRoot = 0;
    if (this != &json)
    {
        // only a pure JSON object can be re-assigned to another object
        assign(json);
    }
}

JSON& JSON::operator=(const JSON& json)
{
    if (this != &json)
    {
        // only a pure JSON object can be re-assigned to another object
        assign(json);
    }
    return *this;
}

JSON::JSON(JSON&& json)
{
    mRoot = 0;
    if (this != &json)
    {
        std::swap(this->mRoot,json.mRoot);
    }
}

JSON& JSON::operator=(JSON&& json)
{
    if (this != &json)
    {
        std::swap(this->mRoot,json.mRoot);
    }
    return *this;
}



size_t JSON::size()
{
   return mRoot->getSize();
}

bool JSON::getIsValid()
{
   return false;
}

size_t JSON::getSize()
{
   return -1;
}

std::vector<std::string> JSON::keys()
{
   return mRoot->getKeys();
}

std::vector<std::string> JSON::getKeys()
{
   return std::vector<std::string>();
}


void JSON::assign(const JSON& json)
{
    if (this==&mInvalid) return;

    JSON *pTemp = json.mRoot->copy();
    clean();
    mRoot = pTemp;
    if (mRoot==0) mRoot = this;
}

JSON* JSON::copy()
{
    // if we get in here, it means we a re a pure instance
    return 0;
}

JSON& JSON::operator=(const std::string& json)
{
   mRoot->setStringValue(json);
   return *this;
}

JSON& JSON::operator=(const char* json)
{
    mRoot->setStringValue(json);
    return *this;
}


JSON& JSON::operator=(unsigned int val)
{
    mRoot->setUInt(val);
    return *this;
}
JSON& JSON::operator=(int val)
{
    mRoot->setInt(val);
    return *this;
}
JSON& JSON::operator=(bool val)
{
    mRoot->setBool(val);
    return *this;
}
JSON& JSON::operator=(double val)
{
    mRoot->setDouble(val);
    return *this;
}

JSON& JSON::operator[](int i)
{
    return *mRoot->getByIndex(i);
}

JSON& JSON::operator[](std::string str)
{
    return *mRoot->getByKey(str);
}

std::string JSON::str()
{
    return mRoot->toString();
}

JSON* JSON::getByIndex(size_t i)
{
    return this;
}

JSON* JSON::getByKey(std::string key)
{
    return this;
}

std::string JSON::toString()
{
    return "{invalid}";
}

double JSON::toDouble()
{
    return mRoot->convertToDouble();
}

int JSON::toInt()
{
    return mRoot->convertToInt();
}

unsigned int JSON::toUInt()
{
    return mRoot->convertToUInt();
}

bool JSON::toBool()
{
    return mRoot->convertToBool();
}


double JSON::convertToDouble()
{
    return 0;
}

int JSON::convertToInt()
{
    return 0;
}

unsigned int JSON::convertToUInt()
{
    return 0;
}

bool JSON::convertToBool()
{
    return false;
}

bool JSON::isValid()
{
   return mRoot->getIsValid();
}

void JSON::parseFile(const std::string& filename)
{
   if (this==&mInvalid) return;

   FILE *f = fopen(filename.c_str(),"r");
   if (f)
   {
      std::string st;
      char buf[1024];
      size_t size;
      while (size=fread((char*)&buf,1,1023,f))
      {
         buf[size]=0;
         st+=(char*)&buf;
      }
      fclose(f);
      parse(st);
   }

}


void JSON::parse(std::string& json)
{
    if (this==&mInvalid) return;

    //if we parse, then we are root. We should rememer that because this is the only instance that will be allowed to cleanup
    if (mRoot!=this) delete mRoot;

    // first pass: remove all spaces/tabs that are not part of strings
    Parser parser;
    json = mParser.removeSpaces(json);

    //root is always an object and never a list
    JSON *obj= new Object(json);
    if (obj)
    {
        mRoot = obj;
    } else {
        mRoot = this;
    }

}


std::string JSON::stringify(int level)
{
    return "{}";
}

std::string JSON::stringify(bool formatted)
{
    int level = -1;
    if (formatted) level = 1;

    return mRoot->stringify(level);
}

void JSON::setStringValue(std::string value)
{
}

JSON& JSON::addObject(const std::string& name)
{
    if (this==&mInvalid) return mInvalid;
    // if we get into this method, it means that a pure JSON was instanciated. if mRoot=this, it means it was not
    // even assigned yet.
    if (mRoot == this)
    {
        mRoot = new Object("{}");
    }

    return mRoot->addObject(name);
}

JSON& JSON::addList(const std::string& name)
{
    if (this==&mInvalid) return mInvalid;
    // if we get into this method, it means that a pure JSON was instanciated. if mRoot=this, it means it was not
    // even assigned yet.
    if (mRoot == this)
    {
        mRoot = new Object("{}");
    }

    return mRoot->addList(name);
}

JSON& JSON::addValue(const std::string& val,const std::string& name)
{
    if (this==&mInvalid) return mInvalid;
    // if we get into this method, it means that a pure JSON was instanciated. if mRoot=this, it means it was not
    // even assigned yet.
    if (mRoot == this)
    {
        mRoot = new Object("{}");
    }

    return mRoot->addValue(val,name);
}

JSON& JSON::addValue(const char* val,const std::string& name)
{
    if (this==&mInvalid) return mInvalid;
    std::string v = val;
    return addValue(v,name);
}


JSON& JSON::addValue(int val, const std::string& name)
{
    if (this==&mInvalid) return mInvalid;
    std::stringstream ss;
    ss << val;

    // if we get into this method, it means that a pure JSON was instanciated. if mRoot=this, it means it was not
    // even assigned yet.
    if (mRoot == this)
    {
        mRoot = new Object("{}");
    }

    return mRoot->addValue(val,name);
}

JSON& JSON::addValue(unsigned int val, const std::string& name)
{
    if (this==&mInvalid) return mInvalid;
// if we get into this method, it means that a pure JSON was instanciated. if mRoot=this, it means it was not
    // even assigned yet.
    if (mRoot == this)
    {
        mRoot = new Object("{}");
    }

    return mRoot->addValue(val,name);
}


JSON& JSON::addValue(double val, const std::string& name)
{
    if (this==&mInvalid) return mInvalid;
// if we get into this method, it means that a pure JSON was instanciated. if mRoot=this, it means it was not
    // even assigned yet.
    if (mRoot == this)
    {
        mRoot = new Object("{}");
    }

    return mRoot->addValue(val,name);
}

JSON& JSON::addValue(bool val, const std::string& name)
{
    if (this==&mInvalid) return mInvalid;
// if we get into this method, it means that a pure JSON was instanciated. if mRoot=this, it means it was not
    // even assigned yet.
    if (mRoot == this)
    {
        mRoot = new Object("{}");
    }

    return mRoot->addValue(val,name);
}

void JSON::setBool(bool val)
{
}
void JSON::setUInt(unsigned int val)
{
}
void JSON::setInt(int val)
{
}
void JSON::setDouble(double val)
{
}

JSON* JSON::matchNode(std::queue<std::string>& query)
{
   if (query.size() == 0) return this;

   std::string node = query.front();
   query.pop();
   std::regex r("(.+)\\[([0-9]*)\\]");
   std::smatch match;
   JSON* pNextNode = NULL;
   if (std::regex_search(node,match,r))
   {
      size_t index = std::stoi(match[2].str());
      pNextNode = mRoot->getByKey(match[1])->getByIndex(index);
   }
   else
   {
      pNextNode = mRoot->getByKey(node);
   }

   return pNextNode->matchNode(query);
}


