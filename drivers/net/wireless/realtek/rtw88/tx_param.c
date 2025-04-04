// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause

#include "main.h"
#include "tx.h"
#include "tx_param.h"

struct tx_rate_ctl {
	bool present;

	u8 bw;
	u8 rate_id;
	u8 rate;
	u8 nss;
	bool short_gi;
	bool stbc;
	bool ldpc;

	u8 modulation;
};

/* Extracted from Radiotap Header */
static struct tx_rate_ctl skb_tx_rate_ctl = {
	.present = false,
	.bw = 0,
	.rate_id = 0,
	.rate = 0,
	.nss = 0,
	.short_gi = false,
	.stbc = false,
	.ldpc = false,
	.modulation = 0
};

/* Extracted from module parameters */
static struct tx_rate_ctl skb_tx_rate_ctl_bcast = {
	.present = false,
	.bw = RTW_CHANNEL_WIDTH_20,
	.rate_id = RTW_RATEID_BGN_20M_1SS,
	.rate = DESC_RATEMCS1,
	.nss = 0,
	.short_gi = false,
	.stbc = false,
	.ldpc = false,
	.modulation = RTW_RATE_SECTION_HT_1S
};

static bool bcast_rate_ctl = false;

static bool bcast_rate_update(void)
{
	switch (skb_tx_rate_ctl_bcast.modulation) {
		case RTW_RATE_SECTION_CCK:
			skb_tx_rate_ctl_bcast.rate_id = RTW_RATEID_B_20M;
			if (skb_tx_rate_ctl_bcast.rate > DESC_RATE11M)
				return false;
			break;
		case RTW_RATE_SECTION_OFDM:
			skb_tx_rate_ctl_bcast.rate_id = RTW_RATEID_BG;
			if (skb_tx_rate_ctl_bcast.rate > DESC_RATE54M)
				return false;
			break;
		case RTW_RATE_SECTION_HT_1S:
		case RTW_RATE_SECTION_HT_2S:
		case RTW_RATE_SECTION_HT_3S:
		case RTW_RATE_SECTION_HT_4S:
			if (skb_tx_rate_ctl_bcast.rate <= DESC_RATEMCS7)
				if (skb_tx_rate_ctl_bcast.bw == RTW_CHANNEL_WIDTH_40)
					skb_tx_rate_ctl_bcast.rate_id =
					    RTW_RATEID_BGN_40M_1SS;
				else
					skb_tx_rate_ctl_bcast.rate_id =
					    RTW_RATEID_BGN_20M_1SS;
			else if (skb_tx_rate_ctl_bcast.rate <= DESC_RATEMCS15)
				if (skb_tx_rate_ctl_bcast.bw == RTW_CHANNEL_WIDTH_40)
					skb_tx_rate_ctl_bcast.rate_id =
					    RTW_RATEID_BGN_40M_2SS;
				else
					skb_tx_rate_ctl_bcast.rate_id =
					    RTW_RATEID_BGN_20M_2SS;
			else if (skb_tx_rate_ctl_bcast.rate <= DESC_RATEMCS23)
				skb_tx_rate_ctl_bcast.rate_id = RTW_RATEID_ARFR5_N_3SS;
			else if (skb_tx_rate_ctl_bcast.rate <= DESC_RATEMCS31)
				skb_tx_rate_ctl_bcast.rate_id = RTW_RATEID_ARFR7_N_4SS;
			else
				return false;
			break;
		case RTW_RATE_SECTION_VHT_1S:
			if (skb_tx_rate_ctl_bcast.bw >= RTW_CHANNEL_WIDTH_80)
				skb_tx_rate_ctl_bcast.rate_id =
				    RTW_RATEID_ARFR1_AC_1SS;
			else
				skb_tx_rate_ctl_bcast.rate_id =
				    RTW_RATEID_ARFR2_AC_2G_1SS;
			if (skb_tx_rate_ctl_bcast.rate < DESC_RATEMCS0 ||
			    skb_tx_rate_ctl_bcast.rate > DESC_RATEMCS9)
				return false;
			break;
		case RTW_RATE_SECTION_VHT_2S:
			if (skb_tx_rate_ctl_bcast.bw >= RTW_CHANNEL_WIDTH_80)
				skb_tx_rate_ctl_bcast.rate_id =
				    RTW_RATEID_ARFR0_AC_2SS;
			else
				skb_tx_rate_ctl_bcast.rate_id =
				    RTW_RATEID_ARFR3_AC_2G_2SS;
			if (skb_tx_rate_ctl_bcast.rate < DESC_RATEMCS0 ||
			    skb_tx_rate_ctl_bcast.rate > DESC_RATEMCS9)
				return false;
			break;
		case RTW_RATE_SECTION_VHT_3S:
			skb_tx_rate_ctl_bcast.rate_id = RTW_RATEID_ARFR4_AC_3SS;
			if (skb_tx_rate_ctl_bcast.rate < DESC_RATEMCS0 ||
			    skb_tx_rate_ctl_bcast.rate > DESC_RATEMCS9)
				return false;
			break;
		case RTW_RATE_SECTION_VHT_4S:
			skb_tx_rate_ctl_bcast.rate_id = RTW_RATEID_ARFR6_AC_4SS;
			if (skb_tx_rate_ctl_bcast.rate < DESC_RATEMCS0 ||
			    skb_tx_rate_ctl_bcast.rate > DESC_RATEMCS9)
				return false;
			break;
		default:
			return false;
	}

	return true;
}

