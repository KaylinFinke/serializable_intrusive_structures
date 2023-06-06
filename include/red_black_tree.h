#if not defined(E7A959942F3144E59D41B23FC49920AF)
#define E7A959942F3144E59D41B23FC49920AF
#if defined(E7A959942F3144E59D41B23FC49920AF)

#include "intrusive.h"
#include <iterator>
#include <limits>
#include <optional>

namespace red_black_tree {
	namespace detail {
		enum class color : bool { red, black };

		template <typename tree_type, std::random_access_iterator base_type>
		struct red_black_tree_node final
		{
			base_type base;
			using color_type = typename tree_type::color_type;
			using left_type = typename tree_type::left_type;
			using right_type = typename tree_type::right_type;
			using parent_type = typename tree_type::parent_type;
			using key_type = typename tree_type::key_type;
			using difference_type = typename std::iterator_traits<base_type>::difference_type;

			difference_type root;
			[[nodiscard]] constexpr decltype(auto) left() const noexcept { return red_black_tree_node{ base, difference_type(left_type(intrusive::_get<left_type>(base[root - 1]))) }; }
			[[nodiscard]] constexpr decltype(auto) right() const noexcept { return red_black_tree_node{ base, difference_type(right_type(intrusive::_get<right_type>(base[root - 1]))) }; }
			[[nodiscard]] constexpr decltype(auto) parent() const noexcept { return red_black_tree_node{ base, difference_type(parent_type(intrusive::_get<parent_type>(base[root - 1]))) }; }
			[[nodiscard]] constexpr decltype(auto) color() const noexcept { return root ? static_cast<enum color>(color_type(intrusive::_get<color_type>(base[root - 1]))) : color::black; }

			constexpr auto left(const red_black_tree_node& left) const noexcept { intrusive::_get<left_type>(base[root - 1]) = left_type(left.root); }
			constexpr auto right(const red_black_tree_node& right) const noexcept { intrusive::_get<right_type>(base[root - 1]) = right_type(right.root); }
			constexpr auto parent(const red_black_tree_node& parent) const noexcept { intrusive::_get<parent_type>(base[root - 1]) = parent_type(parent.root); }
			constexpr auto color(enum color color) const noexcept { intrusive::_get<color_type>(base[root - 1]) = color_type(color); }
			[[nodiscard]] constexpr decltype(auto) key() const noexcept
			{
				if constexpr (std::is_same_v<key_type, std::remove_cvref_t<decltype(intrusive::_get<key_type>(*std::declval<base_type>()))>> )
					return intrusive::_get<key_type>(base[root - 1]);
				else
					return key_type(intrusive::_get<key_type>(base[root - 1]));
			}
			[[nodiscard]] constexpr auto operator==(const red_black_tree_node& other) const noexcept { return root == other.root; }
			[[nodiscard]] constexpr auto operator!=(const red_black_tree_node& other) const noexcept { return root not_eq other.root; }
			[[nodiscard]] explicit constexpr operator bool() const noexcept { return bool(root); }
			[[nodiscard]] explicit constexpr operator difference_type() const noexcept { return root; }
		};

		template <typename tree_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto left_rotate(red_black_tree_node<tree_type, base_type> root, const red_black_tree_node<tree_type, base_type> x) noexcept
		{
			const auto y = x.right();
			x.right(y.left());
			if (y.left())
				y.left().parent(x);
			y.parent(x.parent());
			if (not x.parent())
				root = y;
			else if (x == x.parent().left())
				x.parent().left(y);
			else
				x.parent().right(y);
			y.left(x);
			x.parent(y);
			return root;
		}

		template <typename tree_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto right_rotate(red_black_tree_node<tree_type, base_type> root, const red_black_tree_node<tree_type, base_type> x) noexcept
		{
			const auto y = x.left();
			x.left(y.right());
			if (y.right())
				y.right().parent(x);
			y.parent(x.parent());
			if (not x.parent())
				root = y;
			else if (x == x.parent().left())
				x.parent().left(y);
			else
				x.parent().right(y);
			y.right(x);
			x.parent(y);
			return root;
		}

