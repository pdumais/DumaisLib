#include <stdio.h>
#include "RESTEngine.h"

class class1
{
public:
    void c1(Dumais::JSON::JSON&,RESTParameters* p)
    {
        printf("c1: p1=[%s], p2=[%s], p3=[%s], p4=[%s]\r\n",p->getParam("p1").c_str(),p->getParam("p2").c_str(),p->getParam("p3").c_str(),p->getParam("p4").c_str());
    }
    void c2(Dumais::JSON::JSON&,RESTParameters* p)
    {
        printf("c2: p1=[%s], p2=[%s], p3=[%s], p4=[%s]\r\n",p->getParam("p1").c_str(),p->getParam("p2").c_str(),p->getParam("p3").c_str(),p->getParam("p4").c_str());
    }
};




int main(int argc, char**argv)
{
    RESTEngine engine;
    class1* p = new class1();

    RESTCallBack<class1> *pc1 = new RESTCallBack<class1>(p,&class1::c1,"d1");
    RESTCallBack<class1> *pc2 = new RESTCallBack<class1>(p,&class1::c2,"d2");
    pc1->addParam("p1","p1 description");
    pc1->addParam("p2","p1 description");
    pc2->addParam("p3","p3 description");
    pc2->addParam("p4","p4 description");

    engine.addCallBack("/test1/blah",pc1);
    engine.addCallBack("/test2/blah2",pc2);

    Dumais::JSON::JSON j;
    engine.invoke(j,"/test1/blah?p1=test1 ");
    engine.invoke(j,"/test2/blah2?p1=test1&p2=test2&p3=test3&p4=test4");

    Dumais::JSON::JSON json;
    engine.documentInterface(json);
    printf("%s\r\n",json.stringify(true).c_str());

    return 0;
}
