DEBUG?=1
RUN_AFTER_BUILD?=1
USE_HOT_RELOADING?=1
USE_DEP_FILES?=1
USE_PCH?=1
USE_CCACHE?=1
USE_CLANGD?=1
# USE_VERBOSE_LOG?=1
# USE_SANITIZER?=1
# USE_DYNLIB_ENGINE?=1
# USE_COMPILER_DUMP?=1

.DEFAULT_GOAL := all

ifeq ($(OS),Windows_NT)
    DETECTED_OS := windows
	PLATFORM_FOLDER=win
	EXE := .exe
	COMPILE_FLAGS += -DXPLATFORM_WINDOWS
else
    DETECTED_OS := $(shell uname | tr "[:upper:]" "[:lower:]")
	PLATFORM_FOLDER=posix
	EXE :=
	COMPILE_FLAGS += -DXPLATFORM_LINUX
	INCLUDE_DIR += -I/usr/include
endif

BUILD_DIR=build/$(if ${DEBUG},debug,release)
INSTALL_DIR+=${BUILD_DIR}/install
MAKEFILE_DIR=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
APP_MAIN_EXE=${BUILD_DIR}/app/run_app${EXE}
APP_HOTRELOAD_EXE=${BUILD_DIR}/app/run_app_hotreload${EXE}
APP_EXE=$(if ${USE_HOT_RELOADING},${APP_HOTRELOAD_EXE},${APP_MAIN_EXE})
APP_LIB=${BUILD_DIR}/app/libapp.so
ENGINE_LIB=$(if ${USE_DYNLIB_ENGINE},${BUILD_DIR}/engine/libengine.so,${BUILD_DIR}/engine/libengine.a)
PRECOMPILED_HEADER=${BUILD_DIR}/engine/Precompiled.hpp.pch

THIRD_PARTY_DEPS=\
	${BUILD_DIR}/third_party/imgui/libimgui.a \
	${BUILD_DIR}/third_party/spdlog/libspdlog.a \
	${BUILD_DIR}/third_party/glfw/src/libglfw.so \
	${BUILD_DIR}/third_party/glad/gl.o \
	${BUILD_DIR}/third_party/glm/glm/libglm.a \
	${BUILD_DIR}/third_party/stb/stb_image.o \

APP_THIRD_PARTY_DEPS=$(if ${USE_DYNLIB_ENGINE},,${THIRD_PARTY_DEPS})
ENGINE_THIRD_PARTY_DEPS=$(if ${USE_DYNLIB_ENGINE},${THIRD_PARTY_DEPS},)

CXX=$(if ${USE_CCACHE},./ccache,) clang++

# NOTE: -MMD generates .d files alongside .o files (targets with all dependent headers)
COMPILE_FLAGS += -std=c++20 \
	$(if ${USE_DYNLIB_ENGINE},-fPIC,) \
	$(if ${USE_HOT_RELOADING},-fPIC,) \
	$(if $(USE_DEP_FILES),-MMD,) \
	$(if $(USE_COMPILER_DUMP),-save-stats,) \
	$(if ${DEBUG},-g -DXDEBUG,) \
	$(if ${USE_SANITIZER},-fno-omit-frame-pointer -fsanitize=address,) \
	$(if ${USE_VERBOSE_LOG},-DXVERBOSE,) \
	-fvisibility=hidden -fvisibility-inlines-hidden \
	-fno-exceptions -fno-rtti \
	-Wno-switch-enum \
	-Wno-c++98-compat-pedantic \
	-Wno-c++98-compat \
	-Wno-c++98-c++11-compat-pedantic \
	-Wno-c99-designator \
	-Wno-padded \
	-Wno-unknown-attributes \
#-Weverything \
#-rdynamic

INCLUDE_DIR+=-I./src/engine/include
# root of 3rd party, because for imgui, cr and concurrentqueue have headers in their root
INCLUDE_DIR+=-I./third_party/imgui
INCLUDE_DIR+=-I./third_party/imgui/backends
INCLUDE_DIR+=-I./third_party/cr
INCLUDE_DIR+=-I./third_party/concurrentqueue
INCLUDE_DIR+=-I./third_party/spdlog/include
INCLUDE_DIR+=-I./third_party/glad/include
INCLUDE_DIR+=-I./third_party/glm/
INCLUDE_DIR+=-I./third_party/stb/
INCLUDE_DIR+=-I./data
LDFLAGS+=-pthread -ldl -lGL
CLANG_FORMAT=clang-format-17

src_app_ = App.cpp
outpaths_app = $(addprefix ${BUILD_DIR}/app/, ${src_app_})
outdirs_app = $(sort $(dir ${outpaths_app}) ${BUILD_DIR}/app)
obj_app = ${outpaths_app:.cpp=.o}

