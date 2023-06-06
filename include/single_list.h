#if not defined(BB5940288AC64C22BDB9118C9DAFD296)
#define BB5940288AC64C22BDB9118C9DAFD296
#if defined(BB5940288AC64C22BDB9118C9DAFD296)

#include "intrusive.h"
#include <iterator>
#include <limits>

namespace single_list {
	namespace detail {
		template <typename list_type, std::random_access_iterator base_type>
		struct single_list_node final
		{
			base_type base;
			using next_type = typename list_type::next_type;
			using difference_type = typename std::iterator_traits<base_type>::difference_type;

			difference_type link;
			[[nodiscard]] constexpr decltype(auto) next() const noexcept { return single_list_node{ base, difference_type(next_type(intrusive::_get<next_type>(base[link - 1]))) }; }

			constexpr auto next(const single_list_node& next) const noexcept { intrusive::_get<next_type>(base[link - 1]) = next_type(next.link); }

			[[nodiscard]] constexpr auto operator==(const single_list_node& other) const noexcept { return link == other.link; }
			[[nodiscard]] constexpr auto operator!=(const single_list_node& other) const noexcept { return link not_eq other.link; }
			[[nodiscard]] explicit constexpr operator bool() const noexcept { return bool(link); }
			[[nodiscard]] explicit constexpr operator difference_type() const noexcept { return link; }
		};

		template <typename list_type, std::random_access_iterator base_type>
		constexpr auto do_insert_after(single_list_node<list_type, base_type> x, single_list_node<list_type, base_type> z) noexcept
		{
			z.next(x.next());
			x.next(z);
		}
		template <typename list_type, std::random_access_iterator base_type>
		constexpr auto do_erase_after(single_list_node<list_type, base_type> z) noexcept
		{
			auto y = z.next().next();
			z.next(y);
		}
		template <typename list_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto do_init(single_list_node<list_type, base_type> z) noexcept
		{
			z.next(single_list_node<list_type, base_type>{});
			return z;
		}
		template <typename list_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto do_pop_front(single_list_node<list_type, base_type> x) noexcept
		{
			return x.next();
		}

		template <typename list_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto do_push_front(single_list_node<list_type, base_type> head, single_list_node<list_type, base_type> x) noexcept
		{
			x.next(head);
			return x;
		}

		template <typename list_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto do_node_relink(single_list_node<list_type, base_type> head, single_list_node<list_type, base_type> out_of_list, single_list_node<list_type, base_type> prev) noexcept
		{
			auto in_list = prev ? prev.next() : head;

			if (prev)
				prev.next(out_of_list);

			out_of_list.next(in_list.next());

			return in_list == head ? out_of_list : head;
		}

		template <typename list_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto do_node_swap(single_list_node<list_type, base_type> head, single_list_node<list_type, base_type> a_prev, single_list_node<list_type, base_type> b_prev) noexcept
		{
			auto a = a_prev ? a_prev.next() : head;
			auto b = b_prev ? b_prev.next() : head;

			auto a_next = a.next();
			auto b_next = b.next();

			if (a_prev)
				a_prev.next(b);
			if (b_prev)
				b_prev.next(a);


			a.next(a not_eq b_next ? b_next : b);
			b.next(b not_eq a_next ? a_next : a);

			return head == a ? b : head == b ? a : head;
		}

		template <typename list_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto check_index(single_list_node<list_type, base_type> curr, typename std::iterator_traits<base_type>::difference_type extent) noexcept
		{
			using T = typename std::iterator_traits<base_type>::difference_type;
			return T{} < T(curr) and T(curr) <= extent;
		}


		template <typename list_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto do_validate(single_list_node<list_type, base_type> curr, typename std::iterator_traits<base_type>::difference_type extent) noexcept
		{
			auto sz = decltype(extent){1};

			if (not curr)
				return true;
			if (not detail::check_index(curr, extent))
				return false;
			for (auto prev = curr; (curr = curr.next()); prev = curr, ++sz) {
				if (sz == extent)
					return false;
				if (not detail::check_index(curr, extent))
					return false;
			}

			return true;
		}
	}

	template <typename list_type, std::random_access_iterator base_type>
	struct forward_iter final
	{
		using value_type = std::iterator_traits<base_type>::value_type;
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using reference = std::iterator_traits<base_type>::reference;
		using pointer = std::iterator_traits<base_type>::pointer;
		using iterator_category = std::forward_iterator_tag;

