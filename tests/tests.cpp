#include "red_black_tree.h"
#include "double_list.h"
#include "single_list.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <functional>
#include <ranges>
#include <span>
#include <tuple>

struct tree_type
{
	enum class left_type : std::size_t {};
	enum class right_type : std::size_t {};
	enum class parent_type : std::size_t {};
	enum class color_type : bool {};
	enum class key_type : std::size_t {};
};

struct list_type
{
	enum class next_type : std::size_t {};
	enum class prev_type : std::size_t {};
};


template <typename E, typename T>
struct proxy
{
	T t;
	constexpr decltype(auto) operator=(const E& e) noexcept
	{
		std::get<E>(t.ts) = e;
		return *this;
	}
	[[nodiscard]] explicit constexpr operator E() const noexcept
	{
		return std::get<E>(t.ts);
	}
};

struct example_node {std::tuple<tree_type::left_type, tree_type::right_type, tree_type::parent_type, tree_type::color_type, tree_type::key_type> ts;};

struct example_list {std::tuple<list_type::next_type, list_type::prev_type> ts;};

template <typename T>
constexpr auto get(const example_node& node)
{
	return proxy<T, const example_node&>{node};
};

template <typename T>
constexpr auto get(const example_node&& node)
{
	return proxy<T, const example_node&&>{node};
};

template <typename T>
constexpr auto get(example_node& node)
{
	return proxy<T, example_node&>{node};
};

template <typename T>
constexpr auto get(example_node&& node)
{
	return proxy<T, example_node&&>{node};
};

template <typename T>
constexpr auto get(const example_list& node)
{
	return proxy<T, const example_list&>{node};
};

template <typename T>
constexpr auto get(const example_list&& node)
{
	return proxy<T, const example_list&&>{node};
};

template <typename T>
constexpr auto get(example_list& node)
{
	return proxy<T, example_list&>{node};
};

template <typename T>
constexpr auto get(example_list&& node)
{
	return proxy<T, example_list&&>{node};
};

template <typename key_type>
[[nodiscard]] constexpr auto init_tree_nodes(std::ranges::contiguous_range auto nodes) noexcept
{
	for (auto& node : nodes)
		get<key_type>(node) = key_type(std::distance(nodes.data(), &node));

	return std::ptrdiff_t{};
}

constexpr auto assert_valid_tree(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto root) noexcept
{
	assert(red_black_tree::validate<tree_type>(nodes.begin(), nodes.end(), root, std::less<>{}));
	assert(red_black_tree::validate<tree_type>(nodes, root, std::less<>{}));
}

template <typename iterator>
constexpr auto assert_empty_tree(std::same_as<std::ptrdiff_t> auto root) noexcept
{
	assert(red_black_tree::empty<iterator>(root));
}

template <typename iterator>
constexpr auto assert_not_empty_tree(std::same_as<std::ptrdiff_t> auto root) noexcept
{
	assert(not red_black_tree::empty<iterator>(root));
}

[[nodiscard]] constexpr auto assert_add_all_tree_nodes(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto root) noexcept
{
	for (const auto& node : std::views::iota(nodes.begin(), nodes.end()))
		assert_valid_tree(nodes, root = red_black_tree::insert<tree_type>(nodes.begin(), root, node, std::less<>{}));

	return root;
}

constexpr auto assert_tree_size(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto root, std::same_as<std::ptrdiff_t> auto size) noexcept
{
	assert(red_black_tree::size<tree_type>(nodes.begin(), root) == size);
}

[[nodiscard]] constexpr auto assert_remove_odd_tree_nodes(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto root) noexcept
{
	auto is_odd = [&](auto&& node) { return bool(std::distance(nodes.begin(), node) % 2); };

	for (const auto& it : std::views::iota(nodes.begin(), nodes.end()) | std::views::filter(is_odd))
		assert_valid_tree(nodes, root = red_black_tree::erase<tree_type>(root, red_black_tree::make_iterator<tree_type>(nodes.begin(), it)));
	
	return root;
}

template <typename key_type>
constexpr auto assert_ordered_even_tree_nodes(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto root) noexcept
{
	std::for_each(red_black_tree::begin<tree_type>(nodes.begin(), root), red_black_tree::end<tree_type>(nodes.begin()), [i = std::size_t{}](const auto& node) mutable
	{
		assert(key_type(i) == key_type(get<key_type>(node)));
		i += std::size_t{2};
	});

	std::for_each(red_black_tree::rbegin<tree_type>(nodes.begin(), root), red_black_tree::rend<tree_type>(nodes.begin()), [i = std::size_t{100}](const auto& node) mutable
	{
		i -= std::size_t{2};
		assert(key_type(i) == key_type(get<key_type>(node)));
	});
}

