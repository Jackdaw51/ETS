################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccs1280/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./main.obj" \
"./startup_msp432p401r_ccs.obj" \
"./system_msp432p401r.obj" \
"./games/game_files/dino_runner.obj" \
"./games/game_files/example.obj" \
"./games/game_files/pong_wall.obj" \
"./games/game_files/rglite.obj" \
"./games/game_files/snake.obj" \
"./games/game_files/space_invaders.obj" \
"./games/game_files/test_main.obj" \
"./games/game_files/display/fb_display.obj" \
"./games/game_files/display/pixel_mask_lut.obj" \
"./games/game_files/display/text_lut.obj" \
"./games/game_files/sprites/palettes.obj" \
"./games/game_files/sprites/sprites.obj" \
"./incl/esp32.obj" \
"./incl/hcsr04.obj" \
"./incl/joystick.obj" \
"./incl/timers.obj" \
"../msp432p401r.cmd" \
$(GEN_CMDS__FLAG) \
-llibc.a \
-l"C:/ti/simplelink_msp432p4_sdk_3_40_01_02/source/ti/devices/msp432p4xx/driverlib/ccs/msp432p4xx_driverlib.lib" \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include games/game_files/subdir_vars.mk
-include games/game_files/display/subdir_vars.mk
-include games/game_files/sprites/subdir_vars.mk
-include incl/subdir_vars.mk
-include subdir_rules.mk
-include games/game_files/subdir_rules.mk
-include games/game_files/display/subdir_rules.mk
-include games/game_files/sprites/subdir_rules.mk
-include incl/subdir_rules.mk
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
ETS.out 

EXE_OUTPUTS__QUOTED += \
"ETS.out" 


# All Target
all: $(OBJS) $(CMD_SRCS) $(A_SRCS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "ETS.out"

# Tool invocations
ETS.out: $(OBJS) $(CMD_SRCS) $(A_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: Arm Linker'
	"C:/ti/ccs1280/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -Ooff --opt_for_speed=3 --advice:power=all --define=__MSP432P401R__ --define=ccs -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi -z -m"ETS.map" --heap_size=1024 --stack_size=512 -i"C:/ti/ccs1280/ccs/ccs_base/arm/include" -i"C:/ti/ccs1280/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/lib" -i"C:/ti/ccs1280/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --reread_libs --diag_wrap=off --display_error_number --warn_sections --xml_link_info="ETS_linkInfo.xml" --rom_model -o "ETS.out" $(ORDERED_OBJS) $(A_SRCS)
	@echo 'Finished building target: "$@"'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXE_OUTPUTS__QUOTED)
	-$(RM) "main.obj" "startup_msp432p401r_ccs.obj" "system_msp432p401r.obj" "games\game_files\dino_runner.obj" "games\game_files\example.obj" "games\game_files\pong_wall.obj" "games\game_files\rglite.obj" "games\game_files\snake.obj" "games\game_files\space_invaders.obj" "games\game_files\test_main.obj" "games\game_files\display\fb_display.obj" "games\game_files\display\pixel_mask_lut.obj" "games\game_files\display\text_lut.obj" "games\game_files\sprites\palettes.obj" "games\game_files\sprites\sprites.obj" "incl\esp32.obj" "incl\hcsr04.obj" "incl\joystick.obj" "incl\timers.obj" 
	-$(RM) "main.d" "startup_msp432p401r_ccs.d" "system_msp432p401r.d" "games\game_files\dino_runner.d" "games\game_files\example.d" "games\game_files\pong_wall.d" "games\game_files\rglite.d" "games\game_files\snake.d" "games\game_files\space_invaders.d" "games\game_files\test_main.d" "games\game_files\display\fb_display.d" "games\game_files\display\pixel_mask_lut.d" "games\game_files\display\text_lut.d" "games\game_files\sprites\palettes.d" "games\game_files\sprites\sprites.d" "incl\esp32.d" "incl\hcsr04.d" "incl\joystick.d" "incl\timers.d" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

