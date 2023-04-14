#if !defined(DEF_TTIMING)
#define DEF_TTIMING

#include <mach/mach_time.h>

class TTiming
{
protected:
    uint64_t m_startTime;
    uint64_t m_endTime;
    uint64_t m_frequency;

public:
    TTiming(void);

    void Begin(void);
    double End(void);
};

inline TTiming::TTiming(void)
{
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    m_frequency = info.denom * 1000000000 / info.numer;
}

inline void TTiming::Begin(void)
{
    m_startTime = mach_absolute_time();
}

inline double TTiming::End(void)
{
    m_endTime = mach_absolute_time();

    return (double)(m_endTime - m_startTime) * 1e+3 / m_frequency;
}

#endif
