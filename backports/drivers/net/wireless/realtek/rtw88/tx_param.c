// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause

#include "main.h"
#include "tx.h"
#include "tx_param.h"

bool bcast_rate_override = true;
bool bcast_rate_valid = true;

uint bcast_rate_id = RTW_RATEID_BGN_20M_1SS;
uint bcast_bw = RTW_CHANNEL_WIDTH_20;
uint bcast_rate = DESC_RATEMCS1;
bool bcast_stbc = false;
bool bcast_ldpc = true;

static uint bcast_modulation = RTW_RATE_SECTION_HT_1S;

static bool bcast_update_rate(void)
{
	switch (bcast_modulation) {
		case RTW_RATE_SECTION_CCK:
			bcast_rate_id = RTW_RATEID_B_20M;
			if (bcast_rate > DESC_RATE11M)
				return false;
			break;
		case RTW_RATE_SECTION_OFDM:
			bcast_rate_id = RTW_RATEID_BG;
			if (bcast_rate > DESC_RATE54M)
				return false;
			break;
		case RTW_RATE_SECTION_HT_1S:
			if (bcast_bw == RTW_CHANNEL_WIDTH_40)
				bcast_rate_id = RTW_RATEID_BGN_40M_1SS;
			else
				bcast_rate_id = RTW_RATEID_BGN_20M_1SS;
			if (bcast_rate < DESC_RATEMCS0 ||
			    bcast_rate > DESC_RATEMCS7)
				return false;
			break;
		case RTW_RATE_SECTION_HT_2S:
			if (bcast_bw == RTW_CHANNEL_WIDTH_40)
				bcast_rate_id = RTW_RATEID_BGN_40M_2SS;
			else
				bcast_rate_id = RTW_RATEID_BGN_20M_2SS;
			if (bcast_rate < DESC_RATEMCS8 ||
			    bcast_rate > DESC_RATEMCS15)
				return false;
			break;
		case RTW_RATE_SECTION_HT_3S:
			bcast_rate_id = RTW_RATEID_ARFR5_N_3SS;
			if (bcast_rate < DESC_RATEMCS16 ||
			    bcast_rate > DESC_RATEMCS23)
				return false;
			break;
		case RTW_RATE_SECTION_HT_4S:
			bcast_rate_id = RTW_RATEID_ARFR7_N_4SS;
			if (bcast_rate < DESC_RATEMCS24 ||
			    bcast_rate > DESC_RATEMCS31)
				return false;
			break;
		case RTW_RATE_SECTION_VHT_1S:
			if (bcast_bw >= RTW_CHANNEL_WIDTH_80)
				bcast_rate_id = RTW_RATEID_ARFR1_AC_1SS;
			else
				bcast_rate_id = RTW_RATEID_ARFR2_AC_2G_1SS;
			if (bcast_rate < DESC_RATEMCS0 ||
			    bcast_rate > DESC_RATEMCS9)
				return false;
			break;
		case RTW_RATE_SECTION_VHT_2S:
			if (bcast_bw >= RTW_CHANNEL_WIDTH_80)
				bcast_rate_id = RTW_RATEID_ARFR0_AC_2SS;
			else
				bcast_rate_id = RTW_RATEID_ARFR3_AC_2G_2SS;
			if (bcast_rate < DESC_RATEMCS0 ||
			    bcast_rate > DESC_RATEMCS9)
				return false;
			break;
		case RTW_RATE_SECTION_VHT_3S:
			bcast_rate_id = RTW_RATEID_ARFR4_AC_3SS;
			if (bcast_rate < DESC_RATEMCS0 ||
			    bcast_rate > DESC_RATEMCS9)
				return false;
			break;
		case RTW_RATE_SECTION_VHT_4S:
			bcast_rate_id = RTW_RATEID_ARFR6_AC_4SS;
			if (bcast_rate < DESC_RATEMCS0 ||
			    bcast_rate > DESC_RATEMCS9)
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

	bcast_rate_valid = false;
	bcast_bw = bw;
	bcast_rate_valid = bcast_update_rate();

	return 0;
}
static int bcast_bw_get(char *buf, const struct kernel_param *kp)
{
	switch (bcast_bw) {
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
	else
		return -EINVAL;

	bcast_rate_valid = false;
	bcast_rate = rate;
	bcast_rate_valid = bcast_update_rate();

	return 0;
}

static int bcast_rate_get(char *buf, const struct kernel_param *kp)
{
	switch (bcast_rate) {
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

	bcast_rate_valid = false;
	bcast_modulation = modulation;
	bcast_rate_valid = bcast_update_rate();

	return 0;
}
static int bcast_modulation_get(char *buf, const struct kernel_param *kp)
{
	switch (bcast_modulation) {
		case RTW_RATE_SECTION_CCK:
			strcpy(buf, "CCK");
			break;
		case RTW_RATE_SECTION_OFDM:
			strcpy(buf, "OFDM");
			break;
		case RTW_RATE_SECTION_HT_1S:
			strcpy(buf, "HT-1S");
			break;
		case RTW_RATE_SECTION_HT_2S:
			strcpy(buf, "HT-2S");
			break;
		case RTW_RATE_SECTION_HT_3S:
			strcpy(buf, "HT-3S");
			break;
		case RTW_RATE_SECTION_HT_4S:
			strcpy(buf, "HT-4S");
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
		bcast_stbc = true;
	else if (strcasecmp(s, "disable") == 0)
		bcast_stbc = false;
	else if (strcmp(s, "0") == 0)
		bcast_stbc = false;
	else if (strcmp(s, "1") == 0)
		bcast_stbc = true;
	else
		return -EINVAL;

	return 0;
}
static int bcast_stbc_get(char *buf, const struct kernel_param *kp)
{
	if (bcast_stbc)
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
		bcast_ldpc = true;
	else if (strcasecmp(s, "disable") == 0)
		bcast_ldpc = false;
	else if (strcmp(s, "0") == 0)
		bcast_ldpc = false;
	else if (strcmp(s, "1") == 0)
		bcast_ldpc = true;
	else
		return -EINVAL;

	return 0;
}
static int bcast_ldpc_get(char *buf, const struct kernel_param *kp)
{
	if (bcast_ldpc)
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

static int bcast_rate_override_set(const char *val0,
    const struct kernel_param *kp)
{
	char val[8];
	char *s = safe_str(val, val0, sizeof(val));

	if (strcasecmp(s, "enable") == 0)
		bcast_rate_override = true;
	else if (strcasecmp(s, "disable") == 0)
		bcast_rate_override = false;
	else if (strcmp(s, "0") == 0)
		bcast_rate_override = false;
	else if (strcmp(s, "1") == 0)
		bcast_rate_override = true;
	else
		return -EINVAL;

	return 0;
}
static int bcast_rate_override_get(char *buf, const struct kernel_param *kp)
{
	if (bcast_rate_override)
		strcpy(buf, "enable");
	else
		strcpy(buf, "disable");

	return strlen(buf);
}
static const struct kernel_param_ops bcast_rate_override_ops = {
	.set = bcast_rate_override_set,
	.get = bcast_rate_override_get
};
module_param_cb(bcast_rate_override, &bcast_rate_override_ops, NULL, 0600);

static int bcast_rate_valid_set(const char *val0, const struct kernel_param *kp)
{
	return -EINVAL;
}
static int bcast_rate_valid_get(char *buf, const struct kernel_param *kp)
{
	if (bcast_rate_valid)
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
