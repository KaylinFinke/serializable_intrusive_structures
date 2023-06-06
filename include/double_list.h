#if not defined(C133C1CF8A2F41C59893CB2734D85167)
#define C133C1CF8A2F41C59893CB2734D85167
#if defined(C133C1CF8A2F41C59893CB2734D85167)

#include "intrusive.h"
#include <iterator>
#include <limits>

namespace double_list {
	namespace detail {

		template <typename list_type, std::random_access_iterator base_type>
		struct double_list_node final
		{
			base_type base;
			using next_type = typename list_type::next_type;
			using prev_type = typename list_type::prev_type;
			using difference_type = typename std::iterator_traits<base_type>::difference_type;

			difference_type link;
			[[nodiscard]] constexpr decltype(auto) next() const noexcept { return double_list_node{ base, difference_type(next_type(intrusive::_get<next_type>(base[link - 1]))) }; }
			[[nodiscard]] constexpr decltype(auto) prev() const noexcept { return double_list_node{ base, difference_type(prev_type(intrusive::_get<prev_type>(base[link - 1]))) }; }

			constexpr auto next(const double_list_node& next) const noexcept { intrusive::_get<next_type>(base[link - 1]) = next_type(next.link); }
			constexpr auto prev(const double_list_node& prev) const noexcept { intrusive::_get<prev_type>(base[link - 1]) = prev_type(prev.link); }

			[[nodiscard]] constexpr auto operator==(const double_list_node& other) const noexcept { return link == other.link; }
			[[nodiscard]] constexpr auto operator!=(const double_list_node& other) const noexcept { return link not_eq other.link; }
			[[nodiscard]] explicit constexpr operator bool() const noexcept { return bool(link); }
			[[nodiscard]] explicit constexpr operator difference_type() const noexcept { return link; }
		};

		template <typename list_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto do_insert_after(double_list_node<list_type, base_type> tail, double_list_node<list_type, base_type> x, double_list_node<list_type, base_type> z) noexcept
		{
			z.prev(x);
			z.next(x.next());
			auto y = z.next();
			if (y)
				y.prev(z);
			x.next(z);
			return y ? tail : z;
		}
		template <typename list_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto do_insert_before(double_list_node<list_type, base_type> head, double_list_node<list_type, base_type> x, double_list_node<list_type, base_type> z) noexcept
		{
			z.next(x);
			z.prev(x.prev());
			auto y = z.prev();
			if (y)
				y.next(z);
			x.prev(z);
			return y ? head : z;
		}
		template <typename list_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto do_erase_after(double_list_node<list_type, base_type> tail, double_list_node<list_type, base_type> z) noexcept
		{
			auto y = z.next().next();
			if (y)
				y.prev(z);
			z.next(y);
			return y ? tail : z;
		}
		template <typename list_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto do_erase_before(double_list_node<list_type, base_type> head, double_list_node<list_type, base_type> z) noexcept
		{
			auto y = z.prev().prev();
			if (y)
				y.next(z);
			z.prev(y);
			return y ? head : z;
		}
		template <typename list_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto do_init(double_list_node<list_type, base_type> z) noexcept
		{
			z.next(double_list_node<list_type, base_type>{});
			z.prev(double_list_node<list_type, base_type>{});
			return z;
		}
		template <typename list_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto do_pop_front(double_list_node<list_type, base_type> x) noexcept
		{
			auto y = x.next();
			if (y)
				y.prev(double_list_node<list_type, base_type>{});
			return y;
		}
		template <typename list_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto do_pop_back(double_list_node<list_type, base_type> z) noexcept
		{
			auto y = z.prev();
			if (y)
				y.next(double_list_node<list_type, base_type>{});
			return y;
		}

		template <typename list_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto do_node_relink(double_list_node<list_type, base_type> head, double_list_node<list_type, base_type> tail, double_list_node<list_type, base_type> out_of_list, double_list_node<list_type, base_type> in_list) noexcept
		{
			auto next = in_list.next();
			auto prev = in_list.prev();
			out_of_list.prev(prev);
			out_of_list.next(next);
			if (prev)
				prev.next(out_of_list);
			if (next)
				next.prev(out_of_list);
			return std::pair{ (in_list == head ? out_of_list : head).link, (in_list == tail ? out_of_list : tail).link };
		}

