#include "HTTPParser.h"

using namespace Dumais::WebServer;

HTTPParser::HTTPParser(const char* url)
{
   std::string st = url;
   mRequest = "";
   if (st.length()<5) return;

   if (st.substr(0,4)=="GET "){
      std::string params;
      std::map<std::string,std::string> paramList;
      st = st.substr(4);
      size_t paramStart = st.find_first_of("?");
      size_t requestEnd = st.find_first_of(" ");
      if (requestEnd==std::string::npos) return;

      if (paramStart!=std::string::npos)
      {
         mRequest = st.substr(0,paramStart);
         params = st.substr(paramStart+1);
         std::string key;
         std::string value;
         bool keyFlag = true;
         int n=0;
         while (params[n]!=' ' && n<params.size())
         {
            if (params[n]=='=')
            {
               keyFlag = false;
            }
            else if (params[n]=='&' || params[n]==' ')
            {
               mParamList[key]=value;
               keyFlag = true;
               key="";
               value="";
            } else 
            {
               if (keyFlag)
               {
                  key+=params[n];
               }
               else
               {
                  value+=params[n];
               }
            }

            n++;
            if (params[n]==' ')
            {
               mParamList[key]=value;
               key="";
               value="";
            }
         }
      }
      else
      {
         mRequest = st.substr(0,requestEnd);
      }
    }
    
}

HTTPParser::~HTTPParser(){
}


unsigned int HTTPParser::getHeaderSize()
{
    return this->headerSize;
}

std::string HTTPParser::getRequest()
{
    return mRequest;
}

std::string HTTPParser::getParam(std::string name)
{
    if (mParamList.find(name)==mParamList.end()) return "";
    return mParamList[name];
}

std::map<std::string,std::string> HTTPParser::getParamList()
{
    return mParamList;
}
