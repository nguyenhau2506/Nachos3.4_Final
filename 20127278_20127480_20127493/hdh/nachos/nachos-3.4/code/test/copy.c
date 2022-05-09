#include "syscall.h"
#include "copyright.h"

int main()
{
	OpenFileId srcId;
	OpenFileId dstId;
	char source[255], dest[255];
	char *c;
	int isCreated = 0;
	
	PrintString("Input source file name:");
	ReadString(source, 255);
	PrintString("Input destination file name:");
	ReadString(dest, 255);

	srcId = OpenFileByName(source, 1);
	isCreated = CreateFile(dest);
	dstId = OpenFileByName(dest, 1);

	if (srcId == -1 || dstId == -1)
	{
		int errorId = srcId == 0 ? 1 : 2;
		PrintString("Open file failed!\n");
		return 0;
	}
	
	ReadFile(c, 255, srcId);
	WriteFile(c, 255, dstId);
	CloseFile(srcId);
	CloseFile(dstId);
	return 0;
}