template <typename key_type>
constexpr auto assert_only_even_in_tree(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto root) noexcept
{
	for (auto end = red_black_tree::end<tree_type>(nodes.begin()); auto i : std::ranges::iota_view(std::ptrdiff_t{}, std::ptrdiff_t{nodes.size()})) {
		auto it = red_black_tree::find<tree_type>(nodes.begin(), root, key_type(i), std::less<>{});

		assert(bool(i % 2) == (end == it));

		if (it not_eq end)
			assert(std::ptrdiff_t(key_type(get<key_type>(*it))) == i);
	}
}

template <typename key_type>
[[nodiscard]] constexpr auto relink_tree_to_start(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto root) noexcept
{
	auto is_even = [&](auto&& node) {return not (std::distance(nodes.begin(), node) % 2); };

	for (auto dst = nodes.begin(); const auto& src : std::views::iota(nodes.begin(), nodes.end()) | std::views::filter(is_even) | std::views::drop(1)) {
		root = red_black_tree::node_relink<tree_type>(root, ++dst, red_black_tree::make_iterator<tree_type>(nodes.begin(), src));
		get<key_type>(*dst) = key_type(get<key_type>(*src));
		assert_valid_tree(nodes, root);
	}
	return root;
}

constexpr auto assert_tree_nodes_increase_addr(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto root) noexcept
{
	std::for_each(red_black_tree::begin<tree_type>(nodes.begin(), root), red_black_tree::end<tree_type>(nodes.begin()), [&, i = std::size_t{}](const auto& node) mutable
	{
		assert(&nodes[i++] == &node);
	});
}

constexpr auto assert_tree_nodes_decrease_addr(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto root) noexcept
{
	std::for_each(red_black_tree::begin<tree_type>(nodes.begin(), root), red_black_tree::end<tree_type>(nodes.begin()), [&, i = std::size_t{50}](const auto& node) mutable
	{
		assert(&nodes[--i] == &node);
	});
}

template <typename key_type>
[[nodiscard]] constexpr auto make_tree_nodes_increase_addr(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto root) noexcept
{
	auto dst = nodes.begin();
	for (auto src = red_black_tree::begin<tree_type>(nodes.begin(), root);
		src != red_black_tree::end<tree_type>(nodes.begin());
		src = ++red_black_tree::make_iterator<tree_type>(nodes.begin(), dst++)) {
		root = red_black_tree::node_swap<tree_type>(root, red_black_tree::make_iterator<tree_type>(nodes.begin(), dst), src);
		auto tmp = key_type(get<key_type>(*dst));
		get<key_type>(*dst) = key_type(get<key_type>(*src));
		get<key_type>(*src) = tmp;
		assert_valid_tree(nodes, root);
	}
	return root;
}

template <typename key_type>
[[nodiscard]] constexpr auto make_tree_nodes_decrease_addr(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto root) noexcept
{
	auto dst = nodes.end();
	for (auto src = red_black_tree::begin<tree_type>(nodes.begin(), root);
		src != red_black_tree::end<tree_type>(nodes.begin());
		src = ++red_black_tree::make_iterator<tree_type>(nodes.begin(), dst)) {
		root = red_black_tree::node_swap<tree_type>(root, red_black_tree::make_iterator<tree_type>(nodes.begin(), --dst), src);
		auto tmp = key_type(get<key_type>(*dst));
		get<key_type>(*dst) = key_type(get<key_type>(*src));
		get<key_type>(*src) = tmp;
		assert_valid_tree(nodes, root);
	}
	return root;
}

