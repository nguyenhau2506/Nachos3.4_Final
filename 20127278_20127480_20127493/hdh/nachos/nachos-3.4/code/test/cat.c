#include "syscall.h"
#include "copyright.h"

int main()
{
	OpenFileId fileID;
	int length, readNumByte;
	char *read;

	char *fileName;
	// int writeByte;

	PrintString("Input file name: ");
	ReadString(fileName, 255);
	fileID = OpenFileByName(fileName, 1);
	// writeByte = WriteFile("Lorem Ipsum has been the industry's standard dummy text ever since the 1500s", 255, fileID);
	if ( fileID == -1)
	{
		PrintString("Can not open file ");
		return 0;
	}
	readNumByte = ReadFile(read, 255, fileID);
	if(readNumByte == -1){
		PrintString("Can't cat this file!\n");
	}  
	PrintString(read);
	CloseFile(fileID);
	return 0;
}