static char *safe_str(char *dst, const char *src, size_t sz)
{
	if (dst == NULL || src == NULL || sz == 0)
		return dst;
	strncpy(dst, src, sz);
	dst[sz - 1] = '\0';

	return strstrip(dst);
}

static int bcast_bw_set(const char *val0, const struct kernel_param *kp)
{
	char val[8];
	char *s = safe_str(val, val0, sizeof(val));
	uint bw;

	if (strcmp(s, "20") == 0)
		bw = RTW_CHANNEL_WIDTH_20;
	else if (strcmp(s, "40") == 0)
		bw = RTW_CHANNEL_WIDTH_40;
	else if (strcmp(s, "80") == 0)
		bw = RTW_CHANNEL_WIDTH_80;
	else if (strcmp(s, "160") == 0)
		bw = RTW_CHANNEL_WIDTH_160;
	else if (strcmp(s, "80+80") == 0)
		bw = RTW_CHANNEL_WIDTH_80_80;
	else if (strcmp(s, "5") == 0)
		bw = RTW_CHANNEL_WIDTH_5;
	else if (strcmp(s, "10") == 0)
		bw = RTW_CHANNEL_WIDTH_10;
	else
		return -EINVAL;

	if (bw > RTW_MAX_CHANNEL_WIDTH)
		return -ENOTSUPP;

	skb_tx_rate_ctl_bcast.present = false;
	skb_tx_rate_ctl_bcast.bw = bw;
	skb_tx_rate_ctl_bcast.present = bcast_rate_update();

	return 0;
}
static int bcast_bw_get(char *buf, const struct kernel_param *kp)
{
	switch (skb_tx_rate_ctl_bcast.bw) {
		case RTW_CHANNEL_WIDTH_20:
			strcpy(buf, "20");
			break;
		case RTW_CHANNEL_WIDTH_40:
			strcpy(buf, "40");
			break;
		case RTW_CHANNEL_WIDTH_80:
			strcpy(buf, "80");
			break;
		case RTW_CHANNEL_WIDTH_160:
			strcpy(buf, "160");
			break;
		case RTW_CHANNEL_WIDTH_80_80:
			strcpy(buf, "80+80");
			break;
		case RTW_CHANNEL_WIDTH_5:
			strcpy(buf, "5");
			break;
		case RTW_CHANNEL_WIDTH_10:
			strcpy(buf, "10");
			break;
		default:
			strcpy(buf, "unknown");
			break;
	}

	return strlen(buf);
}
static const struct kernel_param_ops bcast_bw_ops = {
	.set = bcast_bw_set,
	.get = bcast_bw_get
};
module_param_cb(bcast_bw, &bcast_bw_ops, NULL, 0600);

