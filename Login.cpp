// Login.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "facedetection.h"
#include "Login.h"
#include "afxdialogex.h"
#include "Login.h"

// Login �Ի���

IMPLEMENT_DYNAMIC(Login, CDialogEx)

Login::Login(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG1, pParent)
	, m_strUser(_T(""))
	, m_strPassword(_T(""))
{

}

Login::~Login()
{
}

void Login::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_USER, m_strUser);
	DDX_Text(pDX, IDC_PASSWORD, m_strPassword);
}


BEGIN_MESSAGE_MAP(Login, CDialogEx)
	ON_BN_CLICKED(IDOK, &Login::OnBnClickedOk)
END_MESSAGE_MAP()


// Login ��Ϣ�������


void Login::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(true);
	if (m_strUser.IsEmpty() || m_strPassword.IsEmpty())
	{
		MessageBox(_T("�û��������벻��Ϊ�գ�"), _T("�û���¼��Ϣ"));
		return;
	}
	else if (m_strUser == "1"&& m_strPassword == "1")
	{
		MessageBox(_T("��¼�ɹ���"), _T("��½"));//�����п���

	}
	else
	{
		MessageBox(_T("�û��������벻��ȷ"), _T("��ʾ"));
		return;
	}
	CDialogEx::OnOK();
}
