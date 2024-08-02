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
LDFLAGS+=-ldl -pthread -lGL -lX11 -lXrandr -lXi

.PHONY: all
all: mkdirs
all: ${BUILD_DIR}/third_party/spdlog
all: ${BUILD_DIR}/third_party/glad
all: ${BUILD_DIR}/third_party/glfw
all: build_engine build_app run

_obj_app = Main.o
_obj_engine = GlHelpers.o
obj_app = $(addprefix ${BUILD_DIR}/app/,$(_obj_app))
obj_engine = $(addprefix ${BUILD_DIR}/engine/,$(_obj_engine))

mkdirs:
	@mkdir -p ${BUILD_DIR}/app
	@mkdir -p ${BUILD_DIR}/engine

$(obj_app): $(addprefix src/app/, $(basename $(notdir ${obj_app})).cpp)
	$(info Compiling $@)
	$(CC) \
		${COMPILE_FLAGS} \
		${INCLUDE_DIR} \
		-c $^ \
		-o $@

$(obj_engine):
	$(info Compiling $@)
	@$(CC) \
		${COMPILE_FLAGS} -fPIC \
		${INCLUDE_DIR} \
		-c $(addprefix src/engine/, $(basename $(notdir $@)).cpp) \
		-o $@

.PHONY: build_app
build_app: $(obj_app)
	${CC} \
		$^ \
		${LDFLAGS} \
		${BUILD_DIR}/engine/libengine.a \
		-o ${BUILD_DIR}/run_app
	@echo "TARGET $@ OK ========"

.PHONY: build_app
build_engine: $(obj_engine)
	ar r ${BUILD_DIR}/engine/libengine.a $^
	@echo "TARGET $@ OK ========"

${BUILD_DIR}/third_party/spdlog:
	mkdir -p $@
	cd $@ && cmake ../../../third_party/spdlog && make -j
	@echo "TARGET $@ OK ========"

${BUILD_DIR}/third_party/glfw:
	mkdir -p $@
	cd $@ && cmake ../../../third_party/glfw && make -j
	@echo "TARGET $@ OK ========"

${BUILD_DIR}/third_party/glad:
	mkdir -p $@
	@$(CC) \
		${COMPILE_FLAGS} \
		-I third_party/glad/include \
		-c third_party/glad/src/gl.c \
		-o $@/gl.o
	@echo "TARGET $@ OK ========"

.PHONY: run
run:
	$(info ======= RUN ========)
	@${BUILD_DIR}/run_app

.PHONY: init_repo
init_repo:
	sudo apt install libwayland-dev
	git submodule update --init --remote
	git submodule status