static int bcast_rate_set(const char *val0, const struct kernel_param *kp)
{
	char val[8];
	char *s = safe_str(val, val0, sizeof(val));
	uint rate;

	if (strcasecmp(s, "1M") == 0)
		rate = DESC_RATE1M;
	else if (strcasecmp(s, "2M") == 0)
		rate = DESC_RATE2M;
	else if (strcasecmp(s, "5.5M") == 0)
		rate = DESC_RATE5_5M;
	else if (strcasecmp(s, "6M") == 0)
		rate = DESC_RATE6M;
	else if (strcasecmp(s, "9M") == 0)
		rate = DESC_RATE9M;
	else if (strcasecmp(s, "11M") == 0)
		rate = DESC_RATE11M;
	else if (strcasecmp(s, "12M") == 0)
		rate = DESC_RATE12M;
	else if (strcasecmp(s, "18M") == 0)
		rate = DESC_RATE18M;
	else if (strcasecmp(s, "24M") == 0)
		rate = DESC_RATE24M;
	else if (strcasecmp(s, "36M") == 0)
		rate = DESC_RATE36M;
	else if (strcasecmp(s, "48M") == 0)
		rate = DESC_RATE48M;
	else if (strcasecmp(s, "54M") == 0)
		rate = DESC_RATE54M;
	else if (strcasecmp(s, "MCS0") == 0)
		rate = DESC_RATEMCS0;
	else if (strcasecmp(s, "MCS1") == 0)
		rate = DESC_RATEMCS1;
	else if (strcasecmp(s, "MCS2") == 0)
		rate = DESC_RATEMCS2;
	else if (strcasecmp(s, "MCS3") == 0)
		rate = DESC_RATEMCS3;
	else if (strcasecmp(s, "MCS4") == 0)
		rate = DESC_RATEMCS4;
	else if (strcasecmp(s, "MCS5") == 0)
		rate = DESC_RATEMCS5;
	else if (strcasecmp(s, "MCS6") == 0)
		rate = DESC_RATEMCS6;
	else if (strcasecmp(s, "MCS7") == 0)
		rate = DESC_RATEMCS7;
	else if (strcasecmp(s, "MCS8") == 0)
		rate = DESC_RATEMCS8;
	else if (strcasecmp(s, "MCS9") == 0)
		rate = DESC_RATEMCS9;
	else if (strcasecmp(s, "MCS10") == 0)
		rate = DESC_RATEMCS10;
	else if (strcasecmp(s, "MCS11") == 0)
		rate = DESC_RATEMCS11;
	else if (strcasecmp(s, "MCS12") == 0)
		rate = DESC_RATEMCS12;
	else if (strcasecmp(s, "MCS13") == 0)
		rate = DESC_RATEMCS13;
	else if (strcasecmp(s, "MCS14") == 0)
		rate = DESC_RATEMCS14;
	else if (strcasecmp(s, "MCS15") == 0)
		rate = DESC_RATEMCS15;
	else if (strcasecmp(s, "MCS16") == 0)
		rate = DESC_RATEMCS16;
	else if (strcasecmp(s, "MCS17") == 0)
		rate = DESC_RATEMCS17;
	else if (strcasecmp(s, "MCS18") == 0)
		rate = DESC_RATEMCS18;
	else if (strcasecmp(s, "MCS19") == 0)
		rate = DESC_RATEMCS19;
	else if (strcasecmp(s, "MCS20") == 0)
		rate = DESC_RATEMCS20;
	else if (strcasecmp(s, "MCS21") == 0)
		rate = DESC_RATEMCS21;
	else if (strcasecmp(s, "MCS22") == 0)
		rate = DESC_RATEMCS22;
	else if (strcasecmp(s, "MCS23") == 0)
		rate = DESC_RATEMCS23;
	else if (strcasecmp(s, "MCS24") == 0)
		rate = DESC_RATEMCS24;
	else if (strcasecmp(s, "MCS25") == 0)
		rate = DESC_RATEMCS25;
	else if (strcasecmp(s, "MCS26") == 0)
		rate = DESC_RATEMCS26;
	else if (strcasecmp(s, "MCS27") == 0)
		rate = DESC_RATEMCS27;
	else if (strcasecmp(s, "MCS28") == 0)
		rate = DESC_RATEMCS28;
	else if (strcasecmp(s, "MCS29") == 0)
		rate = DESC_RATEMCS29;
	else if (strcasecmp(s, "MCS30") == 0)
		rate = DESC_RATEMCS30;
	else if (strcasecmp(s, "MCS31") == 0)
		rate = DESC_RATEMCS31;
	else if (strncasecmp(s, "MCS", 3) == 0)
		return -ENOTSUPP;
	else
		return -EINVAL;

	skb_tx_rate_ctl_bcast.present = false;
	skb_tx_rate_ctl_bcast.rate = rate;
	skb_tx_rate_ctl_bcast.present = bcast_rate_update();

	return 0;
}

