
// facedetectionDlg.cpp : ʵ���ļ�
//
#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>  
#include <opencv2/imgproc/imgproc.hpp>  
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <afxwin.h>
#include <io.h>
#include <fcntl.h>
#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include "stdafx.h"
#include "conio.h"
#include "facedetection.h"
#include "facedetectionDlg.h"
#include "afxdialogex.h"
#include "facedetect-dll.h"
#include "BCnnAPI.h"
#include "HCNetSDK.h"
#include "plaympeg4.h"
#include "corecrt_io.h"
#include "fcntl.h"
#define  weight 1280
#define  height 720

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define DETECT_BUFFER_SIZE 0x20000
#define threshold_distance 2.3
#pragma comment(lib,"libfacedetect-x64.lib")

//define the buffer size. Do not change the size!

using namespace cv;
using namespace std;
CString FOLK[57];
CRITICAL_SECTION cs;
CRITICAL_SECTION cs1;
//Mat resize_roi_of_card;
//Mat resize_roi_of_camera;
bool brun;
bool brun1;

LONG nPort = -1;
volatile int gbHandling = 3;
struct Data //���ݰ�  
{
	int length;
	int count;
	char receivemessage[4000]; //������Ϣ  
	int fin;
};
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���
//unsigned char * pBuffer = (unsigned char *)malloc(DETECT_BUFFER_SIZE);
struct info {
	char buffer[256];//����
	char buffer1[256];//�Ա�
	char buffer2[256];//����
	char buffer3[256];//����
	char buffer4[256];//סַ
	char buffer5[256];//�������֤��
	char buffer6[256];//ǩ������
	char buffer7[256];//��Ч������
	char buffer8[256];//��Ч����ĩ
	char buffer9[256];
	//char receivemessage[40000];
};

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAboutDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CfacedetectionDlg �Ի���



CfacedetectionDlg::CfacedetectionDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_FACEDETECTION_DIALOG, pParent)
, match_result(_T(""))
, similarity(0)
, match_of_distance(0)
, m_strName(_T(""))
, m_strSex(_T(""))
, m_strFolk(_T(""))
, m_strBirth(_T(""))
, m_strAddr(_T(""))
, m_strIdnum(_T(""))
, m_strDep(_T(""))
, m_strBegin(_T(""))
, m_strEnd(_T(""))
, m_strTimer(_T(""))
, m_self_counts(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}
LRESULT CfacedetectionDlg::OnUpdateData(WPARAM wParam, LPARAM lParam)
{
	UpdateData(wParam);
	return 0;
}
void CfacedetectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MATCH_RESULT, match_result);
	DDX_Text(pDX, IDC_SIMILARITY, similarity);
	DDV_MinMaxDouble(pDX, similarity, 0.0, 1.0);
	DDX_Text(pDX, IDC_MATCH_DISTANCE, match_of_distance);
	DDV_MinMaxDouble(pDX, match_of_distance, 0, 2.0);
	DDX_Text(pDX, IDC_EDIT_NAME, m_strName);
	DDX_Text(pDX, IDC_EDIT_SEX, m_strSex);
	DDX_Text(pDX, IDC_EDIT_FOLK, m_strFolk);
	DDX_Text(pDX, IDC_EDIT_BIRTH, m_strBirth);
	DDX_Text(pDX, IDC_EDIT_ADDR, m_strAddr);
	DDX_Text(pDX, IDC_EDIT_IDNUM, m_strIdnum);
	DDX_Text(pDX, IDC_EDIT_DEP, m_strDep);
	DDX_Text(pDX, IDC_EDIT_DEGIN, m_strBegin);
	DDX_Text(pDX, IDC_EDIT_END, m_strEnd);
	DDX_Control(pDX, IDC_CARD, m_ctPicCard);
	DDX_Control(pDX, IDC_roi_of_card, m_ctCardroi);
	DDX_Text(pDX, IDC_TIMER, m_strTimer);
	DDX_Text(pDX, IDC_EDIT_SELF_COUNTS, m_self_counts);
}

BEGIN_MESSAGE_MAP(CfacedetectionDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CfacedetectionDlg::OnBnClickedButton1)
    ON_BN_CLICKED(IDC_BUTTON2_EXIT, &CfacedetectionDlg::OnBnClickedButton2Exit)
    ON_MESSAGE(WM_UPDATEDATA, OnUpdateData)
    ON_WM_TIMER()
//	ON_STN_CLICKED(IDC_Self_Counts, &CfacedetectionDlg::OnStnClickedSelfCounts)
END_MESSAGE_MAP()


