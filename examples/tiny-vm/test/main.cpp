//
// Created by kiva on 2019-01-03.
//
#include <vm/vm.hpp>
#include <vm/instructions.hpp>
#include <vm/bytestream.hpp>
#include <chrono>

vm::instruction_set tisc;

void write_int32(vm::byte_t *buff, std::int32_t v) {
    *reinterpret_cast<std::int32_t *>(buff) = v;
    vm::to_vm_endian(buff, sizeof(std::int32_t));
}

void write_int64(vm::byte_t *buff, std::int64_t v) {
    *reinterpret_cast<std::int64_t *>(buff) = v;
    vm::to_vm_endian(buff, sizeof(std::int64_t));
}

int main(int argc, char **argv) {
    /*
	TISC ASM:
	    push_i32 10000
	    push_i32 1000
	    mul_i32
		loop: jmp_if $exit
	    push_i32 1
	    sub_i32
	    jmp $loop
	    exit: pop
		eof
	C Code:
		int a=100;
		a=a*100;
		while(a>0)
			a=a-1;
	*/
    vm::byte_t buffer[]{
        0x5, 0, 0, 0, 0,
        0x5, 0, 0, 0, 0,
        0xb,
        0x3, 0, 0, 0, 0, 0, 0, 0, 0,
        0x5, 0, 0, 0, 0,
        0x9,
        0x2, 0, 0, 0, 0, 0, 0, 0, 0,
        0x4,
        255
    };
    write_int32(buffer + 1, 10000);
    write_int32(buffer + 6, 1000);
    write_int64(buffer + 12, 7);
    write_int32(buffer + 21, 1);
    write_int64(buffer + 27, 3);

    vm::byte_stream code;
    code.init(buffer, sizeof(buffer) / sizeof(buffer[0]));

    tisc
        .add_instruction(0x1, vm::instruction_builder(vm::type_container<vm::interrupt>()))
        .add_instruction(0x2, vm::instruction_builder(vm::type_container<vm::jump>()))
        .add_instruction(0x3, vm::instruction_builder(vm::type_container<vm::jump_if>()))
        .add_instruction(0x4, vm::instruction_builder(vm::type_container<vm::pop>()))
        .add_instruction(0x5, vm::instruction_builder(vm::type_container<vm::push_i32>()))
        .add_instruction(0x6, vm::instruction_builder(vm::type_container<vm::push_i64>()))
        .add_instruction(0x7, vm::instruction_builder(vm::type_container<vm::add_i32>()))
        .add_instruction(0x8, vm::instruction_builder(vm::type_container<vm::add_i64>()))
        .add_instruction(0x9, vm::instruction_builder(vm::type_container<vm::sub_i32>()))
        .add_instruction(0xa, vm::instruction_builder(vm::type_container<vm::sub_i64>()))
        .add_instruction(0xb, vm::instruction_builder(vm::type_container<vm::mul_i32>()))
        .add_instruction(0xc, vm::instruction_builder(vm::type_container<vm::mul_i64>()))
        .add_instruction(0xd, vm::instruction_builder(vm::type_container<vm::div_i32>()))
        .add_instruction(0xe, vm::instruction_builder(vm::type_container<vm::div_i64>()));
    vm::instance_t instance(256);
    vm::bytecode_interpreter interpreter(&vm::default_alloc, &tisc, &instance);
    interpreter.assemble_bytecode(code);

    auto start = std::chrono::system_clock::now();
    interpreter.interpret();
    auto end = std::chrono::system_clock::now();
    auto cost = end - start;
    printf("Time spent: %lld\n", cost.count());
    return 0;
}

