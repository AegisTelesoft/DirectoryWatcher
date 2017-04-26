#pragma once

#include <mutex>

/**************************************************************************************************/
/*  My implementation of thread cancelation token used in my filesystem observer implementation   */
/**************************************************************************************************/
template <typename Enum_type> 
class StatusFlag
{
public:
	StatusFlag(Enum_type status);
	
	void SetStatus(Enum_type status);
	Enum_type GetStatus();

private:
	Enum_type m_status;
	std::mutex m_mutex;
};

#include "StatusFlag.tpp"