		template <typename list_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto do_node_swap(double_list_node<list_type, base_type> head, double_list_node<list_type, base_type> tail, double_list_node<list_type, base_type> a, double_list_node<list_type, base_type> b) noexcept
		{
			auto a_prev = a.prev();
			auto a_next = a.next();
			auto b_prev = b.prev();
			auto b_next = b.next();

			if (a_prev)
				a_prev.next(b);
			if (a_next)
				a_next.prev(b);
			if (b_prev)
				b_prev.next(a);
			if (b_next)
				b_next.prev(a);

			a.prev(a not_eq b_prev ? b_prev : b);
			a.next(a not_eq b_next ? b_next : b);
			b.prev(b not_eq a_prev ? a_prev : a);
			b.next(b not_eq a_next ? a_next : a);

			return std::pair{ (head == a ? b : head == b ? a : head).link, (tail == a ? b : tail == b ? a : tail).link };
		}

		template <typename list_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto check_index(double_list_node<list_type, base_type> curr, typename std::iterator_traits<base_type>::difference_type extent) noexcept
		{
			using T = typename std::iterator_traits<base_type>::difference_type;
			return T{} < T(curr) and T(curr) <= extent;
		}

		template <typename list_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto check_link(double_list_node<list_type, base_type> curr, double_list_node<list_type, base_type> prev) noexcept
		{
			return curr.prev() == prev;
		}

		template <typename list_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto do_validate(double_list_node<list_type, base_type> curr, typename std::iterator_traits<base_type>::difference_type extent) noexcept
		{
			if (not curr)
				return true;
			if (not detail::check_index(curr, extent))
				return false;
			if (not detail::check_link(curr, decltype(curr){}))
				return false;
			for (auto prev = curr; (curr = curr.next()); prev = curr) {
				if (not detail::check_index(curr, extent))
					return false;
				if (not detail::check_link(curr, prev))
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
			link = difference_type(detail::double_list_node<list_type, base_type>{ base, link }.next());
			return *this;
		}
		[[nodiscard]] constexpr auto operator++(int) noexcept { auto copy = *this; ++(*this); return copy; }
		constexpr decltype(auto) operator--() noexcept
		{
			link = difference_type(detail::double_list_node<list_type, base_type>{ base, link }.prev());
			return *this;
		}
		[[nodiscard]] constexpr auto operator--(int) noexcept { auto copy = *this; --(*this); return copy; }
		[[nodiscard]] constexpr decltype(auto) operator*() const noexcept { return base[link - 1]; }
		[[nodiscard]] constexpr auto operator->() const noexcept { return base + (link - 1); }
		[[nodiscard]] constexpr decltype(auto) operator*() noexcept { return base[link - 1]; }
		[[nodiscard]] constexpr auto operator->() noexcept { return base + (link - 1); }
		[[nodiscard]] constexpr auto operator==(const forward_iter& other) const noexcept { return link == other.link; }
		[[nodiscard]] constexpr auto operator!=(const forward_iter& other) const noexcept { return link not_eq other.link; }
	};

	template <typename list_type, std::random_access_iterator base_type>
	struct reverse_iter final
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
			link = difference_type(detail::double_list_node<list_type, base_type>{ base, link }.prev());
			return *this;
		}
		[[nodiscard]] constexpr auto operator++(int) noexcept { auto copy = *this; ++(*this); return copy; }
		constexpr decltype(auto) operator--() noexcept
		{
			link = difference_type(detail::double_list_node<list_type, base_type>{ base, link }.next());
			return *this;
		}
		[[nodiscard]] constexpr auto operator--(int) noexcept { auto copy = *this; --(*this); return copy; }
		[[nodiscard]] constexpr decltype(auto) operator*() const noexcept { return base[link - 1]; }
		[[nodiscard]] constexpr auto operator->() const noexcept { return base + (link - 1); }
		[[nodiscard]] constexpr decltype(auto) operator*() noexcept { return base[link - 1]; }
		[[nodiscard]] constexpr auto operator->() noexcept { return base + (link - 1); }
		[[nodiscard]] constexpr auto operator==(const reverse_iter& other) const noexcept { return link == other.link; }
		[[nodiscard]] constexpr auto operator!=(const reverse_iter& other) const noexcept { return link not_eq other.link; }
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
	[[nodiscard]] constexpr auto rend(base_type base) noexcept
	{
		return reverse_iter<list_type, base_type>{ base, {} };
	}

