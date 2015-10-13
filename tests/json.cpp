#include "stdio.h"
#include "json/JSON.h"
#include "json/Object.h"

using namespace Dumais;

bool teststr(JSON::JSON& val, std::string str)
{
    std::string v = val.str();
    printf("Testing '%s' == '%s'\r\n",v.c_str(),str.c_str());
    return (v==str);
}

JSON::JSON testret2()
{
    JSON::JSON j;
    return j;
}


JSON::JSON testret()
{
    JSON::JSON j = testret2();
    j.addValue("test","test");
    return j;
}


int main(int argc, char **argv)
{
    double d1;
    unsigned int ui1;
    int i1;
    bool b1,b2,b3,b4;

  /*  char data[1024];
    FILE *f= fopen("test.json","r");
    if (!f) return -1;
    int n = fread((char*)&data,1,1024,f);
    data[n]=0;
    fclose(f);
    std::string test = data;*/


    JSON::JSON json;
    json.parseFile("test.json");

   /* JSON::JSON& jq = JSON::JSONPathQuery(json,"key2/sub3[1]/sub3.3");
    if (!teststr(jq,std::string("{test}"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(JSON::JSONPathQuery(json,"key2/sub3[1]/sub3.3"),std::string("{test}"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(JSON::JSONPathQuery(json,"invalidkey/invalid[6]/keyu"),std::string("{invalid}"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(JSON::JSONPathQuery(json,"key2/sub3"),std::string("{list}"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(JSON::JSONPathQuery(json,"key2/sub3[1]/sub3.3invalid"),std::string("{invalid}"))) printf("ERROR %i\r\n",__LINE__);*/


    if (!teststr(json["key1"],std::string("val1"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key2"],std::string("{object}"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key2"]["sub1"],std::string("[test] ] [ { } }{ "))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key2"]["sub2"],std::string("{object}"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key2"]["sub2"]["list1"],std::string("{list}"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key2"]["sub2"]["list1"][0],std::string("val1"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key2"]["sub2"]["list1"][1],std::string("val2"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key2"]["sub2"]["list1"][2],std::string("val3"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key2"]["sub2"]["list1"][3],std::string("{invalid}"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key2"]["sub2"]["sub2.2"],std::string("val2.2"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key2"]["sub3"],std::string("{list}"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key2"]["sub3"][0]["sub3.1"],std::string("valsub3.1"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key2"]["sub3"][0]["sub3.2"],std::string("valsub3.2"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key2"]["sub3"][1]["sub3.3"],std::string("{test}"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key2"]["sub3"][1]["sub3.4"],std::string("[test] ] [ { } }{"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key3"]["sub2"],std::string("valsub2"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key4"][0],std::string("l1 \" "))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key3"]["sub3"],std::string("\r\n\\\t?\'\""))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key4"][1],std::string("l2"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key4"][2],std::string("{object}"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key4"][2]["l3"],std::string("vall3"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key4"][3],std::string("{list}"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key4"][3][0],std::string("l4.1"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key4"][3][1],std::string("l4.2"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key4"][3][0][0],std::string("{invalid}"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key4"][3][0]["test"],std::string("{invalid}"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key5"][0],std::string("-992.1"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key5"][1],std::string("true"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key5"][2],std::string("false"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key5"][3],std::string("{illegal}"))) printf("ERROR %i\r\n",__LINE__);

    // operations on invalid item    
    if (!teststr(json["key555"],std::string("{invalid}"))) printf("ERROR %i\r\n",__LINE__);
    json["key555"].addValue("1","test");
    if (!teststr(json["key555"],std::string("{invalid}"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key555"]["test"],std::string("{invalid}"))) printf("ERROR %i\r\n",__LINE__);

    if (!teststr(json["key556"],std::string("{invalid}"))) printf("ERROR %i\r\n",__LINE__);
    json["key556"][4].addValue("2","test2");
    if (!teststr(json["key556"][0],std::string("{invalid}"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key556"][4],std::string("{invalid}"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key556"],std::string("{invalid}"))) printf("ERROR %i\r\n",__LINE__);
    json["key556"] = json["key5"][2];
    if (!teststr(json["key556"],std::string("{invalid}"))) printf("ERROR %i\r\n",__LINE__);

    // test reading values
    d1 = json["key6"][0].toDouble();
    ui1 = json["key6"][1].toInt();
    i1 = json["key6"][2].toUInt();
    b1 = json["key6"][3].toBool();
    b2 = json["key6"][4].toBool();
    b3 = json["key6"][6].toBool();
    b4 = json["key6"][7].toBool();
    if (d1!=-242.1) printf("ERROR %i\r\n",__LINE__);
    if (ui1!=123) printf("ERROR %i\r\n",__LINE__);
    if (i1!=-456) printf("ERROR %i\r\n",__LINE__);
    if (b1!=true) printf("ERROR %i\r\n",__LINE__);
    if (b2!=false) printf("ERROR %i\r\n",__LINE__);
    if (b3!=true) printf("ERROR %i\r\n",__LINE__);
    if (b4!=true) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key6"][5],std::string("{illegal}"))) printf("ERROR %i\r\n",__LINE__);


    // test reassigning values
    json["key6"][0]=(bool)true;
    json["key6"][4]=(double)567.8;
    d1 = json["key6"][4].toDouble();
    b1 = json["key6"][0].toBool();
    if (d1!=567.8) printf("ERROR %i\r\n",__LINE__);
    if (b1!=true) printf("ERROR %i\r\n",__LINE__);

    //test converting values to string
    json["key6"][2]=(unsigned int)89;
    json["key6"][3]=(int)-789;
    if (!teststr(json["key6"][0],std::string("true"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key6"][4],std::string("567.8"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key6"][2],std::string("89"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["key6"][3],std::string("-789"))) printf("ERROR %i\r\n",__LINE__);

    // test writing values


    json["key5"][1] = json["key5"]; // operator= should refuse it. Can only overwrite a pure JSON object
    JSON::JSON j1 = json["key5"][1];
    if (!teststr(j1,std::string("true"))) printf("ERROR %i\r\n",__LINE__);

    json["key5"][1] = "meow";
    if (!teststr(json["key5"][1],std::string("meow"))) printf("ERROR %i\r\n",__LINE__);

    JSON::JSON j2 = json["key2"].addValue("val2");
    if (!teststr(j2,std::string("val2"))) printf("ERROR %i\r\n",__LINE__);

    JSON::JSON j3 = json["key2"]["key3"].addObject();
    if (!teststr(j3,std::string("{invalid}"))) printf("ERROR %i\r\n",__LINE__);
    
    json.addValue("val6","newkey");
    if (!teststr(json["newkey"],std::string("val6"))) printf("ERROR %i\r\n",__LINE__);

    json.addObject("newobj1");
    json["newobj1"].addList("list1");
    json["newobj1"]["list1"].addObject();
    json["newobj1"]["list1"].addValue("val1");
    json["newobj1"]["list1"].addList();
    json["newobj1"]["list1"][2].addValue("test");
    json["newobj1"]["list1"][0].addValue("test2");
    json["newobj1"]["list1"][0].addValue("test3","testkey");
    json["newobj1"]["list1"][0].addValue(-707.43,"testkey3");
    if (!teststr(json["newobj1"],std::string("{object}"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["newobj1"]["list1"],std::string("{list}"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["newobj1"]["list1"][0],std::string("{object}"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["newobj1"]["list1"][1],std::string("val1"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["newobj1"]["list1"][2],std::string("{list}"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["newobj1"]["list1"][2][0],std::string("test"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["newobj1"]["list1"][0]["key0"],std::string("test2"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["newobj1"]["list1"][0]["testkey"],std::string("test3"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(json["newobj1"]["list1"][0]["testkey3"],std::string("-707.43"))) printf("ERROR %i\r\n",__LINE__);

    bool bb1 = true;
    json["newobj1"]["list1"][0].addValue(bb1,"testkey4");
    json["newobj1"]["list1"][0].addValue(false,"testkey5");
    json["newobj1"]["list1"][0].addValue(4,"testkey6");
    json["newobj1"]["list1"][0].addValue(-242,"testkey7");
    printf("Formatted output:\r\n%s\r\n=================]\r\n",json.stringify(true).c_str());
    printf("Unormatted output:\r\n%s\r\n=================]\r\n",json.stringify(false).c_str());

    JSON::JSON json2;
    json2.addValue("test","key");
    if (!teststr(json2["key"],std::string("test"))) printf("ERROR %i\r\n",__LINE__);
    printf("Formatted output:\r\n%s\r\n=================]\r\n",json2.stringify(true).c_str());

    JSON::JSON j = testret();
    if (!teststr(j["test"],std::string("test"))) printf("ERROR %i\r\n",__LINE__);

    printf(" ==== Testing object copy ====\r\n");
    JSON::JSON jorig;
    jorig.addObject("obj1");
    jorig["obj1"].addList("list1");
    jorig["obj1"]["list1"].addValue("test");
    JSON::JSON jcopy = jorig;
    if (!teststr(jcopy["obj1"]["list1"][0],std::string("test"))) printf("ERROR %i\r\n",__LINE__);
    jcopy["obj1"]["list1"][0]="test2";
    if (!teststr(jcopy["obj1"]["list1"][0],std::string("test2"))) printf("ERROR %i\r\n",__LINE__);
    JSON::JSON& jo = jorig["obj1"].addObject("obj2");
    jo.addValue("test","val");
    if (!teststr(jo["val"],std::string("test"))) printf("ERROR %i\r\n",__LINE__);
    if (!teststr(jorig["obj1"]["obj2"]["val"],std::string("test"))) printf("ERROR %i\r\n",__LINE__);

    jorig = jorig["obj1"]["list1"][0];
    if (!teststr(jorig,std::string("test"))) printf("ERROR %i\r\n",__LINE__);

}