static int bcast_rate_get(char *buf, const struct kernel_param *kp)
{
	switch (skb_tx_rate_ctl_bcast.rate) {
		case DESC_RATE1M:
			strcpy(buf, "1M");
			break;
		case DESC_RATE2M:
			strcpy(buf, "2M");
			break;
		case DESC_RATE5_5M:
			strcpy(buf, "5.5M");
			break;
		case DESC_RATE6M:
			strcpy(buf, "6M");
			break;
		case DESC_RATE9M:
			strcpy(buf, "9M");
			break;
		case DESC_RATE11M:
			strcpy(buf, "11M");
			break;
		case DESC_RATE12M:
			strcpy(buf, "12M");
			break;
		case DESC_RATE18M:
			strcpy(buf, "18M");
			break;
		case DESC_RATE24M:
			strcpy(buf, "24M");
			break;
		case DESC_RATE36M:
			strcpy(buf, "36M");
			break;
		case DESC_RATE48M:
			strcpy(buf, "48M");
			break;
		case DESC_RATE54M:
			strcpy(buf, "54M");
			break;
		case DESC_RATEMCS1:
			strcpy(buf, "MCS1");
			break;
		case DESC_RATEMCS2:
			strcpy(buf, "MCS2");
			break;
		case DESC_RATEMCS3:
			strcpy(buf, "MCS3");
			break;
		case DESC_RATEMCS4:
			strcpy(buf, "MCS4");
			break;
		case DESC_RATEMCS5:
			strcpy(buf, "MCS5");
			break;
		case DESC_RATEMCS6:
			strcpy(buf, "MCS6");
			break;
		case DESC_RATEMCS7:
			strcpy(buf, "MCS7");
			break;
		case DESC_RATEMCS8:
			strcpy(buf, "MCS8");
			break;
		case DESC_RATEMCS9:
			strcpy(buf, "MCS9");
			break;
		case DESC_RATEMCS10:
			strcpy(buf, "MCS10");
			break;
		case DESC_RATEMCS11:
			strcpy(buf, "MCS11");
			break;
		case DESC_RATEMCS12:
			strcpy(buf, "MCS12");
			break;
		case DESC_RATEMCS13:
			strcpy(buf, "MCS13");
			break;
		case DESC_RATEMCS14:
			strcpy(buf, "MCS14");
			break;
		case DESC_RATEMCS15:
			strcpy(buf, "MCS15");
			break;
		case DESC_RATEMCS16:
			strcpy(buf, "MCS16");
			break;
		case DESC_RATEMCS17:
			strcpy(buf, "MCS17");
			break;
		case DESC_RATEMCS18:
			strcpy(buf, "MCS18");
			break;
		case DESC_RATEMCS19:
			strcpy(buf, "MCS19");
			break;
		case DESC_RATEMCS20:
			strcpy(buf, "MCS20");
			break;
		case DESC_RATEMCS21:
			strcpy(buf, "MCS21");
			break;
		case DESC_RATEMCS22:
			strcpy(buf, "MCS22");
			break;
		case DESC_RATEMCS23:
			strcpy(buf, "MCS23");
			break;
		case DESC_RATEMCS24:
			strcpy(buf, "MCS24");
			break;
		case DESC_RATEMCS25:
			strcpy(buf, "MCS25");
			break;
		case DESC_RATEMCS26:
			strcpy(buf, "MCS26");
			break;
		case DESC_RATEMCS27:
			strcpy(buf, "MCS27");
			break;
		case DESC_RATEMCS28:
			strcpy(buf, "MCS28");
			break;
		case DESC_RATEMCS29:
			strcpy(buf, "MCS29");
			break;
		case DESC_RATEMCS30:
			strcpy(buf, "MCS30");
			break;
		case DESC_RATEMCS31:
			strcpy(buf, "MCS31");
			break;
		default:
			strcpy(buf, "unknown");
			break;
	}

	return strlen(buf);
}
static const struct kernel_param_ops bcast_rate_ops = {
	.set = bcast_rate_set,
	.get = bcast_rate_get
};
module_param_cb(bcast_rate, &bcast_rate_ops, NULL, 0600);