		template <typename tree_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto insert_fixup(red_black_tree_node<tree_type, base_type> root, red_black_tree_node<tree_type, base_type> z) noexcept
		{
			auto y = red_black_tree_node<tree_type, base_type>{};
			while (color::red == z.parent().color()) {
				if (z.parent() == z.parent().parent().left()) {
					y = z.parent().parent().right();
					if (color::red == y.color()) {
						z.parent().color(color::black);
						y.color(color::black);
						z.parent().parent().color(color::red);
						z = z.parent().parent();
					} else {
						if (z == z.parent().right()) {
							z = z.parent();
							root = detail::left_rotate(root, z);
						}
						z.parent().color(color::black);
						z.parent().parent().color(color::red);
						root = detail::right_rotate(root, z.parent().parent());
					}
				} else {
					y = z.parent().parent().left();
					if (color::red == y.color()) {
						z.parent().color(color::black);
						y.color(color::black);
						z.parent().parent().color(color::red);
						z = z.parent().parent();
					} else {
						if (z == z.parent().left()) {
							z = z.parent();
							root = detail::right_rotate(root, z);
						}
						z.parent().color(color::black);
						z.parent().parent().color(color::red);
						root = detail::left_rotate(root, z.parent().parent());
					}
				}
			}
			root.color(color::black);
			return root;
		}

		template <typename tree_type, std::random_access_iterator base_type, typename less_type>
		[[nodiscard]] constexpr auto do_insert(red_black_tree_node<tree_type, base_type> root, red_black_tree_node<tree_type, base_type> z, less_type less) noexcept
		{
			auto y = red_black_tree_node<tree_type, base_type>{};
			auto x = root;
			while (x) {
				y = x;
				if (less(z.key(), x.key()))
					x = x.left();
				else
					x = x.right();
			}
			z.color(color::red);
			z.left(red_black_tree_node<tree_type, base_type>{});
			z.right(red_black_tree_node<tree_type, base_type>{});
			z.parent(y);
			if (not y)
				root = z;
			else if (less(z.key(), y.key()))
				y.left(z);
			else
				y.right(z);
			return detail::insert_fixup(root, z);
		}

		template <typename tree_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto min(red_black_tree_node<tree_type, base_type> x) noexcept
		{
			while (x.left())
				x = x.left();
			return x;
		}

		template <typename tree_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto max(red_black_tree_node<tree_type, base_type> x) noexcept
		{
			while (x.right())
				x = x.right();
			return x;
		}

		template <typename tree_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto successor(red_black_tree_node<tree_type, base_type> x) noexcept
		{
			auto y = red_black_tree_node<tree_type, base_type>{};
			if (x.right())
				return detail::min(x.right());
			y = x.parent();
			while (y and x == y.right()) {
				x = y;
				y = y.parent();
			}
			return y;
		}

		template <typename tree_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto predecessor(red_black_tree_node<tree_type, base_type> x) noexcept
		{
			auto y = red_black_tree_node<tree_type, base_type>{};
			if (x.left())
				return detail::max(x.left());
			y = x.parent();
			while (y and x == y.left()) {
				x = y;
				y = y.parent();
			}
			return y;
		}

		template <typename tree_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto delete_fixup(red_black_tree_node<tree_type, base_type> root, red_black_tree_node<tree_type, base_type> x, red_black_tree_node<tree_type, base_type> xp) noexcept
		{
			auto w = red_black_tree_node<tree_type, base_type>{};
			while (root not_eq x and color::black == x.color()) {
				if (xp and xp.left() == x) {
					w = xp.right();
					if (color::red == w.color()) {
						w.color(color::black);
						xp.color(color::red);
						root = detail::left_rotate(root, xp);
						w = xp.right();
					}
					if (color::black == w.left().color() and color::black == w.right().color()) {
						w.color(color::red);
						x = xp;
						xp = x.parent();
					} else {
						if (color::black == w.right().color()) {
							w.left().color(color::black);
							w.color(color::red);
							root = detail::right_rotate(root, w);
							w = xp.right();
						}
						w.color(xp.color());
						xp.color(color::black);
						if (w.right())
							w.right().color(color::black);
						x = root = detail::left_rotate(root, xp);
					}
				} else {
					w = xp.left();
					if (color::red == w.color()) {
						w.color(color::black);
						xp.color(color::red);
						root = detail::right_rotate(root, xp);
						w = xp.left();
					}
					if (color::black == w.left().color() and color::black == w.right().color()) {
						w.color(color::red);
						x = xp;
						xp = x.parent();
					} else {
						if (color::black == w.left().color()) {
							w.right().color(color::black);
							w.color(color::red);
							root = detail::left_rotate(root, w);
							w = xp.left();
						}
						w.color(xp.color());
						xp.color(color::black);
						if (w.left())
							w.left().color(color::black);
						x = root = detail::right_rotate(root, xp);
					}
				}
			}
			if (x)
				x.color(color::black);
			return root;
		}