		base_type base;
		std::iterator_traits<base_type>::difference_type link;
		constexpr decltype(auto) operator++() noexcept
		{
			link = difference_type(detail::single_list_node<list_type, base_type>{ base, link }.next());
			return *this;
		}
		[[nodiscard]] constexpr auto operator++(int) noexcept { auto copy = *this; ++(*this); return copy; }
		[[nodiscard]] constexpr decltype(auto) operator*() const noexcept { return base[link - 1]; }
		[[nodiscard]] constexpr auto operator->() const noexcept { return base + (link - 1); }
		[[nodiscard]] constexpr decltype(auto) operator*() noexcept { return base[link - 1]; }
		[[nodiscard]] constexpr auto operator->() noexcept { return base + (link - 1); }
		[[nodiscard]] constexpr auto operator==(const forward_iter& other) const noexcept { return link == other.link; }
		[[nodiscard]] constexpr auto operator!=(const forward_iter& other) const noexcept { return link not_eq other.link; }
	};

	template <typename list_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto end(base_type base) noexcept
	{
		return forward_iter<list_type, base_type>{ base, {} };
	}

	template <typename list_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto begin(base_type base, typename std::iterator_traits<base_type>::difference_type head) noexcept
	{
		return forward_iter<list_type, base_type>{ base, head };
	}

	template <typename list_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto make_iterator(base_type base, base_type it) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		return forward_iter<list_type, base_type>{ base, difference_type(std::distance(base, it) + 1) };
	}

	template <typename list_type, std::random_access_iterator base_type>
	constexpr auto insert_after(forward_iter<list_type, base_type> pos, base_type x) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using node_type = detail::single_list_node<list_type, base_type>;
		detail::do_insert_after(node_type{ pos.base, pos.link }, node_type{ pos.base, difference_type(std::distance(pos.base, x) + 1) });
	}

	template <typename list_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto push_front(base_type base, typename std::iterator_traits<base_type>::difference_type head, base_type it) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using node_type = detail::single_list_node<list_type, base_type>;
		if (head)
			return difference_type(detail::do_push_front(node_type{ base, head }, node_type{ base, difference_type(std::distance(base, it) + 1) }));
		else
			return difference_type(detail::do_init(node_type{ base, difference_type(std::distance(base, it) + 1) }));
	}

	template <typename list_type, std::random_access_iterator base_type>
	constexpr auto erase_after(forward_iter<list_type, base_type> it) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using node_type = detail::single_list_node<list_type, base_type>;
		detail::do_erase_after(node_type{ it.base, it.link });
	}

	template <typename list_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto pop_front(base_type base, typename std::iterator_traits<base_type>::difference_type head) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using node_type = detail::single_list_node<list_type, base_type>;
		return difference_type(detail::do_pop_front(node_type{ base, head }));
	}

	template <std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto empty(typename std::iterator_traits<base_type>::difference_type head) noexcept
	{
		return not head;
	}

	template <std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto max_size() noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		return difference_type(std::numeric_limits<difference_type>::max() - 1);
	}

	template <typename list_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto size(base_type base, typename std::iterator_traits<base_type>::difference_type root) noexcept
	{
		return std::distance(begin<list_type>(base, root), end<list_type>(base));
	}

	template <typename list_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto node_swap(typename std::iterator_traits<base_type>::difference_type head, forward_iter<list_type, base_type> a_prev, forward_iter<list_type, base_type> b_prev) noexcept
	{
		using node_type = detail::single_list_node<list_type, base_type>;
		using difference_type = std::iterator_traits<base_type>::difference_type;
		return difference_type(detail::do_node_swap(node_type{ a_prev.base, head }, node_type{ a_prev.base, a_prev.link }, node_type{ b_prev.base, b_prev.link }));
	}

	template <typename list_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto node_relink(typename std::iterator_traits<base_type>::difference_type head, base_type dst, forward_iter<list_type, base_type> src_prev) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using node_type = detail::single_list_node<list_type, base_type>;
		return difference_type(detail::do_node_relink(node_type{ src_prev.base, head }, node_type{ src_prev.base, difference_type(std::distance(src_prev.base, dst) + 1) }, node_type{ src_prev.base, src_prev.link}));
	}

	template <typename list_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto validate(base_type first, base_type last, typename std::iterator_traits<base_type>::difference_type head) noexcept
	{
		using node_type = detail::single_list_node<list_type, base_type>;
		return detail::do_validate(node_type{ first, head }, std::distance(first, last));
	}
}

#endif
#endif