static int bcast_modulation_set(const char *val0,
    const struct kernel_param *kp)
{
	char val[8];
	char *s = safe_str(val, val0, sizeof(val));
	uint modulation;

	if (strcasecmp(s, "CCK") == 0)
		modulation = RTW_RATE_SECTION_CCK;
	else if (strcasecmp(s, "OFDM") == 0)
		modulation = RTW_RATE_SECTION_OFDM;
	else if (strcasecmp(s, "HT") == 0)
		modulation = RTW_RATE_SECTION_HT_1S;
	else if (strcasecmp(s, "HT-1S") == 0)
		modulation = RTW_RATE_SECTION_HT_1S;
	else if (strcasecmp(s, "HT-2S") == 0)
		modulation = RTW_RATE_SECTION_HT_2S;
	else if (strcasecmp(s, "HT-3S") == 0)
		modulation = RTW_RATE_SECTION_HT_3S;
	else if (strcasecmp(s, "HT-4S") == 0)
		modulation = RTW_RATE_SECTION_HT_4S;
	else if (strcasecmp(s, "VHT-1S") == 0)
		modulation = RTW_RATE_SECTION_VHT_1S;
	else if (strcasecmp(s, "VHT-2S") == 0)
		modulation = RTW_RATE_SECTION_VHT_2S;
	else if (strcasecmp(s, "VHT-3S") == 0)
		modulation = RTW_RATE_SECTION_VHT_3S;
	else if (strcasecmp(s, "VHT-4S") == 0)
		modulation = RTW_RATE_SECTION_VHT_4S;
	else
		return -EINVAL;

	skb_tx_rate_ctl_bcast.present = false;
	skb_tx_rate_ctl_bcast.modulation = modulation;
	skb_tx_rate_ctl_bcast.present = bcast_rate_update();

	return 0;
}
static int bcast_modulation_get(char *buf, const struct kernel_param *kp)
{
	switch (skb_tx_rate_ctl_bcast.modulation) {
		case RTW_RATE_SECTION_CCK:
			strcpy(buf, "CCK");
			break;
		case RTW_RATE_SECTION_OFDM:
			strcpy(buf, "OFDM");
			break;
		case RTW_RATE_SECTION_HT_1S:
		case RTW_RATE_SECTION_HT_2S:
		case RTW_RATE_SECTION_HT_3S:
		case RTW_RATE_SECTION_HT_4S:
			strcpy(buf, "HT");
			break;
		case RTW_RATE_SECTION_VHT_1S:
			strcpy(buf, "VHT-1S");
			break;
		case RTW_RATE_SECTION_VHT_2S:
			strcpy(buf, "VHT-2S");
			break;
		case RTW_RATE_SECTION_VHT_3S:
			strcpy(buf, "VHT-3S");
			break;
		case RTW_RATE_SECTION_VHT_4S:
			strcpy(buf, "VHT-4S");
			break;
		default:
			strcpy(buf, "unknown");
			break;
	}

	return strlen(buf);
}
static const struct kernel_param_ops bcast_modulation_ops = {
	.set = bcast_modulation_set,
	.get = bcast_modulation_get
};
module_param_cb(bcast_modulation, &bcast_modulation_ops, NULL, 0600);

