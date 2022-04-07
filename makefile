SHELL = /usr/bin/env bash
MAKEFLAGS+= --no-print-directory

# various configurations
o3:
	@${MAKE} singeli=0 t=o3         f="-O3" c
o3g:
	@${MAKE} singeli=0 t=o3g        f="-O3 -g" c
o3n:
	@${MAKE} singeli=0 t=o3n        f="-O3 -march=native" c
debug:
	@${MAKE} singeli=0 t=debug      f="-g -DDEBUG" c
debug1:
	@${MAKE} singeli=0 t=debug1     f="-g -DDEBUG" manualJobs=1 c
rtperf:
	@${MAKE} singeli=0 t=rtperf     f="-O3 -DRT_PERF" c
rtverify:
	@${MAKE} singeli=0 t=rtverify   f="-DDEBUG -O3 -DRT_VERIFY -DEEQUAL_NEGZERO" c
heapverify:
	@${MAKE} singeli=0 t=heapverify f="-DDEBUG -g -DHEAP_VERIFY" c
o3n-singeli:
	@${MAKE} singeli=1 t=o3n_si     f="-O3 -march=native" c
o3ng-singeli:
	@${MAKE} singeli=1 t=o3ng_si    f="-g -O3 -march=native" c
debugn-singeli:
	@${MAKE} singeli=1 t=debugn_si  f="-g -DDEBUG -march=native" c
heapverifyn-singeli:
	@${MAKE} singeli=1 t=heapverifyn_si f="-g -DDEBUG -DHEAP_VERIFY -march=native" c
rtverifyn-singeli:
	@${MAKE} singeli=1 t=rtverifyn_si f="-O3 -DRT_VERIFY -DEEQUAL_NEGZERO -march=native" c
wasi-o3:
	@${MAKE} singeli=0 t=wasi_o3    f="-DWASM -DWASI -DNO_MMAP -O3 -DCATCH_ERRORS=0 -D_WASI_EMULATED_MMAN --target=wasm32-wasi" LDFLAGS="-lwasi-emulated-mman --target=wasm32-wasi -Wl,-z,stack-size=8388608 -Wl,--initial-memory=67108864" LD_LIBS= PIE= c
emcc-o3:
	@${MAKE} singeli=0 t=emcc_o3    f='-DWASM -DEMCC -O3' LDFLAGS='-s EXPORTED_FUNCTIONS=_main,_cbqn_runLine -s EXPORTED_RUNTIME_METHODS=ccall,cwrap -s ALLOW_MEMORY_GROWTH=1 -s ENVIRONMENT=worker -s FILESYSTEM=0 -s MODULARIZE=1' OUTPUT=BQN.js CC=emcc c


# compiler setup
CC = clang
PIE = -no-pie
LD_LIBS = -lm
OUTPUT = BQN

# test if we are running gcc or clang
CC_IS_CLANG = $(shell $(CC) --version | head -n1 | grep -m 1 -c "clang")
ifeq (${CC_IS_CLANG}, 1)
	NOWARN = -Wno-microsoft-anon-tag -Wno-bitwise-instead-of-logical -Wno-unknown-warning-option
else
	NOWARN = -Wno-parentheses
endif
ifeq (${singeli}, 1)
	SINGELIFLAGS = '-DSINGELI'
else
	singeli = 0
	SINGELIFLAGS =
endif

ALL_CC_FLAGS = -std=gnu11 -Wall -Wno-unused-function -fms-extensions $(CCFLAGS) $(SINGELIFLAGS) $(NOWARN) $(f)
CMD = $(CC) $(ALL_CC_FLAGS) -MMD -MP -MF

ifneq (${manualJobs},1)
	ifeq (${MAKECMDGOALS},gen)
		MAKEFLAGS+= -j4 manualJobs=1
	endif
	ifeq (${MAKECMDGOALS},gen-singeli)
		MAKEFLAGS+= -j4 manualJobs=1
	endif
endif



# actual build
bd = obj/${t}
c: # custom
	@mkdir -p ${bd}
	@if [ "${singeli}" -eq 1 ]; then \
		mkdir -p src/singeli/gen;      \
		${MAKE} postmsg="post-singeli build:" gen-singeli; \
	fi
	@${MAKE} gen

