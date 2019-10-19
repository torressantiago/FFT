################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccs910/ccs/tools/compiler/ti-cgt-c6000_8.3.3

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./AM335X_device.obj" \
"./AM335X_ecc.obj" \
"./aic31.obj" \
"./cache.obj" \
"./codecif.obj" \
"./debug.obj" \
"./edma.obj" \
"./edmaV2.obj" \
"./gpio.obj" \
"./gpiov2.obj" \
"./i2c.obj" \
"./i2cv2.obj" \
"./interrupt.obj" \
"./intvecs.obj" \
"./main.obj" \
"./mcasp.obj" \
"./mcasp1.obj" \
"./psc.obj" \
"./uart.obj" \
"./uartv2.obj" \
"./util.obj" \
"../C6748.cmd" \
$(GEN_CMDS__FLAG) \
-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
Proyecto2_Transformada_de_Fourier.out \

EXE_OUTPUTS__QUOTED += \
"Proyecto2_Transformada_de_Fourier.out" \

BIN_OUTPUTS += \
Proyecto2_Transformada_de_Fourier.hex \

BIN_OUTPUTS__QUOTED += \
"Proyecto2_Transformada_de_Fourier.hex" \


# All Target
all: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "Proyecto2_Transformada_de_Fourier.out"

# Tool invocations
Proyecto2_Transformada_de_Fourier.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: C6000 Linker'
	"C:/ti/ccs910/ccs/tools/compiler/ti-cgt-c6000_8.3.3/bin/cl6x" -mv6740 --define=c6748 -g --diag_warning=225 --diag_wrap=off --display_error_number -z -m"Proyecto2_Transformada_de_Fourier.map" --heap_size=0x800 --stack_size=0x800 -i"C:/ti/ccs910/ccs/tools/compiler/ti-cgt-c6000_8.3.3/lib" -i"C:/ti/ccs910/ccs/tools/compiler/ti-cgt-c6000_8.3.3/include" --reread_libs --diag_wrap=off --display_error_number --warn_sections --xml_link_info="Proyecto2_Transformada_de_Fourier_linkInfo.xml" --rom_model -o "Proyecto2_Transformada_de_Fourier.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

Proyecto2_Transformada_de_Fourier.hex: $(EXE_OUTPUTS)
	@echo 'Building files: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo 'Invoking: C6000 Hex Utility'
	"C:/ti/ccs910/ccs/tools/compiler/ti-cgt-c6000_8.3.3/bin/hex6x"  -o "Proyecto2_Transformada_de_Fourier.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(BIN_OUTPUTS__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "AM335X_device.obj" "AM335X_ecc.obj" "aic31.obj" "cache.obj" "codecif.obj" "debug.obj" "edma.obj" "edmaV2.obj" "gpio.obj" "gpiov2.obj" "i2c.obj" "i2cv2.obj" "interrupt.obj" "intvecs.obj" "main.obj" "mcasp.obj" "mcasp1.obj" "psc.obj" "uart.obj" "uartv2.obj" "util.obj" 
	-$(RM) "AM335X_device.d" "AM335X_ecc.d" "aic31.d" "cache.d" "codecif.d" "debug.d" "edma.d" "edmaV2.d" "gpio.d" "gpiov2.d" "i2c.d" "i2cv2.d" "interrupt.d" "main.d" "mcasp.d" "mcasp1.d" "psc.d" "uart.d" "uartv2.d" "util.d" 
	-$(RM) "intvecs.d" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

