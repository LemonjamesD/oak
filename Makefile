# Jordan Dehmel, 2023
# jdehmel@outlook.com
# github.com/jorbDehmel/oak

##########################################
# Comparisons of build compilers         #
# time make clean uninstall install test #
# clang++: 125.25s 						 #
# g++: 143.43							 #
##########################################

CC := clang++ -pedantic -Wall
OBJS := build/lexer.o build/symbol-table.o \
	build/type-builder.o build/reconstruct.o \
	build/macros.o build/sequence.o \
	build/packages.o build/sizer.o build/op-sub.o \
	build/mem.o build/acorn_resources.o \
	build/document.o build/rules.o build/enums.o \
	build/mangler.o build/generics.o

HEADS := lexer.hpp reconstruct.hpp symbol-table.hpp \
	type-builder.hpp macros.hpp tags.hpp \
	sequence.hpp packages.hpp sizer.hpp op-sub.hpp \
	acorn_resources.hpp document.hpp rules.hpp \
	enums.hpp mangler.hpp generics.hpp

# -O3 is best for actual installs, not for testing.
# -g should be used when size doesn't matter
FLAGS := -O3 -g

TEST := acorn

all: bin/acorn.out

install: bin/acorn.out std_oak_header.hpp
	sudo mkdir -p /usr/include/oak

	sudo cp std_oak_header.hpp /usr/include/oak
	sudo cp packages_list.txt /usr/include/oak
	sudo cp -r std /usr/include/oak

	sudo $(MAKE) -C /usr/include/oak/std
	sudo rm -f /usr/include/oak/std/*.cpp
	sudo rm -f /usr/include/oak/std/Makefile

	sudo cp bin/acorn.out /usr/bin/acorn

	acorn -S sdl

uninstall:
	sudo rm -rf /usr/bin/acorn /usr/include/oak /usr/include/std_oak_header.hpp

reinstall:
	$(MAKE) -C . uninstall clean install

test: install
	$(TEST) oak_demos/access_test.oak -o access_test.out
	$(TEST) oak_demos/assert_test.oak -o assert_test.out
	$(TEST) oak_demos/bool_test.oak -o bool_test.out
	$(TEST) oak_demos/color_test.oak -o color_test.out
	$(TEST) oak_demos/cond_test.oak -o cond_test.out
	$(TEST) oak_demos/conv_test.oak -o conv_test.out
	$(TEST) oak_demos/def_test.oak -o def_test.out
	$(TEST) oak_demos/enum_test.oak -o enum_test.out
	$(TEST) oak_demos/erase_test.oak -o erase_test.out
	$(TEST) oak_demos/file_test.oak -o file_test.out
	$(TEST) oak_demos/fn_ptr_test.oak -o fn_ptr_test.out
	$(TEST) oak_demos/fn_ptr_test_2.oak -o fn_ptr_test_2.out
	$(TEST) oak_demos/hello_world.oak -o hello_world.out
	$(TEST) oak_demos/i_file_test.oak -o i_file_test.out
	$(TEST) oak_demos/loop_test.oak -o loop_test.out
	$(TEST) oak_demos/macro_test.oak -o macro_test.out
	$(TEST) oak_demos/math_test.oak -o math_test.out
	$(TEST) oak_demos/mem_test.oak -o mem_test.out
	$(TEST) oak_demos/quine.oak -o quine.out
	$(TEST) oak_demos/rec_test.oak -o rec_test.out
	$(TEST) oak_demos/rule_test.oak -o rule_test.out
	$(TEST) oak_demos/rule_test_2.oak -o rule_test_2.out
	$(TEST) oak_demos/string_test.oak -o string_test.out
	$(TEST) oak_demos/thread_test.oak -o thread_test.out
	$(TEST) oak_demos/panic_test.oak -o panic_test.out
	$(TEST) oak_demos/rule_test_3.oak -o rule_test_3.out

	$(MAKE) sdltest
	$(MAKE) gentest

sdltest:
	$(TEST) oak_demos/sdl_test.oak -o sdl_test.out
	$(TEST) oak_demos/sdl_test_2.oak -o sdl_test_2.out
	$(TEST) oak_demos/sdl_test_3.oak -o sdl_test_3.out

gentest:
	$(TEST) oak_demos/arr_test.oak -o arr_test.out
	$(TEST) oak_demos/generic_test.oak -o generic_test.out
	$(TEST) oak_demos/gen_enum_test.oak -o gen_enum_test.out
	$(TEST) oak_demos/gen_struct_test.oak -o gen_struct_test.out
	$(TEST) oak_demos/gen_test_2.oak -o gen_test_2.out
	$(TEST) oak_demos/nested_gen_test.oak -o nested_gen_test.out

build/%.o:	%.cpp $(HEADS)
	mkdir -p build
	$(CC) $(FLAGS) -c -o $@ $<

bin/%.out:	build/%.o $(OBJS) $(HEADS)
	mkdir -p bin
	$(CC) $(FLAGS) -o $@ $< $(OBJS)

tclean:
	rm -rf *.out

clean:
	rm -rf bin/* build/* *.o *.out *.log
