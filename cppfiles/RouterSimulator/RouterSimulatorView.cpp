
// RouterSimulatorView.cpp : CRouterSimulatorView 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "RouterSimulator.h"
#endif

#include "RouterSimulatorDoc.h"
#include "RouterSimulatorView.h"
#include "RoadNet.h"
#include "commonmsg.h"
#include <fstream>
#include <string>
#include "RoutingProtocolHslpo.h"
#include "RoutingProtocolEncAnony.h"
#include "MobileSocialNetworkHost.h"
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CRouterSimulatorView

IMPLEMENT_DYNCREATE(CRouterSimulatorView, CScrollView)

BEGIN_MESSAGE_MAP(CRouterSimulatorView, CScrollView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CRouterSimulatorView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	ON_MESSAGE(AFTERONEDIJNODE, &CRouterSimulatorView::OnAfteronedijnode)
	ON_MESSAGE(MSG_ID_DATA_PREPARE_FINISHED, &CRouterSimulatorView::OnDataPrepareFinished)
	ON_MESSAGE(MSG_ID_TEST_COMPLETE, &CRouterSimulatorView::OnTestComplete)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(DYN_IDC_CREATE_MSGS, &CRouterSimulatorView::OnButtonCreateMsgs)
	ON_BN_CLICKED(DYN_IDC_CPY_SUMMARY, &CRouterSimulatorView::OnButtonCpySummary)
END_MESSAGE_MAP()

// CRouterSimulatorView 构造/析构

CRouterSimulatorView::CRouterSimulatorView()
	:m_pMapGui(NULL)
	, m_pBtnCopySummary(NULL)
	, m_pEngine(NULL)
	, m_pEditSpeed(NULL)
	, m_pEditMsgStatistic(NULL)
	, m_pEditPickCount(NULL)
	, m_pEditTimeOut(NULL)
	, m_pEditLabel(NULL)
	, m_pBtnCreateMsgs(NULL)
	, m_pAveLatency(NULL)
	, m_pAveAnonyDistance(NULL)
	, m_pAveAnonyTime(NULL)
{
	// TODO: 在此处添加构造代码

}

CRouterSimulatorView::~CRouterSimulatorView()
{
}

BOOL CRouterSimulatorView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式
	   
	return CScrollView::PreCreateWindow(cs);
}

void CRouterSimulatorView::OnEngineTimer(int nCommandId)
{

}

void CRouterSimulatorView::OnEngineSpeedChanged()
{
	int nSpeed = m_pEngine->GetSpeed();
	CString strSpeed;
	strSpeed.Format(_T("X %d"),nSpeed);
	m_pEditSpeed->SetWindowText(strSpeed);
}

void CRouterSimulatorView::OnEngineMessageStatisticsChanged(const CStatisticsReport & report)
{
	m_Summare.m_nMsgCount = report.m_nStartedPackages;
	m_Summare.m_nDeliveryCount = report.m_nDeliveredPackages;
	m_Summare.m_fLatency = (int)(report.m_fAveLatency);
	m_Summare.m_fAveInterHopCount = report.m_fAveTotalHops;

	m_Summare.m_nAnonymityStartCount = report.m_nStartAnonyCount;
	m_Summare.m_nAnonymityEndCount = report.m_nFinishAnonyCount;
	m_Summare.m_nAnonymityTimeCost = (int)(report.m_fAveAnonyTimeCost / 1000);
	m_Summare.m_fAnonymityDistance = report.m_fAveAnonyDistance;
	m_Summare.m_fAveObfuscationNum = report.m_fAveObfuscationNum;
	m_Summare.m_fAnonymityMaxDistance = report.m_fMaxAnonyDistance;

	CString strStatistic = m_Summare.GetUiString();
	m_pEditMsgStatistic->SetWindowText(strStatistic);
}

// CRouterSimulatorView 绘制

void CRouterSimulatorView::OnDraw(CDC* pDC)
{
	CRouterSimulatorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	m_pMapGui->RefreshUi(false);

// 	for (int i = 0; i < 100000; i++)
// 	{
// 		pDC->MoveTo(0, i);
// 		pDC->LineTo(10, i);
// 	}

	// TODO: 在此处为本机数据添加绘制代码
}