		template <typename tree_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto do_node_relink(red_black_tree_node<tree_type, base_type> root, red_black_tree_node<tree_type, base_type> out_of_tree, red_black_tree_node<tree_type, base_type> in_tree) noexcept
		{
			if (in_tree.parent()) {
				if (in_tree.parent().left() == in_tree)
					in_tree.parent().left(out_of_tree);
				else
					in_tree.parent().right(out_of_tree);
			}
			if (in_tree.left())
				in_tree.left().parent(out_of_tree);
			if (in_tree.right())
				in_tree.right().parent(out_of_tree);
			if (root == in_tree)
				root = out_of_tree;
			out_of_tree.color(in_tree.color());
			out_of_tree.left(in_tree.left());
			out_of_tree.right(in_tree.right());
			out_of_tree.parent(in_tree.parent());
			return root;
		}

		template <typename tree_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto do_node_swap(red_black_tree_node<tree_type, base_type> root, red_black_tree_node<tree_type, base_type> a, red_black_tree_node<tree_type, base_type> b) noexcept
		{
			auto a_parent = a.parent();
			auto a_left = a.left();
			auto a_right = a.right();
			auto a_is_left = a_parent and a_parent.left() == a;
			auto b_parent = b.parent();
			auto b_left = b.left();
			auto b_right = b.right();
			auto b_is_left = b_parent and b_parent.left() == b;

			if (b_is_left)
				b_parent.left(a);
			else if (b_parent)
				b_parent.right(a);
			if (b_right)
				b_right.parent(a);
			if (b_left)
				b_left.parent(a);

			if (a_is_left)
				a_parent.left(b);
			else if (a_parent)
				a_parent.right(b);
			if (a_right)
				a_right.parent(b);
			if (a_left)
				a_left.parent(b);

			auto a_color = a.color();
			a.color(b.color());
			b.color(a_color);

			a.left(b_left not_eq a ? b_left : b);
			a.right(b_right not_eq a ? b_right : b);
			a.parent(b_parent not_eq a ? b_parent : b);
	
			b.left(a_left not_eq b ? a_left : a);
			b.right(a_right not_eq b ? a_right : a);
			b.parent(a_parent not_eq b ? a_parent : a);

			return a == root ? b : b == root ? a : root;
		}

		template <typename tree_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto do_erase(red_black_tree_node<tree_type, base_type> root, const red_black_tree_node<tree_type, base_type> target_node) noexcept
		{
			auto node_removed = target_node;
			if (target_node.left() and target_node.right())
				node_removed = detail::successor(target_node);
			auto removed_child = node_removed.left() ? node_removed.left() : node_removed.right();
			if (not node_removed.parent())
				root = removed_child;
			else if (node_removed == node_removed.parent().left())
				node_removed.parent().left(removed_child);
			else
				node_removed.parent().right(removed_child);
			if (removed_child)
				removed_child.parent(node_removed.parent());
			auto removed_color = node_removed.color();
			auto removed_parent = node_removed.parent() == target_node ? node_removed : node_removed.parent();
			if (node_removed not_eq target_node)
				root = detail::do_node_relink(root, node_removed, target_node);
			if (color::black == removed_color)
				root = detail::delete_fixup(root, removed_child, removed_parent);
			return root;
		}

