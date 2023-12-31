#include <stdio.h>
#include "uT-KernelWrapper.h"

int main(int argc, void* argv[])
{
	utkWrapperInit();

	extern void utkernelWrapperTest(void);
	utkernelWrapperTest();
}