[[nodiscard]] constexpr auto test_tree() noexcept
{
	std::array<example_node, 100> nodes{};
	using tree_key = typename tree_type::key_type;
	using tree_iter = typename decltype(std::span(nodes))::iterator;

	auto root = init_tree_nodes<tree_key>(std::span(nodes));

	assert_valid_tree(std::span(nodes), root);
	assert_empty_tree<tree_iter>(root);

	root = assert_add_all_tree_nodes(std::span(nodes), root);

	assert_tree_size(std::span(nodes), root, std::ptrdiff_t{100});

	root = assert_remove_odd_tree_nodes(std::span(nodes), root);
	assert_tree_size(std::span(nodes), root, std::ptrdiff_t{50});

	assert_not_empty_tree<tree_iter>(root);

	assert_ordered_even_tree_nodes<tree_key>(std::span(nodes), root);

	assert_only_even_in_tree<tree_key>(std::span(nodes), root);

	assert(red_black_tree::max_size<tree_iter>() == std::numeric_limits<std::ptrdiff_t>::max() - std::ptrdiff_t{1});

	assert(red_black_tree::equal<tree_type>(std::span(nodes).begin(), root, std::span(nodes).begin(), root, std::equal_to<>{}));
	assert(not red_black_tree::not_equal<tree_type>(std::span(nodes).begin(), root, std::span(nodes).begin(), root, std::equal_to<>{}));
	assert(red_black_tree::equal_range<tree_type>(std::span(nodes).begin(), root, tree_type::key_type{}, std::less<>{}).first == red_black_tree::begin<tree_type>(std::span(nodes).begin(), root));
	assert(red_black_tree::equal_range<tree_type>(std::span(nodes).begin(), root, tree_type::key_type{}, std::less<>{}).second == ++red_black_tree::begin<tree_type>(std::span(nodes).begin(), root));

	root = relink_tree_to_start<tree_key>(std::span(nodes), root);

	assert_tree_size(std::span(nodes), root, std::ptrdiff_t{50});

	assert_not_empty_tree<tree_iter>(root);

	assert_valid_tree(std::span(nodes).first(50), root);

	assert_tree_nodes_increase_addr(std::span(nodes).first(50), root);

	root = make_tree_nodes_increase_addr<tree_key>(std::span(nodes).first(50), root);

	assert_tree_size(std::span(nodes), root, std::ptrdiff_t{50});

	assert_not_empty_tree<tree_iter>(root);

	assert_valid_tree(std::span(nodes).first(50), root);

	assert_tree_nodes_increase_addr(std::span(nodes).first(50), root);

	root = make_tree_nodes_decrease_addr<tree_key>(std::span(nodes).first(50), root);

	assert_tree_size(std::span(nodes), root, std::ptrdiff_t{50});

	assert_not_empty_tree<tree_iter>(root);

	assert_tree_nodes_decrease_addr(std::span(nodes).first(50), root);

	while (not (red_black_tree::empty<decltype(std::span(nodes).begin())>(root))) {
		root = red_black_tree::erase<tree_type>(root, red_black_tree::begin<tree_type>(std::span(nodes).begin(), root));
		assert_valid_tree(std::span(nodes), root);
	}

	assert_empty_tree<tree_iter>(root);

	assert_valid_tree(std::span(nodes), root);
	std::get<tree_type::color_type>(nodes[0].ts) = tree_type::color_type{};
	std::get<tree_type::parent_type>(nodes[0].ts) = tree_type::parent_type{};
	std::get<tree_type::left_type>(nodes[0].ts) = tree_type::left_type{};
	std::get<tree_type::right_type>(nodes[0].ts) = tree_type::right_type{};
	assert(not red_black_tree::validate<tree_type>(std::span(nodes).begin(), std::span(nodes).end(), std::ptrdiff_t{1}, std::less<>{}));
	std::get<tree_type::color_type>(nodes[0].ts) = tree_type::color_type{true};
	assert_valid_tree(std::span(nodes), std::ptrdiff_t{1});
	std::get<tree_type::parent_type>(nodes[0].ts) = tree_type::parent_type{1};
	assert(not red_black_tree::validate<tree_type>(std::span(nodes).begin(), std::span(nodes).end(), std::ptrdiff_t{1}, std::less<>{}));
	assert_valid_tree(std::span(nodes).first(0), root);

	return true;
}

constexpr auto assert_valid_list(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto head) noexcept
{
	assert(double_list::validate<list_type>(nodes.begin(), nodes.end(), head));
	assert(double_list::validate<list_type>(nodes, head));
}

[[nodiscard]] constexpr auto assert_add_all_nodes_back(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto head, std::same_as<std::ptrdiff_t> auto tail) noexcept
{
	head = tail = double_list::push_back<list_type>(nodes.begin(), tail, nodes.begin());
	assert_valid_list(nodes, head);
	for (const auto& node : std::views::iota(nodes.begin(), nodes.end()) | std::views::drop(1)) {
		tail = double_list::push_back<list_type>(nodes.begin(), tail, node);
		assert_valid_list(nodes, head);
	}
	return std::pair{head, tail};
}

[[nodiscard]] constexpr auto assert_add_all_nodes_front(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto head, std::same_as<std::ptrdiff_t> auto tail) noexcept
{
	tail = head = double_list::push_front<list_type>(nodes.begin(), head, nodes.begin());
	assert_valid_list(nodes, head);
	for (const auto& node : std::views::iota(nodes.begin(), nodes.end()) | std::views::drop(1)) {
		head = double_list::push_front<list_type>(nodes.begin(), head, node);
		assert_valid_list(nodes, head);
	}
	return std::pair{head, tail};
}

[[nodiscard]] constexpr auto assert_remove_odd_nodes_after(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto head, std::same_as<std::ptrdiff_t> auto tail) noexcept
{
	auto is_even = [&](auto&& node) { return not (std::distance(nodes.begin(), node) % 2); };

	for (const auto& it : std::views::iota(nodes.begin(), nodes.end()) | std::views::filter(is_even)) {
		tail = double_list::erase_after<list_type>(tail, double_list::make_iterator<list_type>(nodes.begin(), it));
		assert(double_list::validate<list_type>(nodes.begin(), nodes.end(), head));
	}

	return tail;
}

[[nodiscard]] constexpr auto assert_remove_odd_nodes_before(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto head) noexcept
{
	auto is_even = [&](auto&& node) { return not (std::distance(nodes.begin(), node) % 2); };

	for (const auto& it : std::views::iota(nodes.begin(), nodes.end()) | std::views::filter(is_even)) {
		head = double_list::erase_before<list_type>(head, double_list::make_iterator<list_type>(nodes.begin(), it));
		assert_valid_list(nodes, head);
	}
	return head;
}

