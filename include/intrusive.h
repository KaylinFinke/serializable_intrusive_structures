#if not defined(A8E14AE30B8745A7846C112EB15E0FCD)
#define A8E14AE30B8745A7846C112EB15E0FCD
#if defined(A8E14AE30B8745A7846C112EB15E0FCD)

#include <concepts>
#include <tuple>
#include <type_traits>
#include <utility>

namespace intrusive {
	namespace detail {
		template <typename T, typename E>
		concept std_get = requires(T t)
		{
			requires std::same_as<decltype(std::get<std::remove_cvref_t<E>>(t)), E>;
		};

		template <typename T, typename E, typename U = std::remove_const_t<T>, typename V = std::remove_const_t<E>>
		concept mem_get = requires(T t, U u, E e, V v)
		{
			v = static_cast<std::remove_cvref_t<E>>(t.template get<std::remove_cvref_t<E>>());
			u. template get<std::remove_cvref_t<E>>() = e;
		};

		template <typename T, typename E, typename U = std::remove_const_t<T>, typename V = std::remove_const_t<E>>
		concept usr_get = requires(T t, U u, E e, V v)
		{
			v = static_cast<std::remove_cvref_t<E>>(get<std::remove_cvref_t<E>>(t));
			get<std::remove_cvref_t<E>>(u) = e;
		};

		template <typename T, typename E>
		concept has_get = usr_get<T, E> or std_get<T, E> or mem_get<T, E>;

		template <typename E, typename T, typename V>
		[[nodiscard]] constexpr decltype(auto) dispatch(T t) noexcept
		{
			if constexpr (detail::mem_get<T, V>)
				return t. template get<E>();
			else if constexpr (detail::usr_get<T, V>)
				return get<E>(t);
			else
				return std::get<E>(t);
		}
	}

	template <typename E, typename T>
	requires detail::has_get<const T&, const E&>
	[[nodiscard]] constexpr decltype(auto) _get(const T& t) noexcept
	{
		return detail::dispatch<E, const T&, const E&>(t);
	}

	template <typename E, typename T>
	requires detail::has_get<const T&&, const E&&>
	[[nodiscard]] constexpr decltype(auto) _get(const T&& t) noexcept
	{
		return detail::dispatch<E, const T&&, const E&&>(t);
	}

	template <typename E, typename T>
	requires detail::has_get<T&, E&>
	[[nodiscard]] constexpr decltype(auto) _get(T& t) noexcept
	{
		return detail::dispatch<E, T&, E&>(t);
	}

	template <typename E, typename T>
	requires detail::has_get<T&&, E&&>
	[[nodiscard]] constexpr decltype(auto) _get(T&& t) noexcept
	{
		return detail::dispatch<E, T&&, E&&>(t);
	}
}

#endif
#endif
