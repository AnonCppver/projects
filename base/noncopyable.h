#ifndef _LEEF_BASE_NONCOPYABLE_H
#define _LEEF_BASE_NONCOPYABLE_H

namespace leef
{

    class noncopyable
    {
    public:
        noncopyable(const noncopyable &) = delete;
        void operator=(const noncopyable &) = delete;

    protected:
        noncopyable() = default;
        ~noncopyable() = default;
    };

} // namespace leef

#endif // _LEEF_BASE_NONCOPYABLE_H