constexpr auto assert_nodes_increase_addr(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto head) noexcept
{
	std::for_each(double_list::begin<list_type>(nodes.begin(), head), double_list::end<list_type>(nodes.begin()), [&, i = std::size_t{}](const auto& node) mutable
	{
		assert(&nodes[i++] == &node);
	});
}

constexpr auto assert_nodes_decrease_addr(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto head) noexcept
{
	std::for_each(double_list::begin<list_type>(nodes.begin(), head), double_list::end<list_type>(nodes.begin()), [&, i = std::size_t{50}](const auto& node) mutable
	{
		assert(&nodes[--i] == &node);
	});
}

[[nodiscard]] constexpr auto make_nodes_increase_addr(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto head, std::same_as<std::ptrdiff_t> auto tail) noexcept
{
	auto dst = nodes.begin();
	assert_valid_list(nodes, head);
	for (auto src = double_list::begin<list_type>(nodes.begin(), head);
		src != double_list::end<list_type>(nodes.begin());
		src = ++double_list::make_iterator<list_type>(nodes.begin(), dst++)) {
		std::tie(head, tail) = double_list::node_swap<list_type>(head, tail, double_list::make_iterator<list_type>(nodes.begin(), dst), src);
		assert_valid_list(nodes, head);
	}
	return std::pair{head, tail};
}

[[nodiscard]] constexpr auto make_nodes_decrease_addr(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto head, std::same_as<std::ptrdiff_t> auto tail) noexcept
{
	auto dst = nodes.end();
	assert_valid_list(nodes, head);
	for (auto src = double_list::begin<list_type>(nodes.begin(), head);
		src != double_list::end<list_type>(nodes.begin());
		src = ++double_list::make_iterator<list_type>(nodes.begin(), dst)) {
		std::tie(head, tail) = double_list::node_swap<list_type>(head, tail, double_list::make_iterator<list_type>(nodes.begin(), --dst), src);
		assert_valid_list(nodes, head);
	}
	return std::pair{head, tail};
}

constexpr auto assert_ordered_even_nodes(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto head, std::same_as<std::ptrdiff_t> auto tail) noexcept
{
	std::for_each(double_list::begin<list_type>(nodes.begin(), head), double_list::end<list_type>(nodes.begin()), [&, i = std::size_t{}](const auto& node) mutable
	{
		assert(&nodes[i] == &node);
		i += std::size_t{2};
	});

	std::for_each(double_list::rbegin<list_type>(nodes.begin(), tail), double_list::rend<list_type>(nodes.begin()), [&, i = nodes.size()](const auto& node) mutable
	{
		i -= std::size_t{2};
		assert(&nodes[i] == &node);
	});
}

constexpr auto assert_reverse_ordered_even_nodes(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto head, std::same_as<std::ptrdiff_t> auto tail) noexcept
{
	std::for_each(double_list::rbegin<list_type>(nodes.begin(), tail), double_list::rend<list_type>(nodes.begin()), [&, i = std::size_t{}](const auto& node) mutable
	{
		assert(&nodes[i] == &node);
		i += std::size_t{2};
	});

	std::for_each(double_list::begin<list_type>(nodes.begin(), head), double_list::end<list_type>(nodes.begin()), [&, i = nodes.size()](const auto& node) mutable
	{
		i -= std::size_t{2};
		assert(&nodes[i] == &node);
	});
}

[[nodiscard]] constexpr auto make_list_empty_pop_front(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto head, std::same_as<std::ptrdiff_t> auto tail) noexcept
{
	while (head) {
		head = double_list::pop_front<list_type>(nodes.begin(), head);
		assert_valid_list(nodes, head);
	}
	return std::pair{head, tail = head};
}

[[nodiscard]] constexpr auto make_list_empty_pop_back(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto head, std::same_as<std::ptrdiff_t> auto tail) noexcept
{
	while (tail) {
		tail = double_list::pop_back<list_type>(nodes.begin(), tail);
		assert_valid_list(nodes, head);
	}
	return std::pair{head = tail, tail};
}


template <typename iterator>
constexpr auto assert_empty_list(std::same_as<std::ptrdiff_t> auto head) noexcept
{
	assert(double_list::empty<iterator>(head));
}

template <typename iterator>
constexpr auto assert_not_empty_list(std::same_as<std::ptrdiff_t> auto head) noexcept
{
	assert(not double_list::empty<iterator>(head));
}

constexpr auto assert_list_size(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto head, std::same_as<std::ptrdiff_t> auto size) noexcept
{
	assert(double_list::size<list_type>(nodes.begin(), head) == size);
}

[[nodiscard]] constexpr auto relink_list_to_start(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto head, std::same_as<std::ptrdiff_t> auto tail) noexcept
{
	auto is_even = [&](auto&& node) {return not (std::distance(nodes.begin(), node) % 2); };

	for (auto dst = nodes.begin(); const auto& src : std::views::iota(nodes.begin(), nodes.end()) | std::views::filter(is_even) | std::views::drop(1)) {
		std::tie(head, tail) = double_list::node_relink<list_type>(head, tail, ++dst, double_list::make_iterator<list_type>(nodes.begin(), src));
		assert_valid_list(nodes, head);
	}
	return std::pair{head, tail};
}

