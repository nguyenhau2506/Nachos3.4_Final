#include "syscall.h"
#include "copyright.h"

int main()
{
	char* fileName;
	int isCreated = 0;

	PrintString("Innput file name: ");
	ReadString(fileName, 255);
	isCreated = CreateFile(fileName);
	if (isCreated == -1)
	{
		PrintString("Create file failed!\n");
	} else{
		PrintString("Create file completely!\n");
	}
	return 0;
}