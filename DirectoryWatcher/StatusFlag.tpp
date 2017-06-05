
template <typename Enum_type> 
StatusFlag<Enum_type>::StatusFlag(Enum_type status) : m_status(status)
{

}

template <typename Enum_type> 
void StatusFlag<Enum_type>::SetStatus(Enum_type status)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_status = status;
}

template <typename Enum_type> 
Enum_type StatusFlag<Enum_type>::GetStatus()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_status;
}