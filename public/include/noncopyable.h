#ifndef _PUBLIC_NONCOPYABLE_H
#define _PUBLIC_NONCOPYABLE_H

namespace prj
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

} // namespace prj

#endif // _PUBLIC_NONCOPYABLE_H
