BUILD_TYPE=debug

BUILD_DIR=build
CC=ccache clang++
COMPILE_FLAGS=-std=c++20 $(if $(findstring debug,${BUILD_TYPE}),-g -DXDEBUG,)
INCLUDE_DIR+=-Iinclude
INCLUDE_DIR+=-Ithird_party/spdlog/include
LDFLAGS=${BUILD_DIR}/third_party/spdlog/libspdlog.a -pthread

.PHONY: all
all: ${BUILD_DIR}/third_party/spdlog build_engine build_app run

obj_app = Main.o
obj_engine = GlHelpers.o

$(obj_app): %.o: src/app/%.cpp
	$(info "Compiling $^")
	@mkdir -p ${BUILD_DIR}/app
	$(CC) \
		${COMPILE_FLAGS} \
		${INCLUDE_DIR} \
		-c $^ \
		-o $(addprefix ${BUILD_DIR}/app/,$@)

$(obj_engine): %.o: src/engine/%.cpp
	$(info "Compiling $^")
	@mkdir -p ${BUILD_DIR}/engine
	@$(CC) \
		${COMPILE_FLAGS} -fPIC \
		${INCLUDE_DIR} \
		-c $^ \
		-o $(addprefix ${BUILD_DIR}/engine/,$@)

.PHONY: build_app
build_app: $(obj_app)
	$(info "Finishing $@")
	${CC} \
		$(addprefix ${BUILD_DIR}/app/,$^) \
		${LDFLAGS} \
		${BUILD_DIR}/engine/libengine.a \
		-o ${BUILD_DIR}/run_app
	@echo "BUILD APP OK"
	@echo "========"

.PHONY: build_app
build_engine: $(obj_engine)
	$(info "Finishing $@")
	ar r ${BUILD_DIR}/engine/libengine.a $(addprefix ${BUILD_DIR}/engine/,$^)
	@echo "BUILD ENGINE OK"
	@echo "========"

${BUILD_DIR}/third_party/spdlog:
	mkdir -p $@
	cd $@ && cmake ../../../third_party/spdlog && make -j

.PHONY: run
run:
	@echo "RUNNING"
	@echo "========"
	@${BUILD_DIR}/run_app

.PHONY: init_repo
init_repo:
	git submodule update --init