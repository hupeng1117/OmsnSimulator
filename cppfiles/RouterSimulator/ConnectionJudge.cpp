// ConnectionJudge.cpp : 实现文件
//

#include "stdafx.h"
#include "RouterSimulator.h"
#include "ConnectionJudge.h"
#include "PositionForecast.h"
#include "RoadNet.h"
#include "MsgCntJudgeReceiverReport.h"
#include "HostEngine.h"



// CConnectionJudge

IMPLEMENT_DYNCREATE(CConnectionJudge, CWinThread)

CConnectionJudge::CConnectionJudge()
	: m_pData(NULL)
	, m_pForecast(NULL)
	, m_pEngine(NULL)
{
}

CConnectionJudge::~CConnectionJudge()
{
}

BOOL CConnectionJudge::InitInstance()
{
	// TODO:    在此执行任意逐线程初始化
	return TRUE;
}

int CConnectionJudge::ExitInstance()
{
	// TODO:    在此执行任意逐线程清理
	return CWinThread::ExitInstance();
}

void CConnectionJudge::SetData(CRoadNet * pData)
{
	m_pData = pData;
}

void CConnectionJudge::SetForecast(CPositionForecast * pForecast)
{
	m_pForecast = pForecast;
}

void CConnectionJudge::SetEngine(CHostEngine * pEngine)
{
	m_pEngine = pEngine;
}

void CConnectionJudge::OnFinishedForecastOnce(SIM_TIME lnSimTime)
{
	SIM_TIME * pMsg = new SIM_TIME(lnSimTime);
	PostThreadMessage(MSG_ID_JUDGE_FORECAST_OK, (WPARAM)pMsg, 0);
}

void CConnectionJudge::OnFinishedOneForecast(WPARAM wParam, LPARAM lParam)
{
	SIM_TIME * pMsg = (SIM_TIME *)wParam;
	SIM_TIME lnSimTime = *pMsg;
	delete pMsg;
	pMsg = NULL;
}

void CConnectionJudge::OnNewSend(WPARAM wParam, LPARAM lParam)
{
	CMsgNewSendJudge * pMsg = (CMsgNewSendJudge *)wParam;
	SIM_TIME lnSimTimeHalfBlock = m_pData->GetSimTimeCrossHalfBlank();
	CMsgPosFrcstReport * pReport = m_pForecast->GetReport(pMsg->m_fSecondId, lnSimTimeHalfBlock);
	if (!pReport)
	{
		pReport = m_pForecast->GetReport(pMsg->m_fSecondId, lnSimTimeHalfBlock);
	}
	ASSERT(pReport);

	CDoublePoint centerPosition = pMsg->m_pHost->m_schedule.GetPosition(pMsg->m_fSecondId);
	CMsgCntJudgeReceiverReport * pRet = new CMsgCntJudgeReceiverReport();
	pRet->m_nMsgId = pMsg->m_nMsgId;
	int nLimit = pMsg->m_fRadius / m_pData->GetHashInterval() + 2;
	for (int i = -nLimit; i <= nLimit; ++i)
	{
		for (int j = -nLimit; j <= nLimit; ++j)
		{
			double fX = centerPosition.m_X + i * m_pData->GetHashInterval();
			double fY = centerPosition.m_Y + j * m_pData->GetHashInterval();
			DWORD HashValue = m_pData->GetHashValue(fX, fY);
			CHostReference tmpReference;
			if (pReport->m_Reference.Lookup(HashValue, tmpReference))
			{
				POSITION pos = tmpReference.m_Hosts.GetHeadPosition();
				while (pos)
				{
					CHostGui tmpTarget = tmpReference.m_Hosts.GetNext(pos);
					CDoublePoint targetPosition = tmpTarget.m_pHost->GetPosition(pMsg->m_fSecondId);
					double fDistance = CDoublePoint::GetDistance(targetPosition, centerPosition);
					if (fDistance <= pMsg->m_fRadius)
					{
						tmpTarget.m_Position = targetPosition;
						pRet->m_Hosts.AddTail(tmpTarget);
					}
				}
			}
		}
	}
	if (m_pEngine)
	{
		m_pEngine->PostThreadMessage(MSG_ID_ENGINE_JUDGE_OK, (WPARAM)pRet, 0);
	}

	delete pMsg;
}

BEGIN_MESSAGE_MAP(CConnectionJudge, CWinThread)
	ON_THREAD_MESSAGE(MSG_ID_JUDGE_FORECAST_OK, OnFinishedOneForecast)
	ON_THREAD_MESSAGE(MSG_ID_JUDGE_NEW_SEND, OnNewSend)
END_MESSAGE_MAP()


// CConnectionJudge 消息处理程序