[[nodiscard]] constexpr auto test_list() noexcept
{
	std::array<example_list, 100> nodes{};
	auto head = std::ptrdiff_t{};
	auto tail = std::ptrdiff_t{};

	using list_iter = typename decltype(std::span(nodes))::iterator;

	assert_valid_list(std::span(nodes), head);
	assert_empty_list<list_iter>(head);

	std::tie(head, tail) = assert_add_all_nodes_back(std::span(nodes), head, tail);

	assert_list_size(std::span(nodes), head, std::ptrdiff_t{100});
	tail = assert_remove_odd_nodes_after(std::span(nodes), head, tail);
	assert_list_size(std::span(nodes), head, std::ptrdiff_t{50});

	assert_not_empty_list<list_iter>(head);

	assert_ordered_even_nodes(std::span(nodes), head, tail);

	std::tie(head, tail) = make_list_empty_pop_front(std::span(nodes), head, tail);

	assert_empty_list<list_iter>(head);

	std::tie(head, tail) = assert_add_all_nodes_front(std::span(nodes), head, tail);

	assert_list_size(std::span(nodes), head, std::ptrdiff_t{100});
	head = assert_remove_odd_nodes_before(std::span(nodes), head);
	assert_list_size(std::span(nodes), head, std::ptrdiff_t{50});

	assert_not_empty_list<list_iter>(head);

	assert_reverse_ordered_even_nodes(std::span(nodes), head, tail);

	std::tie(head, tail) = relink_list_to_start(std::span(nodes), head, tail);
		
	assert_valid_list(std::span(nodes).first(50), head);

	assert_list_size(std::span(nodes), head, std::ptrdiff_t{50});

	assert_not_empty_list<list_iter>(head);

	std::for_each(double_list::begin<list_type>(std::span(nodes).begin(), head), double_list::end<list_type>(std::span(nodes).begin()), [&, i = std::size_t{50}](const auto& node) mutable
	{
		assert(&nodes[--i] == &node);
	});

	std::tie(head, tail) = make_nodes_increase_addr(std::span(nodes).first(50), head, tail);

	assert_list_size(std::span(nodes), head, std::ptrdiff_t{50});

	assert_not_empty_list<list_iter>(head);

	assert_nodes_increase_addr(std::span(nodes), head);

	std::tie(head, tail) = make_nodes_decrease_addr(std::span(nodes).first(50), head, tail);

	assert_list_size(std::span(nodes), head, std::ptrdiff_t{50});

	assert_not_empty_list<list_iter>(head);

	assert_nodes_decrease_addr(std::span(nodes), head);

	assert(double_list::max_size<decltype(std::span(nodes).begin())>() == std::numeric_limits<std::ptrdiff_t>::max() - std::ptrdiff_t{1});

	std::tie(head, tail) = make_list_empty_pop_back(std::span(nodes), head, tail);

	assert_valid_list(std::span(nodes), head);
	assert_empty_list<list_iter>(head);

	return true;
}

constexpr auto assert_valid_slist(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto head) noexcept
{
	assert(single_list::validate<list_type>(nodes.begin(), nodes.end(), head));
	assert(single_list::validate<list_type>(nodes, head));
}

template <typename iterator>
constexpr auto assert_empty_slist(std::same_as<std::ptrdiff_t> auto head) noexcept
{
	assert(single_list::empty<iterator>(head));
}

[[nodiscard]] constexpr auto assert_slist_add_all_nodes(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto head) noexcept
{
	for (const auto& node : std::views::iota(nodes.begin(), nodes.end())) {
		head = single_list::push_front<list_type>(nodes.begin(), head, node);
		assert_valid_slist(nodes, head);
	}
	return head;
}

[[nodiscard]] constexpr auto assert_slist_add_last_nodes_reverse(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto head) noexcept
{	
	for (const auto& node : std::views::iota(std::next(nodes.begin(), 50), nodes.end()) | std::views::reverse) {
		head = single_list::push_front<list_type>(nodes.begin(), head, node);
		assert_valid_slist(nodes, head);
	}
	return head;
}

[[nodiscard]] constexpr auto assert_slist_remove_all_nodes(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto head) noexcept
{
	while(head)
		assert_valid_slist(nodes, head = single_list::pop_front<list_type>(nodes.begin(), head));
	return head;
}

[[nodiscard]] constexpr auto assert_slist_remove_even_nodes(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto head) noexcept
{
	auto is_odd = [&](auto&& node) { return bool(std::distance(nodes.begin(), node) % 2); };

	for (const auto& node : std::views::iota(nodes.begin(), nodes.end()) | std::views::filter(is_odd)) {
		auto it = single_list::make_iterator<list_type>(nodes.begin(), node);
		assert(std::next(it) not_eq single_list::end<list_type>(nodes.begin()));
		single_list::erase_after<list_type>(it);
		assert_valid_slist(nodes, head);
	}
	return head;
}