		template <typename tree_type, std::random_access_iterator base_type, typename V, typename less_type>
		[[nodiscard]] constexpr auto do_upper_bound(red_black_tree_node<tree_type, base_type> x, const V& value, less_type less) noexcept
		{
			auto z = red_black_tree_node<tree_type, base_type>{};
			while (x)
				if (less(value, x.key())) {
					z = x;
					x = x.left();
				} else
					x = x.right();
			return z;
		}

		template <typename tree_type, std::random_access_iterator base_type, typename V, typename less_type>
		[[nodiscard]] constexpr auto do_lower_bound(red_black_tree_node<tree_type, base_type> x, const V& value, less_type less) noexcept
		{
			auto z = red_black_tree_node<tree_type, base_type>{};
			while (x)
				if (not less(x.key(), value)) {
					z = x;
					x = x.left();
				} else
					x = x.right();
			return z;
		}

		template <typename tree_type, std::random_access_iterator base_type, typename V, typename less_type>
		[[nodiscard]] constexpr auto do_find(red_black_tree_node<tree_type, base_type> x, const V& value, less_type less) noexcept
		{
			x = detail::do_lower_bound(x, value, less);
			if (x and not less(value, x.key()))
				return x;
			return red_black_tree_node<tree_type, base_type>{};
		}

		template <typename tree_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto check_index(red_black_tree_node<tree_type, base_type> curr, typename std::iterator_traits<base_type>::difference_type extent) noexcept
		{
			using T = typename std::iterator_traits<base_type>::difference_type;
			return T{} < T(curr) and T(curr) <= extent;
		}

		template <typename tree_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto check_link(red_black_tree_node<tree_type, base_type> curr, red_black_tree_node<tree_type, base_type> parent) noexcept
		{
			return curr.parent() == parent and (not curr.left() or not curr.right() or curr.left() not_eq curr.right());
		}


		template <typename tree_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto checked_minimum(red_black_tree_node<tree_type, base_type> curr, typename std::iterator_traits<base_type>::difference_type extent, typename std::iterator_traits<base_type>::difference_type curr_height) noexcept -> decltype(std::make_optional(std::pair{ curr, curr_height }))
		{
			for (; curr.left(); curr = curr.left()) {
				if (not detail::check_index(curr.left(), extent))
					return std::nullopt;
				if (not detail::check_link(curr.left(), curr))
					return std::nullopt;
				if (curr.left().color() == color::black) 
					++curr_height;
			}
			return std::make_optional(std::pair{ curr, curr_height });
		}

		template <typename tree_type, std::random_access_iterator base_type>
		[[nodiscard]] constexpr auto checked_successor(red_black_tree_node<tree_type, base_type> curr, typename std::iterator_traits<base_type>::difference_type extent, typename std::iterator_traits<base_type>::difference_type curr_height) noexcept -> decltype(std::make_optional(std::pair{ curr, curr_height }))
		{
			if (curr.right()) {
				if (not detail::check_index(curr.right(), extent))
					return std::nullopt;
				if (not detail::check_link(curr.right(), curr))
					return std::nullopt;
				if (curr.right().color() == color::black)
					++curr_height;
				if (auto min = detail::checked_minimum(curr.right(), extent, curr_height))
					std::tie(curr, curr_height) = min.value();
				else
					return std::nullopt;
			} else 
				for (auto done = false; not done; done = not curr.parent() or curr.parent().right() not_eq curr, curr = curr.parent())
					if (curr.color() == color::black) --curr_height;

			return std::make_optional(std::pair{curr, curr_height});
		}


