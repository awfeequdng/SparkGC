#include <vm/instructions.hpp>
#include <mozart/timer.hpp>
auto get_time()->cov::timer::timer_t
{
	return cov::timer::time(cov::timer::time_unit::milli_sec);
}
vm::instruction_set tisc;
int main(int argc, char** argv)
{
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
	interpreter.assemble_bytecode(argv[1]);
	while(true) {
		auto ts=get_time();
		interpreter.interpret();
		::printf("Time spend: %u\n", get_time()-ts);
	}
	return 0;
}