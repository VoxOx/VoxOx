// SFPTest.cpp : définit le point d'entrée pour l'application.
//

#include "stdafx.h"
#include "SFPTest.h"

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
	LoadString(hInstance, IDC_SFPTEST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Effectue l'initialisation de l'application :
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SFPTEST));

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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SFPTEST));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_SFPTEST);
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
	owplConfigAddAudioCodecByName("PCMU/8000");
	owplConfigAddAudioCodecByName("PCMA/8000");

	owplAudioSetCallInputDevice(0);
	owplAudioSetCallOutputDevice(0);

}

void RegisterNewLine(int wx_wengo)
{
	int n = 200;
	if(wx_wengo == 1) {
		phAddAuthInfo("sfp01","sfp01", "sfp01", NULL, "voip.wengo.fr");
		owplLineAdd("sfp01","sfp01","voip.wengo.fr", "80.118.132.87:5061",  3500, &gVline);
	} else if(wx_wengo == 10) {
		phAddAuthInfo("sfp10","sfp10", "sfp10", NULL, "voip.wengo.fr");
		owplLineAdd("sfp10","sfp10","voip.wengo.fr", "80.118.132.87:5061",  3500, &gVline);
	}
	owplLineSetOpts(gVline, OWPL_LINE_OPT_REG_TIMEOUT, &n);
	owplLineRegister(gVline, 1);
}


int inviteToTransferHandler(int cid, char * uri, char * short_filename, char * file_type, char * file_size) {
	gCall = cid;
	MessageBoxA(0, short_filename, uri, 0);
	return 0;
}


int newIncomingFileHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size) {
	gCall = cid;
	MessageBoxA(0, short_filename, username, 0);
	return 0;
}

int waitingForAnswerHandler(int cid, char * uri) {
	gCall = cid;
	MessageBoxA(0, "Waiting", uri, 0);
	return 0;
}


int transferCancelledHandler(int cid, char * short_filename, char * file_type, char * file_size) {
	gCall = cid;
	MessageBoxA(0, short_filename, "cancelled", 0);
	return 0;
}


int transferCancelledByPeerHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size) {
	gCall = cid;
	MessageBoxA(0, short_filename, "cancelled by peer", 0);
	return 0;
}


int sendingFileBeginHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size) {
	gCall = cid;
	MessageBoxA(0, short_filename, "send began", 0);
	return 0;
}


int receivingFileBeginHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size) {
	gCall = cid;
	MessageBoxA(0, short_filename, "receive began", 0);
	return 0;
}


int transferClosedByPeerHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size) {
	gCall = cid;
	return 0;
}


int transferClosedHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size) {
	gCall = cid;
	return 0;
}


int transferFromPeerFinishedHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size) {
	gCall = cid;
	MessageBoxA(0, short_filename, "from peer done", 0);
	return 0;
}


int transferToPeerFinishedHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size) {
	gCall = cid;
	MessageBoxA(0, short_filename, "to peer done", 0);
	return 0;
}


int transferFromPeerFailedHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size) {
	gCall = cid;
	MessageBoxA(0, short_filename, "from peer failed", 0);
	return 0;
}


int transferToPeerFailedHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size) {
	gCall = cid;
	MessageBoxA(0, short_filename, "to peer failed", 0);
	return 0;
}


int transferFromPeerStoppedHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size) {
	gCall = cid;
	return 0;
}


int transferToPeerStoppedHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size) {
	gCall = cid;
	return 0;
}


int transferProgressionHandler(int cid, int percentage) {
	gCall = cid;
	return 0;
}


int transferPausedByPeerHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size) {
	gCall = cid;
	MessageBoxA(0, short_filename, "paused by peer", 0);
	return 0;
}


int transferPausedHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size) {
	gCall = cid;
	MessageBoxA(0, short_filename, "paused", 0);
	return 0;
}


int transferResumedByPeerHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size) {
	gCall = cid;
	MessageBoxA(0, short_filename, "resumed by peer", 0);
	return 0;
}


int transferResumedHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size) {
	gCall = cid;
	MessageBoxA(0, short_filename, "resumed", 0);
	return 0;
}


int peerNeedUpgradeHandler(void) {
	return 0;
}