		template <typename tree_type, std::random_access_iterator base_type, typename less_type>
		[[nodiscard]] constexpr auto do_validate(red_black_tree_node<tree_type, base_type> curr, typename std::iterator_traits<base_type>::difference_type extent, less_type less) noexcept
		{	
			auto black_height = typename std::iterator_traits<base_type>::difference_type{1};

			if (not curr) 
				return true;
			if (not detail::check_index(curr, extent))
				return false;
			if (not detail::check_link(curr, decltype(curr){}))
				return false;
			if (curr.color() not_eq color::black)
				return false;

			if (auto min = detail::checked_minimum(curr, extent, black_height))
				std::tie(curr, black_height) = min.value();
			else
				return false;

			auto curr_height = black_height;

			while (auto prev = curr) {
				if ((not curr.left() or not curr.right()) and curr_height not_eq black_height)
					return false;
				if (curr.color() == color::red and curr.parent().color() == color::red)
					return false;

				if (auto next = detail::checked_successor(curr, extent, curr_height))
					std::tie(curr, curr_height) = next.value();
				else
					return false;

				if (curr and less(curr.key(), prev.key()))
					return false;
			}

			return true;
		}
	}

	/* Implements a forward iterator for an intrusive balanced binary search tree. tree_type provides types named
	 * left_type, right_type, parent_type, key_type, and color_type which are unique types explicitly convertable to
	 * difference_type for the first 3, and bool for color. the value_type of base_type must implement type based get
	 * and set methods for these types, and these types must be unique to each other. get and set methods need not return
	 * values can can return references instead, though this probably isn't meaningful for types other than the key type.
	 * This class implements bidirectional iterator except end is not decrementable.
	*/
	template <typename tree_type, std::random_access_iterator base_type>
	struct forward_iter final
	{
		using value_type = std::iterator_traits<base_type>::value_type;
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using reference = std::iterator_traits<base_type>::reference;
		using pointer = std::iterator_traits<base_type>::pointer;
		using iterator_category = std::forward_iterator_tag;

		base_type base;
		std::iterator_traits<base_type>::difference_type root;
		constexpr decltype(auto) operator++() noexcept
		{
			root = difference_type(detail::successor(detail::red_black_tree_node<tree_type, base_type>{ base, root }));
			return *this;
		}
		[[nodiscard]] constexpr auto operator++(int) noexcept { auto copy = *this; ++(*this); return copy; }
		constexpr decltype(auto) operator--() noexcept
		{
			root = difference_type(detail::predecessor(detail::red_black_tree_node<tree_type, base_type>{ base, root }));
			return *this;
		}
		[[nodiscard]] constexpr auto operator--(int) noexcept { auto copy = *this; --(*this); return copy; }
		[[nodiscard]] constexpr decltype(auto) operator*() const noexcept { return base[root - 1]; }
		[[nodiscard]] constexpr auto operator->() const noexcept { return base + (root - 1); }
		[[nodiscard]] constexpr decltype(auto) operator*() noexcept { return base[root - 1]; }
		[[nodiscard]] constexpr auto operator->() noexcept { return base + (root - 1); }
		[[nodiscard]] constexpr auto operator==(const forward_iter& other) const noexcept { return root == other.root; }
		[[nodiscard]] constexpr auto operator!=(const forward_iter& other) const noexcept { return root not_eq other.root; }
	};

	/* Implements reverse iterator. This is the mirror of the forward iterator class with all the same guarentees.
	*/
	template <typename tree_type, std::random_access_iterator base_type>
	struct reverse_iter final
	{
		using value_type = std::iterator_traits<base_type>::value_type;
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using reference = std::iterator_traits<base_type>::reference;
		using pointer = std::iterator_traits<base_type>::pointer;
		using iterator_category = std::forward_iterator_tag;

		base_type base;
		std::iterator_traits<base_type>::difference_type root;
		constexpr decltype(auto) operator++() noexcept
		{
			root = difference_type(detail::predecessor(detail::red_black_tree_node<tree_type, base_type>{ base, root }));
			return *this;
		}
		[[nodiscard]] constexpr auto operator++(int) noexcept { auto copy = *this; ++(*this); return copy; }
		constexpr decltype(auto) operator--() noexcept
		{
			root = difference_type(detail::successor(detail::red_black_tree_node<tree_type, base_type>{ base, root }));
			return *this;
		}
		[[nodiscard]] constexpr auto operator--(int) noexcept { auto copy = *this; --(*this); return copy; }
		[[nodiscard]] constexpr decltype(auto) operator*() const noexcept { return base[root - 1]; }
		[[nodiscard]] constexpr auto operator->() const noexcept { return base + (root - 1); }
		[[nodiscard]] constexpr decltype(auto) operator*() noexcept { return base[root - 1]; }
		[[nodiscard]] constexpr auto operator->() noexcept { return base + (root - 1); }
		[[nodiscard]] constexpr auto operator==(const reverse_iter& other) const noexcept { return root == other.root; }
		[[nodiscard]] constexpr auto operator!=(const reverse_iter& other) const noexcept { return root not_eq other.root; }
	};

