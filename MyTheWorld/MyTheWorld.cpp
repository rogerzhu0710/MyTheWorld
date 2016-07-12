// MyTheWorld.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include <windows.h>
                                                                                                                                                                                    
// set event for resource
using namespace std;

void MoveOutputToPos(int x,int y,char* c,bool bString);

HANDLE hOut=GetStdHandle(STD_OUTPUT_HANDLE);

struct Planet
{
    int type;
    int resource;
    int speed;
    int index;
};

CRITICAL_SECTION cs;

const struct Planet& InitPlanet(int type, int resource,int speed,int index);
void PlanetSpin(const struct Planet &p,int spinInterval,int picType);
void PlanetMove(struct Planet &p);
void UpdatePlanetResource(struct Planet &p);

DWORD WINAPI ThreadProc(LPVOID lpParameter);
DWORD WINAPI UpdateResourceProc(LPVOID lpParameter);
int nMaxResource = 0;
HANDLE hEvent;
bool bFin;

HANDLE ghMutex; 

int _tmain(int argc, _TCHAR* argv[])
{  
    CONSOLE_SCREEN_BUFFER_INFO bInfo; 
    GetConsoleScreenBufferInfo(hOut, &bInfo );
 
    bFin = false;
    hEvent = CreateEvent( NULL , FALSE , FALSE , NULL );

    InitializeCriticalSection(&cs);

    nMaxResource = bInfo.dwSize.X;
    struct Planet homePlanet = InitPlanet(1,nMaxResource,0,0); 
    struct Planet Planet1 = InitPlanet(0,0,1,1); 
    struct Planet Planet2 = InitPlanet(0,0,1,2); 

    int nSpinType = 0;
    HANDLE  hThread1 = CreateThread(0, 
                             16 * 1024L, 
                             ThreadProc, 
                             &Planet1, 
                             STACK_SIZE_PARAM_IS_A_RESERVATION, 
                             NULL);
    /*
    HANDLE  hThreadUpdate = CreateThread(0, 
                             16 * 1024L, 
                             UpdateResourceProc, 
                             &homePlanet, 
                             STACK_SIZE_PARAM_IS_A_RESERVATION, 
                             NULL);
    */
    ghMutex = CreateMutex(NULL,FALSE,NULL);             
   
    while(true)
    {
        PlanetSpin(homePlanet,80,nSpinType);  
        nSpinType++;
        DWORD dStatus = WaitForSingleObject(hEvent,5000);
        if ( dStatus == WAIT_OBJECT_0 )
        {
            homePlanet.resource -= 1;
            UpdatePlanetResource(homePlanet);
            // ResetEvent(hEvent); If the senconde para. is TRUE
        }
    } 
     
    CloseHandle(hThread1);
    //CloseHandle(hThreadUpdate);
    CloseHandle(hEvent);
    CloseHandle(ghMutex);
    DeleteCriticalSection(&cs);
    system("pause");
	return 0;
}

void UpdatePlanetResource(struct Planet &p)
{ 
    char cRes[3];
    itoa(p.resource,cRes,10);

    if( p.type == 1 ) 
    { 
        MoveOutputToPos(2,0,cRes,true); 
    }
    else
    {
        MoveOutputToPos(1,p.index*3,cRes,true);  
    } 
}
 
void PlanetMove(struct Planet &p)
{  
    char cLine[100];
    memset(cLine,0,100*sizeof(char)); 

    if( p.resource < nMaxResource-1 ) 
    {
        p.resource++;
        memset(cLine,'-',p.resource*sizeof(char));
        //p.resource = 0;
        //memset(cLine,' ',nMaxResource*sizeof(char)); 
        MoveOutputToPos(0,p.index*3+1,cLine,true); 
        
        UpdatePlanetResource(p);
        SetEvent(hEvent);

    }  
   
}

void PlanetSpin(const struct Planet &p,int spinInterval,int picType)
{
    char spin[4] = {'|','/','-','\\'}; 

    if( p.type == 1 )
    {   
        MoveOutputToPos(0,0,&spin[picType%4],false); 
        MoveOutputToPos(0,1,&spin[picType%4],false); 
        MoveOutputToPos(1,0,&spin[picType%4],false); 
        MoveOutputToPos(1,1,&spin[picType%4],false);
 
        ::Sleep(spinInterval);
    }
    else
    { 
        char sep = '*';
        
        MoveOutputToPos(0,p.index*3-1,&sep,false);  
        MoveOutputToPos(0,p.index*3,&sep,false);    
        

        MoveOutputToPos(p.resource,p.index*3+1,&spin[picType%4],false); 
        
        ::Sleep(spinInterval);
    }
}

const struct Planet& InitPlanet(int type, int resource,int speed,int index)
{
    struct Planet p;
    p.type = type;
    p.resource = resource;
    p.speed = speed;
    p.index = index;

    return p;
}

void MoveOutputToPos(int x,int y,char* c,bool bString)
{     
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo; 
    csbiInfo.dwCursorPosition.X = x;
    csbiInfo.dwCursorPosition.Y = y; 

   
#if 0
    EnterCriticalSection(&cs);
    if (!SetConsoleCursorPosition(hOut,csbiInfo.dwCursorPosition)) 
    {
        printf("SetConsoleCursorPosition error!!!!! \r\n"); 
        LeaveCriticalSection(&cs);
        return;
    }

    if(!bString)
        printf("%c",c[0]); 
    else
        printf("%s",c);
    LeaveCriticalSection(&cs);
#endif

    DWORD dStatus = WaitForSingleObject(ghMutex,1000*30);
    if ( dStatus == WAIT_OBJECT_0 )
    {
        if (!SetConsoleCursorPosition(hOut,csbiInfo.dwCursorPosition)) 
        {
            printf("SetConsoleCursorPosition error!!!!! \r\n"); 
            ReleaseMutex(ghMutex);
            return;
        }

        if(!bString)
            printf("%c",c[0]); 
        else
            printf("%s",c);
        ReleaseMutex(ghMutex);
    }
    else
    {
        ReleaseMutex(ghMutex);
    }

}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    struct Planet* p = (struct Planet*)lpParameter;
    int nSpinType = 0;

  
    while(true)
    {
        PlanetMove(*p);
        PlanetSpin(*p,500,nSpinType);
        nSpinType++; 
    }
        
    return 1;
}

DWORD WINAPI UpdateResourceProc(LPVOID lpParameter)
{
    struct Planet* p = (struct Planet*)lpParameter; 
    while(true)
    {
        DWORD dStatus = WaitForSingleObject(hEvent,1000*30);
        if ( dStatus == WAIT_OBJECT_0 )
        {
            p->resource -= 1;
            UpdatePlanetResource(*p);
        }
    }
    return 1;
}