[[nodiscard]] constexpr auto assert_slist_add_odd_nodes(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto head) noexcept
{
	auto is_odd = [&](auto&& node) { return bool(std::distance(nodes.begin(), node) % 2); };

	for (const auto& node : std::views::iota(nodes.begin(), nodes.end()) | std::views::filter(is_odd)) {
		single_list::insert_after<list_type>(single_list::make_iterator<list_type>(nodes.begin(), node), std::prev(node));
		assert_valid_slist(nodes, head);
	}
	return head;
}

[[nodiscard]] constexpr auto assert_slist_relink_to_front(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto head) noexcept
{
	if (not head) 
		return head;
	auto it = single_list::end<list_type>(nodes.begin());
	head = single_list::node_relink<list_type>(head, nodes.begin(), it);
	assert_valid_slist(nodes, head);
	for (auto idx = nodes.begin(); single_list::end<list_type>(nodes.begin()) != std::next(it = single_list::make_iterator<list_type>(nodes.begin(), idx));) {
		head = single_list::node_relink<list_type>(head, ++idx, it);
		assert_valid_slist(nodes, head);
	}
	return head;
}

[[nodiscard]] constexpr auto assert_slist_test_swap(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto head) noexcept
{
	auto it = nodes.begin();
	for (const auto& node : std::views::iota(nodes.begin(), nodes.end()))
	{
		auto it1 = single_list::make_iterator<list_type>(nodes.begin(), node);
		auto it2 = single_list::make_iterator<list_type>(nodes.begin(), it);

		if (std::next(it1) == single_list::end<list_type>(nodes.begin()))
			it1 = single_list::end<list_type>(nodes.begin());
		if (std::next(it2) == single_list::end<list_type>(nodes.begin()))
			it2 = single_list::end<list_type>(nodes.begin());
		head = single_list::node_swap<list_type>(head, it1, it2);
		assert_valid_slist(nodes, head);
	}
	return head;
}



constexpr auto assert_slist_size(std::ranges::contiguous_range auto nodes, std::same_as<std::ptrdiff_t> auto head, std::same_as<std::ptrdiff_t> auto size) noexcept
{
	assert(single_list::size<list_type>(nodes.begin(), head) == size);
}


[[nodiscard]] constexpr auto test_slist() noexcept
{
	std::array<example_list, 100> nodes{};
	auto head = std::ptrdiff_t{};

	using list_iter = typename decltype(std::span(nodes))::iterator;

	assert_valid_slist(std::span(nodes), head);
	assert_empty_slist<list_iter>(head);

	head = assert_slist_add_all_nodes(std::span(nodes), head);

	assert_slist_size(std::span(nodes), head, std::ptrdiff_t{100});

	head = assert_slist_remove_even_nodes(std::span(nodes), head);

	assert_slist_size(std::span(nodes), head, std::ptrdiff_t{50});

	head = assert_slist_add_odd_nodes(std::span(nodes), head);

	assert_slist_size(std::span(nodes), head, std::ptrdiff_t{100});

	head = assert_slist_remove_all_nodes(std::span(nodes), head);

	assert_empty_slist<list_iter>(head);

	head = assert_slist_add_last_nodes_reverse(std::span(nodes), head);

	assert_slist_size(std::span(nodes), head, std::ptrdiff_t{50});

	head = assert_slist_relink_to_front(std::span(nodes), head);

	head = assert_slist_test_swap(std::span(nodes), head);

	head = assert_slist_test_swap(std::span(nodes), head);

	assert_slist_size(std::span(nodes), head, std::ptrdiff_t{50});

	return true;
}

struct node
{
	enum class left_type : std::uint8_t {} left;
	enum class right_type : std::uint8_t {} right;
	enum class parent_type : std::uint8_t {} parent;
	enum class color_type : std::uint8_t {} color;
	using key_type = std::u8string_view;
	std::array<char8_t, 32> name;

	template <std::same_as<key_type> T>
	[[nodiscard]] constexpr auto get() const noexcept
	{
		return key_type(std::begin(name), std::find(std::begin(name), std::end(name), u8'\0'));
	}
	template <std::same_as<left_type> T>
	[[nodiscard]] constexpr auto get() noexcept -> T&
	{
		return left;
	}
	template <std::same_as<left_type> T>
	[[nodiscard]] constexpr auto get() const noexcept -> const T&
	{
		return left;
	}
	template <std::same_as<right_type> T>
	[[nodiscard]] constexpr auto get() noexcept -> T&
	{
		return right;
	}
	template <std::same_as<right_type> T>
	[[nodiscard]] constexpr auto get() const noexcept -> const T&
	{
		return right;
	}
	template <std::same_as<parent_type> T>
	[[nodiscard]] constexpr auto get() noexcept -> T&
	{
		return parent;
	}
	template <std::same_as<parent_type> T>
	[[nodiscard]] constexpr auto get() const noexcept -> const T&
	{
		return parent;
	}
	template <std::same_as<color_type> T>
	[[nodiscard]] constexpr auto get() noexcept -> T&
	{
		return color;
	}
	template <std::same_as<color_type> T>
	[[nodiscard]] constexpr auto get() const noexcept -> const T&
	{
		return color;
	}
};

