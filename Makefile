BUILD_TYPE=debug

BUILD_DIR=build
CC=ccache clang++
COMPILE_FLAGS=-std=c++20 $(if $(findstring debug,${BUILD_TYPE}),-g -DXDEBUG,)
INCLUDE_DIR+=-Iinclude
INCLUDE_DIR+=-Ithird_party/spdlog/include
INCLUDE_DIR+=-Ithird_party/glad/include
LDFLAGS+=${BUILD_DIR}/third_party/spdlog/libspdlog.a
LDFLAGS+=${BUILD_DIR}/third_party/glfw/src/libglfw3.a
LDFLAGS+=${BUILD_DIR}/third_party/glad/gl.o
LDFLAGS+=-pthread -ldl

.PHONY: all
all: ${BUILD_DIR}/third_party/spdlog
all: ${BUILD_DIR}/third_party/glad
all: ${BUILD_DIR}/third_party/glfw
all: build_engine
all: build_app
all: run

.PHONY: run
run: ${BUILD_DIR}/run_app
	@echo "====== RUN ======"
	@${BUILD_DIR}/run_app

.PHONY: prettify
prettify:
	find src include -regex '.*\.\(cpp\|hpp\|cu\|cuh\|c\|h\)' -exec clang-format --verbose -style=file -i {} \;

.PHONY: clean
clean:
	rm -r ${BUILD_DIR}

.PHONY: clean_own
clean_own:
	rm -r ${BUILD_DIR}/run_app ${BUILD_DIR}/app ${BUILD_DIR}/engine

obj_app = $(addprefix ${BUILD_DIR}/app/,\
	Main.o \
)
obj_engine = $(addprefix ${BUILD_DIR}/engine/,\
	GlHelpers.o \
)
hpp_engine = $(addprefix include/engine/,\
	Prelude.hpp GlHelpers.hpp \
)
hpp_engine_private = $(addprefix src/engine/details/, \
	Prelude.hpp \
)

.PHONY: build_app
build_app: ${BUILD_DIR}/app ${BUILD_DIR}/run_app

.PHONY: build_engine
build_engine: ${BUILD_DIR}/engine ${BUILD_DIR}/engine/libengine.a

# linking main executable
${BUILD_DIR}/run_app: ${obj_app} ${BUILD_DIR}/engine/libengine.a
	$(info > Linking $@)
	@${CC} $^ ${LDFLAGS} -o $@

# linking engine library
${BUILD_DIR}/engine/libengine.a: ${obj_engine}
	$(info > Linking $@)
	@ar r $@ $^

# compiling main executable sources
${BUILD_DIR}/app/%.o: src/app/%.cpp ${hpp_engine}
	$(info > Compiling $@)
	@$(CC) ${COMPILE_FLAGS} ${INCLUDE_DIR} -c $(filter %.cpp,$^) -o $@

# compiling engine sources
${BUILD_DIR}/engine/%.o: src/engine/%.cpp ${hpp_engine} ${hpp_engine_private}
	$(info > Compiling $@)
	$(CC) ${COMPILE_FLAGS} ${INCLUDE_DIR} -Isrc/engine -c $(filter %.cpp,$^) -o $@

# compiling third party
${BUILD_DIR}/third_party/spdlog:
	mkdir -p $@
	cd $@ && cmake ../../../third_party/spdlog && make -j

${BUILD_DIR}/third_party/glfw:
	mkdir -p $@
	cd $@ && cmake ../../../third_party/glfw && make -j

${BUILD_DIR}/third_party/glad:
	mkdir -p $@
	$(CC) ${COMPILE_FLAGS} -I third_party/glad/include -c third_party/glad/src/gl.c -o $@/gl.o

${BUILD_DIR}/app ${BUILD_DIR}/engine:
	mkdir -p $@

.PHONY: init_repo
init_repo:
	sudo apt install libwayland-dev
	git submodule update --init --remote
	git submodule status