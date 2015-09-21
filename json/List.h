#ifndef JSONLIST_H
#define JSONLIST_H

#include <vector>
#include "JSON.h"
namespace Dumais
{
    namespace JSON
    {
    	class List: public JSON{
    	private:
    	    friend class Parser;
            friend class Object;
            List(std::string json);
    	    std::vector<JSON*> mMembers;
    	    void parseList(std::string str);
            std::string stringify(int level);
            virtual JSON& addObject(const std::string& name="");
            virtual JSON& addList(const std::string& name="");
            virtual JSON& addValue(const std::string& val,const std::string& name="");
            virtual JSON& addValue(unsigned int val,const std::string& name="");
            virtual JSON& addValue(int val,const std::string& name="");
            virtual JSON& addValue(double val,const std::string& name="");
            virtual JSON& addValue(bool val,const std::string& name="");
            virtual JSON* getByIndex(size_t i);
    	    virtual JSON* getByKey(std::string key);
            virtual JSON* copy();
    	    virtual std::string toString();
    	    virtual void setStringValue(std::string value);
    	    virtual void assign(const JSON& json);
            virtual void setBool(bool val);
            virtual void setUInt(unsigned int val);
            virtual void setInt(int val);
            virtual void setDouble(double val);
            virtual bool getIsValid();

    	public:
    		virtual ~List();
            virtual size_t getSize();

    	};
    }
}
#endif

