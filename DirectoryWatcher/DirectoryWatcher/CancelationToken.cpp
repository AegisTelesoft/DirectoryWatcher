#include "CancelationToken.h"

/**************************************************************************************************/
CancelationToken::CancelationToken() : m_globalCancel(false), m_id(-1)
{

}

/**************************************************************************************************/
void CancelationToken::CancelGlobally()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_globalCancel = true;
	lock.unlock();
}

/**************************************************************************************************/
bool CancelationToken::Cancel(int id)
{
	// Returns true if cancel action is successful. 
	// Otherwise returns false if previous thread isn't canceled yet
	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_id == -1) 
	{
		m_id = id;
		return true;
	}
	return false;
}

/**************************************************************************************************/
void CancelationToken::ResetIdToken()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_id = -1;
	lock.unlock();
}

/**************************************************************************************************/
void CancelationToken::ResetGlobalToken()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_globalCancel = false;
	lock.unlock();
}

/**************************************************************************************************/
bool CancelationToken::IsGloballyCanceled()
{
	// ~lock_guard(); effectively calls m.unlock() where m is the mutex passed to the lock_guard's 
	// constructor.
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_globalCancel;
}

/**************************************************************************************************/
bool CancelationToken::IsCanceled(int id)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_id == id)
		return true;
	else
		return false;
}