static int bcast_stbc_set(const char *val0, const struct kernel_param *kp)
{
	char val[8];
	char *s = safe_str(val, val0, sizeof(val));

	if (strcasecmp(s, "enable") == 0)
		skb_tx_rate_ctl_bcast.stbc = true;
	else if (strcasecmp(s, "disable") == 0)
		skb_tx_rate_ctl_bcast.stbc = false;
	else if (strcmp(s, "0") == 0)
		skb_tx_rate_ctl_bcast.stbc = false;
	else if (strcmp(s, "1") == 0)
		skb_tx_rate_ctl_bcast.stbc = true;
	else
		return -EINVAL;

	return 0;
}
static int bcast_stbc_get(char *buf, const struct kernel_param *kp)
{
	if (skb_tx_rate_ctl_bcast.stbc)
		strcpy(buf, "enable");
	else
		strcpy(buf, "disable");

	return strlen(buf);
}
static const struct kernel_param_ops bcast_stbc_ops = {
	.set = bcast_stbc_set,
	.get = bcast_stbc_get
};
module_param_cb(bcast_stbc, &bcast_stbc_ops, NULL, 0600);

static int bcast_ldpc_set(const char *val0, const struct kernel_param *kp)
{
	char val[8];
	char *s = safe_str(val, val0, sizeof(val));

	if (strcasecmp(s, "enable") == 0)
		skb_tx_rate_ctl_bcast.ldpc = true;
	else if (strcasecmp(s, "disable") == 0)
		skb_tx_rate_ctl_bcast.ldpc = false;
	else if (strcmp(s, "0") == 0)
		skb_tx_rate_ctl_bcast.ldpc = false;
	else if (strcmp(s, "1") == 0)
		skb_tx_rate_ctl_bcast.ldpc = true;
	else
		return -EINVAL;

	return 0;
}
static int bcast_ldpc_get(char *buf, const struct kernel_param *kp)
{
	if (skb_tx_rate_ctl_bcast.ldpc)
		strcpy(buf, "enable");
	else
		strcpy(buf, "disable");

	return strlen(buf);
}
static const struct kernel_param_ops bcast_ldpc_ops = {
	.set = bcast_ldpc_set,
	.get = bcast_ldpc_get
};
module_param_cb(bcast_ldpc, &bcast_ldpc_ops, NULL, 0600);

static int bcast_sgi_set(const char *val0, const struct kernel_param *kp)
{
	char val[8];
	char *s = safe_str(val, val0, sizeof(val));

	if (strcasecmp(s, "enable") == 0)
		skb_tx_rate_ctl_bcast.short_gi = true;
	else if (strcasecmp(s, "disable") == 0)
		skb_tx_rate_ctl_bcast.short_gi = false;
	else if (strcmp(s, "0") == 0)
		skb_tx_rate_ctl_bcast.short_gi = false;
	else if (strcmp(s, "1") == 0)
		skb_tx_rate_ctl_bcast.short_gi = true;
	else
		return -EINVAL;

	return 0;
}
static int bcast_sgi_get(char *buf, const struct kernel_param *kp)
{
	if (skb_tx_rate_ctl_bcast.short_gi)
		strcpy(buf, "enable");
	else
		strcpy(buf, "disable");

	return strlen(buf);
}
static const struct kernel_param_ops bcast_sgi_ops = {
	.set = bcast_sgi_set,
	.get = bcast_sgi_get
};
module_param_cb(bcast_short_gi, &bcast_sgi_ops, NULL, 0600);

static int bcast_rate_ctl_set(const char *val0,
    const struct kernel_param *kp)
{
	char val[8];
	char *s = safe_str(val, val0, sizeof(val));

	if (strcasecmp(s, "enable") == 0)
		bcast_rate_ctl = true;
	else if (strcasecmp(s, "disable") == 0)
		bcast_rate_ctl = false;
	else if (strcmp(s, "0") == 0)
		bcast_rate_ctl = false;
	else if (strcmp(s, "1") == 0)
		bcast_rate_ctl = true;
	else
		return -EINVAL;

	return 0;
}