src_engine_ = \
	Assets.cpp BoxMesh.cpp \
	EngineLoop.cpp IcosphereMesh.cpp \
	LineRendererInput.cpp Log.cpp PointRendererInput.cpp \
	PlaneMesh.cpp Unprojection.cpp \
	UvSphereMesh.cpp \
	Precompiled.cpp WindowContext.cpp \
	platform/GpuConfiguration.cpp \
	platform/Filesystem.cpp \
	platform/${PLATFORM_FOLDER}/FileChangeNotifier.cpp \
	gl/AxesRenderer.cpp \
	gl/BoxRenderer.cpp gl/ProceduralMeshes.cpp \
	gl/BillboardRenderer.cpp \
	gl/GpuBuffer.cpp gl/GlCapabilities.cpp \
	gl/Context.cpp \
	gl/Common.cpp gl/CommonRenderers.cpp \
	gl/PointRenderer.cpp \
	gl/Debug.cpp gl/EditorGridRenderer.cpp \
	gl/FlatRenderer.cpp \
	gl/FrustumRenderer.cpp gl/Guard.cpp \
	gl/LineRenderer.cpp \
	gl/GlExtensions.cpp gl/Framebuffer.cpp \
	gl/GpuProgram.cpp gl/GpuProgramRegistry.cpp \
	gl/Renderbuffer.cpp \
	gl/GlRenderStateRegistry.cpp \
	gl/GpuSampler.cpp gl/SamplersCache.cpp \
	gl/Shader.cpp gl/Texture.cpp \
	gl/TextureUnits.cpp gl/Uniform.cpp \
	gl/Vao.cpp

outpaths_engine=$(addprefix ${BUILD_DIR}/engine/src/, ${src_engine_})
outdirs_engine=$(sort $(dir ${outpaths_engine}) ${BUILD_DIR}/engine/src)
obj_engine=${outpaths_engine:.cpp=.o}

ifeq (1,${USE_DEP_FILES})
-include $(obj_engine:.o=.d)
-include $(obj_app:.o=.d)
-include ${PRECOMPILED_HEADER:.pch=.d}
endif

ifneq ($(f),) # force rebulid
.PHONY: ${APP_LIB}
endif

.PHONY: all
all: build_engine build_app ${INSTALL_DIR}/app $(if ${RUN_AFTER_BUILD},run,)

# Auto-install prerequisites
.PHONY: init_repo
init_repo:
	sudo apt update
	sudo apt install clang cmake -y
	sudo apt install libwayland-dev libxkbcommon-dev xorg-dev -y
	git submodule update --init --remote
	git submodule status

# NOTE: `bear` is a CLI tool, that hooks every clang call
# and generates a `compile_commands.json` file designated for clangd
# `bear` must be installed by you (e.g. apt-get install bear)
.PHONY: intellisense
intellisense: rm
	-rm compile_commands.json
	$(if ${USE_CLANGD},bear -- make -j16 DEBUG=${DEBUG})

