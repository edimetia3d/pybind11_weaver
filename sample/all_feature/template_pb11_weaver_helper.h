//
// License: MIT
//

/**
 * This file is manually crafted to help binding generation, it is not a part of
 * the public API.
 */

#include "sample.h"

/**
 * Two things need to be done to bind a template function:
 * 1. Use syntax of "template specialization" to force generate the binding of
 * template function.
 * 2. Use syntax of extern template to avoid duplicate symbol.
 * Note that some instance must exist in the translation unit.
 */
template <> std::string Foo<Q::R<int>, 8>(Q::R<int>, int);
extern template std::string Foo<Q::R<int>, 8>(Q::R<int>, int);

/**
 * Another one
 */
template <> std::string Foo<float, 9>(float, int);
extern template std::string Foo<Q::R<int>, 8>(Q::R<int>, int);

namespace template_ns {
// For template class, an extern explicit instantiation is enough.

extern template class TemplateClass<int>;
} // namespace template_ns