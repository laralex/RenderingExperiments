.PHONY: all
all: build_engine
all: build_app

.PHONY: init_repo
init_repo:
	sudo apt install libwayland-dev libxkbcommon-dev xorg-dev
	git submodule update --init --remote
	git submodule status

USE_DEP_FILES?=1
USE_PCH?=1
# COMPILER_DUMP?=1

BUILD_TYPE=debug

BUILD_DIR=build
INSTALL_DIR=${BUILD_DIR}/install
MAKEFILE_DIR=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
APP_EXE=${BUILD_DIR}/run_app
PRECOMPILED_HEADER=${BUILD_DIR}/Precompiled.hpp.pch

THIRD_PARTY_DEPS=\
	${BUILD_DIR}/third_party/spdlog/libspdlog.a \
	${BUILD_DIR}/third_party/glfw/src/src/libglfw3.a \
	${BUILD_DIR}/third_party/glad/gl.o \
	${BUILD_DIR}/third_party/glm/glm/libglm.a \
	${BUILD_DIR}/third_party/stb/stb_image.o

CC=ccache clang++

# NOTE: -MMD generates .d files alongside .o files (targets with all dependent headers)
COMPILE_FLAGS=-std=c++20 $(if $(USE_DEP_FILES),-MMD,) $(if $(COMPILER_DUMP),-save-stats,) $(if $(findstring debug,${BUILD_TYPE}),-g -DXDEBUG,)
INCLUDE_DIR+=-I src/engine/include
INCLUDE_DIR+=-I third_party/spdlog/include
INCLUDE_DIR+=-I third_party/glad/include
INCLUDE_DIR+=-I third_party/glm/
INCLUDE_DIR+=-I third_party/stb/
INCLUDE_DIR+=-I data
LDFLAGS+=-pthread -ldl
CLANG_FORMAT=clang-format-17

src_app_ = Main.cpp
src_app = $(addprefix ${BUILD_DIR}/app/, ${src_app_})
obj_app = ${src_app:.cpp=.o}

src_engine_ = \
	Assets.cpp BoxMesh.cpp \
	EngineLoop.cpp IcosphereMesh.cpp \
	LineRendererInput.cpp PointRendererInput.cpp \
	PlaneMesh.cpp Unprojection.cpp \
	UvSphereMesh.cpp \
	Precompiled.cpp WindowContext.cpp \
	gl/AxesRenderer.cpp \
	gl/BoxRenderer.cpp gl/ProceduralMeshes.cpp \
	gl/BillboardRenderer.cpp \
	gl/Buffer.cpp gl/Capabilities.cpp \
	gl/Common.cpp gl/CommonRenderers.cpp \
	gl/PointRenderer.cpp \
	gl/Debug.cpp gl/FlatRenderer.cpp \
	gl/FrustumRenderer.cpp gl/Guard.cpp \
	gl/Init.cpp gl/LineRenderer.cpp \
	gl/Extensions.cpp gl/Framebuffer.cpp \
	gl/Program.cpp \
	gl/Renderbuffer.cpp \
	gl/Sampler.cpp gl/SamplersCache.cpp \
	gl/Shader.cpp gl/Texture.cpp \
	gl/TextureUnits.cpp gl/Uniform.cpp \
	gl/Vao.cpp

src_engine = $(addprefix ${BUILD_DIR}/engine/src/, ${src_engine_})
obj_engine = ${src_engine:.cpp=.o}

ifeq (1,${USE_DEP_FILES})
-include $(src_engine:.cpp=.d)
-include $(src_app:.cpp=.d)
-include ${PRECOMPILED_HEADER:.pch=.d}
endif

.PHONY: wtf
wtf:
	$(info > SRC files: ${src_engine} ${src_app})

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
build_engine: ${BUILD_DIR}/engine/src/gl ${BUILD_DIR}/engine/libengine.a

.PHONY: patch_d_files

.PHONY: ${INSTALL_DIR}/run_app
${INSTALL_DIR}/run_app: ${INSTALL_DIR} ${APP_EXE}
	find data -regex '.*\.\(vert\|frag\)' -exec cp --parents \{\} ${INSTALL_DIR} \;
	find data -regex '.*\.\(jpg\|jpeg\|png\)' -exec cp --parents \{\} ${INSTALL_DIR} \;
	cp ${APP_EXE} $@

${APP_EXE}: ${obj_app} ${THIRD_PARTY_DEPS} ${BUILD_DIR}/engine/libengine.a
	$(info > Linking $@)
	${CC} $^ ${LDFLAGS} -o $@

# linking engine library
${BUILD_DIR}/engine/libengine.a: ${THIRD_PARTY_DEPS} ${obj_engine}
	$(info > Linking $@)
	@ar r $@ ${obj_engine}

# compiling main executable sources
${BUILD_DIR}/app/%.o: src/app/%.cpp
	$(info > Compiling $@)
	@$(CC) ${COMPILE_FLAGS} ${INCLUDE_DIR} -c $< -o $@

# compiling engine sources
${BUILD_DIR}/engine/%.o: src/engine/%.cpp $(if $(USE_PCH),${PRECOMPILED_HEADER},)
	$(info > Compiling $@)
	@$(CC) ${COMPILE_FLAGS} ${INCLUDE_DIR} -I src/engine/include_private $(if $(USE_PCH),-include-pch ${PRECOMPILED_HEADER},) -c $< -o $@

${PRECOMPILED_HEADER}: src/engine/include/engine/Precompiled.hpp
	$(info > Precompiled header $@)
	$(CC) ${COMPILE_FLAGS} ${INCLUDE_DIR} -c -o $@ -xc++-header $<

# compiling third party
${BUILD_DIR}/third_party/spdlog/libspdlog.a:
	cmake -S third_party/spdlog -B $(dir $@) && cmake --build $(dir $@)

${BUILD_DIR}/third_party/glfw/src/libglfw3.a:
	cmake -S third_party/glfw -B $(dir $@) && cmake --build $(dir $@)

${BUILD_DIR}/third_party/glad/gl.o:
	mkdir -p $(dir $@)
	$(CC) ${COMPILE_FLAGS} -I third_party/glad/include -c third_party/glad/src/gl.c -o $@

${BUILD_DIR}/third_party/glm/glm/libglm.a:
	cmake -S third_party/glm \
		-DCMAKE_INSTALL_INCLUDEDIR=${MAKEFILE_DIR}/${BUILD_DIR}/third_party/glm_install/include \
		-DGLM_BUILD_TESTS=OFF \
		-DBUILD_SHARED_LIBS=OFF \
		-B ${BUILD_DIR}/third_party/glm .
	cmake --build ${BUILD_DIR}/third_party/glm -- all
	cmake --build ${BUILD_DIR}/third_party/glm -- install

${BUILD_DIR}/third_party/stb/stb_image.o:
	mkdir -p $(dir $@)
	$(CC) ${COMPILE_FLAGS} -I third_party/stb -c src/third_party/StbImage.cpp -o $@

${BUILD_DIR}/app ${BUILD_DIR}/engine/src/gl ${INSTALL_DIR}:
	mkdir -p $@