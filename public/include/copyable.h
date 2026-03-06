#ifndef _PUBLIC_COPYABLE_H
#define _PUBLIC_COPYABLE_H

namespace prj
{

/// A tag class emphasises the objects are copyable.
/// The empty base class optimization applies.
/// Any derived class of copyable should be a value type.
class copyable
{
 protected:
  copyable() = default;
  ~copyable() = default;
};

}  // namespace prj

#endif  // _PUBLIC_COPYABLE_H