static int bcast_rate_ctl_get(char *buf, const struct kernel_param *kp)
{
	if (bcast_rate_ctl)
		strcpy(buf, "enable");
	else
		strcpy(buf, "disable");

	return strlen(buf);
}
static const struct kernel_param_ops bcast_rate_ctl_ops = {
	.set = bcast_rate_ctl_set,
	.get = bcast_rate_ctl_get
};
module_param_cb(bcast_rate_ctl, &bcast_rate_ctl_ops, NULL, 0600);

static int bcast_rate_valid_set(const char *val0, const struct kernel_param *kp)
{
	return -EINVAL;
}
static int bcast_rate_valid_get(char *buf, const struct kernel_param *kp)
{
	if (skb_tx_rate_ctl_bcast.present)
		strcpy(buf, "valid");
	else
		strcpy(buf, "invalid");

	return strlen(buf);
}
static const struct kernel_param_ops bcast_rate_valid_ops = {
	.set = bcast_rate_valid_set,
	.get = bcast_rate_valid_get
};
module_param_cb(bcast_rate_valid, &bcast_rate_valid_ops, NULL, 0600);

void __tx_rate_ctl_extract(struct ieee80211_tx_info *info,
    struct tx_rate_ctl *txr)
{
	txr->present = false;

	if (info->control.rates[0].flags & IEEE80211_TX_RC_VHT_MCS) {
		/* VHT: actual rate is defined by MCS index & NSS & BW */
		u8 mcs = info->control.rates[0].idx & 0x0f;
		txr->nss = ((info->control.rates[0].idx >> 4) & 0x7);

		if (txr->nss > 3 || mcs > 9)
			return;

		if (info->control.flags & IEEE80211_TX_RC_40_MHZ_WIDTH)
			txr->bw = RTW_CHANNEL_WIDTH_40;
		else if (info->control.flags & IEEE80211_TX_RC_80_MHZ_WIDTH)
			txr->bw = RTW_CHANNEL_WIDTH_80;
		else if (info->control.flags & IEEE80211_TX_RC_160_MHZ_WIDTH)
			txr->bw = RTW_CHANNEL_WIDTH_160;
		else
			txr->bw = RTW_CHANNEL_WIDTH_20;

		if (txr->bw > RTW_MAX_CHANNEL_WIDTH)
			return;

		txr->rate = DESC_RATEVHT1SS_MCS0 + txr->nss * 10 + mcs;

		switch (txr->nss) {
			case 0:
				if (txr->bw >= RTW_CHANNEL_WIDTH_80)
					txr->rate_id =
					    RTW_RATEID_ARFR1_AC_1SS;
				else
					txr->rate_id =
					    RTW_RATEID_ARFR2_AC_2G_1SS;
				break;
			case 1:
				if (txr->bw >= RTW_CHANNEL_WIDTH_80)
					txr->rate_id =
					    RTW_RATEID_ARFR0_AC_2SS;
				else
					txr->rate_id =
					    RTW_RATEID_ARFR3_AC_2G_2SS;
			case 2:
				txr->rate_id = RTW_RATEID_ARFR4_AC_3SS;
				break;
			case 3:
				txr->rate_id = RTW_RATEID_ARFR6_AC_4SS;
				break;
			default:
				return;
		}
	}
	else if (info->control.rates[0].flags & IEEE80211_TX_RC_MCS) {
		/* HT: actual rate is defined by MCS index only */
		txr->nss = 0; /* not used */
		txr->rate = DESC_RATEMCS0 + info->control.rates[0].idx;

		if (info->control.flags & IEEE80211_TX_RC_40_MHZ_WIDTH)
			txr->bw = RTW_CHANNEL_WIDTH_40;
		else if (info->control.flags & IEEE80211_TX_RC_80_MHZ_WIDTH)
			return;
		else if (info->control.flags & IEEE80211_TX_RC_160_MHZ_WIDTH)
			return;
		else
			txr->bw = RTW_CHANNEL_WIDTH_20;

		if (txr->rate <= DESC_RATEMCS7)
			if (txr->bw >= RTW_CHANNEL_WIDTH_40)
				txr->rate_id = RTW_RATEID_BGN_40M_1SS;
			else
				txr->rate_id = RTW_RATEID_BGN_20M_1SS;
		else if (txr->rate <= DESC_RATEMCS15)
			if (txr->bw >= RTW_CHANNEL_WIDTH_40)
				txr->rate_id = RTW_RATEID_BGN_40M_2SS;
			else
				txr->rate_id = RTW_RATEID_BGN_20M_2SS;
		else if (txr->rate <= DESC_RATEMCS23)
			txr->rate_id = RTW_RATEID_ARFR5_N_3SS;
		else if (txr->rate <= DESC_RATEMCS31)
			txr->rate_id = RTW_RATEID_ARFR7_N_4SS;
		else
			return;
	}
	else {
		/* Legacy: rate is directly defined by supported bands index */
		/*  => wiphy->bands[index] unit of 100 kbps */
		u16 rate = info->control.rates[0].idx;
		txr->nss = 0; /* not used */
		txr->bw = RTW_CHANNEL_WIDTH_20;
		txr->rate_id = RTW_RATEID_BG;

		if (rate <= 10)
			txr->rate = DESC_RATE1M;
		else if (rate <= 20)
			txr->rate = DESC_RATE2M;
		else if (rate <= 55)
			txr->rate = DESC_RATE5_5M;
		else if (rate <= 60)
			txr->rate = DESC_RATE6M;
		else if (rate <= 90)
			txr->rate = DESC_RATE9M;
		else if (rate <= 110)
			txr->rate = DESC_RATE11M;
		else if (rate <= 120)
			txr->rate = DESC_RATE12M;
		else if (rate <= 240)
			txr->rate = DESC_RATE24M;
		else if (rate <= 360)
			txr->rate = DESC_RATE36M;
		else if (rate <= 480)
			txr->rate = DESC_RATE48M;
		else if (rate <= 540)
			txr->rate = DESC_RATE54M;
		else 
			return;

		txr->short_gi = false;
		txr->ldpc = false;
		txr->stbc = false;
		txr->present = true;
		return;
	}

