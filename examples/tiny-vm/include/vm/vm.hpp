#pragma once

#include <vm/allocator.hpp>
#include <vm/memory.hpp>
#include <vm/bytestream.hpp>
#include <unordered_map>
#include <functional>
#include <stdexcept>
#include <exception>
#include <cstdlib>
#include <vector>

namespace vm {
    class instruction_base : public vm_object {
    public:
        instruction_base() = default;

        virtual ~instruction_base() = default;

        virtual void vm_obj_destroy() const override {
            this->~instruction_base();
        }

        virtual size_t get_args_byte_count() const {
            return 0;
        }

        virtual void execute(byte_t *, instance_t *) = 0;
    };

    class instruction_builder final {
        friend class instruction_set;

        instruction_base *iptr = nullptr;
    public:
        instruction_builder() = delete;

        instruction_builder(const instruction_builder &) = default;

        instruction_builder(instruction_builder &&) noexcept = default;

        template<typename T, typename...ArgsT>
        instruction_builder(type_container<T>, ArgsT &&...args):iptr(vm_new<T>(std::forward<ArgsT>(args)...)) {}
    };

    class instruction_set final {
        std::unordered_map<byte_t, instruction_base *> m_set;
    public:
        instruction_set() = default;

        instruction_set(const instruction_set &) = delete;

        ~instruction_set() {
            for (auto &it:m_set)
                vm_delete(it.second);
        }

        instruction_set &add_instruction(byte_t opcode, instruction_builder ib) {
            if (m_set.count(opcode) == 0)
                m_set.insert({opcode, ib.iptr});
            else
                vm_throw<std::logic_error>(20, "Optcode already exists.");
            return *this;
        }

        instruction_base *get_instruction(byte_t opcode) {
            if (m_set.count(opcode) > 0)
                return m_set[opcode];
            else
                vm_throw<std::logic_error>(20, "Optcode do not exists.");
            return nullptr;
        }
    };

    class instance_t {
    protected:
        stack m_stack;
        size_t m_pc = 0;
    public:
        instance_t() = delete;

        instance_t(size_t stack_size, allocator_base *alloc = &default_alloc) : m_stack(stack_size, alloc) {}

        inline void pop() {
            m_stack.pop();
        }

        inline void push(byte_t *data, size_t size) {
            memcpy_n(data, m_stack.push(size), size);
        }

        inline byte_t *top() {
            return m_stack.top();
        }

        inline size_t get_size(byte_t *ptr) {
            return m_stack.size_of(ptr);
        }

        inline void jump(size_t index) {
            m_pc = index;
        }

        inline void rst_pc() {
            m_pc = 0;
        }

        inline void inc_pc() {
            ++m_pc;
        }

        inline size_t &get_pc() {
            return m_pc;
        }
    };

    class bytecode_interpreter final {
        allocator_base *alloc;
        instruction_set *inst_set;
        instance_t *instance;
        std::vector<std::pair<instruction_base *, byte_t *>> assembly;
    public:
        bytecode_interpreter() = delete;

        bytecode_interpreter(allocator_base *al_ptr, instruction_set *is_ptr, instance_t *ib_ptr) : alloc(al_ptr),
                                                                                                    inst_set(is_ptr),
                                                                                                    instance(ib_ptr) {}

        bytecode_interpreter(const bytecode_interpreter &) = delete;

        ~bytecode_interpreter() {
            for (auto &it:assembly)
                alloc->free(it.second);
        }

        void assemble_bytecode(byte_stream &stream) {
            for (byte_t byte = stream.get1(); byte != EOF; byte = stream.get1()) {
                if (byte == 0)
                    continue;
                if (byte == 255)
                    break;
                instruction_base *ptr = inst_set->get_instruction(byte);
                size_t count = ptr->get_args_byte_count();
                if (count > 0) {
                    byte_t *buffer = alloc->malloc(count);
                    for (size_t i = 0; i < count; ++i)
                        buffer[i] = stream.get1();
                    to_local_endian(buffer, count);
                    assembly.emplace_back(ptr, buffer);
                } else
                    assembly.emplace_back(ptr, nullptr);
            }
        }

        void assemble_bytecode(const char *path) {
            FILE *f = ::fopen(path, "r+");
            if (f == nullptr) {
                vm_throw<std::runtime_error>(20, "File is not exists.");
                return;
            }
            for (byte_t byte = ::fgetc(f); byte != EOF; byte = ::fgetc(f)) {
                if (byte == 0)
                    continue;
                if (byte == 255)
                    break;
                instruction_base *ptr = inst_set->get_instruction(byte);
                size_t count = ptr->get_args_byte_count();
                if (count > 0) {
                    byte_t *buffer = alloc->malloc(count);
                    for (size_t i = 0; i < count; ++i)
                        buffer[i] = ::fgetc(f);
                    to_local_endian(buffer, count);
                    assembly.emplace_back(ptr, buffer);
                } else
                    assembly.emplace_back(ptr, nullptr);
            }
            ::fclose(f);
        }

        void interpret() {
            instance->rst_pc();
            size_t count = assembly.size();
            for (size_t &pc = instance->get_pc(); pc < count;) {
                auto &it = assembly[pc];
                instance->inc_pc();
                it.first->execute(it.second, instance);
            }
        }
    };
}