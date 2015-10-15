#include <stdio.h>
#include <iostream>
#include <fstream>
#include "rest/RESTEngine.h"

class class1
{
public:
    void c1(RESTContext* context)
    {
        RESTParameters *p = context->params;
        printf("c1: p1=[%s], p2=[%s], p3=[%s], p4=[%s]\r\n",p->getParam("p1").c_str(),p->getParam("p2").c_str(),
                p->getParam("p3").c_str(),p->getParam("p4").c_str());
    }
    void c2(RESTContext* context)
    {
        RESTParameters *p = context->params;
        printf("c2: p1=[%s], p2=[%s], p3=[%s], p4=[%s]\r\n",p->getParam("p1").c_str(),p->getParam("p2").c_str(),
                p->getParam("p3").c_str(),p->getParam("p4").c_str());
    }
    void c3(RESTContext* context)
    {
        RESTParameters *p = context->params;
        printf("c3: [%s]\r\n", context->matches[1].str().c_str());
    }
};


#define CHECK(X) if (!(X)) { \
    std::cerr << "Failed to check " << #X << " at line " << __LINE__ << std::endl; \
    return EXIT_FAILURE; }

int main(int argc, char**argv)
{
    RESTEngine engine;
    class1* p = new class1();

    RESTCallBack *pc1 = new RESTCallBack(p,&class1::c1,"d1");
    RESTCallBack *pc2 = new RESTCallBack(p,&class1::c2,"d2");
    RESTCallBack *pc3 = new RESTCallBack(p,&class1::c2,"d2");
    RESTCallBack *pc4 = new RESTCallBack(p,&class1::c3,"d4");
    pc1->addParam("p1","p1 description");
    pc1->addParam("p2","p1 description");
    pc2->addParam("p3","p3 description");
    pc2->addParam("p4","p4 description");
    pc3->addParam("p3","p3 description");
    pc3->addParam("p4","p4 description");
    pc4->addParam("ID", "product ID", true, "string", "path");

    engine.addCallBack("/test1/blah","get",pc1);
    engine.addCallBack("/test2/blah2","GET",pc2);
    engine.addCallBack("/test2/t.*","DELETE",pc3);
    engine.addCallBack("/test3/([a-z0-9]*)","POST",pc4);

    Dumais::JSON::JSON j;
    RESTEngine::ResponseCode rc;
    rc = engine.invoke(j,"/test1/blah?p1=test1 ","GET","");
    CHECK(rc == RESTEngine::OK);
    rc = engine.invoke(j,"/test2/blah2?p1=test1&p2=test2&p3=test3&p4=test4","GET","");
    CHECK(rc == RESTEngine::OK);
    rc = engine.invoke(j,"/test2/test?p1=test1&p2=test2&p3=test33&p4=test44","DELETE","");
    CHECK(rc == RESTEngine::OK);
    rc = engine.invoke(j,"/test2/rest","DELETE","");
    CHECK(rc == RESTEngine::NotFound);
    rc = engine.invoke(j,"/test3/product144","POST","");
    CHECK(rc == RESTEngine::OK);

    Dumais::JSON::JSON json;
    engine.documentInterface(json);
    printf("%s\r\n",json.stringify(true).c_str());

    Dumais::JSON::JSON swaggerSchema;
    engine.documentSwaggerInterface(swaggerSchema, "1", "title", "description", "http", "localhost", "/api");
    if (!swaggerSchema.isValid()) {
        std::cerr << "invalid JSON swagger schema" << std::endl;
    } else {
        std::ofstream swaggerSchemaFile(argc>1?argv[1]:"swagger.json");
        swaggerSchemaFile << swaggerSchema.stringify(true);
        swaggerSchemaFile.close();
    }

    engine.removeCallBack(pc1);
    engine.removeCallBack(pc2);
    engine.removeCallBack(pc3);
    engine.removeCallBack(pc4);

    delete p;
    return EXIT_SUCCESS;
}
