# name of your application
APPLICATION = lasagna_receiver

# If no BOARD is found in the environment, use this default:
BOARD ?= b-l072z-lrwan1

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../RIOT

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# shell
USEMODULE += shell
USEMODULE += shell_commands

USEMODULE += printf_float

# GPS
USEPKG += minmea

USEMODULE += xtimer
USEMODULE += fmt

# LoRa
# Default region is Europe and default band is 868MHz
LORA_REGION ?= EU868
USEMODULE += semtech_loramac_rx

# Include the Semtech-loramac package
USEPKG += semtech-loramac

# Default radio driver is Semtech SX1276 (used by the B-L072Z-LRWAN1 board)
LORA_DRIVER ?= sx1276
USEMODULE += $(LORA_DRIVER) 	# Variable for Source code (main.c)

# Include the SPBTLE-RF library
USEMODULE += spbtlerf
USEMODULE += SimpleBlueNRG_HCI
EXTERNAL_MODULE_DIRS += $(CURDIR)/../SPBTLE-RF-RIOT
EXTERNAL_MODULE_DIRS += $(CURDIR)/../SPBTLE-RF-RIOT/spbtlerf

CFLAGS += -Wno-missing-field-initializers

include $(RIOTBASE)/Makefile.include
