#ifndef INCLUDED_REDIS_META_H
#define INCLUDED_REDIS_META_H

#include <net_tagged_encoder.h>

namespace meta {

template <typename... Ts>
struct TypeList {
    template <template <typename> typename WRAPPER>
    using Map = TypeList<WRAPPER<Ts>...>;

    template <template <typename...> typename CONTAINER>
    using To = CONTAINER<Ts...>;

    template <typename... NewTs>
    using WithBackAppend = TypeList<Ts..., NewTs...>;

    template <typename... NewTs>
    using WithFrontAppend = TypeList<NewTs..., Ts...>;

    template <typename LIST>
    using Concatenate = LIST::template WithFrontAppend<Ts...>;
};

}  // namespace meta

#endif
