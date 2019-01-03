#pragma once
#include <utility>
#include <cstdint>

namespace vm {
	namespace types {
		using i32=int32_t;
		using i64=int64_t;
	}
	using byte_t=uint8_t;
	using size_t=uint32_t;
	class instance_t;
	class instruction_base;
	template<typename T>struct type_container final {
		using type=T;
	};
	template<typename T>T* ptr_remove_const(const T* ptr)
	{
		return const_cast<T*>(ptr);
	}
	template<typename data_t, typename size_t>
	void memcpy_n(data_t *src, data_t *dest, size_t count)
	{
		if (count > 0) {
			byte_t *_src = reinterpret_cast<byte_t *>(src);
			byte_t *_dest = reinterpret_cast<byte_t *>(dest);
			size_t _count = count * sizeof(data_t);
			for (size_t idx = 0; idx < _count; ++idx)
				*(_dest + idx) = *(_src + idx);
		}
	}
}