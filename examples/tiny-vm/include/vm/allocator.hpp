#pragma once
#include <vm/core.hpp>
#include <cstdlib>

namespace vm {
	class allocator_base {
	public:
		allocator_base()=default;
		virtual ~allocator_base()=default;
		virtual byte_t* malloc(size_t)=0;
		virtual void free(byte_t*)=0;
	};
	namespace memory_handler {
		template<typename T, typename...ArgsT>static void construct(byte_t* ptr, ArgsT&&...args)
		{
			::new (reinterpret_cast<T*>(ptr)) T(std::forward<ArgsT>(args)...);
		}
		template<typename T>static void destroy(byte_t* ptr)
		{
			reinterpret_cast<T*>(ptr)->~T();
		}
		template<typename T, typename...ArgsT>static T* alloc_new(allocator_base* alloc, ArgsT&&...args)
		{
			byte_t* ptr=alloc->malloc(sizeof(T));
			::new (reinterpret_cast<T*>(ptr)) T(std::forward<ArgsT>(args)...);
			return reinterpret_cast<T*>(ptr);
		}
		template<typename T>static void alloc_delete(allocator_base* alloc, T* ptr)
		{
			reinterpret_cast<T*>(ptr)->~T();
			alloc->free(reinterpret_cast<byte_t*>(ptr));
		}
	};
	class default_allocator final:public allocator_base {
	public:
		virtual byte_t* malloc(size_t size) override
		{
			return reinterpret_cast<byte_t*>(::malloc(size));
		}
		virtual void free(byte_t* ptr) override
		{
			::free(reinterpret_cast<void*>(ptr));
		}
	} default_alloc;
	class vm_object {
	protected:
		allocator_base* vm_obj_alloc=&default_alloc;
	public:
		vm_object()=default;
		virtual ~vm_object()=default;
		virtual void vm_obj_destroy() const
		{
			this->~vm_object();
		}
		void vm_obj_collect() const
		{
			allocator_base* alloc=vm_obj_alloc;
			this->vm_obj_destroy();
			alloc->free(reinterpret_cast<byte_t*>(ptr_remove_const(this)));
		}
	};
	template<typename T, typename...ArgsT>T* vm_new(ArgsT&&...args)
	{
		return memory_handler::alloc_new<T>(&default_alloc, std::forward<ArgsT>(args)...);
	}
	void vm_delete(const vm_object* ptr)
	{
		ptr->vm_obj_collect();
	}
}