// CfacedetectionDlg ��Ϣ�������
void InitConsoleWindow()
{
	AllocConsole();
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	int hCrt = _open_osfhandle((long)handle, _O_TEXT);
	FILE * hf = _fdopen(hCrt, "w");
	*stdout = *hf;
}

BOOL CfacedetectionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�
	InitConsoleWindow();  // add  
	_cprintf("str = %s\n ", "Debug output goes to terminal\n");
	//freopen("log.txt", "w", stdout);
	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	
	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	//m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��
	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	SetTimer(1, 1000, NULL);
	FOLK[1] = "����";//ע�� ����������� �����꣬����ֻ������
	FOLK[2] = "�ɹ���";
	FOLK[3] = "����";//ע�� ����������� �����꣬����ֻ������
	FOLK[4] = "����";//ע�� ����������� �����꣬����ֻ������
	FOLK[5] = "ά�����";
	FOLK[6] = "����";
	FOLK[7] = "����";
	FOLK[8] = "׳��";
	FOLK[9] = "����";
	FOLK[10] = "����";
	FOLK[11] = "����";
	FOLK[12] = "����";
	FOLK[13] = "������";//ע�� ����������� �����꣬����ֻ������
	FOLK[14] = "������";
	FOLK[15] = "��������";
	FOLK[16] = "����";//ע�� ����������� �����꣬����ֻ������
	FOLK[17] = "����";
	FOLK[18] = "���";
	FOLK[19] = "��ɽ��";//ע�� ����������� �����꣬����ֻ������
	FOLK[20] = "������";
	FOLK[21] = "ˮ��";
	FOLK[22] = "������";//ע�� ����������� �����꣬����ֻ������
	FOLK[23] = "������";
	FOLK[24] = "������";
	FOLK[25] = "���Ӷ���";
	FOLK[26] = "������";
	FOLK[27] = "Ǽ��";
	FOLK[28] = "������";
	FOLK[29] = "������";
	FOLK[30] = "ë����";
	FOLK[31] = "������";
	FOLK[32] = "������";
	FOLK[33] = "��������";
	FOLK[34] = "ŭ��";
	FOLK[35] = "���α����";
	FOLK[36] = "����˹��";
	FOLK[37] = "���¿���";
	FOLK[38] = "�°���";
	FOLK[39] = "������";
	FOLK[40] = "ԣ����";
	FOLK[41] = "������";
	FOLK[42] = "���״���";
	FOLK[43] = "������";
	FOLK[44] = "�Ű���";
	FOLK[45] = "�����";
	FOLK[46] = "��ŵ��";
	FOLK[47] = "������";
	FOLK[48] = "������";
	FOLK[49] = "������";
	FOLK[50] = "��������";
	FOLK[51] = "������";
	FOLK[52] = "����";
	FOLK[53] = "�¶�������";
	FOLK[54] = "������";
	FOLK[55] = "����";
	FOLK[56] = "����";
	CString ss;
	ss.Format("%s", "���������֤ϵͳ");
	CDialog::SetWindowText(ss);
	CFont *m_Font;
	CFont *m_Font1;
	m_Font = new CFont;
	m_Font1 = new CFont;
	m_Font->CreateFont(50, 20, 0, 0, 100,
		FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_SWISS, "����");
	m_Font1->CreateFont(20, 10, 0, 0, 100,
		FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_SWISS, "New Times Rom");
	CEdit *m_Edit = (CEdit *)GetDlgItem(IDC_MATCH_RESULT);
	m_Edit->SetFont(m_Font, FALSE);
	GetDlgItem(IDC_MATCH_RESULT)->SetFont(m_Font);

	CStatic *m_Static = (CStatic *)GetDlgItem(IDC_STATIC_RESULT);
	m_Static->SetFont(m_Font1, FALSE);
	GetDlgItem(IDC_STATIC_RESULT)->SetFont(m_Font1);
	//delete m_Font; //�����У����������С����
	CRect rect;
	CRect rect1;
	GetDlgItem(IDC_roi_of_face)->GetWindowRect(&rect);  //IDC_WAVE_DRAWΪPicture Control��ID  
	GetDlgItem(IDC_roi_of_card)->GetWindowRect(&rect1);
	ScreenToClient(&rect);
	ScreenToClient(&rect1);
	GetDlgItem(IDC_roi_of_face)->MoveWindow(rect.left, rect.top, 102, 126 , true);   //�̶�
	GetDlgItem(IDC_roi_of_card)->MoveWindow(rect1.left, rect1.top, 102, 126, true);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}
void CfacedetectionDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�
void CfacedetectionDlg::ReSize(void)
{
float fsp[2];  
    POINT Newp; //��ȡ���ڶԻ���Ĵ�С  
    CRect recta;      
    GetClientRect(&recta);     //ȡ�ͻ�����С    
    Newp.x=recta.right-recta.left;  
    Newp.y=recta.bottom-recta.top;  
    fsp[0]=(float)Newp.x/old.x;  
    fsp[1]=(float)Newp.y/old.y;  
    CRect Rect;  
    int woc;  
    CPoint OldTLPoint,TLPoint; //���Ͻ�  
    CPoint OldBRPoint,BRPoint; //���½�  
    HWND  hwndChild=::GetWindow(m_hWnd,GW_CHILD);  //�г����пؼ�    
    while(hwndChild)      
    {      
        woc=::GetDlgCtrlID(hwndChild);//ȡ��ID  
        GetDlgItem(woc)->GetWindowRect(Rect);    
        ScreenToClient(Rect);    
        OldTLPoint = Rect.TopLeft();    
        TLPoint.x = long(OldTLPoint.x*fsp[0]);    
        TLPoint.y = long(OldTLPoint.y*fsp[1]);    
        OldBRPoint = Rect.BottomRight();    
        BRPoint.x = long(OldBRPoint.x *fsp[0]);    
        BRPoint.y = long(OldBRPoint.y *fsp[1]);    
        Rect.SetRect(TLPoint,BRPoint);    
        GetDlgItem(woc)->MoveWindow(Rect,TRUE);  
        hwndChild=::GetWindow(hwndChild, GW_HWNDNEXT);      
    }  
    old=Newp;  
}  


void CfacedetectionDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
    // ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CfacedetectionDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CAboutDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CDialogEx::OnOK();
}

void CfacedetectionDlg::OnBnClickedLoadPicBut()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	/*CBitmap bitmap;  // CBitmap�������ڼ���λͼ   
	HBITMAP hBmp;    // ����CBitmap���ص�λͼ�ľ��   

	bitmap.LoadBitmap(IDB_BITMAP1);  // ��λͼIDB_BITMAP1���ص�bitmap   
	hBmp = (HBITMAP)bitmap.GetSafeHandle();  // ��ȡbitmap����λͼ�ľ��   
	card.SetBitmap(hBmp); */   // ����ͼƬ�ؼ�m_jzmPicture��λͼͼƬΪIDB_BITMAP1   
}

void CfacedetectionDlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	namedWindow("face", WINDOW_AUTOSIZE);
	HWND hWnd = (HWND)cvGetWindowHandle("face");
	HWND hParent = ::GetParent(hWnd);
	::SetParent(hWnd, GetDlgItem(IDC_face)->m_hWnd);
	::ShowWindow(hParent, SW_HIDE);

	namedWindow("roi_of_face", WINDOW_AUTOSIZE);
	HWND hWnd1 = (HWND)cvGetWindowHandle("roi_of_face");
	HWND hParent1 = ::GetParent(hWnd1);
	::SetParent(hWnd1, GetDlgItem(IDC_roi_of_face)->m_hWnd);
	::ShowWindow(hParent1, SW_HIDE);

	namedWindow("roi_of_card", WINDOW_AUTOSIZE);
	HWND hWnd2 = (HWND)cvGetWindowHandle("roi_of_card");
	HWND hParent2 = ::GetParent(hWnd2);
	::SetParent(hWnd2, GetDlgItem(IDC_roi_of_card)->m_hWnd);
	::ShowWindow(hParent2, SW_HIDE);

	namedWindow("card", WINDOW_AUTOSIZE);
	HWND hWnd3 = (HWND)cvGetWindowHandle("card");
	HWND hParent3 = ::GetParent(hWnd3);
	::SetParent(hWnd3, GetDlgItem(IDC_CARD)->m_hWnd);
	::ShowWindow(hParent3, SW_HIDE);
	///////////////////////////////////////////
	Mat camera_frame;//from camera
	Mat roi_of_camera;//roi of camera_frame
	Mat resize_roi_of_camera;//resize camera roi
	int resize_height = 126;
	int resize_width = 102;

	InitializeCriticalSection(&cs);//��ʼ���ṹ����

	pThread = AfxBeginThread(ThreadFunc, this);
	if (pThread == NULL)
	{
		AfxMessageBox("���ؿ���Ϣ�߳�����ʧ��!", MB_OK | MB_ICONERROR);
		return;
	}
	pThread1 = AfxBeginThread(ThreadFunc1, this);
	if (pThread1 == NULL)
	{
		AfxMessageBox("������ͷ�߳�����ʧ��!", MB_OK | MB_ICONERROR);
		return;
	}
	pThread2 = AfxBeginThread(ThreadFunc2, this);
	if (pThread2 == NULL)
	{
		AfxMessageBox("�����߳�����ʧ��!", MB_OK | MB_ICONERROR);
		return;
	}
}

