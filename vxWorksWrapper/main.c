#include <stdio.h>
#include "vxWorksWrapper.h"

int main(int argc, void* argv[])
{
	taskLibInit();

	extern void vxWorksWrapperTest(void);
	vxWorksWrapperTest();
}