	template <typename list_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto rbegin(base_type base, typename std::iterator_traits<base_type>::difference_type tail) noexcept
	{
		return reverse_iter<list_type, base_type>{ base, tail };
	}

	template <typename list_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto make_reverse_iterator(base_type base, base_type it) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		return reverse_iter<list_type, base_type>{ base, difference_type(std::distance(base, it) + 1) };
	}

	template <typename list_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto insert_after(typename std::iterator_traits<base_type>::difference_type tail, forward_iter<list_type, base_type> pos, base_type x) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using node_type = detail::double_list_node<list_type, base_type>;
		return difference_type(detail::do_insert_after(node_type{ pos.base, tail }, node_type{ pos.base, pos.link }, node_type{ pos.base, difference_type(std::distance(pos.base, x) + 1) }));
	}

	template <typename list_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto insert_before(typename std::iterator_traits<base_type>::difference_type head, forward_iter<list_type, base_type> pos, base_type x) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using node_type = detail::double_list_node<list_type, base_type>;
		return difference_type(detail::do_insert_before(node_type{ pos.base, head }, node_type{ pos.base, pos.link }, node_type{ pos.base, difference_type(std::distance(pos.base, x) + 1) }));
	}

	template <typename list_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto push_front(base_type base, typename std::iterator_traits<base_type>::difference_type head, base_type it) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using node_type = detail::double_list_node<list_type, base_type>;
		if (head)
			return double_list::insert_before(head, forward_iter<list_type, base_type>{base, head}, it);
		else
			return difference_type(detail::do_init(node_type{ base, difference_type(std::distance(base, it) + 1) }));
	}

	template <typename list_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto push_back(base_type base, typename std::iterator_traits<base_type>::difference_type tail, base_type it) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using node_type = detail::double_list_node<list_type, base_type>;
		if (tail)
			return double_list::insert_after(tail, forward_iter<list_type, base_type>{base, tail}, it);
		else
			return difference_type(detail::do_init(node_type{ base, difference_type(std::distance(base, it) + 1) }));
	}

	template <typename list_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto erase_after(typename std::iterator_traits<base_type>::difference_type tail, forward_iter<list_type, base_type> it) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using node_type = detail::double_list_node<list_type, base_type>;
		return difference_type(detail::do_erase_after(node_type{ it.base, tail }, node_type{ it.base, it.link }));
	}

	template <typename list_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto erase_before(typename std::iterator_traits<base_type>::difference_type head, forward_iter<list_type, base_type> it) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using node_type = detail::double_list_node<list_type, base_type>;
		return difference_type(detail::do_erase_before(node_type{ it.base, head }, node_type{ it.base, it.link }));
	}

	template <typename list_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto pop_front(base_type base, typename std::iterator_traits<base_type>::difference_type head) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using node_type = detail::double_list_node<list_type, base_type>;
		return difference_type(detail::do_pop_front(node_type{ base, head }));
	}

	template <typename list_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto pop_back(base_type base, typename std::iterator_traits<base_type>::difference_type tail) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using node_type = detail::double_list_node<list_type, base_type>;
		return difference_type(detail::do_pop_back(node_type{ base, tail }));
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
	[[nodiscard]] constexpr auto node_swap(typename std::iterator_traits<base_type>::difference_type head, typename std::iterator_traits<base_type>::difference_type tail, forward_iter<list_type, base_type> a, forward_iter<list_type, base_type> b) noexcept
	{
		using node_type = detail::double_list_node<list_type, base_type>;
		return detail::do_node_swap(node_type{ a.base, head }, node_type{ a.base, tail }, node_type{ a.base, a.link }, node_type{ b.base, b.link });
	}

	template <typename list_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto node_relink(typename std::iterator_traits<base_type>::difference_type head, typename std::iterator_traits<base_type>::difference_type tail, base_type dst, forward_iter<list_type, base_type> src) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using node_type = detail::double_list_node<list_type, base_type>;
		return detail::do_node_relink(node_type{ src.base, head }, node_type{ src.base, tail }, node_type{ src.base, difference_type(std::distance(src.base, dst) + 1) }, node_type{ src.base, src.link });
	}

	template <typename list_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto validate(base_type first, base_type last, typename std::iterator_traits<base_type>::difference_type head) noexcept
	{
		using node_type = detail::double_list_node<list_type, base_type>;
		return detail::do_validate(node_type{ first, head }, std::distance(first, last));
	}
}

#endif
#endif