// CRouterSimulatorView 打印


void CRouterSimulatorView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CRouterSimulatorView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CRouterSimulatorView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CRouterSimulatorView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}

void CRouterSimulatorView::DestroyEngine()
{
	DWORD ExitCode = 0;
	if (m_pEngine)
	{
		GetExitCodeThread(m_pEngine->m_hThread, &ExitCode);
		TerminateThread(m_pEngine->m_hThread, ExitCode);
	}
}

void CRouterSimulatorView::InitHostProtocol(int nCopyCount, char * strProtocolName, double fCommunicateRadius, int nK, double fHigh, double fLow, double fAnonymityRadius)
{
	fHigh /= 100.0;
	fLow /= 100.0;
	CRouterSimulatorDoc * pDoc = GetDocument();
	int nLength = pDoc->m_pRoadNet->m_allHosts.GetSize();
	m_Summare.m_ProtocolName = strProtocolName;
	m_Summare.m_nRandomSeed = theApp.nRandSeed;
	for (int i = 0; i < nLength; ++i)
	{
		CMobileSocialNetworkHost * pHost = (CMobileSocialNetworkHost *)pDoc->m_pRoadNet->m_allHosts[i];
		if (strcmp(strProtocolName, PROTOCOL_NAME_HSLPO) == 0)
		{
			CRoutingProtocolHslpo * pHslpo = new CRoutingProtocolHslpo();
			pHslpo->SetCommunicateRadius(fCommunicateRadius);
			pHslpo->SetEnvironment(pHost, m_pEngine);
			pHslpo->SetPrivacyParam(nK, fHigh, fLow);
			pHslpo->SetCopyCount(nCopyCount);
			pDoc->m_pRoadNet->m_allHosts[i]->m_pProtocol = pHslpo;
		}
		else if (strcmp(strProtocolName, PROTOCOL_NAME_MHLPSP) == 0)
		{
			CRoutingProtocolEncAnony * pMhlpsp = new CRoutingProtocolEncAnony();
			pMhlpsp->SetCommunicateRadius(fCommunicateRadius);
			pMhlpsp->SetEnvironment(pHost, m_pEngine);
			pMhlpsp->SetPrivacyParam(fHigh, fAnonymityRadius, 3 * fAnonymityRadius, 10 * fAnonymityRadius);
			pMhlpsp->SetCopyCount(nCopyCount);
			pDoc->m_pRoadNet->m_allHosts[i]->m_pProtocol = pMhlpsp;
		}
		else
		{
			CRoutingProtocolBSW * pBsw = new CRoutingProtocolBSW();
			pBsw->SetCommunicateRadius(fCommunicateRadius);
			pBsw->SetEnvironment(pHost, m_pEngine);
			pBsw->SetCopyCount(nCopyCount);
			pDoc->m_pRoadNet->m_allHosts[i]->m_pProtocol = pBsw;
		}
	}
}

void CRouterSimulatorView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CRouterSimulatorView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


void CRouterSimulatorView::OnButtonCreateMsgs()
{
	CString strNum;
	m_pEditPickCount->GetWindowText(strNum);
	int nNum = _ttoi(strNum);
	CString strTimeOut;
	m_pEditTimeOut->GetWindowText(strTimeOut);
	int nTimeOut = _ttoi(strTimeOut);
	CRouterSimulatorDoc * pDoc = GetDocument();
	CRoadNet * pRoadNet = pDoc->m_pRoadNet;
	int nHostCount = pRoadNet->m_allHosts.GetSize() - SERVER_NODE_COUNT;
	int nCreatedCount = 0;
	int nIndex = -1;
	int * pEmpty = new int[nHostCount];
	memset(pEmpty, 0, sizeof(int)*nHostCount);
	while (nCreatedCount < nNum)
	{
		int nRand = rand() % (nHostCount-nCreatedCount) + 1;
		int nEmptyCount = 0;
		for (int i = 0; i < nHostCount; ++i)
		{
			if (pEmpty[i] == 0)
			{
				nEmptyCount++;
				if (nEmptyCount == nRand)
				{
					pEmpty[i] = 1;
					++nCreatedCount;
					break;
				}
			}
		}
	}
	CRoutingDataEnc encData;
	CHost * pHostFrom = NULL;
	CHost * pHostTo = pRoadNet->m_allHosts.GetAt(0);
	SIM_TIME lnTimeOut = m_pEngine->GetSimTime() + nTimeOut;

	for (int i = SERVER_NODE_COUNT; i < nHostCount + SERVER_NODE_COUNT; ++i)
	{
		if (pEmpty[i - 1] == 1)
		{
			pHostFrom = pRoadNet->m_allHosts[i];
			encData.SetValue(pHostFrom, pHostTo, lnTimeOut);
			encData.ChangeDataId();
			pHostFrom->m_pProtocol->SendPackage(encData);
		}
	}
	delete[] pEmpty;
}

