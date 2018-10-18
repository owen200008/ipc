#include <stdlib.h>
#include <stdio.h>
#include <thread>


//func call is success
#define PrintSuccessOrFail(Func)\
{\
    char szFormat[128] = {'/'};\
    int nRetValue = sprintf(szFormat, "F:%s", #Func);\
    if(nRetValue > 0)\
        for(int i = nRetValue;i < 64;i++)\
            szFormat[i] = '/';\
    printf(szFormat);\
    printf("\n");\
    if(Func())\
        nRetValue = sprintf(szFormat, "R:Success");\
    else\
        nRetValue = sprintf(szFormat, "R:Fail???");\
    if(nRetValue > 0)\
        for(int i = nRetValue;i < 64;i++)\
            szFormat[i] = '/';\
    printf(szFormat);\
    printf("\n");\
}

int main(){

	PrintSuccessOrFail(SmartBufferTest);
	PrintSuccessOrFail(IPCLogTest);
	printf("End Test IPC\n");
	getchar();
	return 0;
}