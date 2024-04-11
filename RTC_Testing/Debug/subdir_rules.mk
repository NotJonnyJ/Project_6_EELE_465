################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"/Applications/ti/ccstheia131/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/bin/cl430" -vmspx --use_hw_mpy=F5 --include_path="/Applications/ti/ccstheia131/ccs/ccs_base/msp430/include" --include_path="/Users/jon/Documents/EELE465/Project_6_EELE_465/RTC_Testing" --include_path="/Users/jon/Documents/EELE465/Project_6_EELE_465/RTC_Testing/driverlib/MSP430FR2xx_4xx" --include_path="/Applications/ti/ccstheia131/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/include" --advice:power=all --advice:hw_config=all --define=__MSP430FR2355__ --define=_FRWP_ENABLE --define=_INFO_FRWP_ENABLE -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


