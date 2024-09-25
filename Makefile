DEBUG?=1
USE_HOT_RELOADING?=1
USE_DEP_FILES?=1
USE_PCH?=1
# USE_DYNLIB_ENGINE?=1
# COMPILER_DUMP?=1

.DEFAULT_GOAL := all

.PHONY: init_repo
init_repo:
	sudo apt install libwayland-dev libxkbcommon-dev xorg-dev
	wget https://github.com/ccache/ccache/releases/download/v4.8.3/ccache-4.8.3-linux-x86_64.tar.xz \
		-O ccache.tar.xz && mkdir -p ccache_prebuilt \
		&& tar -xJf ccache.tar.xz --directory ccache_prebuilt \
		&& cp ccache_prebuilt/*/ccache . \
		&& rm -rf ccache_prebuilt ccache.tar.xz
	git submodule update --init --remote
	git submodule status

.PHONY: init_dev
init_dev:
	$(MAKE) rm
	bear -a make -j16 DEBUG=y


ifeq ($(OS),Windows_NT)
    DETECTED_OS := windows
else
    DETECTED_OS := $(shell uname | tr "[:upper:]" "[:lower:]")
endif

BUILD_DIR=build/$(if ${DEBUG},debug,release)
INSTALL_DIR=${BUILD_DIR}/install
MAKEFILE_DIR=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
APP_MAIN_EXE=${BUILD_DIR}/app/run_app
APP_HOTRELOAD_EXE=${BUILD_DIR}/app/run_app_hotreload
APP_EXE=$(if ${USE_HOT_RELOADING},${APP_HOTRELOAD_EXE},${APP_MAIN_EXE})
APP_LIB=${BUILD_DIR}/app/libapp.so
ENGINE_LIB=$(if ${USE_DYNLIB_ENGINE},${BUILD_DIR}/engine/libengine.so,${BUILD_DIR}/engine/libengine.a)
PRECOMPILED_HEADER=${BUILD_DIR}/engine/Precompiled.hpp.pch

THIRD_PARTY_DEPS=\
	$(if ${DEBUG},${BUILD_DIR}/third_party/spdlog/libspdlog.a,) \
	${BUILD_DIR}/third_party/glfw/src/libglfw.so \
	${BUILD_DIR}/third_party/glad/gl.o \
	${BUILD_DIR}/third_party/glm/glm/libglm.a \
	${BUILD_DIR}/third_party/stb/stb_image.o

CC=./ccache clang++-17

# NOTE: -MMD generates .d files alongside .o files (targets with all dependent headers)
COMPILE_FLAGS=-std=c++20 \
	$(if ${USE_DYNLIB_ENGINE},-fPIC,) \
	$(if ${USE_HOT_RELOADING},-fPIC,) \
	$(if $(USE_DEP_FILES),-MMD,) \
	$(if $(COMPILER_DUMP),-save-stats,) \
	$(if ${DEBUG},-g -DXDEBUG,) \
	-fvisibility=hidden -fvisibility-inlines-hidden \
	-fno-exceptions -fno-rtti \
	-Wno-switch-enum \
	-Wno-c++98-compat-pedantic \
	-Wno-c++98-compat \
	-Wno-c++98-c++11-compat-pedantic \
	-Wno-c99-designator \
	-Wno-padded \
	-Wno-newline-eof \
#-Weverything \

INCLUDE_DIR+=-I src/engine/include
INCLUDE_DIR+=-I third_party/spdlog/include
INCLUDE_DIR+=-I third_party/glad/include
INCLUDE_DIR+=-I third_party/glm/
INCLUDE_DIR+=-I third_party/stb/
INCLUDE_DIR+=-I third_party/cr/
INCLUDE_DIR+=-I data
LDFLAGS+=-pthread -ldl
CLANG_FORMAT=clang-format-17

src_app_ = App.cpp Main.cpp
outpaths_app = $(addprefix ${BUILD_DIR}/app/, ${src_app_})
outdirs_app = $(sort $(dir ${outpaths_app}) ${BUILD_DIR}/app)
obj_app = ${outpaths_app:.cpp=.o}

src_engine_ = \
	platform/GpuConfiguration.cpp \
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
	gl/Context.cpp \
	gl/Common.cpp gl/CommonRenderers.cpp \
	gl/PointRenderer.cpp \
	gl/Debug.cpp gl/FlatRenderer.cpp \
	gl/FrustumRenderer.cpp gl/Guard.cpp \
	gl/LineRenderer.cpp \
	gl/Extensions.cpp gl/Framebuffer.cpp \
	gl/Program.cpp \
	gl/Renderbuffer.cpp \
	gl/Sampler.cpp gl/SamplersCache.cpp \
	gl/Shader.cpp gl/Texture.cpp \
	gl/TextureUnits.cpp gl/Uniform.cpp \
	gl/Vao.cpp

ifeq (windows,${DETECTED_OS})
	COMPILE_FLAGS += -DXPLATFORM_WINDOWS
else ifeq (linux,${DETECTED_OS})
	COMPILE_FLAGS += -DXPLATFORM_LINUX
	src_engine_ += platform/linux/FileChangeWatcher.cpp
	INCLUDE_DIR+=-I/usr/include
endif

outpaths_engine=$(addprefix ${BUILD_DIR}/engine/src/, ${src_engine_})
outdirs_engine=$(sort $(dir ${outpaths_engine}) ${BUILD_DIR}/engine/src)
obj_engine=${outpaths_engine:.cpp=.o}

ifeq (1,${USE_DEP_FILES})
-include $(src_engine:.o=.d)
-include $(src_app:.o=.d)
-include ${PRECOMPILED_HEADER:.pch=.d}
endif

ifneq ($(f),) # force rebulid
.PHONY: ${APP_LIB}
endif

.PHONY: all
all: build_engine build_app ${INSTALL_DIR}/app

# hot reload
.PHONY: hot
hot: ${APP_LIB} ${outdirs_app}
	-cp ${ENGINE_LIB} ${INSTALL_DIR}
	rm ${INSTALL_DIR}/$(notdir ${APP_LIB})
	-cp ${APP_LIB} ${INSTALL_DIR}

.PHONY: wtf
wtf:
	$(info > ${APP_THIRD_PARTY_DEPS})
	$(info > ${ENGINE_THIRD_PARTY_DEPS})

.PHONY: run
run: ${INSTALL_DIR}/app
	@echo "====== RUN ======"
	@cd ${INSTALL_DIR} && LD_LIBRARY_PATH=. ./app

.PHONY: prettify
prettify:
	find src -regex '.*\.\(cpp\|hpp\|cu\|cuh\|c\|h\)' -exec ${CLANG_FORMAT} --verbose -style=file -i {} \;

.PHONY: rm_all
rm_all:
	rm -r ${BUILD_DIR}

.PHONY: rm
rm:
	-rm -r ${BUILD_DIR}/app ${BUILD_DIR}/engine ${BUILD_DIR}/install
	-rm -f ${APP_MAIN_EXE} ${APP_HOTRELOAD_EXE}

.PHONY: build_app
build_app: ${outdirs_app} ${ENGINE_LIB} ${APP_EXE}

.PHONY: build_engine
build_engine: ${outdirs_engine} ${ENGINE_LIB}

APP_THIRD_PARTY_DEPS=$(if ${USE_DYNLIB_ENGINE},,${THIRD_PARTY_DEPS})
ENGINE_THIRD_PARTY_DEPS=$(if ${USE_DYNLIB_ENGINE},${THIRD_PARTY_DEPS},)

.PHONY: ${INSTALL_DIR}/app
${INSTALL_DIR}/app: ${INSTALL_DIR} ${APP_EXE}
	$(info > Installing ${INSTALL_DIR}/app)
	cp -asf $(realpath data) ${INSTALL_DIR}
	-cp ${ENGINE_LIB} ${INSTALL_DIR}
	-cp ${APP_LIB} ${INSTALL_DIR}
	cp ${APP_EXE} $@

# linking app
${APP_MAIN_EXE}: ${obj_app} ${APP_THIRD_PARTY_DEPS} ${ENGINE_LIB}
	$(info > Linking executable $@)
	${CC} ${COMPILE_FLAGS} $^ ${LDFLAGS} -o $@

${APP_LIB}: ${APP_THIRD_PARTY_DEPS} ${obj_app} ${ENGINE_LIB}
	$(info > Linking dynamic $@)
	${CC} -shared $^ -o $@ -Wl,-soname,$(notdir $@)

${APP_HOTRELOAD_EXE}: ${BUILD_DIR}/app/MainHotreload.o ${APP_THIRD_PARTY_DEPS} ${ENGINE_LIB} ${APP_LIB}
	$(info > Linking executable $@)
	${CC} ${COMPILE_FLAGS} $^ ${LDFLAGS} -o $@

# linking engine library
${BUILD_DIR}/engine/libengine.a: ${obj_engine} ${ENGINE_THIRD_PARTY_DEPS}
	$(info > Linking static $@)
	@ar r $@ ${obj_engine}

${BUILD_DIR}/engine/libengine.so: ${obj_engine} ${ENGINE_THIRD_PARTY_DEPS}
	$(info > Linking dynamic $@)
	${CC} -shared $^ -o $@ -Wl,-soname,$(notdir $@)

# compiling main executable sources
${BUILD_DIR}/app/%.o: src/app/%.cpp
	$(info > Compiling $@)
	$(CC) ${COMPILE_FLAGS} ${INCLUDE_DIR} -I src/app/include -c $< -o $@

# compiling engine sources
${BUILD_DIR}/engine/%.o: src/engine/%.cpp $(if $(USE_PCH),${PRECOMPILED_HEADER},) ${THIRD_PARTY_DEPS}
	$(info > Compiling $@)
	@$(CC) ${COMPILE_FLAGS} ${INCLUDE_DIR} -I src/engine/include_private $(if $(USE_PCH),-include-pch ${PRECOMPILED_HEADER},) -c $< -o $@

${PRECOMPILED_HEADER}: src/engine/include/engine/Precompiled.hpp
	$(info > Precompiled header $@)
	$(CC) ${COMPILE_FLAGS} ${INCLUDE_DIR} -c -o $@ -xc++-header $<

# compiling third party
${BUILD_DIR}/third_party/spdlog/libspdlog.a:
	cmake $(if ${USE_HOT_RELOADING},-D CMAKE_CXX_FLAGS="-fPIC",) -S third_party/spdlog -B $(dir $@) && cmake --build $(dir $@)

${BUILD_DIR}/third_party/glfw/src/libglfw.so:
	cmake $(if ${USE_HOT_RELOADING},-D CMAKE_CXX_FLAGS="-fPIC",) -DBUILD_SHARED_LIBS=ON -S third_party/glfw -B ${BUILD_DIR}/third_party/glfw && cmake --build $(dir $@)

${BUILD_DIR}/third_party/glfw/src/libglfw3.a:
	cmake -S third_party/glfw -B ${BUILD_DIR}/third_party/glfw && cmake --build $(dir $@)

${BUILD_DIR}/third_party/glad/gl.o:
	mkdir -p $(dir $@)
	$(CC) ${COMPILE_FLAGS} -DGLAD_API_CALL_EXPORT=1 -DGLAD_API_CALL_EXPORT_BUILD=1 -I third_party/glad/include -c third_party/glad/src/gl.c -o $@

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

${outdirs_engine} ${outdirs_app} ${INSTALL_DIR}:
	mkdir -p $@