#include "SmartBufferTest.h"
#include "ZIPC/base/mem/SmartBuffer.h"
#include <stdio.h>
#include <stdlib.h>

bool SmartBufferTest(){
	zilliz::lib::SmartBuffer smBuf;
	smBuf.AppendString("123");
	if(smBuf.GetDataLength() != 3)
		return false;
	smBuf.SetDataLength(100);
	if(smBuf.GetDataLength() != 100)
		return false;
	return true;
}