BUILD_TYPE=debug

BUILD_DIR=build
INSTALL_DIR=${BUILD_DIR}/install
APP_EXE=${BUILD_DIR}/run_app

THIRD_PARTY_DEPS=\
	${BUILD_DIR}/third_party/spdlog/libspdlog.a \
	${BUILD_DIR}/third_party/glfw/libglfw3.a \
	${BUILD_DIR}/third_party/glad/gl.o

.PHONY: all
all: build_engine
all: build_app

CC=ccache clang++
COMPILE_FLAGS=-std=c++20 $(if $(findstring debug,${BUILD_TYPE}),-g -DXDEBUG,)
INCLUDE_DIR+=-I src/engine/include
INCLUDE_DIR+=-I third_party/spdlog/include
INCLUDE_DIR+=-I third_party/glad/include
INCLUDE_DIR+=-I data
LDFLAGS+=-pthread -ldl
CLANG_FORMAT=clang-format-17

obj_app_ = Main.o

obj_engine_ = Assets.o GlCapabilities.o GlHelpers.o GlProgram.o \
	GlTextureUnits.o GlGuard.o Prelude.o RenderLoop.o WindowContext.o

hpp_engine_ = Assets.hpp GlBuffer.hpp GlCapabilities.hpp GlHelpers.hpp \
	GlProgram.hpp GlTextureUnits.hpp GlGuard.hpp Prelude.hpp \
	RenderContext.hpp RenderLoop.hpp WindowContext.hpp

hpp_engine_private_ = Prelude.hpp

obj_app = $(addprefix ${BUILD_DIR}/app/, ${obj_app_})
obj_engine = $(addprefix ${BUILD_DIR}/engine/, ${obj_engine_})
hpp_engine = $(addprefix src/engine/include/engine/, ${hpp_engine_})
hpp_engine_private = $(addprefix src/engine/include_private/engine_private/, ${hpp_engine_private_})

.PHONY: run
run: ${INSTALL_DIR}/run_app
	@echo "====== RUN ======"
	@${INSTALL_DIR}/run_app

.PHONY: prettify
prettify:
	find src -regex '.*\.\(cpp\|hpp\|cu\|cuh\|c\|h\)' -exec ${CLANG_FORMAT} --verbose -style=file -i {} \;

.PHONY: clean
clean:
	rm -r ${BUILD_DIR}

.PHONY: clean_own
clean_own:
	rm -r ${APP_EXE} ${BUILD_DIR}/app ${BUILD_DIR}/engine ${BUILD_DIR}/install

.PHONY: build_app
build_app: ${BUILD_DIR}/engine/libengine.a ${BUILD_DIR}/app ${APP_EXE}

.PHONY: build_engine
build_engine: ${BUILD_DIR}/engine ${BUILD_DIR}/engine/libengine.a

${INSTALL_DIR}/run_app: ${INSTALL_DIR} ${APP_EXE}
	find data -regex '.*\.\(vert\|frag\)' -exec cp --parents \{\} ${INSTALL_DIR} \;
	cp ${APP_EXE} $@

${APP_EXE}: ${obj_app} ${THIRD_PARTY_DEPS} ${BUILD_DIR}/engine/libengine.a
	$(info > Linking $@)
	@${CC} $^ ${LDFLAGS} -o $@

# linking engine library
${BUILD_DIR}/engine/libengine.a: ${THIRD_PARTY_DEPS} ${obj_engine}
	$(info > Linking $@)
	@ar r $@ ${obj_engine}

# compiling main executable sources
${BUILD_DIR}/app/%.o: src/app/%.cpp ${hpp_engine}
	$(info > Compiling $@)
	@$(CC) ${COMPILE_FLAGS} ${INCLUDE_DIR} -c $(filter %.cpp,$^) -o $@

# compiling engine sources
${BUILD_DIR}/engine/%.o: src/engine/%.cpp ${hpp_engine} ${hpp_engine_private}
	$(info > Compiling $@)
	@$(CC) ${COMPILE_FLAGS} ${INCLUDE_DIR} -I src/engine/include_private -c $(filter %.cpp,$^) -o $@

# compiling third party
${BUILD_DIR}/third_party/spdlog/libspdlog.a:
	cmake -S third_party/spdlog -B $(dir $@) && cmake --build $(dir $@)

${BUILD_DIR}/third_party/glfw/libglfw3.a:
	cmake -S third_party/glfw -B $(dir $@) && cmake --build $(dir $@)
	cp $(dir $@)/src/libglfw3.a $(dir $@)

${BUILD_DIR}/third_party/glad/gl.o:
	mkdir -p $(dir $@)
	$(CC) ${COMPILE_FLAGS} -I third_party/glad/include -c third_party/glad/src/gl.c -o $@

${BUILD_DIR}/app ${BUILD_DIR}/engine ${INSTALL_DIR}:
	mkdir -p $@

.PHONY: init_repo
init_repo:
	sudo apt install libwayland-dev libxkbcommon-dev xorg-dev
	git submodule update --init --remote
	git submodule status