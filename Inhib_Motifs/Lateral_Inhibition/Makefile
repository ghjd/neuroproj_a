# Location of the CUDA Toolkit
CUDA_PATH ?= /usr/local/cuda


# architecture
HOST_ARCH   := $(shell uname -m)
TARGET_ARCH ?= $(HOST_ARCH)
TARGET_SIZE := 64


# operating system
HOST_OS   := $(shell uname -s 2>/dev/null | tr "[:upper:]" "[:lower:]")
TARGET_OS ?= $(HOST_OS)

HOST_COMPILER ?= g++
NVCC          := $(CUDA_PATH)/bin/nvcc -ccbin $(HOST_COMPILER)

# internal flags
NVCCFLAGS   := -m${TARGET_SIZE}
CCFLAGS     := 
LDFLAGS     := -lglut -lGL -lGLU -lGLEW -lcurand


# Debug build flags
ifeq ($(dbg),1)
      NVCCFLAGS += -g -G
      BUILD_TYPE := debug
else
      BUILD_TYPE := release
endif

ALL_CCFLAGS :=
#lineinfo for nsight compute
ALL_CCFLAGS += -lineinfo
ALL_CCFLAGS += $(NVCCFLAGS)
ALL_CCFLAGS += $(EXTRA_NVCCFLAGS)
ALL_CCFLAGS += $(addprefix -Xcompiler ,$(CCFLAGS))
ALL_CCFLAGS += $(addprefix -Xcompiler ,$(EXTRA_CCFLAGS))



ALL_LDFLAGS :=
ALL_LDFLAGS += $(LDFLAGS)
ALL_LDFLAGS += $(ALL_CCFLAGS)
ALL_LDFLAGS += $(addprefix -Xlinker ,$(LDFLAGS))
ALL_LDFLAGS += $(addprefix -Xlinker ,$(EXTRA_LDFLAGS))

# Common includes and paths for CUDA
INCLUDES  := 
LIBRARIES :=

################################################################################

# Gencode arguments
SMS ?= 75 80 86 87 90 120

ifeq ($(GENCODE_FLAGS),)
# Generate SASS code for each SM architecture listed in $(SMS)
$(foreach sm,$(SMS),$(eval GENCODE_FLAGS += -gencode arch=compute_$(sm),code=sm_$(sm)))

# Generate PTX code from the highest SM architecture in $(SMS) to guarantee forward-compatibility
HIGHEST_SM := $(lastword $(sort $(SMS)))
ifneq ($(HIGHEST_SM),)
GENCODE_FLAGS += -gencode arch=compute_$(HIGHEST_SM),code=compute_$(HIGHEST_SM)
endif
endif

ALL_CCFLAGS += --threads 0 --std=c++11

################################################################################

# Target rules
all: build

build: main.exe

main.o:main.cpp activity.inc misc.inc declarations.h ../../src/kernels.h ../../src/cpp_common.h
	$(EXEC) $(NVCC) $(INCLUDES) $(ALL_CCFLAGS) $(GENCODE_FLAGS) -o $@ -c $<

cpp_common.o:../../src/cpp_common.cpp ../../src/cpp_common.h
	$(EXEC) $(NVCC) $(INCLUDES) $(ALL_CCFLAGS) $(GENCODE_FLAGS) -o $@ -c $<

kernels.o:../../src/kernels.cu ../../src/kernels.h ../../src/kernel_common.h ../../src/synapseKernel_a.inc ../../src/somaKernel_aelif.inc ../../src/synapseKernel_a_s2b.inc
	$(EXEC) $(NVCC) $(INCLUDES) $(ALL_CCFLAGS) $(GENCODE_FLAGS) -o $@ -c $<

main.exe: kernels.o main.o cpp_common.o
	$(EXEC) $(NVCC) $(ALL_LDFLAGS) $(GENCODE_FLAGS) $(INCLUDES) -o $@ $+ $(LIBRARIES)

run: build
	$(EXEC) ./main.exe

clean:
	rm -f  *.o *.exe

clobber: clean