void CRouterSimulatorView::OnButtonCpySummary()
{
	if (OpenClipboard())
	{
		CRouterSimulatorDoc * pDoc = GetDocument();
		m_Summare.m_nTrustValue = pDoc->m_Cfg.m_fPrivacyHigh;
		m_Summare.m_nAnonymityK = pDoc->m_Cfg.m_nK;
		m_Summare.m_nTotleNodeCount = pDoc->m_Cfg.m_nNodeCount;
		m_Summare.m_fRadius = pDoc->m_Cfg.m_fCommunicateRadius;
		m_Summare.m_fAveAnonySurroundingCount = m_pEngine->GetAveSurroundingHosts(pDoc->m_Cfg.m_nK * pDoc->m_Cfg.m_fCommunicateRadius, MSG_HOP_STATE_BSW_BEGIN);
		m_Summare.m_fAveRealSurrounding = m_pEngine->GetAveSurroundingHosts(pDoc->m_Cfg.m_nK * pDoc->m_Cfg.m_fCommunicateRadius, MSG_HOP_STATE_ANONYMITY_END);
		double fInsideCount = 0, fOutsideCount = 0;
		fInsideCount = m_pEngine->GetAveSurroundingHosts(pDoc->m_Cfg.m_fAnonyRadius, MSG_HOP_STATE_ANONYMITY_END);
		fOutsideCount = m_pEngine->GetAveSurroundingHosts(m_Summare.m_fAnonymityMaxDistance, MSG_HOP_STATE_ANONYMITY_END);
		m_Summare.m_fAveRingCount = fOutsideCount - fInsideCount;
		m_Summare.m_nOnRingCount = m_pEngine->GetSourceOnRing(pDoc->m_Cfg.m_fAnonyRadius, m_Summare.m_fAnonymityMaxDistance);
		HGLOBAL clipbuffer;
		char   *   buffeclipbufferr;
		EmptyClipboard();
		char * pOut = new char[300];
		m_Summare.GetString(pOut);
		clipbuffer = GlobalAlloc(GMEM_DDESHARE, strlen(pOut) + 1);
		buffeclipbufferr = (char*)GlobalLock(clipbuffer);
		int a = 9;
		strcpy_s(buffeclipbufferr, strlen(pOut) + 1, pOut);
		GlobalUnlock(clipbuffer);
		SetClipboardData(CF_TEXT, clipbuffer);
		CloseClipboard();

		std::ofstream fout;
		fout.open("statistics.txt", std::ofstream::out | std::ofstream::app);
		string strOutFile(pOut);
		fout << strOutFile << endl;
		fout.close();

		delete[] pOut;
	}
}

// CRouterSimulatorView 诊断

#ifdef _DEBUG
void CRouterSimulatorView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CRouterSimulatorView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CRouterSimulatorDoc* CRouterSimulatorView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CRouterSimulatorDoc)));
	return (CRouterSimulatorDoc*)m_pDocument;
}

#endif //_DEBUG


// CRouterSimulatorView 消息处理程序


