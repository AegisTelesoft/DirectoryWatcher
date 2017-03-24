#include "CancelationToken.h"

CancelationToken::CancelationToken() :  m_cancel(false)
{

}

void CancelationToken::Cancel()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_cancel = true;
	lock.unlock();
}

bool CancelationToken::IsCanceled()
{
	return m_cancel;
}