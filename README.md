# Imperative algorithms for serializable data structures.

The C++ standard library includes many algorithms to manipulate data as well as many containers that own data and structure it in order to give it specific properties. Functions like std::make_heap and std::sort reorder elements in an existing range according to a supplied relation by moving objects to a new location while node-based container objects like std::forward_list and std::set require objects not move and change their logical position in the container by manipulating metadata which relies on object location in-memory. Elements belonging to more than one relation with either model thus require some sort of indirection and objects must be placed elsewhere to logically achieve being elements of more than one relation which can impact performance. Node-based containers can have poor cache performance and are usually allocation heavy with performant solutions making use of specialized allocation strategies like arenas and slab allocators.

One approach to solving some of these issues, where they are issues, is using intrusive structures. This approach has users explicitly create the bookkeeping fields in their data elements instead of having the container do it for them. Lifetime and ownership of objects are not necessarily tied to the logical container and an object can be part of multiple containers easily if it contains different bookkeeping fields. boost has a wonderful implementation of many structures as intrusive containers and many of these problems are solved for objects whose allocation outlives its use in the logical container. This can completely remove the problem of indirection when elements are members of more than one container. While solving this problem is great, it doesn't necessarily offer all the benefits necessary for use in real time systems. Often it's necessary to relocate data by serializing them to disk or transmitting them across a network.

 This library intends to solve serializing data structures by implementing algorithms for a few common structures like balanced binary search trees and linked lists using a straightforward index-based approach. Instead of representing organized data using an object, these methods operate on a logical range similar to std::make_heap or std::sort but unlike these algorithms they can manipulate objects without moving them by assigning each element a persistent index within the range shifting the indirection from pointer indirection to pointer arithmetic. As a side effect of the relationship between objects being represented using indices, data organized this way can be trivially serialized to persistent storage or across the network. This makes such a representation ideal for sharing small to medium sized sets between machines or across the compile time boundary without requiring a deserialization step to be able to use and manipulate it at runtime. It also means such relationships can be embedded into existing structures like vectors without worrying about vector resizing or needing to separately track elements from indices.

Because algorithms in this library do not themselves allocate memory and merely operate on a range of existing objects, all provided methods are constexpr. They are designed to work with objects that provide a type-based get template for bookkeeping fields in the logical structure which might be index links, a color property, or a key depending on the method and structure type. The returned type from get must be able to be assigned the templated type, and convertible to the template type. Link types must be able to be (explicitly) convertible to ptrdiff_t while color annotations must be able to be explicitly convertible to an enum class of underlying type bool. If get<key_type> for tree does not return a, perhaps, const volatile qualified value or reference to key_type, it must be explicitly convertible to key_type.

Some suggested use cases are:

Serializing a map or list of portably laid out types to disk or across the network.

Manipulating a map or list inside a vector/array using 'swap and pop' to keep nodes contiguous. Index based allows resizing the vector and can save space on links.


Manipulating a map or list inside a vector/array using a second 'free list' with the same links. Additional bookkeeping fields could allow iterating over the entire range out of order, skipping free elements.

Manipulating a map or list embedded in a std::array of compile time initial known elements at compile time and continuing to do so at runtime.

Using indexes from one structure to create "array based" logical structures allowing SIMD operations on contiguous data out of order from the actual list or map.

Building actual objects that implement the above concepts.

Building a semi-intrusive hash_set/map using the supplied primitives and a hash function.

Building hybrid data structures using the intrusive properties of this library. For example, a threaded binary search tree could be constructed from a double_list and a red_black_tree, or a map that implements multiple parallel views over the same elements from multiple red_black_trees.

tests.cpp implements a simple container-like interface on top of std::array to illustrate a very simple use case. Consider using this library with classes that implement logical indices using a fixed memory layout to increase portability when building your own types.