void CfacedetectionDlg::OnBnClickedButton2Exit()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//CDialog::OnOK();
	OnCancel();
	exit(0);
}
char* load_port(char *str, int len) {
	FILE *fp;
	//char str[128];
	if ((fp = fopen("port.ini", "r")) == NULL) {
		_cprintf("cannot open file/n");
		exit(1);
	}
	while (!feof(fp)) {
		if (fgets(str, 128, fp) != NULL)
		{
			//	_cprintf("%s\n", str);
		}
	}
	fclose(fp);
	//return str;
}
UINT ThreadFunc(LPVOID pParm) {

		//EnterCriticalSection(&cs);
		CfacedetectionDlg *pDlg1 = (CfacedetectionDlg*)pParm;
		Mat card_frame; //from card
		Mat roi_of_card;//roi of card
		Mat resize_roi_of_card;//resize card roi
		int resize_height = 126;
		int resize_width = 102;
		Mat card_image;
		int port;
		WORD wVersionRequested;
		WSADATA wsaData;
		int ret, nLeft, length;
		SOCKET sListen, sServer; //�����׽��֣������׽���
		struct sockaddr_in saServer, saClient; //��ַ��Ϣ   
		char *ptr;//���ڱ�����Ϣ��ָ��   
				  //WinSock��ʼ��
		wVersionRequested = MAKEWORD(2, 2); //ϣ��ʹ�õ�WinSock DLL �İ汾
		ret = WSAStartup(wVersionRequested, &wsaData);
		if (ret != 0)
		{
			_cprintf("WSAStartup() failed!\n");		
			//	return;
		}
		//����Socket,ʹ��TCPЭ��
		sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sListen == INVALID_SOCKET)
		{
			WSACleanup();
			_cprintf("socket() faild!\n");
			//	return;
		}
		//�������ص�ַ��Ϣ
		char str[128];
		load_port(str, sizeof(str));
		port = atoi(str);
	//	_cprintf("%d\n", port);
		saServer.sin_family = AF_INET; //��ַ����
		saServer.sin_port = htons(/*SERVER_PORT*/port);//ע��ת��Ϊ�����ֽ���
		saServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY); //ʹ��INADDR_ANY ָʾ�����ַ
	//	_cprintf("port is %d\n", saServer.sin_port);
		//��
		ret = bind(sListen, (struct sockaddr *)&saServer, sizeof(saServer));
		if (ret == SOCKET_ERROR)
		{
			//_cprintf("bind() faild! code:%d\n", WSAGetLastError());
			_cprintf(".");
			closesocket(sListen); //�ر��׽���
			WSACleanup();
			Sleep(10);
			//	return;
		}

		//������������
		while (true) {
		ret = listen(sListen, 5);
		if (ret == SOCKET_ERROR)
		{
			_cprintf("listen() faild! code:%d\n", WSAGetLastError());
			closesocket(sListen); //�ر��׽���
		//	return;
		}
		_cprintf("Waiting for client connecting!\n");
		_cprintf("Tips: Ctrl+c to quit!\n");


	//	while (true) {
		length = sizeof(saClient);
		sServer = accept(sListen, (struct sockaddr *)&saClient, &length);
		if (sServer == INVALID_SOCKET)
		{
			_cprintf("accept() faild! code:%d\n", WSAGetLastError());
			closesocket(sListen); //�ر��׽���
			WSACleanup();
			continue;
			//		return;
		}
		char buffer[5000];
	//	char buffer1[50000];
		struct info recvInfo;
		struct Data data;
		data.fin = 0;
		int ret1 = recv(sServer, buffer, sizeof(buffer), 0);//ret1���ؽ��յ�����buffer�Ĵ�С
		_cprintf("%d\n", ret1);
			memcpy(&recvInfo, buffer, sizeof(buffer));
			pDlg1->m_strName = recvInfo.buffer;
			pDlg1->m_strSex = recvInfo.buffer1;
			pDlg1->m_strFolk = recvInfo.buffer2;
			pDlg1->m_strBirth = recvInfo.buffer3;
			pDlg1->m_strAddr = recvInfo.buffer4;
			pDlg1->m_strIdnum = recvInfo.buffer5;
			pDlg1->m_strDep = recvInfo.buffer6;
			pDlg1->m_strBegin = recvInfo.buffer7;
			pDlg1->m_strEnd = recvInfo.buffer8;

			FILE * fp1;
			int num= 0;
			if (!(fp1 = fopen("zp.bmp", "wb+")))
			{
				_cprintf("open zp.bmp error");
				//	exit(0);
			}
	//		Sleep(10);
	/*		memset(data.receivemessage, '0', sizeof(data.receivemessage));
			fwrite(data.receivemessage, 1024, 38, fp1);*/
			
			while (!data.fin)
			{
				memset(data.receivemessage, '0', sizeof(data.receivemessage));
				ret = recv(sServer, (char *)&data, sizeof(struct Data), 0);  //�ڶ�������ʹ��ǿ�����ͣ�Ϊһ�����ݰ�  
				if (ret == SOCKET_ERROR)
				{
					printf("recv() failed!\n");
					return 0;
				}
				if (ret == 0) //�ͻ����Ѿ��ر�����  
				{
					printf("Client has closed the connection\n");
					break;
				}
				if (data.length == 1024 && num!=37) {
					fwrite(data.receivemessage, 1024, 1, fp1);
//					_cprintf("1024--%d \n", data.length);
					num++;
//					_cprintf("%d\n", num);
//					_cprintf("%d\n", data.length);
				}
				if (num == 37 && data.length!=1024)
				{
					fwrite(data.receivemessage, data.length, 1, fp1);
					_cprintf("%d\n", data.length);
				}
//				_cprintf("%d \n", data.fin);
			}
			fclose(fp1);
			EnterCriticalSection(&cs);
			card_frame = imread("zp.bmp");
	//		if(card_frame.size().area== )
			cv::imshow("card", card_frame);
			Mat gray1;
			cvtColor(card_frame, gray1, CV_BGR2GRAY);
			int * pResults = NULL;
			unsigned char * pBuffer1 = (unsigned char *)malloc(DETECT_BUFFER_SIZE);
			pResults = facedetect_frontal(pBuffer1, (unsigned char*)(gray1.ptr(0)), gray1.cols, gray1.rows, gray1.step,
				1.2f, 2, 48);//5 24
		//	_cprintf("%d faces detected.\n", (pResults ? *pResults : 0));//�ظ�����  													   //print the detection results  %
			for (int i = 0; i < (pResults ? *pResults : 0); i++)
			{
				short * p = ((short*)(pResults + 1)) + 142 * i;
				int x = p[0];
				int y = p[1];
				int w = p[2];
				int h = p[3];
				//int neighbors = p[4];

				//	_cprintf("face_rect=[%d, %d, %d, %d], neighbors=%d\n", x, y, w, h, neighbors);
				//	Point left(x, y);
				//	Point right(x + w, y + h);
				//	rectangle(card_frame, left, right, Scalar(0, 255, 0), 2);//1,8,0
				Rect roi(x, y, (w + x > 102 ? (102 - x) : w), (y + h > 126 ? (126 - y) : h));
				//Rect roi(x, y, w, h);
				roi_of_card = card_frame(roi);
				roi_of_card = card_frame(roi).clone();
				cv::resize(roi_of_card, resize_roi_of_card, Size(resize_width, resize_height), (0, 0), (0, 0), INTER_LINEAR);
				if (resize_roi_of_card.rows != 0)
				{
					//imshow("roi_of_card", resize_roi_of_card);
					imwrite("card.bmp", resize_roi_of_card);
					LeaveCriticalSection(&cs);
					brun = 1;
				}
				else
				{
					continue;
				}
			}
			free(pBuffer1);
			//LeaveCriticalSection(&cs);
			Sleep(10);
	}
	ExitThread(0);
	return 0;
}

