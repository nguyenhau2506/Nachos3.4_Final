#include "syscall.h"
#include "copyright.h"

int main(){
	//ghep file 1 voi file 2
	//file 1 la file goc
	OpenFileId fileID1;
	OpenFileId fileID2;
	char file1[255], file2[255];
	char *read;
	int seekPos = 0;
	int temp = 0;
	PrintString("Input file name 1:");
	ReadString(file1, 255);
	PrintString("Input file name 2:");
	ReadString(file2, 255);
	fileID1 = OpenFileByName(file1, 1);
	fileID2 = OpenFileByName(file2, 1);
	if(fileID1 == -1 || file2 == -1){
		PrintString("Can't open file\n");
		return 0;
	}	
	seekPos = SeekFile(-1, fileID1);
	temp = ReadFile(read, 255, fileID2);
	WriteFile(read, 255, fileID1);
	return 0;
}