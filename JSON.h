#ifndef JSON_H
#define JSON_H

#include <string>
#include "Parser.h"
#include <vector>
#include <queue>

// This library was written for ease of use and definitely not for performance

namespace Dumais
{
    namespace JSON
    {

        // WARNING: It is not possible to use this object as a return value of a function. See "assign" method 
        // for description of why we can't do this.
       	class JSON{
        private:
            friend JSON& JSONPathQuery(JSON& json, std::string query);
            JSON* matchNode(std::queue<std::string>& query);
    	protected:
    	    virtual JSON* getByIndex(size_t i);
    	    virtual JSON* getByKey(std::string key);
    	    virtual std::string toString();
    	    JSON *mRoot;
    	    Parser mParser;
            static JSON mInvalid;

    	    virtual void clean();
    	    std::string parseString(const char *buf); 
    	    virtual void setStringValue(std::string value);
    	    virtual void assign(const JSON& json);
            virtual void setBool(bool val);
            virtual void setUInt(unsigned int val);
            virtual void setInt(int val);
            virtual void setDouble(double val);
            virtual double convertToDouble();
            virtual int convertToInt();
            virtual unsigned int convertToUInt();
            virtual bool convertToBool();
            virtual bool getIsValid();
            virtual size_t getSize();
            virtual std::vector<std::string> getKeys();
    	public:
    	    JSON();
            JSON(const JSON& json);
            JSON(JSON&& json);
    		virtual ~JSON();

    	    JSON& operator[](int i);
    	    JSON& operator[](std::string str);
    	    JSON& operator=(const std::string& val);
            JSON& operator=(const char* val);
    	    JSON& operator=(const JSON& json);
    	    JSON& operator=(JSON&& json);
            JSON& operator=(int val);
            JSON& operator=(double val);
            JSON& operator=(unsigned int val);
            JSON& operator=(bool val);

            bool isValid();
            size_t size();
            virtual JSON* copy();
    	    std::string str();
            double toDouble();
            int toInt();
            unsigned int toUInt();
            bool toBool();
    
    	    void parse(std::string& json);
            void parseFile(const std::string& filename);

            std::vector<std::string> keys();
	        std::string stringify(bool formatted=false);
	        virtual std::string stringify(int level=-1);
        
            virtual JSON& addObject(const std::string& name="");
            virtual JSON& addList(const std::string& name="");
            virtual JSON& addValue(const std::string& val,const std::string& name="");
            virtual JSON& addValue(const char* val,const std::string& name="");
            virtual JSON& addValue(int val, const std::string& name="");
            virtual JSON& addValue(unsigned int val, const std::string& name="");
            virtual JSON& addValue(double val, const std::string& name="");
            virtual JSON& addValue(bool val, const std::string& name="");

    	};
        JSON& JSONPathQuery(JSON& json, std::string query);

    }
}
#endif

