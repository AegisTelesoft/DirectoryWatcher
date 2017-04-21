#pragma once

#ifdef _WIN32
	#ifdef DIRECTORYWATCHERDLL_EXPORTS  
		#define DW_EXPORT __declspec(dllexport)   
	#else  
		#define DW_EXPORT __declspec(dllimport)   
	#endif
#else 
	#define DW_EXPORT
#endif

#include <mutex>

/**************************************************************************************************/
/*  My implementation of thread cancelation token used in my filesystem observer implementation   */
/**************************************************************************************************/
class CancelationToken
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