template <typename node_type, std::size_t max_size_>
class multiset
{
	static_assert(max_size_ < 255);
	enum class e1 : std::uint8_t {} root;
	enum class e2 : std::uint8_t {} size_;
	std::array<node_type, max_size_> data_;
public:
	using iterator = red_black_tree::forward_iter<node_type, typename decltype(std::span(std::declval<std::array<node_type, max_size_>&>()))::iterator>;
	using const_iterator = red_black_tree::forward_iter<node_type, typename decltype(std::span(std::declval<const std::array<node_type, max_size_>&>()))::iterator>;
	using reverse_iterator = red_black_tree::reverse_iter<node_type, typename decltype(std::span(std::declval<std::array<node_type, max_size_>&>()))::iterator>;
	using reverse_const_iterator = red_black_tree::reverse_iter<node_type, typename decltype(std::span(std::declval<const std::array<node_type, max_size_>&>()))::iterator>;
	using value_type = typename std::iterator_traits<iterator>::value_type;
	using reference = typename std::iterator_traits<iterator>::reference;
	using difference_type = typename std::iterator_traits<iterator>::difference_type;

	using const_reference = typename std::iterator_traits<const_iterator>::reference;
	using size_type = std::make_unsigned_t<difference_type>;

	[[nodiscard]] constexpr auto end() noexcept
	{
		return red_black_tree::end<node_type>(std::span(data_).begin());
	}
	[[nodiscard]] constexpr auto end() const noexcept
	{
		return cend();
	}
	[[nodiscard]] constexpr auto cend() const noexcept
	{
		return red_black_tree::end<node_type>(std::span(data_).begin());
	}

	[[nodiscard]] constexpr auto begin() noexcept
	{
		return red_black_tree::begin<node_type>(std::span(data_).begin(), difference_type(root));
	}
	[[nodiscard]] constexpr auto begin() const noexcept
	{
		return cbegin();
	}
	[[nodiscard]] constexpr auto cbegin() const noexcept
	{
		return red_black_tree::begin<node_type>(std::span(data_).begin(), difference_type(root));
	}

	[[nodiscard]] constexpr auto rend() noexcept
	{
		return red_black_tree::rend<node_type>(std::span(data_).begin());
	}
	[[nodiscard]] constexpr auto rend() const noexcept
	{
		return crend();
	}
	[[nodiscard]] constexpr auto crend() const noexcept
	{
		return red_black_tree::rend<node_type>(std::span(data_).begin());
	}

	[[nodiscard]] constexpr auto rbegin() noexcept
	{
		return red_black_tree::rbegin<node_type>(std::span(data_).begin(), difference_type(root));
	}
	[[nodiscard]] constexpr auto rbegin() const noexcept
	{
		return crbegin();
	}
	[[nodiscard]] constexpr auto crbegin() const noexcept
	{
		return red_black_tree::rbegin<node_type>(std::span(data_).begin(), difference_type(root));
	}

	constexpr auto insert(std::u8string_view name) noexcept
	{
		auto base = std::span(data_).begin();
		auto it = std::next(base, difference_type(size()));
		std::fill(std::copy_n(name.begin(), name.size(), it->name.begin()), it->name.end(), u8'\0');
		size_ = static_cast<decltype(size_)>(difference_type(size()) + 1);
		root = static_cast<decltype(root)>(red_black_tree::insert<node_type>(base, difference_type(root), it, std::less<>{}));
		return red_black_tree::make_iterator<node_type>(base, it);
	}

	constexpr auto erase(iterator pos) noexcept
	{
		auto base = std::span(data_).begin();
		size_ = static_cast<decltype(size_)>(difference_type(size()) - 1);
		auto it = red_black_tree::make_iterator<node_type>(base, std::next(base, difference_type(size())));
		using std::swap;
		swap(pos->name, it->name);
		root = static_cast<decltype(root)>(red_black_tree::node_swap<node_type>(difference_type(root), pos, it));
		root = static_cast<decltype(root)>(red_black_tree::erase<node_type>(difference_type(root), it++));
		return it;
	}

	[[nodiscard]] constexpr auto find(std::u8string_view name) const noexcept
	{
		return red_black_tree::find<node_type>(std::span(data_).begin(), difference_type(root), name, std::less<>{});
	}

	[[nodiscard]] constexpr auto find(std::u8string_view name) noexcept
	{
		return red_black_tree::find<node_type>(std::span(data_).begin(), difference_type(root), name, std::less<>{});
	}

	[[nodiscard]] constexpr auto lower_bound(std::u8string_view name) const noexcept
	{
		return red_black_tree::lower_bound<node_type>(std::span(data_).begin(), difference_type(root), name, std::less<>{});
	}

