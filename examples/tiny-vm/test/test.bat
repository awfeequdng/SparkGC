@g++ -std=c++11 -I ..\include .\gen_elf.cpp -o gen
@g++ -std=c++11 -I ..\include .\run_elf.cpp -O3 -o run
@.\gen .\test.elf
@.\run .\test.elf