################################################################################
#
# backport-mac80211
#
################################################################################

BACKPORT_MAC80211_VERSION = 6.1.97-1
BACKPORT_MAC80211_SOURCE = backports-$(BACKPORT_MAC80211_VERSION).tar.xz
BACKPORT_MAC80211_SITE = https://cdn.kernel.org/pub/linux/kernel/projects/backports/stable/v6.1.97

BACKPORT_MAC80211_LICENSE = GPL-2.0
BACKPORT_MAC80211_LICENSE_FILES = COPYING

BACKPORT_MAC80211_KCONFIG_FILE = openipc.kconfig

BACKPORT_MAC80211_MODULE_MAKE_OPTS = \
	KVER=$(LINUX_VERSION_PROBED) \
	KSRC=$(LINUX_DIR)

$(eval $(kconfig-package))
$(eval $(kernel-module))
$(eval $(generic-package))
