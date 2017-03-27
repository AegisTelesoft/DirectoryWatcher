#pragma once

#include <mutex>

class CancelationToken
{
public:
	CancelationToken();
	
	void CancelGlobally();
	bool Cancel(int id);
	void Reset();
	bool IsGloballyCanceled();
	bool IsCanceled(int id);

private:
	bool m_globalCancel;
	int m_id;
	std::mutex m_mutex;
};