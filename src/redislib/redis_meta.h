#ifndef INCLUDED_REDIS_META_H
#define INCLUDED_REDIS_META_H

#include <net_tagged_encoder.h>

namespace meta {

template <typename... Ts>
struct TypeList {
    template <template <typename> typename WRAPPER>
    using Apply = TypeList<WRAPPER<Ts>...>;
};

template <typename A, typename B>
struct Concat;

template <template <typename...> typename A,
          typename... A_TS,
          template <typename...> typename B,
          typename... B_TS>
struct Concat<A<A_TS...>, B<B_TS...> > {
    using Value = TypeList<A_TS..., B_TS...>;
};

template <typename A, typename... B>
struct Append;

template <template <typename...> typename LIST, typename... Ts, typename... Vs>
struct Append<LIST<Ts...>, Vs...> {
    using Value = TypeList<Ts..., Vs...>;
};

template <typename TYPE_LIST>
struct As;

template <template <typename...> typename TYPE_LIST, typename... Ts>
struct As<TYPE_LIST<Ts...> > {
    using Messages = net::Messages<Ts...>;
};

}  // namespace meta

#endif
