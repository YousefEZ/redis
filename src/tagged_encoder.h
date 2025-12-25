#ifndef INCLUDED_TAGGED_ENCODER_H
#define INCLUDED_TAGGED_ENCODER_H

#include "buffer.h"
#include "codec.h"

#include <variant>

template <typename T, typename... Ts>
struct index_of;

template <typename T, typename... Ts>
struct index_of<T, T, Ts...> : std::integral_constant<unsigned short, 0> {};

template <typename T, typename U, typename... Ts>
struct index_of<T, U, Ts...>
: std::integral_constant<unsigned short, 1 + index_of<T, Ts...>::value> {};

template <MessageCodec... Types>
struct Messages {
    using MessageVariant = std::variant<Types...>;
    using Tag            = unsigned short;

    template <typename T>
    static consteval Tag id()
    {
        return index_of<T, Types...>::value;
    }

    // we expect F to be an overloaded function that will take the type
    // that is deduced via the id passed in.
    // i.e. f.template operator<T>(args...)
    template <typename F, typename... ARGS>
    static auto dispatch(unsigned short id, F&& f, ARGS&&... args)
    {
        return dispatch_impl<F, Types...>(id,
                                          std::forward<F>(f),
                                          std::forward<ARGS>(args)...);
    }

  private:
    template <typename F, typename T, typename... Rest, typename... ARGS>
    static auto dispatch_impl(Tag id, F&& f, ARGS&&... args)
    {
        if (id == 0) {
            if constexpr (std::is_void_v<decltype(
                              f.template operator()<T>(args...))>) {
                f.template operator()<T>(std::forward<ARGS>(args)...);
            }
            else {
                return f.template operator()<T>(std::forward<ARGS>(args)...);
            }
        }
        else {
            if constexpr (sizeof...(Rest) > 0)
                dispatch_impl<F, Rest...>(id - 1,
                                          std::forward<F>(f),
                                          std::forward<ARGS>(args)...);
            else
                __builtin_unreachable();  // or throw/assert
        }
    }
};

template <template <typename> typename CODEC, class MESSAGES>
struct TaggedEncoder {
    using Tag         = MESSAGES::Tag;
    using MessageType = MESSAGES::MessageVariant;

    template <typename T>
    static void write(T&& message, Buffer& buffer)
    {
        Tag id = MESSAGES::template id<std::decay_t<T> >();
        buffer.append((char*)&id, sizeof(id));
        CODEC<T>::serialize(std::forward<T>(message), buffer);
    }

    template <typename T>
    static std::optional<typename MESSAGES::MessageVariant>
    deserialize(Buffer& buffer)
    {
        return CODEC<T>::deserialize(buffer);
    }

    static std::optional<typename MESSAGES::MessageVariant>
    consume_message(Buffer& buffer)
    {
        if (buffer.size() <= sizeof(Tag)) {
            return {};
        }
        Tag tag;
        buffer.cpy(&tag, sizeof(Tag));
        buffer.consume(sizeof(Tag));
        return MESSAGES::dispatch(tag, deserialize);
    }
};

#endif
