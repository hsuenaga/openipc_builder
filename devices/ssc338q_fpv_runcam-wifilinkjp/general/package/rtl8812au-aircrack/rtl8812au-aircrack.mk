################################################################################
#
# rtl8812au-openipc
#
################################################################################

RTL8812AU_AIRCRACK_SITE = $(call github,aircrack-ng,rtl8812au,$(RTL8812AU_AIRCRACK_VERSION))
RTL8812AU_AIRCRACK_VERSION = b44d288f423ede0fc7cdbf92d07a7772cd727de4

RTL8812AU_AIRCRACK_LICENSE = GPL-2.0
RTL8812AU_AIRCRACK_LICENSE_FILES = COPYING

RTL8812AU_AIRCRACK_MODULE_MAKE_OPTS = CONFIG_RTL8812AU=m \
	KVER=$(LINUX_VERSION_PROBED) \
	KSRC=$(LINUX_DIR)

$(eval $(kernel-module))
$(eval $(generic-package))