	[[nodiscard]] constexpr auto lower_bound(std::u8string_view name) noexcept
	{
		return red_black_tree::lower_bound<node_type>(std::span(data_).begin(), difference_type(root), name, std::less<>{});
	}

	[[nodiscard]] constexpr auto upper_bound(std::u8string_view name) const noexcept
	{
		return red_black_tree::upper_bound<node_type>(std::span(data_).begin(), difference_type(root), name, std::less<>{});
	}

	[[nodiscard]] constexpr auto upper_bound(std::u8string_view name) noexcept
	{
		return red_black_tree::upper_bound<node_type>(std::span(data_).begin(), difference_type(root), name, std::less<>{});
	}

	[[nodiscard]] constexpr auto equal_range(std::u8string_view name) const noexcept
	{
		return red_black_tree::equal_range<node_type>(std::span(data_).begin(), difference_type(root), name, std::less<>{});
	}

	[[nodiscard]] constexpr auto equal_range(std::u8string_view name) noexcept
	{
		return red_black_tree::equal_range<node_type>(std::span(data_).begin(), difference_type(root), name, std::less<>{});
	}

	[[nodiscard]] constexpr auto operator==(const multiset& other) const noexcept
	{
		if (other.size() not_eq size())
			return false;
		auto base = std::span(data_).begin();
		auto other_base = std::span(other.data_).begin();
		return red_black_tree::equal<node_type>(base, difference_type(root), other_base, difference_type(other.root), std::equal_to<>{});
	}
	[[nodiscard]] constexpr auto operator!=(const multiset& other) const noexcept
	{
		if (other.size() not_eq size())
			return true;
		auto base = std::span(data_).begin();
		auto other_base = std::span(other.data_).begin();
		return red_black_tree::not_equal<node_type>(base, difference_type(root), other_base, difference_type(other.root), std::equal_to<>{});
	}
	[[nodiscard]] constexpr auto size() const noexcept
	{
		return size_type(size_);
	}
	[[nodiscard]] static constexpr auto max_size() noexcept
	{
		return size_type(max_size_);
	}
	[[nodiscard]] constexpr auto empty() const noexcept
	{
		return not size();
	}

	auto swap(multiset& other) noexcept
	{
		using std::swap;
		swap(root, other.root);
		swap(size_, other.size_);
		swap(data_, other.data_);
	}

	[[nodiscard]] static auto start_lifetime(void* p) noexcept -> multiset*
	{
		auto t = std::launder(reinterpret_cast<multiset*>(std::memmove(p, p, sizeof(multiset))));
		if (t->size() > t->max_size())
			return nullptr;
		if (std::ranges::any_of(std::span(t->data_).first(t->size()), [](const auto& v) { return v. template get<std::u8string_view>().size() >= v.name.size(); }))
			return nullptr;
		if (not red_black_tree::validate<node>(std::span(t->data_).first(t->size()), difference_type(t->root), std::less<>{}))
			return nullptr;
		return t->size() == size_type(red_black_tree::size<value_type>(std::span(t->data_).begin(), difference_type(t->root))) ? t : nullptr;
	}
};




template <typename node_type, std::size_t max_size_>
auto swap(multiset<node_type, max_size_>& a, multiset<node_type, max_size_>& b)
{
	a.swap(b);
}

constexpr auto fun() noexcept
{
	multiset<node, 254> t{};

	using namespace std::string_view_literals;

	std::array<std::u8string_view, 6> values{ u8"hello"sv, u8"world,"sv, u8"I'm"sv, u8"a"sv, u8"C++"sv, u8"program." };
	
	static_assert(std::is_standard_layout_v<decltype(t)>);
	static_assert(std::is_trivial_v<decltype(t)>);

	for (const auto& v : values)
		t.insert(v);
	std::ranges::sort(values);
	for (auto v = std::span(values).begin(); const auto & s : t)
		assert(*v++ == s.template get<std::u8string_view>());
	assert(t.size() == 6);
	assert(t.find(u8"hello") not_eq t.end());
	assert(t.lower_bound(u8"C++"sv) == t.begin());
	assert(t.upper_bound(u8"C++"sv) == std::next(t.begin()));
	assert(t.equal_range(u8"C++"sv).first == t.begin());
	assert(t.equal_range(u8"C++"sv).second == std::next(t.begin()));

	auto t2 = t;
	assert(t == t2);
	for (auto it = t.begin(); it not_eq t.cend(); it = t.erase(it));
	assert(t not_eq t2);
	return t;
}

int main()
{
	static_assert(test_tree());

	static_assert(test_list());

	static_assert(test_slist());

	[[maybe_unused]] static constexpr auto t = fun();

	alignas(decltype(t)) std::byte s[sizeof(t)]{};

	auto p = multiset<node, 254>::start_lifetime(s);
	assert(p);
	*reinterpret_cast<std::byte*>(p) = std::byte{255};

	auto q = multiset<node, 254>::start_lifetime(p);
	assert(not q);
}