void CRouterSimulatorView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	// TODO: 在此添加专用代码和/或调用基类
	CSize   sizeTotal;
	sizeTotal.cx = sizeTotal.cy = 0;
	SetScrollSizes(MM_TEXT, sizeTotal);

	m_pBtnCopySummary = new CButton();
	m_pBtnCopySummary->Create(_T("Cpy Sum"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(0, 0, 100, 30), this, DYN_IDC_CPY_SUMMARY);
	m_pEditSpeed = new CEdit();
	m_pEditSpeed->Create(WS_DISABLED | WS_CHILD | WS_VISIBLE | WS_TABSTOP | DT_CENTER | DT_VCENTER, CRect(0, 0, 100, 30), this, 3000);
	m_pEditMsgStatistic = new CEdit();
	m_pEditMsgStatistic->Create(WS_DISABLED | WS_CHILD | WS_VISIBLE | WS_TABSTOP | DT_CENTER | DT_VCENTER, CRect(0, 0, 100, 30), this, 3000);
	m_pEditPickCount = new CEdit();
	m_pEditPickCount->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | DT_CENTER | DT_VCENTER, CRect(0, 0, 100, 30), this, 3000);
	m_pBtnCreateMsgs = new CButton();
	m_pBtnCreateMsgs->Create(_T("Create Msgs"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(0, 0, 100, 30), this, DYN_IDC_CREATE_MSGS);
	m_pEditTimeOut = new CEdit();
	m_pEditTimeOut->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | DT_CENTER | DT_VCENTER, CRect(0, 0, 100, 30), this, 3000);
	m_pEditLabel = new CEdit();
	m_pEditLabel->Create(WS_DISABLED | WS_CHILD | WS_VISIBLE | WS_TABSTOP | DT_CENTER | DT_VCENTER, CRect(0, 0, 100, 30), this, 3000);
	m_pAveLatency = new CEdit();
	m_pAveLatency->Create(WS_DISABLED | WS_CHILD | WS_VISIBLE | WS_TABSTOP | DT_CENTER | DT_VCENTER, CRect(0, 0, 100, 30), this, 3000);
	m_pAveAnonyDistance = new CEdit();
	m_pAveAnonyDistance->Create(WS_DISABLED | WS_CHILD | WS_VISIBLE | WS_TABSTOP | DT_CENTER | DT_VCENTER, CRect(0, 0, 100, 30), this, 3000);
	m_pAveAnonyTime = new CEdit();
	m_pAveAnonyTime->Create(WS_DISABLED | WS_CHILD | WS_VISIBLE | WS_TABSTOP | DT_CENTER | DT_VCENTER, CRect(0, 0, 100, 30), this, 3000);

	m_pMapGui = (CMapGui *)RUNTIME_CLASS(CMapGui)->CreateObject();
	m_pMapGui->m_pView = this;
	CRouterSimulatorDoc * pDoc = GetDocument();
	m_pMapGui->m_Cfg = pDoc->m_Cfg;
	m_pMapGui->SetRoadNet(pDoc->m_pRoadNet);
	if (NULL == m_pMapGui)
	{
		return;
	}
	CRect rectClient = { 0, 0, 500, 500 };
	m_pMapGui->Create(NULL, NULL, AFX_WS_DEFAULT_VIEW, rectClient, this, 0);

	m_pEngine = (CHostEngine*)AfxBeginThread(RUNTIME_CLASS(CHostEngine));
	m_pEngine->SetValue(pDoc->m_pRoadNet, m_pMapGui);

	m_pMapGui->SetEngine(m_pEngine);

	m_pEngine->RegisterUser(this);
	OnEngineSpeedChanged();
	CString strTimeOut; 
	strTimeOut.Format(_T("%d"), pDoc->m_Cfg.m_nTimeOutSecond);
	m_pEditTimeOut->SetWindowText(strTimeOut);
	m_pEditPickCount->SetWindowText(_T("100"));
// 	CRect rectClient;
// 	GetClientRect(&rectClient);
}


BOOL CRouterSimulatorView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: 在此添加专用代码和/或调用基类

	return CScrollView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


void CRouterSimulatorView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	// TODO: 在此添加专用代码和/或调用基类
	int i = 0;
	i = 3;
}


