#pragma once
#include <vm/core.hpp>
#include <vm/allocator.hpp>
#include <vm/exception.hpp>
#include <cstdlib>
#include <list>

namespace vm {
	class heap final:public allocator_base {
	public:
		enum class allocate_policy {
			first_fit, best_fit, worst_fit
		};
	private:
		// Heap Start
		void *hs = nullptr;
		// Heap Pointer
		byte_t *hp = nullptr;
		// Heap Limit
		byte_t *hl = nullptr;
		allocate_policy policy = allocate_policy::best_fit;
		bool no_truncate = false;
		std::list<byte_t *> free_list;

		inline size_t &get_size(byte_t *ptr)
		{
			return *reinterpret_cast<size_t *>(ptr);
		}

		void compress()
		{
			std::list<byte_t *> new_list;
			byte_t *ptr = nullptr;
			// Sort the spaces by address.
			free_list.sort([](byte_t *lhs, byte_t *rhs) {
				return lhs < rhs;
			});
			// Compress the free list.
			for (auto p:free_list) {
				if (ptr != nullptr) {
					size_t &size = get_size(ptr);
					if (ptr + size + sizeof(size_t) == p) {
						size += get_size(p) + sizeof(size_t);
					}
					else {
						new_list.push_back(ptr);
						ptr = p;
					}
				}
				else
					ptr = p;
			}
			// Connect the final space and remain spaces.
			if (ptr != nullptr) {
				if (ptr + get_size(ptr) + sizeof(size_t) == hp)
					hp = ptr;
				else
					new_list.push_back(ptr);
			}
			// Swap the new list and old list.
			std::swap(new_list, free_list);
		}

		byte_t *allocate(size_t size)
		{
			// Try to find usable spaces in free list
			if (!free_list.empty()) {
				auto it = free_list.begin();
				switch (policy) {
				case allocate_policy::first_fit: {
					// Find the first fit space.
					for (; it != free_list.end(); ++it)
						if (get_size(*it) >= size)
							break;
					break;
				}
				case allocate_policy::best_fit: {
					// Find the best fit space.
					auto best = it;
					for (; it != free_list.end(); ++it)
						if (get_size(*it) >= size && get_size(*it) < get_size(*best))
							best = it;
					it = best;
					break;
				}
				case allocate_policy::worst_fit: {
					// Find the worst fit space.
					auto max = it;
					for (; it != free_list.end(); ++it)
						if (get_size(*it) > get_size(*max))
							max = it;
					it = max;
					break;
				}
				}
				if (it != free_list.end() && get_size(*it) >= size) {
					// Remove from free list.
					byte_t *raw = *it;
					free_list.erase(it);
					// Truncate remain spaces
					if (!no_truncate && get_size(raw) - size > sizeof(size_t)) {
						byte_t *ptr = raw + sizeof(size_t) + size;
						get_size(ptr) = get_size(raw) - size - sizeof(size_t);
						get_size(raw) = size;
						free_list.push_back(ptr);
					}
					return raw + sizeof(size_t);
				}
			}
			// Checkout remain spaces,if enough,return.
			if (hl - hp >= size + sizeof(size_t)) {
				get_size(hp) = size;
				byte_t *ptr = hp + sizeof(size_t);
				hp += size + sizeof(size_t);
				return ptr;
			}
			return nullptr;
		}

	public:
		heap() = delete;

		heap(const heap &) = delete;

		explicit heap(size_t size, allocate_policy p = allocate_policy::first_fit, bool nt = false) : hs(
			    ::malloc(size)), policy(p), no_truncate(nt)
		{
			hp = reinterpret_cast<byte_t *>(hs);
			hl = hp + size;
		}

		~heap()
		{
			::free(hs);
		}

		byte_t *malloc(size_t size) override
		{
			// Try to allocate.
			byte_t *ptr = allocate(size);
			// If successed,return
			if (ptr != nullptr)
				return ptr;
			// Compress the memory spaces
			compress();
			// Try to allocate again.
			ptr = allocate(size);
			// If successed,return.
			if (ptr != nullptr)
				return ptr;
			else { // There have no usable spaces,throw bad alloc exception.
				vm_throw<std::runtime_error>(20, "Bad alloc.");
				return nullptr;
			}
		}

		void free(byte_t *ptr) override
		{
			free_list.push_back(ptr - sizeof(size_t));
		}

		size_t size_of(byte_t *ptr)
		{
			return get_size(ptr);
		}
	};

	class stack final {
	private:
		// Allocator
		allocator_base* m_alloc;
		// Stack Start
		byte_t *ss = nullptr;
		// Stack Pointer
		byte_t *sp = nullptr;
		// Stack Limit
		byte_t *sl = nullptr;
	public:
		stack() = delete;

		stack(const stack &) = delete;

		stack(size_t size, allocator_base* alloc=&default_alloc) : m_alloc(alloc), ss(alloc->malloc(size))
		{
			sp = ss + size;
			sl = sp;
		}

		~stack()
		{
			::free(ss);
		}

		inline bool empty() const
		{
			return sp == sl;
		}

		byte_t *top()
		{
			if (sp == sl) {
				vm_throw<std::runtime_error>(10, "Stack is empty.");
				return nullptr;
			}
			else
				return sp + sizeof(size_t);
		}

		byte_t *push(size_t size)
		{
			if (sp - reinterpret_cast<byte_t *>(ss) < size + sizeof(size_t))
				vm_throw<std::runtime_error>(-1, "Stack overflow.");
			sp -= size + sizeof(size_t);
			*reinterpret_cast<size_t *>(sp) = size;
			return sp + sizeof(size_t);
		}

		void pop()
		{
			if (sp == sl)
				vm_throw<std::runtime_error>(10, "Stack is empty.");
			else
				sp += *reinterpret_cast<size_t *>(sp) + sizeof(size_t);
		}

		size_t size_of(byte_t *ptr)
		{
			return *reinterpret_cast<size_t *>(ptr - sizeof(size_t));
		}
	};
	/*
		This Virtual Machine uses MSB.
		So when VM reads the bytecode, it will convert the VM endian to local endian.
		And when compiler generate bytecode, it must convert the the local endian to VM endian so that VM can read it.
	*/
	inline bool is_big_endian()
	{
		std::uint32_t i=1;
		std::uint8_t* p=reinterpret_cast<std::uint8_t*>(&i);
		return *p==0;
	}
	inline void to_vm_endian(byte_t* ptr, size_t count, allocator_base* alloc=&default_alloc)
	{
		if(!is_big_endian()) {
			byte_t* buff=alloc->malloc(count);
			for(size_t idx=0; idx<count; ++idx)
				buff[count-idx-1]=ptr[idx];
			memcpy_n(buff, ptr, count);
			alloc->free(buff);
		}
	}
	inline void to_local_endian(byte_t* ptr, size_t count, allocator_base* alloc=&default_alloc)
	{
		if(!is_big_endian()) {
			byte_t* buff=alloc->malloc(count);
			for(size_t idx=0; idx<count; ++idx)
				buff[idx]=ptr[count-idx-1];
			memcpy_n(buff, ptr, count);
			alloc->free(buff);
		}
	}
}