ccache:
	wget https://github.com/ccache/ccache/releases/download/v4.8.3/ccache-4.8.3-linux-x86_64.tar.xz \
		-O ccache.tar.xz && mkdir -p ccache_prebuilt \
		&& tar -xJf ccache.tar.xz --directory ccache_prebuilt \
		&& cp ccache_prebuilt/*/ccache . \
		&& rm -rf ccache_prebuilt ccache.tar.xz

# hot reload
.PHONY: hot
hot: ${APP_LIB} ${outdirs_app}
	-cp ${ENGINE_LIB} ${INSTALL_DIR}
	-rm ${INSTALL_DIR}/$(notdir ${APP_LIB})
	-cp ${APP_LIB} ${INSTALL_DIR}

.PHONY: wtf
wtf:
	$(info > ${outpaths_engine})
	$(info > ${APP_LIB})

# NOTE: for hot-reloaded applicatoin, first argument is the full path to the dynamic library
.PHONY: run
run: ${INSTALL_DIR}/app
	@echo "====== RUN ======"
	cd ${INSTALL_DIR} && LD_LIBRARY_PATH=. ./app $(shell realpath ${INSTALL_DIR}/libapp.so)
#$(if ${USE_HOT_RELOADING} ${MAKEFILE_DIR}/${APP_LIB},)

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

.PHONY: build_tools
build_tools: $(if ${USE_CCACHE},ccache,)

-include build_tools # run build_tools regardless of the current target

.PHONY: build_app
build_app: ${outdirs_app} ${ENGINE_LIB} ${APP_EXE}

.PHONY: build_engine
build_engine: ${outdirs_engine} ${ENGINE_LIB}

.PHONY: ${INSTALL_DIR}/app
${INSTALL_DIR}/app: ${INSTALL_DIR} ${APP_EXE}
	$(info > Installing ${INSTALL_DIR}/app)
# NOTE: in debug, in build/install only symlinks are stored, to apply shader editing into original files
# in release, in build/install reside copies, thus shader editing is not saved in original files
	cp -af $(if DEBUG,-s,) $(realpath data) ${INSTALL_DIR}
	-cp ${ENGINE_LIB} ${INSTALL_DIR}
	-cp ${APP_LIB} ${INSTALL_DIR}
	cp ${APP_EXE} $@

# linking app
${APP_LIB}: ${obj_app} ${obj_engine} ${APP_THIRD_PARTY_DEPS}
	$(info > Linking dynamic $@)
	${CXX} -shared $^ -o $@ -Wl,-soname,$(notdir $@)

${APP_MAIN_EXE}: ${BUILD_DIR}/app/Main.o ${obj_app} ${ENGINE_LIB} ${APP_THIRD_PARTY_DEPS}
	$(info > Linking executable $@)
	${CXX} ${COMPILE_FLAGS} $^ ${LDFLAGS} -o $@

${APP_HOTRELOAD_EXE}: ${BUILD_DIR}/app/MainHotreload.o ${APP_LIB} ${ENGINE_LIB} ${APP_THIRD_PARTY_DEPS}
	$(info > Linking executable $@)
	${CXX} ${COMPILE_FLAGS} $^ ${LDFLAGS} -o $@

# linking engine library
${BUILD_DIR}/engine/libengine.a: ${obj_engine} ${ENGINE_THIRD_PARTY_DEPS}
	$(info > Linking static $@)
	@ar r $@ ${obj_engine}

${BUILD_DIR}/engine/libengine.so: ${obj_engine} ${ENGINE_THIRD_PARTY_DEPS}
	$(info > Linking dynamic $@)
	${CXX} -shared $^ -o $@ -Wl,-soname,$(notdir $@)

# compiling main executable sources
${BUILD_DIR}/app/%.o: src/app/%.cpp
	$(info > Compiling $@)
	@$(CXX) ${COMPILE_FLAGS} ${INCLUDE_DIR} -I src/app/include -c $< -o $@

# compiling engine sources
${BUILD_DIR}/engine/%.o: src/engine/%.cpp $(if $(USE_PCH),${PRECOMPILED_HEADER},)
	$(info > Compiling $@)
	@$(CXX) ${COMPILE_FLAGS} ${INCLUDE_DIR} -I src/engine/include_private $(if $(USE_PCH),-include-pch ${PRECOMPILED_HEADER},) -c $< -o $@

${PRECOMPILED_HEADER}: src/engine/include/engine/Precompiled.hpp
	$(info > Precompiled header $@)
	$(CXX) ${COMPILE_FLAGS} ${INCLUDE_DIR} -c -o $@ -xc++-header $<

# compiling third party

imgui_dir=third_party/imgui
imgui_obj_=imgui.o imgui_demo.o imgui_draw.o imgui_tables.o \
	imgui_widgets.o backends/imgui_impl_glfw.o backends/imgui_impl_opengl3.o
imgui_obj = $(addprefix ${BUILD_DIR}/${imgui_dir}/, ${imgui_obj_})

${BUILD_DIR}/third_party/imgui/%.o: third_party/imgui/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(if ${USE_HOT_RELOADING},-fPIC,) -I $(imgui_dir) -I $(imgui_dir)/backends -c $< -o $@

${BUILD_DIR}/third_party/imgui/libimgui.a: $(imgui_obj)
	ar r $@ ${imgui_obj}

${BUILD_DIR}/third_party/spdlog/libspdlog.a:
	cmake $(if ${USE_HOT_RELOADING},-D CMAKE_CXX_FLAGS="-fPIC",) -S third_party/spdlog -B $(dir $@) && cmake --build $(dir $@)

${BUILD_DIR}/third_party/glfw/src/libglfw.so:
	cmake $(if ${USE_HOT_RELOADING},-D CMAKE_CXX_FLAGS="-fPIC",) -DBUILD_SHARED_LIBS=ON -S third_party/glfw -B ${BUILD_DIR}/third_party/glfw && cmake --build $(dir $@)

${BUILD_DIR}/third_party/glfw/src/libglfw3.a:
	cmake -S third_party/glfw -B ${BUILD_DIR}/third_party/glfw && cmake --build $(dir $@)

${BUILD_DIR}/third_party/glad/gl.o:
	mkdir -p $(dir $@)
	$(CXX) ${COMPILE_FLAGS} -DGLAD_API_CALL_EXPORT=1 -DGLAD_API_CALL_EXPORT_BUILD=1 -I third_party/glad/include -c third_party/glad/src/gl.c -o $@

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
	$(CXX) ${COMPILE_FLAGS} -I third_party/stb -c src/third_party/StbImage.cpp -o $@

${outdirs_engine} ${outdirs_app} ${INSTALL_DIR}:
	mkdir -p $@