//����ص� ��ƵΪYUV����(YV12)����ƵΪPCM����
void CALLBACK DecCBFun(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2)
{
	if (gbHandling)
	{
		gbHandling--;
		return;
	}

	long lFrameType = pFrameInfo->nType;
	if (lFrameType == T_YV12)
	{
		//Mat camera_frame;//from camera
		Mat roi_of_camera;//roi of camera_frame
		Mat resize_roi_of_camera;//resize camera roi
		int resize_height = 126;
		int resize_width = 102;
			Mat /*pImg*/camera_frame(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC1);
			Mat src(pFrameInfo->nHeight + pFrameInfo->nHeight / 2, pFrameInfo->nWidth, CV_8UC1, pBuf);
			Mat gray;
			cvtColor(src, camera_frame, CV_YUV2BGR_YV12);
			//  Sleep(-1);
			cvtColor(camera_frame, gray, CV_BGR2GRAY);
			//	cvtColor(src, gray, CV_YUV2GRAY_YV12);
			int * pResults = NULL;
			unsigned char * pBuffer = (unsigned char *)malloc(DETECT_BUFFER_SIZE);
	   if (brun == 1) {
	//	   Sleep(10);
			pResults = facedetect_frontal(pBuffer, (unsigned char*)(gray.ptr(0)), gray.cols, gray.rows, gray.step,
				1.2f, 2, 48);
			//	_cprintf("%d faces detected.\n", (pResults ? *pResults : 0));//�ظ�����  
																		   //print the detection results  
			if ((pResults ? *pResults : 0) != 0) {
				int x = 0, y = 0, w = 0, h = 0;
				for (int i = 0; i < (pResults ? *pResults : 0); i++)
				{
					short * p = ((short*)(pResults + 1)) + 142 * i;
					int x1 = p[0];
					int y1 = p[1];
					int w1 = p[2];
					int h1 = p[3];
					if (i == 0) {
						w = w1;
						h = h1;
						x = x1;
						y = y1;
					}
					int neighbors = p[4];
					if (w1*h1 >= w*h) {
						w = w1;
						h = h1;
						x = x1;
						y = y1;
					}
				}
				_cprintf("face_rect=[%d, %d, %d, %d]\n", x, y, w, h);
				Point left(x, y);
				Point right(x + w, y + h);
				cv::rectangle(camera_frame, left, right, Scalar(0, 255, 0), 2);
				//imshow("face", camera_frame);
				Rect roi(x, y, (w + x > weight ? (weight - x) : w), (y + h > height ? (height - y) : h));
				roi_of_camera = camera_frame(roi);
				roi_of_camera = camera_frame(roi).clone();
				cv::resize(roi_of_camera, resize_roi_of_camera, Size(resize_width, resize_height), (0, 0), (0, 0), INTER_LINEAR);
				//			cv::imshow("roi_of_face", resize_roi_of_camera);
							//imwrite("camera.jpg", resize_roi_of_camera);
				if (resize_roi_of_camera.rows != 0 && roi_of_camera.rows != 0)
				{
					//imshow("roi_of_face", resize_roi_of_camera);
					EnterCriticalSection(&cs);
					imwrite("camera.jpg", resize_roi_of_camera);
					LeaveCriticalSection(&cs);
					brun1 = 1;
					//					_cprintf("%d\n", brun1);
				}
				else
				{
					//MessageBox("û�м�⵽����");
					//continue;
				}
			}
			free(pBuffer);
		cv::imshow("face", camera_frame);
		}
	}
		int delay = 30; // ms  
		int key = waitKey(delay);
	//	if (27 == key || 'Q' == key || 'q' == key)
			//break;
		//LeaveCriticalSection(&cs);
	gbHandling = 3;

}
///ʵʱ���ص�
void CALLBACK fRealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser)
{

	switch (dwDataType)
	{
	case NET_DVR_SYSHEAD: //ϵͳͷ

		if (!PlayM4_GetPort(&nPort))  //��ȡ���ſ�δʹ�õ�ͨ����
		{
			break;
		}
		//m_iPort = lPort; //��һ�λص�����ϵͳͷ������ȡ�Ĳ��ſ�port�Ÿ�ֵ��ȫ��port���´λص�����ʱ��ʹ�ô�port�Ų���
		if (dwBufSize > 0)
		{
			if (!PlayM4_SetStreamOpenMode(nPort, STREAME_REALTIME))  //����ʵʱ������ģʽ
			{
				break;
			}

			if (!PlayM4_OpenStream(nPort, pBuffer, dwBufSize, 1024 * 1024)) //�����ӿ�
			{
				break;
			}

			if (!PlayM4_Play(nPort, NULL)) //���ſ�ʼ
			{
				break;
			}
			if (!PlayM4_SetDecCallBack(nPort, DecCBFun))
			{
				break;
			}
		}
		break;
	case NET_DVR_STREAMDATA:   //��������
		if (dwBufSize > 0 && nPort != -1)
		{
			if (!PlayM4_InputData(nPort, pBuffer, dwBufSize))
			{
				cout << "error" << PlayM4_GetLastError(nPort) << endl;
				break;
			}
		}
		break;
	default: //��������
		if (dwBufSize > 0 && nPort != -1)
		{
			if (!PlayM4_InputData(nPort, pBuffer, dwBufSize))
			{
				break;
			}
		}
		break;
	}
}


