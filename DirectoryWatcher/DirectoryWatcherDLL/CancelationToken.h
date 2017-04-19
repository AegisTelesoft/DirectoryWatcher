#pragma once

#ifdef DIRECTORYWATCHERDLL_EXPORTS  
#define DW_EXPORT __declspec(dllexport)   
#else  
#define DW_EXPORT __declspec(dllimport)   
#endif  

#include <mutex>

class DW_EXPORT CancelationToken
{
public:
	CancelationToken();
	
	void CancelGlobally();
	bool Cancel(int id);
	void ResetGlobalToken();
	void ResetIdToken();
	bool IsGloballyCanceled();
	bool IsCanceled(int id);

private:
	bool m_globalCancel;
	int m_id;
	std::mutex m_mutex;
};