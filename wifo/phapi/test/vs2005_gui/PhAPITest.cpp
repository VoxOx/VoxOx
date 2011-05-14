// PhAPITest.cpp : définit le point d'entrée pour l'application.
//

#include "stdafx.h"
#include "PhAPITest.h"

#include <phapi.h>
#include <owpl_plugin.h>

#define MAX_LOADSTRING 100

phCallbacks_t myPhCb;
OWPL_LINE gVline;
OWPL_CALL gCall;

// Variables globales :
HINSTANCE hInst;								// instance actuelle
TCHAR szTitle[MAX_LOADSTRING];					// Le texte de la barre de titre
TCHAR szWindowClass[MAX_LOADSTRING];			// le nom de la classe de fenêtre principale

// Pré-déclarations des fonctions incluses dans ce module de code :
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO : placez ici le code.
	MSG msg;
	HACCEL hAccelTable;

	// Initialise les chaînes globales
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_PHAPITEST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Effectue l'initialisation de l'application :
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PHAPITEST));

	// Boucle de messages principale :
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FONCTION : MyRegisterClass()
//
//  BUT : inscrit la classe de fenêtre.
//
//  COMMENTAIRES :
//
//    Cette fonction et son utilisation sont nécessaires uniquement si vous souhaitez que ce code
//    soit compatible avec les systèmes Win32 avant la fonction 'RegisterClassEx'
//    qui a été ajoutée à Windows 95. Il est important d'appeler cette fonction
//    afin que l'application dispose des petites icônes correctes qui lui sont
//    associées.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PHAPITEST));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_PHAPITEST);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FONCTION : InitInstance(HINSTANCE, int)
//
//   BUT : enregistre le handle de l'instance et crée une fenêtre principale
//
//   COMMENTAIRES :
//
//        Dans cette fonction, nous enregistrons le handle de l'instance dans une variable globale, puis
//        créons et affichons la fenêtre principale du programme.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Stocke le handle d'instance dans la variable globale

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


void  callProgress(int cid, const phCallStateInfo_t *info)
{

}

void  transferProgress (int cid, const phTransferStateInfo_t *info)
{
}

void  confProgress(int cfid, const phConfStateInfo_t *info)
{
	
}

void  regProgress(int regid, int regStatus)
{
	int i = 0;
}
void  msgProgress(int mid,  const phMsgStateInfo_t *info)
{
}

void  onNotify(const char* event, const char* from, const char* content)
{
}

void  subscriptionProgress(int sid,  const phSubscriptionStateInfo_t *info)
{
}
  
void  errorNotify(enum phErrors error)
{
}

void  debugTrace (const char * message)
{

}



void InitPhAPI()
{
	//myPhCb.callProgress = callProgress;
	//myPhCb.transferProgress = transferProgress;
	//myPhCb.confProgress = confProgress;
	//myPhCb.regProgress = regProgress;
	//myPhCb.msgProgress = msgProgress;
	//myPhCb.onNotify = onNotify;
	//myPhCb.subscriptionProgress = subscriptionProgress;
	//myPhCb.errorNotify = errorNotify;
	//myPhCb.debugTrace = debugTrace;

	owplConfigAddAudioCodecByName("PCMU/8000");
	owplConfigAddAudioCodecByName("PCMA/8000");

	owplAudioSetConfigString("IN=0 OUT=0");

	owplInit(5060, 0, 0, NULL, 0); 
	//phInit(&myPhCb, NULL, 1);
}

void RegisterNewLine()
{
	int n = 200;
	phAddAuthInfo("wx_wengo_9","wx_wengo_9", "testwengo", NULL, "voip.wengo.fr");
	owplLineAdd("wx_wengo_9","wx_wengo_9","voip.wengo.fr", "213.91.9.210",  3500, &gVline);
	owplLineSetOpts(gVline, OWPL_LINE_OPT_REG_TIMEOUT, &n);
	owplLineRegister(gVline, 1);
}

int FileTransferReceivedHander(const char * FileName, const char * From, int FileSize)
{
	MessageBoxA(0, FileName, From, 0);
	return 0;
}

int MyEventHandler(OWPL_EVENT_CATEGORY category, 
                                         void* pInfo, 
                                         void* pUserData)
{
	if (category == EVENT_CATEGORY_LINESTATE)
	{
		OWPL_LINESTATE_INFO * LineInfo = (OWPL_LINESTATE_INFO *) pInfo;
		int i = 0;
	}
	else if (category == EVENT_CATEGORY_CALLSTATE)
	{
		OWPL_CALLSTATE_INFO * CallInfo = (OWPL_CALLSTATE_INFO *) pInfo;
		switch (CallInfo->event) {
			case CALLSTATE_OFFERING:
				owplCallAccept(CallInfo->hCall, 0);
				break;
			case CALLSTATE_ALERTING:
				owplCallAnswer(CallInfo->hCall, OWPL_STREAM_AUDIO);
				break;
		}
		int i = 0;
	}
	return 0;
}

//
//  FONCTION : WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  BUT :  traite les messages pour la fenêtre principale.
//
//  WM_COMMAND	- traite le menu de l'application
//  WM_PAINT	- dessine la fenêtre principale
//  WM_DESTROY	- génère un message d'arrêt et retourne
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	int i = 0;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Analyse les sélections de menu :
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_KEYUP:
		if (VK_NUMPAD0 == LOWORD(wParam)) {
			InitPhAPI();
			owplEventListenerAdd(MyEventHandler, 0);
		}
		else if (VK_NUMPAD1== LOWORD(wParam)) {			
			RegisterNewLine();
		}
		else if (VK_NUMPAD2== LOWORD(wParam)) {
			owplCallCreate(gVline, &gCall);
			owplCallConnect(gCall,"sip:333@voip.wengo.fr", OWPL_STREAM_AUDIO );
		}
		else if (VK_NUMPAD3== LOWORD(wParam)) {
			owplCallDisconnect(gCall);
		}
		else if (VK_NUMPAD4== LOWORD(wParam)) {
			owplPresencePublish(gVline,  1, "Some status message", 0);
		}
		else if (VK_NUMPAD5== LOWORD(wParam)) {			
			owplPresenceSubscribe(gVline,  "sip:minhpq@voip.wengo.fr", 0, &i);
		}
		else if (VK_NUMPAD8== LOWORD(wParam)) {
			int retVal = -1;
			char tmp[200];
			owplPluginCallFunction(&retVal, "MyTestPlugin", "fix_nated_sdp", "%d%s%d%s",100, "First string", 200,"seconde string");
			int i = owplPluginGetParam(tmp, sizeof(tmp), "MyTestPlugin", "rtpproxy_sock");
			i++;

		}
		else if (VK_NUMPAD9== LOWORD(wParam)) {
			owplPluginLoad("TestPhPlugin.dll");
			owplPluginSetCallback("MyTestPlugin", "FileTransferReveiced", (OWPL_PLUGIN_CALLBACK*)FileTransferReceivedHander);
		}		
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO : ajoutez ici le code de dessin...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Gestionnaire de messages pour la boîte de dialogue À propos de.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
