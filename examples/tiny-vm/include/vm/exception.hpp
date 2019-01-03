#pragma once
#include <vm/core.hpp>
#include <vm/allocator.hpp>
#include <initializer_list>
#include <forward_list>
#include <functional>
#include <exception>
#include <stdexcept>
#include <cstdio>

namespace vm {
	enum class error_status {
		ready, warning, error, fatal
	};
	class exception_handler {
	public:
		exception_handler()=default;
		virtual ~exception_handler()=default;
		/*
		Description: Exception Listener Type
		Return: The status of listener, return true means finished
		Argument:
		1) Standard Exception Object
		2) Exception Level: -1=Fatal Error 0=Information 1-10=Warning 11-20=Error
		*/
		using listener_t=std::function<bool(std::exception*, int)>;
		/*
		Description: Add an exception listener
		Return: none
		Argument: Listener Object
		*/
		virtual void add_listener(const listener_t&)=0;
		/*
		Description: Raise an exception
		Return: None
		Argument:
		1) Standard Exception Object
		2) Exception Level: -1=Fatal Error 0=Information 1-10=Warning 11-20=Error
		3) Allocator: Specify the allocator
		*/
		virtual void raise(std::exception* e, int level=0, allocator_base* alloc=&default_alloc)=0;
		/*
		Description: Check status
		Return: Exception Handler's Status
		Argument: None
		*/
		virtual error_status check_status() const=0;
	};
	class default_exception_handler:public exception_handler {
		std::forward_list<exception_handler::listener_t> m_listener_list;
		error_status m_status=error_status::ready;
	public:
		using exception_handler::exception_handler;
		default_exception_handler(const std::initializer_list<exception_handler::listener_t>& lst):m_listener_list(lst) {}
		virtual void add_listener(const listener_t& listener) override
		{
			m_listener_list.emplace_front(listener);
		}
		virtual void raise(std::exception* e, int level, allocator_base* alloc=&default_alloc) override
		{
			for(auto& listener:m_listener_list)
				if(listener(e, level))
					break;
			memory_handler::alloc_delete(alloc, e);
			if(level<0)
				std::terminate();
			else if(level>0) {
				if(level>20)
					raise(memory_handler::alloc_new<std::range_error>(&default_alloc, "CovVM: Wrong range of error level."), 1);
				else if(level>10)
					m_status=error_status::error;
				else
					m_status=error_status::warning;
			}
		}
		virtual error_status check_status() const override
		{
			return m_status;
		}
	} default_except({
		[](std::exception* e, int level)
		{
			if(level<0)
				::printf("CovVM Fatal Error: %s\n", e->what());
			else if(level>20)
				::printf("CovVM Unknown Exception(Level=%d): %s\n", level, e->what());
			else if(level>10)
				::printf("CovVM Error: %s\n", e->what());
			else if(level>0)
				::printf("CovVM Warning: %s\n", e->what());
			else
				::printf("CovVM Information: %s\n", e->what());
			::fflush(stdout);
			return false;
		}
	});
	template<typename T, typename...ArgsT>void vm_throw(int level, ArgsT&&...args)
	{
		default_except.raise(vm_new<T>(std::forward<ArgsT>(args)...), level);
	}
}