void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
	char tempbuf[256] = { 0 };
	switch (dwType)
	{
	case EXCEPTION_RECONNECT:    //Ԥ��ʱ����
		_cprintf("----------reconnect--------%d\n", time(NULL));
		break;
	default:
		break;
	}
}
UINT ThreadFunc1(LPVOID pParm) {
	//---------------------------------------
	// ��ʼ��
	NET_DVR_Init();
	//��������ʱ��������ʱ��
	NET_DVR_SetConnectTime(2000, 1);
	NET_DVR_SetReconnect(10000, true);
	//---------------------------------------
	// ע���豸
	LONG lUserID;
	NET_DVR_DEVICEINFO_V30 struDeviceInfo;
	lUserID = NET_DVR_Login_V30("192.168.192.65", 8000, "admin", "q1w2e3r4", &struDeviceInfo);
	if (lUserID < 0)
	{
		_cprintf("Login error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Cleanup();
	//	return;
	}

	//---------------------------------------
	//�����쳣��Ϣ�ص�����
	NET_DVR_SetExceptionCallBack_V30(0, NULL, g_ExceptionCallBack, NULL);

	//---------------------------------------
	//����Ԥ�������ûص�������
	LONG lRealPlayHandle;
//	cvNamedWindow("Mywindow", 1);
//	cvNamedWindow("IPCamera", 1);

//	HWND  h = (HWND)cvGetWindowHandle("face");
//	HWND h = GetDlgItem(IDC_face)->m_hWnd;
	//if (h == 0)
	//{
	//	cout << "���ڴ���ʧ��" << endl;
	//}

	NET_DVR_PREVIEWINFO struPlayInfo = { 0 };
//	struPlayInfo.hPlayWnd = h;         //��ҪSDK����ʱ�����Ϊ��Чֵ����ȡ��������ʱ����Ϊ��
	struPlayInfo.lChannel = 1;           //Ԥ��ͨ����
	struPlayInfo.dwStreamType = 0;       //0-��������1-��������2-����3��3-����4���Դ�����
	struPlayInfo.dwLinkMode = 0;         //0- TCP��ʽ��1- UDP��ʽ��2- �ಥ��ʽ��3- RTP��ʽ��4-RTP/RTSP��5-RSTP/HTTP
	struPlayInfo.bBlocked = 1;

	lRealPlayHandle = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo, fRealDataCallBack, NULL);
	if (lRealPlayHandle < 0)
	{
		_cprintf("NET_DVR_RealPlay_V40 error\n");
		_cprintf("%d\n", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
//		return;
	}
#if 0
	VideoCapture cap(0);
	if (!cap.isOpened()) {

		// MessageBox(hWnd,"Cannot open camera!","camera", MB_OKCANCEL);
		//	return -1 ;
	}
	//	imshow("face", frame);
	for (;;) {
		//EnterCriticalSection(&cs);//����
		cap >> camera_frame;
			Mat gray;
			cvtColor(camera_frame, gray, CV_BGR2GRAY);

			int * pResults = NULL;
			unsigned char * pBuffer = (unsigned char *)malloc(DETECT_BUFFER_SIZE);
			pResults = facedetect_frontal(pBuffer, (unsigned char*)(gray.ptr(0)), gray.cols, gray.rows, gray.step,
				1.2f, 2, 48);
			_cprintf("%d faces detected.\n", (pResults ? *pResults : 0));//�ظ�����  
			//print the detection results  
			if ((pResults ? *pResults : 0) != 0) {
				int x=0, y=0, w=0, h=0;
				for (int i = 0; i < (pResults ? *pResults : 0); i++)
				{
					short * p = ((short*)(pResults + 1)) + 142 * i;
					int x1 = p[0];
					int y1 = p[1];
					int w1 = p[2];
					int h1 = p[3];
					if (i == 0) {
						w = w1;
						h = h1;
						x = x1;
						y = y1;
					}
					int neighbors = p[4];
					if (w1*h1 >= w*h) {
						w = w1;
						h = h1;
						x = x1;
						y = y1;
					}
				}
				_cprintf("face_rect=[%d, %d, %d, %d]\n", x, y, w, h);
				Point left(x, y);
				Point right(x + w, y + h);
				rectangle(camera_frame, left, right, Scalar(0, 255, 0), 2);
				//imshow("face", camera_frame);
				Rect roi(x, y, (w+x>640?(640-x):w), (y+h>480?(480-y):h));
				roi_of_camera = camera_frame(roi);
				roi_of_camera = camera_frame(roi).clone();
				resize(roi_of_camera, resize_roi_of_camera, Size(resize_width, resize_height), (0, 0), (0, 0), INTER_LINEAR);
				//imshow("roi_of_face", resize_roi_of_camera);
				//imwrite("camera.jpg", resize_roi_of_camera);
				if (resize_roi_of_camera.rows!=0 && roi_of_camera.rows!=0)
				{
					//imshow("roi_of_face", resize_roi_of_camera);
					EnterCriticalSection(&cs);
					imwrite("camera.jpg", resize_roi_of_camera);
					LeaveCriticalSection(&cs);
					brun1 = 1;
				}
				else
				{
					//MessageBox("û�м�⵽����");
				continue;
				}
			}
			free(pBuffer);	
		imshow("face", camera_frame);
		int delay = 30; // ms  
		int key = waitKey(delay);
		if (27 == key || 'Q' == key || 'q' == key)
			break;
		//LeaveCriticalSection(&cs);
		}
#endif
	waitKey();
	Sleep(-1);
	//---------------------------------------
	//�ر�Ԥ��
	NET_DVR_StopRealPlay(lRealPlayHandle);
	//ע���û�
	NET_DVR_Logout(lUserID);
	//�ͷ�SDK��Դ
	NET_DVR_Cleanup();

	ExitThread(0);
	return 0;
}


UINT ThreadFunc2(LPVOID pParm) {
	//string param_file = "C:/zhong/model/param_model.xml";
	//INIT_BCNN_MODEL(param_file);
	CfacedetectionDlg *pDlg = (CfacedetectionDlg*)pParm;
	pDlg->my_accept();
	ExitThread(0);
	return 0;

}
void  CfacedetectionDlg::my_accept(void)
{
	string param_file = "C:/zhong/model/param_model.xml";
	INIT_BCNN_MODEL(param_file);
	Mat card;
	Mat camera;
	_cprintf("enter the third process!\n");
	while (1) {
		Sleep(10);
		_cprintf("brun = %d, brun1 = %d \n ", brun, brun1);
		if (brun == 1 && brun1 == 1) {
			//cout << "enter the third processs!";
			match_result = "";
			m_self_counts = 0;
			card = imread("card.bmp");
			cv::imshow("roi_of_card", card);
			for (int i = 0; i < 5; i++)
			{
				EnterCriticalSection(&cs);
				camera = imread("camera.jpg");
				cv::imshow("roi_of_face", camera);
				LeaveCriticalSection(&cs);
				if (!camera.empty() && !card.empty())
				{
					float distance = bcnn_compare_objects(card, camera, "fc9_1", EUCLIDEAN);
					match_of_distance = distance;
		//			cv::imshow("roi_of_card", card);
		//			cv::imshow("roi_of_face", camera);
					if (distance < threshold_distance)
					{
		//				match_result = "����";
						similarity = (1 - (threshold_distance - distance) / threshold_distance) * 100;
						_cprintf("YourSelf\n");
						m_self_counts++;
					}
					else {
		//				match_result = "�Ǳ���";
						similarity = -(distance - threshold_distance) / threshold_distance * 100;
						_cprintf("Not YourSelsf\n");
					}
	//				imshow("roi_of_face", camera);
				}
				else {
							continue;
                      }
			}
			if (m_self_counts >= 3)
			{
				match_result = "����";
			}
			else
			{
				match_result = "�Ǳ���";
			}
			cv::imshow("roi_of_card", card);
			cv::imshow("roi_of_face", camera);
			brun = 0;
			brun1 = 0;
		}
		else
		{
			continue;
		}
//		else {
//			continue;

//		}
}	
			//UpdateData(false);
		SendMessage(WM_UPDATEDATA, FALSE);
		//LeaveCriticalSection(&cs1);
}
void CfacedetectionDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	CString str;
	CTime t = CTime::GetCurrentTime();
	//m_strTimer.SetPaneText(1, t.Format(_T("%Y��%m��%d�� ����%w %H:%M:%S")));
	m_strTimer = t.Format(_T("%Y��%m��%d�� ����%w %H:%M:%S"));
	CDialog::OnTimer(nIDEvent);
	CDialogEx::OnTimer(nIDEvent);
	CDialogEx::OnTimer(nIDEvent);
	CDialogEx::OnTimer(nIDEvent);
	UpdateData(false);
}
int number = 0;

void thread() {

	while (true) {
		//
		//
		number++;

	}
}

void thread1() {
	while (1) {
		number++;
	}
}

