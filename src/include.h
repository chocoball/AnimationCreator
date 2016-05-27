#ifndef INCLUDE_H
#define INCLUDE_H

#define kExecName "Animation Creator2"
#define kVersion "2.0.0"

#define kAccessor(type, member, func_name)          \
public:                                             \
    type get##func_name() { return member; }        \
    void set##func_name(type val) { member = val; } \
private:                                            \
    type member

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
#include <stdint.h>
#define LP_ADD(p, n) (uint64_t)((uint64_t)((uint64_t *)(p)) + (uint64_t)((uint64_t *)(n)))
#else
#define LP_ADD(p, n) (unsigned int)((unsigned int)(p) + (unsigned int)(n))
#endif

#endif // INCLUDE_H