single-o3:
	$(CC) $(ALL_CC_FLAGS) $(LDFLAGS) $(PIE) -O3 -o ${OUTPUT} src/opt/single.c $(LD_LIBS)
single-o3g:
	$(CC) $(ALL_CC_FLAGS) $(LDFLAGS) $(PIE) -O3 -g -o ${OUTPUT} src/opt/single.c $(LD_LIBS)
single-debug:
	$(CC) $(ALL_CC_FLAGS) $(LDFLAGS) $(PIE) -DDEBUG -g -o ${OUTPUT} src/opt/single.c $(LD_LIBS)
single-c:
	$(CC) $(ALL_CC_FLAGS) $(LDFLAGS) $(PIE) -o ${OUTPUT} src/opt/single.c $(LD_LIBS)





gen: builtins core base jit utils # build the final binary
	@$(CC) ${CCFLAGS} ${LDFLAGS} ${PIE} -o ${OUTPUT} ${bd}/*.o $(LD_LIBS)
	@echo ${postmsg}

# build individual object files
core: ${addprefix ${bd}/, tyarr.o harr.o fillarr.o stuff.o derv.o mm.o heap.o}
${bd}/%.o: src/core/%.c
	@echo $< | cut -c 5-
	@$(CMD) $@.d -o $@ -c $<

base: ${addprefix ${bd}/, load.o main.o rtwrap.o vm.o ns.o nfns.o}
${bd}/%.o: src/%.c
	@echo $< | cut -c 5-
	@$(CMD) $@.d -o $@ -c $<

utils: ${addprefix ${bd}/, utf.o hash.o file.o mut.o each.o bits.o}
${bd}/%.o: src/utils/%.c
	@echo $< | cut -c 5-
	@$(CMD) $@.d -o $@ -c $<

jit: ${addprefix ${bd}/, nvm.o}
${bd}/%.o: src/jit/%.c
	@echo $< | cut -c 5-
	@$(CMD) $@.d -o $@ -c $<

builtins: ${addprefix ${bd}/, arithm.o arithd.o cmp.o sfns.o sort.o md1.o md2.o fns.o sysfn.o internal.o inverse.o}
${bd}/%.o: src/builtins/%.c
	@echo $< | cut -c 5-
	@$(CMD) $@.d -o $@ -c $<

src/gen/customRuntime:
	@echo "Copying precompiled bytecode from the bytecode branch"
	git checkout remotes/origin/bytecode src/gen/{compiles,formatter,runtime0,runtime1,src}
	git reset src/gen/{compiles,formatter,runtime0,runtime1,src}
${bd}/load.o: src/gen/customRuntime



# singeli
.INTERMEDIATE: preSingeliBin
preSingeliBin:
	@if [ ! -d Singeli ]; then \
		echo "Updating Singeli submodule; link custom Singeli to Singeli/ to avoid"; \
		git submodule update --init; \
	fi
	@echo "pre-singeli build:"
	@${MAKE} singeli=0 postmsg="singeli sources:" t=presingeli f='-O1' c
	@mv BQN obj/presingeli/BQN


gen-singeli: ${addprefix src/singeli/gen/, cmp.c dyarith.c slash.c equal.c}
	@echo $(postmsg)
src/singeli/gen/%.c: src/singeli/src/%.singeli preSingeliBin
	@echo $< | cut -c 17- | sed 's/^/  /'
	@obj/presingeli/BQN SingeliMake.bqn "$$(if [ -d Singeli ]; then echo Singeli; else echo SingeliClone; fi)" $< $@ "${bd}"



# dependency files
-include $(bd)/*.d

install:
	cp -f BQN /usr/local/bin/bqn

uninstall:
	rm -f /usr/local/bin/bqn

# `if` to allow `make clean` alone to clean everything, but `make t=debug clean` to just clean obj/debug
ifneq (,$(findstring clean,$(MAKECMDGOALS)))
ifndef t
t = *
clean: clean-singeli clean-runtime
endif
endif

clean-singeli:
	rm -rf src/singeli/gen/
clean-runtime:
	rm -f src/gen/customRuntime
clean-build:
	rm -f ${bd}/*.o
	rm -f ${bd}/*.d

clean: clean-build