int needUpgradeHandler(void) {
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
			int i = 0;
			InitPhAPI();
			i += owplPluginLoad("sfp-plugin.dll");
			i += owplPluginSetCallback("SFPPlugin", "inviteToTransfer", (owplPS_CommandProc)inviteToTransferHandler);
			i += owplPluginSetCallback("SFPPlugin", "newIncomingFile", (owplPS_CommandProc)newIncomingFileHandler);
			i += owplPluginSetCallback("SFPPlugin", "waitingForAnswer", (owplPS_CommandProc)waitingForAnswerHandler);
			i += owplPluginSetCallback("SFPPlugin", "transferCancelled", (owplPS_CommandProc)transferCancelledHandler);
			i += owplPluginSetCallback("SFPPlugin", "transferCancelledByPeer", (owplPS_CommandProc)transferCancelledByPeerHandler);
			i += owplPluginSetCallback("SFPPlugin", "sendingFileBegin", (owplPS_CommandProc)sendingFileBeginHandler);
			i += owplPluginSetCallback("SFPPlugin", "receivingFileBegin", (owplPS_CommandProc)receivingFileBeginHandler);
			i += owplPluginSetCallback("SFPPlugin", "transferClosedByPeer", (owplPS_CommandProc)transferClosedByPeerHandler);
			i += owplPluginSetCallback("SFPPlugin", "transferClosed", (owplPS_CommandProc)transferClosedHandler);
			i += owplPluginSetCallback("SFPPlugin", "transferFromPeerFinished", (owplPS_CommandProc)transferFromPeerFinishedHandler);
			i += owplPluginSetCallback("SFPPlugin", "transferToPeerFinished", (owplPS_CommandProc)transferToPeerFinishedHandler);
			i += owplPluginSetCallback("SFPPlugin", "transferFromPeerFailed", (owplPS_CommandProc)transferFromPeerFailedHandler);
			i += owplPluginSetCallback("SFPPlugin", "transferToPeerFailed", (owplPS_CommandProc)transferToPeerFailedHandler);
			i += owplPluginSetCallback("SFPPlugin", "transferFromPeerStopped", (owplPS_CommandProc)transferFromPeerStoppedHandler);
			i += owplPluginSetCallback("SFPPlugin", "transferToPeerStopped", (owplPS_CommandProc)transferToPeerStoppedHandler);
			i += owplPluginSetCallback("SFPPlugin", "transferProgression", (owplPS_CommandProc)transferProgressionHandler);
			i += owplPluginSetCallback("SFPPlugin", "transferPausedByPeer", (owplPS_CommandProc)transferPausedByPeerHandler);
			i += owplPluginSetCallback("SFPPlugin", "transferPaused", (owplPS_CommandProc)transferPausedHandler);
			i += owplPluginSetCallback("SFPPlugin", "transferResumedByPeer", (owplPS_CommandProc)transferResumedByPeerHandler);
			i += owplPluginSetCallback("SFPPlugin", "transferResumed", (owplPS_CommandProc)transferResumedHandler);
			i += owplPluginSetCallback("SFPPlugin", "peerNeedUpgrade", (owplPS_CommandProc)peerNeedUpgradeHandler);
			i += owplPluginSetCallback("SFPPlugin", "needUpgrade", (owplPS_CommandProc)needUpgradeHandler);
			i += owplPluginSetParam("11000", 5, "SFPPlugin", "sfp_file_transfer_port");
			i += owplPluginSetParam("tcp", 3, "SFPPlugin", "sfp_default_ip_protocol");
		}
		else if (VK_NUMPAD1== LOWORD(wParam)) {			
			owplInit(5060, 0, 0, NULL, 0); 
			int i = owplEventListenerAdd(MyEventHandler, 0);
			RegisterNewLine(1);
		}
		else if (VK_NUMPAD2== LOWORD(wParam)) {

		}
		else if (VK_NUMPAD3== LOWORD(wParam)) {
			owplInit(5061, 0, 0, NULL, 0); 
			int i = owplEventListenerAdd(MyEventHandler, 0);
			RegisterNewLine(10);

		}
		else if (VK_NUMPAD4== LOWORD(wParam)) {
			int retVal = -1;
			int i = owplPluginCallFunction(&retVal, "SFPPlugin", "sfp_send_file", "%d%s%s%s%s%s", gVline, "sip:sfp10@voip.wengo.fr", "C:\\test01.txt", "test01.txt", "TXT", "26476544");
			gCall = retVal;
		}
		else if (VK_NUMPAD5== LOWORD(wParam)) {			
			int retVal = -1;
			int i = owplPluginCallFunction(&retVal, "SFPPlugin", "sfp_receive_file", "%d%s", gCall, "C:\\received.txt");
		}
		else if (VK_NUMPAD6== LOWORD(wParam)) {
			int retVal = -1;
			int i = owplPluginCallFunction(&retVal, "SFPPlugin", "sfp_send_file", "%d%s%s%s%s%s", gVline, "sip:sfp01@voip.wengo.fr", "C:\\test10.txt", "test10.txt", "TXT", "26476544");
			gCall = retVal;
		}
		else if (VK_NUMPAD7== LOWORD(wParam)) {
			int retVal = -1;
			int i = owplPluginCallFunction(&retVal, "SFPPlugin", "sfp_pause_transfer", "%d", gCall);
		}
		else if (VK_NUMPAD8== LOWORD(wParam)) {
			int retVal = -1;
			int i = owplPluginCallFunction(&retVal, "SFPPlugin", "sfp_cancel_transfer", "%d", gCall);
		}
		else if (VK_NUMPAD9== LOWORD(wParam)) {
			int retVal = -1;
			int i = owplPluginCallFunction(&retVal, "SFPPlugin", "sfp_resume_transfer", "%d", gCall);
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
