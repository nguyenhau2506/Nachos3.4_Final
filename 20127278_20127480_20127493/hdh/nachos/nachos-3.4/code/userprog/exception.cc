// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include <stdio.h>
#include "syscall.h"
#include <timer.h>
#define MaxFileLength 255
#define MaxBuffer 255
#define MAX_NUM 2147483647

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
void IncreasePC()
{
    // have three pcregister PrevPCReg,PCReg,NextPCReg
    // PrevPCReg=PCReg
    machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
    // PCReg=NextPCReg
    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
    // NextPCReg=NextPCReg+4
    machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) + 4);
}

char *User2System(int virtAddr, int limit)
{
    int i; // chi so index
    int oneChar;
    char *kernelBuf = NULL;
    kernelBuf = new char[limit + 1]; // can cho chuoi terminal
    if (kernelBuf == NULL)
        return kernelBuf;

    memset(kernelBuf, 0, limit + 1);

    for (i = 0; i < limit; i++)
    {
        machine->ReadMem(virtAddr + i, 1, &oneChar);
        kernelBuf[i] = (char)oneChar;
        if (oneChar == 0)
            break;
    }
    return kernelBuf;
}

int System2User(int virtAddr, int len, char *buffer) // lay gia tri bo vao bien
{
    if (len < 0)
        return -1;
    if (len == 0)
        return len;
    int i = 0;
    int oneChar = 0;
    do
    {
        oneChar = (int)buffer[i];
        machine->WriteMem(virtAddr + i, 1, oneChar);
        i++;
    } while (i < len && oneChar != 0);
    return i;
}
void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
    
    switch (which)
    {
    case NoException:
        return;
    case SyscallException:
        switch (type)
        {
            
            case SC_Halt:
            {
                DEBUG('a', "\n Shutdown, initiated by user program.");
                printf("\n\n Shutdown, initiated by user program.");
                interrupt->Halt();
                break;
            }
            case SC_Create:
            {
                int virtAddr;
                char *filename;
                DEBUG('a', "\n SC_Create call ...");
                DEBUG('a', "\n Reading virtual address of filename");
                // Lấy tham số tên tập tin từ thanh ghi r4
                virtAddr = machine->ReadRegister(4);
                DEBUG('a', "\n Reading filename.");
                // MaxFileLength là = 32
                filename = User2System(virtAddr, MaxFileLength + 1);
                if (filename == NULL)
                {
                    printf("\n Not enough memory in system");
                    DEBUG('a', "\n Not enough memory in system");
                    machine->WriteRegister(2, -1); // trả về lỗi cho chương
                    // trình người dùng
                    IncreasePC();
                    delete filename;
                    return;
                }
                DEBUG('a', "\n Finish reading filename.");
                // DEBUG('a',"\n File name : '"<<filename<<"'");
                //  Create file with size = 0
                //  Dùng đối tượng fileSystem của lớp OpenFile để tạo file,
                //  việc tạo file này là sử dụng các thủ tục tạo file của hệ điều
                //  hành Linux, chúng ta không quản ly trực tiếp các block trên
                //  đĩa cứng cấp phát cho file, việc quản ly các block của file
                //  trên ổ đĩa là một đồ án khác
                if (!fileSystem->Create(filename, 0))
                {
                    printf("\n Error create file '%s'", filename);
                    machine->WriteRegister(2, -1);
                    IncreasePC();
                    delete filename;
                    return;
                }
                machine->WriteRegister(2, 0); // trả về cho chương trình
                // người dùng thành công
                delete filename;
                break;
            }
            case SC_ReadNum:
            {   
                
                char *buffer;
                buffer = new char[MaxBuffer + 1];
                // doc so tu  man hinh console
                int numbytes = gSynchConsole->Read(buffer, MaxBuffer); // doc so nguyen vao buffer có toi da 255 ki tu

                if(numbytes >= 10)
                {
                    printf("\n\n Over size");
                    DEBUG('a', "\n Over size");
                    machine->WriteRegister(2, MAX_NUM);
                    IncreasePC();
                    delete buffer;
                    return;
                }
                int firstIndex, lastIndex;
                bool isNegative;
                if (buffer[0] == '-')
                {
                    isNegative = true;
                    firstIndex = 1;
                }
                else
                {
                    isNegative = false;
                    firstIndex = 0;
                }

                for (int i = firstIndex; i < numbytes ; i++)
                {
                    if ((buffer[i] == '.'))
                    {
                        printf("\n\n The integer number is not valid");
                        DEBUG('a', "\n The integer number is not valid");
                        machine->WriteRegister(2, MAX_NUM);
                        IncreasePC();
                        delete buffer;
                        return;
                    }
                    else if(buffer[i] < '0' || buffer[i] > '9')
                    {
                        printf("\n\n The integer number is not valid");
                        DEBUG('a', "\n The integer number is not valid");
                        machine->WriteRegister(2, MAX_NUM);
                        IncreasePC();
                        delete buffer;
                        return;
                    }
                    lastIndex = i;
                }

                int number = 0;
                if (isNegative)
                {
                    for (int i = firstIndex; i <= lastIndex; i++)
                    {
                        number = number * 10 + (int)(buffer[i] - 48);
                    }
                    number = number * -1;
                }
                else
                {
                    for (int i = firstIndex; i <= lastIndex; i++)
                    {
                        number = number * 10 + (int)(buffer[i] - 48);
                    }
                }

                machine->WriteRegister(2, number);
                IncreasePC();
                delete buffer;
                return;
            }
            case SC_PrintNum:
            {
                int number = machine->ReadRegister(4);
                if(number == 0)//th number la so 0
                {
                    gSynchConsole->Write("0", 1); // in ra man hinh so 0
                    IncreasePC();
                    return;  
                }
                        
                bool isNegative = false; 
                int sizeOfNum = 0; 
                int firstNumIndex = 0; 

                if(number < 0)
                {
                    isNegative = true;
                    number = number * -1; //chuyen am thanh duong
                    firstNumIndex = 1; 
                } 	
                    
                int t_number = number; // bien tam cho number
                while(t_number)
                {
                    sizeOfNum++;
                    t_number /= 10;
                }
        
                // Tao buffer chuoi de in ra man hinh
                char* buffer;
                int MAX_BUFFER = 255;
                buffer = new char[MAX_BUFFER + 1];
                for(int i = firstNumIndex + sizeOfNum - 1; i >= firstNumIndex; i--)
                {
                    buffer[i] = (char)((number % 10) + 48);
                    number /= 10;
                }
                if(isNegative)//neu so am thi de buffer[0] se la dau tru
                {
                    buffer[0] = '-';
                    buffer[sizeOfNum + 1] = 0;
                    gSynchConsole->Write(buffer, sizeOfNum + 1);
                    delete buffer;
                    IncreasePC();
                    return;
                }
                buffer[sizeOfNum] = 0;	
                gSynchConsole->Write(buffer, sizeOfNum);
                delete buffer;
                IncreasePC();
                return;        	
            }
            case SC_ReadChar://doc 1 ki tu do nguoi dung nhap vao
            {
                char* buffer = new char[MaxBuffer];
                int numBytes = gSynchConsole->Read(buffer, MaxBuffer);

                if(numBytes > 1) //Neu nhap nhieu hon 1 ky tu thi khong hop le
                {
                    printf("Only 1 character!");
                    DEBUG('a', "\nERROR: Only 1 character!");
                    machine->WriteRegister(2, 0);
                }
                else if(numBytes == 0) 
                {
                    printf("Empty!");
                    DEBUG('a', "\nERROR: empty!");
                    machine->WriteRegister(2, 0);
                }
                else
                {
                    char c = buffer[0];
                    machine->WriteRegister(2, c);
                }

                delete buffer;
                IncreasePC(); // tang Programming Counter
                return;
            }
            case SC_PrintChar:
            {
                char c = (char)machine->ReadRegister(4); //doc tu thanh ghi r4
                gSynchConsole->Write(&c, 1); //1 byte
                IncreasePC();
                return;
            }
            case SC_RandomNum:
            {
                int n=Random()%10000;
                machine->WriteRegister(2, n);
                IncreasePC();
                return;
            }
            case SC_ReadString:
            {
                int virtAddr, length;
                char* buffer;
                virtAddr = machine->ReadRegister(4); // doc tu thanh ghi r4
                length = machine->ReadRegister(5); // doc tu thanh ghi r5
                buffer = User2System(virtAddr, length); // chuyen vung nho tu User Space sang System Space
                gSynchConsole->Read(buffer, length); // doc chuoi thong qua ham Read cua SynchConsole
                System2User(virtAddr, length, buffer); // chuyen vung nho tu System Space sang User Space
                delete buffer; 
                IncreasePC(); // tang Programming Counter 
                return;
            }
            case SC_PrintString:
            {
                int virtAddr;
                char* buffer;
                virtAddr = machine->ReadRegister(4); // doc tu thanh ghi r4
                buffer = User2System(virtAddr, 255); // chuyen vung nho tu User Space sang System Space
                int length = 0;
                while (buffer[length] != 0) // dem do dai cua chuoi 
                    length++; 
                gSynchConsole->Write(buffer, length + 1); // in chuoi thong qua ham Write cua SynchConsole
                delete buffer; 
                IncreasePC(); // tang Programming Counter 
                return;
            }
            case SC_CreateFile:
            {
                int virtAddr;
                char *filename;
                DEBUG('a', "\n SC_Create call ...");
                DEBUG('a', "\n Reading virtual address of filename");
                // Lấy tham số tên tập tin từ thanh ghi r4
                virtAddr = machine->ReadRegister(4);
                DEBUG('a', "\n Reading filename.");
                // MaxFileLength là = 32
                filename = User2System(virtAddr, MaxFileLength + 1);
                if (filename == NULL)
                {
                    printf("\n Not enough memory in system");
                    DEBUG('a', "\n Not enough memory in system");
                    machine->WriteRegister(2, -1); // trả về lỗi cho chương
                    // trình người dùng
                    IncreasePC();
                    delete filename;
                    return;
                }
                DEBUG('a', "\n Finish reading filename.");
                if (!fileSystem->Create(filename, 0))
                {
                    printf("\n Error create file '%s'", filename);
                    machine->WriteRegister(2, -1);
                    IncreasePC();
                    delete filename;
                    return;
                }
                machine->WriteRegister(2, 0); // trả về cho chương trình
                // người dùng thành công
                delete filename;
                IncreasePC();

                return;
            }
            case SC_OpenFileByName:
            {
                int virtAddr = machine->ReadRegister(4);
                int typeOfFile = machine->ReadRegister(5);
                char* filename = new char[MaxBuffer];
                //Case: check neu mo da mo hon 10 file thi ko mo duoc nua
                if (fileSystem->index >= 11){
                    machine->WriteRegister(2, -1);
                    IncreasePC();
                    printf("Can't open file\n");
                    delete[] filename;
                    return;
                }
                filename = User2System(virtAddr, MaxFileLength);
                //Check cac mode can mo
                if(strcmp(filename, "stdin") == 0){
                    printf("Stdin mode\n");
                    machine->WriteRegister(2, 0);
                    IncreasePC();
                    delete filename;
                    return;
                }
                if(strcmp(filename, "stdout") == 0){
                    printf("Stdout mode\n");
                    machine->WriteRegister(2, 1);
                    IncreasePC();
                    delete filename;
                    return;
                }

                // fileSystem->openedFile[fileSystem->index] = fileSystem->Open(filename, _type);
                if((fileSystem->openedFile[fileSystem->index] = fileSystem->Open(filename, typeOfFile)) != NULL){
                    printf("Open file compeletely!\n");
                    // DEBUG('a', "Open file compeletely!\n");
                    machine->WriteRegister(2, fileSystem->index-1);      
                } else{
                    printf("Open file failed!\n");
                    // DEBUG('a', "Open file failed!\n");
                    machine->WriteRegister(2, -1);
                }
                // printf("%d\n", fileSystem->openedFile[fileSystem->index-1]->type);
                delete[] filename;
                IncreasePC();
                return;
            } 
            case SC_CloseFile:
            {
                int fileID = (int)machine->ReadRegister(4);
                if(fileSystem->openedFile[fileID] == NULL){
                    printf("Close file failed!\n");
                    machine->WriteRegister(2, -1); // dong file ko duoc
                    IncreasePC();
                    return;
                }
                delete fileSystem->openedFile[fileID];
                fileSystem->openedFile[fileID] = NULL;
                printf("Close file compeletely!\n"); // dong file thanh cong
                machine->WriteRegister(2, 0);
                IncreasePC();
                return;
            }
            case SC_ReadFile:
            {
                int virtAddr = machine->ReadRegister(4);
                int sizeAddr = machine->ReadRegister(5);
                int fileID = machine->ReadRegister(6);
                int oldPos;
                int curPos;
                //check index file co nam trong vung gioi han hay ko
                if(fileID < 0 || fileID > 10){
                    //neu dieu kien nay dung thi ko the doc file
                    machine->WriteRegister(2, -1);
                    IncreasePC();
                    return;
                }    
                //check file voi tham so index coi file do co dang duoc mo hay ko
                if(fileSystem->openedFile[fileID] == NULL){
                    //neu file dang ko duoc mo thi ko the doc file
                    machine->WriteRegister(2, -1);
                    IncreasePC();
                    return;
                }
                //check file co phai la file stdout hay ko
                if(fileSystem->openedFile[fileID]->type == 3){
                    //neu file la file stdout thi ko the doc file
                    machine->WriteRegister(2, -1);
                    IncreasePC();
                    return;
                }

                char *buf = new char[MaxBuffer];
                buf = User2System(virtAddr, sizeAddr);
                //check truong hop la file stdin
                if(fileSystem->openedFile[fileID]->type == 2){
                    int sizeRead = gSynchConsole->Read(buf, sizeAddr);
                    System2User(virtAddr, sizeRead, buf);
                    machine->WriteRegister(2, sizeRead);
                    IncreasePC();
                    delete[] buf;
                    return;
                }
                
                oldPos = fileSystem->openedFile[fileID]->getCurrentOffset();
                //check truong hop doc file binh thuong
                if(fileSystem->openedFile[fileID]->Read(buf, sizeAddr) > 0){//kiem tra xem day co phai la file trong hay ko
                    curPos = fileSystem->openedFile[fileID]->getCurrentOffset();
                    System2User(virtAddr, curPos-oldPos, buf); 
                    machine->WriteRegister(2, curPos-oldPos);
                    IncreasePC();
                } else{ // file trong ko doc duoc gi
                    machine->WriteRegister(2, -2);
                    IncreasePC();
                }
                delete[] buf;
                IncreasePC();
                return;
            }
            case SC_WriteFile:
            {
                int virtAddr = machine->ReadRegister(4);
                int sizeAddr = machine->ReadRegister(5);
                int fileID = machine->ReadRegister(6);
                char *buf = new char[MaxBuffer];
                int oldPos;
                int curPos;
                //check index file co nam trong vung gioi han hay ko
                if(fileID < 0 || fileID > 10){
                    //neu dieu kien nay dung thi ko the ghi file
                    machine->WriteRegister(2, -1);
                    delete[] buf;
                    IncreasePC();
                    return;
                }    
                //check file voi tham so index coi file do co dang duoc mo hay ko
                if(fileSystem->openedFile[fileID] == NULL){
                    //neu file dang ko duoc mo thi ko the ghi file
                    machine->WriteRegister(2, -1);
                    delete[] buf;
                    IncreasePC();
                    return;
                }
                buf = User2System(virtAddr, sizeAddr);
                oldPos = fileSystem->openedFile[fileID]->getCurrentOffset();
                //check coi type cua file do co the ghi hay ko
                //type = 0 => read-only, type = 3 => stdin
                if(fileSystem->openedFile[fileID]->type == 0 || fileSystem->openedFile[fileID]->type == 3){
                    //trong truong hop type cua file dang la 0: read-onnly hoac type = 3 la stdin 
                    //thi ko the ghi file
                    machine->WriteRegister(2, -1);
                    delete[] buf;
                    IncreasePC();
                    return;
                } 
                if(fileSystem->openedFile[fileID]->type == 1){
                    //trong truong hop type = 1: file co the read/write
                    if((fileSystem->openedFile[fileID]->Write(buf, sizeAddr)) > 0){
                        curPos = fileSystem->openedFile[fileID]->getCurrentOffset();
                        machine->WriteRegister(2, curPos-oldPos); // tra ve so byte da ghi vao file
                        delete[] buf;
                        IncreasePC();
                        return;
                    }
                } 
                if(fileSystem->openedFile[fileID]->type == 2){
                    //trong truong hop type = 2: stdout ghi vao console
                    int i = 0;
                    while(buf[i] != 0 && buf[i] != '\n'){
                        gSynchConsole->Write(buf+i, 1); // ghi tung byte 1
                        i++;
                    }
                    buf[i] = '\n';
                    gSynchConsole->Write(buf+i, 1); // them ki tu xuong dong vao cuoi file
                    machine->WriteRegister(2, i-1); //tra ve so da ghi vao file
                    delete[] buf;
                    IncreasePC();
                    return;
                }
            }
            case SC_SeekFile:
            {
                int pos = machine->ReadRegister(4);
                int fileID = machine->ReadRegister(5);
                //check index file co nam trong vung gioi han hay ko
                if(fileID < 0 || fileID > 10){
                    //neu dieu kien nay dung thi ko the ghi file
                    printf("Can't seek this file\n");
                    machine->WriteRegister(2, -1);
                    IncreasePC();
                    return;
                }    
                if(fileID == 0 || fileID == 1){
                    //check de ko goi SeekFile tren man hinh console
                    printf("Can't seek this file\n");
                    machine->WriteRegister(2, -1);
                    IncreasePC();
                    return;
                }
                //check file voi tham so index coi file do co dang duoc mo hay ko
                if(fileSystem->openedFile[fileID] == NULL){
                    //neu file dang ko duoc mo thi ko the ghi file
                    printf("Can't seek this file2\n");
                    machine->WriteRegister(2, -1);
                    IncreasePC();
                    return;
                }
                //neu pos = -1 thi seek toi cuoi file
                if(pos == -1){
                    pos = fileSystem->openedFile[fileID]->Length();
                }
                if(pos > fileSystem->openedFile[fileID]->Length() || pos < 0){
                    printf("Can't seek this file2\n");
                    machine->WriteRegister(2, -1);
                    IncreasePC();
                    return;
                }
                fileSystem->openedFile[fileID]->Seek(pos);
                machine->WriteRegister(2, pos);
                IncreasePC();
                return;
            }
            case SC_RemoveFile:
            {                
                int virtAddr = machine->ReadRegister(4);
                char* filename = new char[MaxBuffer];
                filename = User2System(virtAddr, MaxBuffer); 
                fileSystem->Remove(filename);
                machine->WriteRegister(2, 0);
                IncreasePC();
                return;
            }
            break;
        }
    case PageFaultException:
    {
        DEBUG('a', "\n No valid translation found.");
        interrupt->Halt();
        break;
    }
    case ReadOnlyException:
    {
        DEBUG('a', "\nWrite attempted to page marked.");
        interrupt->Halt();
        break;
    }
    case BusErrorException:
    {
        DEBUG('a', "\nTranslation resulted in an invalid physical address");
        interrupt->Halt();
        break;
    }
    case AddressErrorException:
    {
        DEBUG('a', "\nUnaligned reference or one that was beyond the end of the address space");
        interrupt->Halt();
        break;
    }
    case OverflowException:
    {
        DEBUG('a', "Integer overflow in add or sub");
        interrupt->Halt();
        break;
    }
    case IllegalInstrException:
    {
        DEBUG('a', "\nUnimplemented or reserved instr.");
        interrupt->Halt();
        break;
    }
    case NumExceptionTypes:
    {
        DEBUG('a', "\ninvalid exceprion.");
        interrupt->Halt();
        break;
    }
    default:
        DEBUG('a', "no exception.");
        interrupt->Halt();
}
}