#include <stdio.h>
#include "itronWrapper.h"

int main(int argc, void* argv[])
{
	uitronWrapperInit();

	extern void itronWrapperTest(void);
	itronWrapperTest();
}
