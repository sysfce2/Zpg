#ifndef WORKER_HPP
#define WORKER_HPP
typedef int (*JOBFUNC)(void *pData);

class CWorker;

class CJob
{
	friend class CWorker;

	CWorker *m_pWorker;
	CJob *m_pPrev;
	CJob *m_pNext;

	volatile int m_Status;
	volatile int m_Result;

	JOBFUNC m_pfnFunc;
	void *m_pFuncData;

public:
	CJob()
	{
		m_Status = STATE_DONE;
		m_pFuncData = 0;
		m_pPrev = 0x0;
		m_pNext = 0x0;
		m_pWorker = 0x0;
		m_pfnFunc = 0x0;
	}

	enum
	{
		STATE_PENDING=0,
		STATE_RUNNING,
		STATE_DONE
	};

	int CurrentStatus() const { return m_Status; }
	int Result() const {return m_Result; }
};

class CWorker
{
	LOCK m_Lock;
	CJob *m_pFirstJob;
	CJob *m_pLastJob;

	static void WorkerThread(void *pUser);

public:
	CWorker();

	int Init(int NumThreads);
	int Add(CJob *pJob, JOBFUNC pfnFunc, void *pData);
	int Remove(CJob *pJob);
};

#endif // WORKER_HPP