	if (info->control.flags & IEEE80211_TX_RC_SHORT_GI)
		txr->short_gi = true;
	else
		txr->short_gi = false;
	if (info->control.flags & IEEE80211_TX_CTL_LDPC)
		txr->ldpc = true;
	else
		txr->ldpc = false;
	if (info->control.flags & IEEE80211_TX_CTL_STBC)
		txr->stbc = true;
	else
		txr->stbc = false;

	txr->present = true;
	return;
}

void tx_rate_ctl_extract(struct ieee80211_tx_info *info)
{
	if (!(info->flags & IEEE80211_TX_CTL_INJECTED))
		return;
	if (!(info->control.flags & IEEE80211_TX_CTRL_RATE_INJECT))
		return;

	__tx_rate_ctl_extract(info, &skb_tx_rate_ctl);
}


static void __tx_rate_ctl_apply(struct rtw_tx_pkt_info *pkt_info,
    struct tx_rate_ctl *txr)
{
	if (!txr->present)
		return;

	pkt_info->bw = txr->bw;
	pkt_info->rate = txr->rate;
	pkt_info->rate_id = txr->rate_id;
	pkt_info->short_gi = txr->short_gi;
	pkt_info->stbc = txr->stbc;
	pkt_info->ldpc = txr->ldpc;
	pkt_info->dis_rate_fallback = true;
	pkt_info->use_rate = true;
}

void tx_rate_ctl_apply(struct rtw_tx_pkt_info *pkt_info)
{
	if (!skb_tx_rate_ctl.present)
		return;

	__tx_rate_ctl_apply(pkt_info, &skb_tx_rate_ctl);
}

void bcast_rate_ctl_apply(struct rtw_tx_pkt_info *pkt_info)
{
	if (skb_tx_rate_ctl.present)
		return;
	if (!bcast_rate_ctl)
		return;
	if (!skb_tx_rate_ctl_bcast.present)
		return;

	__tx_rate_ctl_apply(pkt_info, &skb_tx_rate_ctl_bcast);
}

