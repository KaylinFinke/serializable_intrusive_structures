#if not defined(A8E14AE30B8745A7846C112EB15E0FCD)
#define A8E14AE30B8745A7846C112EB15E0FCD
#if defined(A8E14AE30B8745A7846C112EB15E0FCD)

#include <concepts>
#include <tuple>
#include <type_traits>
#include <utility>

namespace intrusive {
	namespace detail {
		template <typename T, typename U>
		concept std_get = requires(U& u)
		{
			requires std::same_as<decltype(std::get<T>(u)), T&>;
		} and requires(U&& u)
		{
			requires std::same_as<decltype(std::get<T>(u)), T&&>;
		} and requires(const U& u)
		{
			requires std::same_as<decltype(std::get<T>(u)), const T&>;
		} and requires(const U&& u)
		{
			requires std::same_as<decltype(std::get<T>(u)), const T&&>;
		};

		template <typename T, typename U>
		concept mem_get = requires(U& u, T t)
		{
			T(u. template get<T>());
			u. template get<T>() = t;
		} and requires(U&& u, T t)
		{
			T(u. template get<T>());
			u. template get<T>() = t;
		} and requires(const U& u)
		{
			T(u. template get<T>());
		} and requires(const U&& u)
		{
			T(u. template get<T>());
		};

		template <typename T, typename U>
		concept usr_get = requires(U& u, T t)
		{
			T(get<T>(u));
			get<T>(u) = t;
		} and requires(U&& u, T t)
		{
			T(get<T>(u));
			get<T>(u) = t;
		} and requires(const U& u)
		{
			T(get<T>(u));
		} and requires(const U&& u)
		{
			T(get<T>(u));
		};

		template <typename T, typename U>
		concept has_get = usr_get<T, U> or std_get<T, U> or mem_get<T, U>;
	}

	template <typename T, typename U>
	requires detail::has_get<T, U>
	[[nodiscard]] constexpr decltype(auto) _get(const U& u) noexcept
	{
		if constexpr (detail::mem_get<T, U>)
			return u. template get<T>();
		else if constexpr (detail::usr_get<T, U>)
			return get<T>(u);
		else
			return std::get<T>(u);
	}
	template <typename T, typename U>
	requires detail::has_get<T, U>
	[[nodiscard]] constexpr decltype(auto) _get(const U&& u) noexcept
	{
		if constexpr (detail::mem_get<T, U>)
			return u. template get<T>();
		else if constexpr (detail::usr_get<T, U>)
			return get<T>(u);
		else
			return std::get<T>(u);
	}
	template <typename T, typename U>
	requires detail::has_get<T, U>
	[[nodiscard]] constexpr decltype(auto) _get(U& u) noexcept
	{
		if constexpr (detail::mem_get<T, U>)
			return u. template get<T>();
		else if constexpr (detail::usr_get<T, U>)
			return get<T>(u);
		else
			return std::get<T>(u);
	}
	template <typename T, typename U>
	requires detail::has_get<T, U>
	[[nodiscard]] constexpr decltype(auto) _get(U&& u) noexcept
	{
		if constexpr (detail::mem_get<T, U>)
			return u. template get<T>();
		else if constexpr (detail::usr_get<T, U>)
			return get<T>(u);
		else
			return std::get<T>(u);
	}
}

#endif
#endif
