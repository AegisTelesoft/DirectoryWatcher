#include "CancelationToken.h"

CancelationToken::CancelationToken() : m_cancel(false)
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
	// ~lock_guard(); effectively calls m.unlock() where m is the mutex passed to the lock_guard's constructor.
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_cancel;
}