	template <typename tree_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto end(base_type base) noexcept
	{
		return forward_iter<tree_type, base_type>{ base, {} };
	}

	template <typename tree_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto begin(base_type base, typename std::iterator_traits<base_type>::difference_type root) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using node_type = detail::red_black_tree_node<tree_type, base_type>;
		return root ? forward_iter<tree_type, base_type>{ base, difference_type(detail::min(node_type{ base, root })) } : end<tree_type>(base);
	}

	// Makes an iterator from a random access iterator and a base pointer. be careful that it points to an element of the structure, or will by the time it's used.
	template <typename tree_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto make_iterator(base_type base, base_type it) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		return forward_iter<tree_type, base_type>{ base, difference_type(std::distance(base, it) + 1) };
	}

	template <typename tree_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto rend(base_type base) noexcept
	{
		return reverse_iter<tree_type, base_type>{ base, {} };
	}

	template <typename tree_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto rbegin(base_type base, typename std::iterator_traits<base_type>::difference_type root) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using node_type = detail::red_black_tree_node<tree_type, base_type>;
		return root ? reverse_iter<tree_type, base_type>{ base, difference_type(detail::max(node_type{ base, root })) } : rend<tree_type>(base);
	}

	// Makes an iterator from a random access iterator and a base pointer. be careful that it points to an element of the structure, or will by the time it's used.
	template <typename tree_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto make_reverse_iterator(base_type base, base_type it) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		return reverse_iter<tree_type, base_type>{ base, difference_type(std::distance(base, it) + 1) };
	}

	/* Inserts an object pointed to by it into the structure rooted at root in the random access range at base. returns the new root.
	 * There's no uniqueness guarentee; if you wish the tree to contain unique elements, check before inserting an elemenet.
	*/
	template <typename tree_type, std::random_access_iterator base_type, typename less_type>
	[[nodiscard]] constexpr auto insert(base_type base, typename std::iterator_traits<base_type>::difference_type root, base_type it, less_type less) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using node_type = detail::red_black_tree_node<tree_type, base_type>;
		return difference_type(detail::do_insert(node_type{ base, root }, node_type{ base, difference_type(std::distance(base, it) + 1) }, less));
	}

	template <typename tree_type, std::random_access_iterator base_type, typename V, typename less_type>
	[[nodiscard]] constexpr auto upper_bound(base_type base, typename std::iterator_traits<base_type>::difference_type root, const V& value, less_type less) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using node_type = detail::red_black_tree_node<tree_type, base_type>;
		return forward_iter<tree_type, base_type>{ base, difference_type(detail::do_upper_bound(node_type{ base, root }, value, less)) };
	}

	template <typename tree_type, std::random_access_iterator base_type, typename V, typename less_type>
	[[nodiscard]] constexpr auto lower_bound(base_type base, typename std::iterator_traits<base_type>::difference_type root, const V& value, less_type less) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using node_type = detail::red_black_tree_node<tree_type, base_type>;
		return forward_iter<tree_type, base_type>{ base, difference_type(detail::do_lower_bound(node_type{ base, root }, value, less)) };
	}

	template <typename tree_type, std::random_access_iterator base_type, typename V, typename less_type>
	[[nodiscard]] constexpr auto find(base_type base, typename std::iterator_traits<base_type>::difference_type root, const V& value, less_type less) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using node_type = detail::red_black_tree_node<tree_type, base_type>;
		return forward_iter<tree_type, base_type>{ base, difference_type(detail::do_find(node_type{ base, root }, value, less)) };
	}

	// Erases an element it from the structure rooted at root in the random access range at base. returns the new root.
	template <typename tree_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto erase(typename std::iterator_traits<base_type>::difference_type root, forward_iter<tree_type, base_type> it) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using node_type = detail::red_black_tree_node<tree_type, base_type>;
		return difference_type(detail::do_erase<tree_type>(node_type{ it.base, root }, node_type{ it.base, it.root }));
	}

	// Given a node in the tree src, and a node out of the tree dst, relink the tree so dst is in the tree where src was. Src's key must be the correct key for dst's position in the tree.
	template <typename tree_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto node_relink(typename std::iterator_traits<base_type>::difference_type root, base_type dst, forward_iter<tree_type, base_type> src) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using node_type = detail::red_black_tree_node<tree_type, base_type>;
		return difference_type(detail::do_node_relink(node_type{ src.base, root }, node_type{ src.base, difference_type(std::distance(src.base, dst) + 1) }, node_type{ src.base, src.root }));
	}

	// Given two nodes with keys and values already swapped, relink the tree so the indices/colors are swapped.
	template <typename tree_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto node_swap(typename std::iterator_traits<base_type>::difference_type root, forward_iter<tree_type, base_type> a, forward_iter<tree_type, base_type> b) noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		using node_type = detail::red_black_tree_node<tree_type, base_type>;
		return difference_type(detail::do_node_swap(node_type{ a.base, root }, node_type{ a.base, a.root }, node_type{ b.base, b.root }));
	}

	template <std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto empty(typename std::iterator_traits<base_type>::difference_type root) noexcept
	{
		return not root;
	}

	template <std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto max_size() noexcept
	{
		using difference_type = std::iterator_traits<base_type>::difference_type;
		return difference_type(std::numeric_limits<difference_type>::max() - 1);
	}

	template <typename tree_type, std::random_access_iterator base_type>
	[[nodiscard]] constexpr auto size(base_type base, typename std::iterator_traits<base_type>::difference_type root) noexcept
	{
		return std::distance(begin<tree_type>(base, root), end<tree_type>(base));
	}

	template <typename tree_type, std::random_access_iterator base_type, typename equiv_type>
	[[nodiscard]] constexpr auto equal(base_type base, typename std::iterator_traits<base_type>::difference_type root, base_type other_base, typename std::iterator_traits<base_type>::difference_type other_root, equiv_type equiv) noexcept
	{
		using node_type = detail::red_black_tree_node<tree_type, base_type>;
		auto it = begin<tree_type>(base, root);
		auto it2 = begin<tree_type>(other_base, other_root);
		while (it not_eq end<tree_type>(base) and it2 not_eq end<tree_type>(other_base))
			if (not equiv(node_type{ base, it++.root }.key(), node_type{ other_base, it2++.root }.key()))
				return false;
		return it == end<tree_type>(base) and it2 == end<tree_type>(other_base);
	}

	template <typename tree_type, std::random_access_iterator base_type, typename equiv_type>
	[[nodiscard]] constexpr auto not_equal(base_type base, typename std::iterator_traits<base_type>::difference_type root, base_type other_base, typename std::iterator_traits<base_type>::difference_type other_root, equiv_type equiv) noexcept
	{
		return not equal<tree_type>(base, root, other_base, other_root, equiv);
	}

	template <typename tree_type, std::random_access_iterator base_type, typename V, typename less_type>
	[[nodiscard]] constexpr auto equal_range(base_type base, typename std::iterator_traits<base_type>::difference_type root, const V& value, less_type less) noexcept
	{
		return std::pair{ lower_bound<tree_type>(base, root, value, less), upper_bound<tree_type>(base, root, value, less) };
	}

	template <typename tree_type, std::random_access_iterator base_type, typename less_type>
	[[nodiscard]] constexpr auto validate(base_type first, base_type last, typename std::iterator_traits<base_type>::difference_type root, less_type less) noexcept
	{
		using node_type = detail::red_black_tree_node<tree_type, base_type>;
		return detail::do_validate(node_type{ first, root }, std::distance(first, last), less);
	}
}

#endif
#endif
