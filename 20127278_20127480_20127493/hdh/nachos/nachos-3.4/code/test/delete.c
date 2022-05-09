#include "syscall.h"
#include "copyright.h"

int main(){
	int isRemoved = 0;
	char* fileName;
	PrintString("Input file name: ");
	ReadString(fileName, 255);
	isRemoved = RemoveFile(fileName);
	if(isRemoved == 0){
		PrintString("Remove file completely!");
	} else{
		PrintString("Remove file failed!");
	}
	return 0;	
}