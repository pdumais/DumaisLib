#ifndef JSONVALUE_H
#define JSONVALUE_H

#include "JSON.h"

namespace Dumais
{
    namespace JSON
    {
	    class Value: public JSON{
    	private:
	        std::string mValue;
            ValueSubType mSubType;

            friend class Object;
            friend class List;
            Value();
            Value(std::string value, ValueSubType subType);
            Value(unsigned int value);
            Value(int value);
            Value(double value);
            Value(bool value);

            std::string escapeString(const std::string& st);
            std::string unEscapeString(const std::string& st);

            std::string stringify(int level);    
            virtual JSON* copy();
            virtual JSON* getByIndex(size_t i);
    	    virtual JSON* getByKey(std::string key);
    	    virtual std::string toString();
    	    virtual void setStringValue(std::string value);
	        virtual void assign(const JSON& json);
            virtual JSON& addObject(const std::string& name="");
            virtual JSON& addList(const std::string& name="");
            virtual JSON& addValue(const std::string& val,const std::string& name="");
            virtual JSON& addValue(unsigned int val,const std::string& name="");
            virtual JSON& addValue(int val,const std::string& name="");
            virtual JSON& addValue(double val,const std::string& name="");
            virtual JSON& addValue(bool val,const std::string& name="");    
            virtual double convertToDouble();
            virtual int convertToInt();
            virtual unsigned int convertToUInt();
            virtual bool convertToBool();
            virtual void setBool(bool val);
            virtual void setUInt(unsigned int val);
            virtual void setInt(int val);
            virtual void setDouble(double val);


    	public:
    		virtual ~Value();
            virtual bool getIsValid();

    	};
    }
}
#endif

