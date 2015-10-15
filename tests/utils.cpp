#include <stdio.h>
#include <string.h>
#include "utils/sha256.h"
#include "utils/sha1.h"
#include "utils/md5.h"

#define SIZE 1000

using namespace Dumais::Utils;

int main(int argc, char** argv)
{
    
    char* test = "test";
    if (argc==2) test = argv[1];
    char* test2 = new char[SIZE];
    for (int i=0;i<SIZE;i++) test2[i] = 'A';
    
    SHA256 s1(test,strlen(test));
    std::string st = s1.toHex();
    if (st!="9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08") printf("ERROR 1:%s\r\n",st.c_str()); else printf("OK 1\r\n");
    SHA256 s2(test2,SIZE);
    st = s2.toHex();
    if (st!="c2e686823489ced2017f6059b8b239318b6364f6dcd835d0a519105a1eadd6e4") printf("ERROR 2:%s\r\n",st.c_str()); else printf("OK 2\r\n");


    SHA1 s3(test,strlen(test));
    st = s3.toHex();
    if (st!="a94a8fe5ccb19ba61c4c0873d391e987982fbbd3") printf("ERROR 3:%s\r\n",st.c_str()); else printf("OK 3\r\n");
    SHA1 s4(test2,SIZE);
    st = s4.toHex();
    if (st!="3ae3644d6777a1f56a1defeabc74af9c4b313e49") printf("ERROR 4:%s\r\n",st.c_str()); else printf("OK 4\r\n");

    MD5 s5(test,strlen(test));
    st = s5.toHex();
    if (st!="098f6bcd4621d373cade4e832627b4f6") printf("ERROR 5:%s\r\n",st.c_str()); else printf("OK 5\r\n");
    MD5 s6(test2,SIZE);
    st = s6.toHex();
    if (st!="7644672d049290f0390d9c993c7d343d") printf("ERROR 6:%s\r\n",st.c_str()); else printf("OK 6\r\n");

    delete test2;

}
