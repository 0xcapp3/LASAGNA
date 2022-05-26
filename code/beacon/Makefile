# name of your application
APPLICATION = lasagna_beacon

# If no BOARD is found in the environment, use this default:
BOARD ?= b-l475e-iot01a

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../RIOT

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Include the SPBTLE-RF library
USEMODULE += spbtlerf
USEMODULE += SimpleBlueNRG_HCI
EXTERNAL_MODULE_DIRS += $(CURDIR)/../SPBTLE-RF-RIOT
EXTERNAL_MODULE_DIRS += $(CURDIR)/../SPBTLE-RF-RIOT/spbtlerf

USEMODULE += ztimer_sec
USEMODULE += fmt

include $(RIOTBASE)/Makefile.include
