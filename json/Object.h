#ifndef JSONOBJECT_H
#define JSONOBJECT_H

#include "JSON.h"
#include <unordered_map>
namespace Dumais
{
    namespace JSON
    {

	    class Object: public JSON{
    	private:
	        friend class Parser;
            friend class List;
            friend class JSON; 
	        std::unordered_map<std::string,JSON*> mMembers;
	        void parseObject(std::string str);
            std::string generateKey();
            Object(std::string str);
            virtual JSON* copy();

            virtual JSON& addObject(const std::string& name="");
            virtual JSON& addList(const std::string& name="");
            virtual JSON& addValue(const std::string& val,const std::string& name="");
            virtual JSON& addValue(unsigned int val,const std::string& name="");
            virtual JSON& addValue(int val,const std::string& name="");
            virtual JSON& addValue(double val,const std::string& name="");
            virtual JSON& addValue(bool val,const std::string& name="");
            virtual JSON* getByIndex(size_t i);
	        virtual JSON* getByKey(std::string key);
    	    virtual std::string stringify(int level);
    	    virtual std::string toString();
    	    virtual void setStringValue(std::string value);
    	    virtual void assign(const JSON& json); 
            virtual void setBool(bool val);
            virtual void setUInt(unsigned int val);
            virtual void setInt(int val);
            virtual void setDouble(double val);
        	virtual bool getIsValid();
            virtual std::vector<std::string> getKeys();
            


	    public:
        	virtual ~Object();

    	};

        //static JSON* Object(std::string name);
    }
}
#endif