void CRouterSimulatorView::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);

	CRect rectClient;
	GetClientRect(&rectClient);

	int nMostRight = rectClient.left;
	int nWidth = 150;
	if (m_pBtnCopySummary)
	{
		nWidth = 40;
		m_pBtnCopySummary->MoveWindow(nMostRight, 0, nWidth, 30);
		nMostRight += nWidth + 5;
	}
	if (m_pEditSpeed)
	{
		nWidth = 70;
		m_pEditSpeed->MoveWindow(nMostRight, 0, nWidth, 30);
		nMostRight += nWidth + 5;
	}
	if (m_pBtnCreateMsgs)
	{
		nWidth = 100;
		m_pBtnCreateMsgs->MoveWindow(nMostRight, 0, nWidth, 30);
		nMostRight += nWidth + 5;
	}
	if (m_pEditPickCount)
	{
		nWidth = 100;
		m_pEditPickCount->MoveWindow(nMostRight, 0, nWidth, 30);
		nMostRight += nWidth + 5;
	}
	if (m_pEditLabel)
	{
		m_pEditLabel->MoveWindow(nMostRight, 0, nWidth, 30);
		nMostRight += nWidth + 5;
	}
	if (m_pEditTimeOut)
	{
		nWidth = 100;
		m_pEditTimeOut->MoveWindow(nMostRight, 0, nWidth, 30);
		nMostRight += nWidth + 5;
	}
	if (m_pEditMsgStatistic)
	{
		nWidth = 700;
		m_pEditMsgStatistic->MoveWindow(nMostRight, 0, nWidth, 30);
		nMostRight += nWidth + 5;
	}
	if (m_pAveLatency)
	{
		nWidth = 150;
		m_pAveLatency->MoveWindow(nMostRight, 0, nWidth, 30);
		nMostRight += nWidth + 5;
	}
	if (m_pAveAnonyDistance)
	{
		nWidth = 150;
		m_pAveAnonyDistance->MoveWindow(nMostRight, 0, nWidth, 30);
		nMostRight += nWidth + 5;
	}
	if (m_pAveAnonyTime)
	{
		nWidth = 150;
		m_pAveAnonyTime->MoveWindow(nMostRight, 0, nWidth, 30);
		nMostRight += nWidth + 5;
	}

	if (m_pMapGui)
	{
		m_pMapGui->MoveWindow(0, 30, rectClient.Width(), rectClient.Height() - 30);
	}
	// TODO: 在此处添加消息处理程序代码
}


afx_msg LRESULT CRouterSimulatorView::OnAfteronedijnode(WPARAM wParam, LPARAM lParam)
{
	int nDijId = wParam;
// 	CString strOut;
// 	strOut.Format(_T("\nDone %d"), nDijId);
// 	OutputDebugString(strOut);
	m_pMapGui->PostMessage(MARKFINISHEDDIJNODES, wParam, lParam);
	return 0;
}

LRESULT CRouterSimulatorView::OnDataPrepareFinished(WPARAM wParam, LPARAM lParam)
{
	CRouterSimulatorDoc * pDoc = GetDocument();
	InitHostProtocol(pDoc->m_Cfg.m_nBswCopyCount, pDoc->m_Cfg.m_strProtocolName, pDoc->m_Cfg.m_fCommunicateRadius, pDoc->m_Cfg.m_nK, pDoc->m_Cfg.m_fPrivacyHigh, pDoc->m_Cfg.m_fPrivacyLow, pDoc->m_Cfg.m_fAnonyRadius);
#ifdef DEBUG
	m_pMapGui->PostMessage(MSG_ID_INIT_OK, 1);
#else
	m_pMapGui->PostMessage(MSG_ID_INIT_OK, 1);
#endif

	CString strLabel;
	strLabel.Format(_T("/%d Time Out: "), pDoc->m_pRoadNet->m_allHosts.GetSize() - SERVER_NODE_COUNT);
	m_pEditLabel->SetWindowText(strLabel);

	return 0;
}

LRESULT CRouterSimulatorView::OnTestComplete(WPARAM wParam, LPARAM lParam)
{
	OnButtonCpySummary();
	PostQuitMessage(0);
	return 0;
}

void CRouterSimulatorView::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CScrollView::OnClose();
}


void CRouterSimulatorView::OnDestroy()
{
	CScrollView::OnDestroy();
	DestroyEngine();
	// TODO: 在此处添加消息处理程序代码
}
