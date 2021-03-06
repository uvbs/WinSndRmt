//application entry point
//command line pharsing
#define WIN32_LEAN_AND_MEAN

#include <stdio.h>

#include "SrvApp.h"
#include "SrvNetwrk.h"
#include "SrvAudio.h"

int PharseCmdLineArgs(char *lpCmdLine, char **ipAddress, int *port){
	int argn = 0;
	char *next_token;
	char *token = strtok_s(lpCmdLine, " ", &next_token);
	while(token){
		//use the token
		if(argn == 0) *ipAddress = token;
		else if(argn == 1){
			if(isdigit(token[0])){
				int portNo = atoi(token);
				if(portNo > 1000)
					*port = portNo;
			}
		}
		else if(argn == 2) break;
		argn++;
		token = strtok_s(NULL, " ", &next_token);
	}
	return argn;
}


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nCmdShow)
{
	//-----------------------------------------
    // Declare and initialize variables
    int iResult = 0;
    BOOL bResult = TRUE;
    HRESULT hr = S_OK;

	CSrvAudio volAudio;
	CSrvApp myAppLogic;
	CSrvNetwrk srvNetork(volAudio, myAppLogic);

    WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
	DWORD EventTotal = 0;

    DWORD Index;

    if (hPrevInstance)
    {
        return 0;
    }
	char *ipAddress = "0.0.0.0"; //all interfaces
	int port = 27015;
	PharseCmdLineArgs(lpCmdLine, &ipAddress, &port);

	hr = volAudio.Initialize(&srvNetork);
	if(FAILED(hr)) return -1;

	hr = srvNetork.Initialize(ipAddress, port);
	if(FAILED(hr)) return -1;

	hr = srvNetork.CreateEvents(EventArray, EventTotal);
	if(FAILED(hr)) return -1;

	//accept loop
	while (1) {
	  hr = srvNetork.Accept();
	  if(FAILED(hr)) return -1;

	  volAudio.PostVolValToClient();
	  srvNetork.ReceiveAsync();

      //-----------------------------------------
      // client loop Process overlapped receives on the socket
      while (1) {

        //-----------------------------------------
        // Wait for the overlapped I/O call to complete
        Index = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE, WSA_INFINITE, FALSE);

        //-----------------------------------------
        // Reset the signaled event
        bResult = WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);
        if (bResult == FALSE) {
			OutputDebugString("WSAResetEvent failed with error \n");
        }

		if((Index - WSA_WAIT_EVENT_0) == 0)
		{
			//read operation completed
			if( srvNetork.OnDataReceived() < 0 )
				//break out of the client loop and go find another client
				break;
		}
		else if((Index - WSA_WAIT_EVENT_0) == 1)
		{
			//write operation completed
			srvNetork.OnDataSent();
		}
		else if((Index - WSA_WAIT_EVENT_0) == 2)
		{
			//write operation completed
			srvNetork.OnSndEvent();
		}
	  }
	  // client disconnected close socket
	  srvNetork.CloseClientSocket();
	}

    return 0;
}
