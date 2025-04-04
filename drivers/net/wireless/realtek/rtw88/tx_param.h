// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
#ifndef __TX_PARAM_H_
#define __TX_PARAM_H_
void tx_rate_ctl_extract(struct ieee80211_tx_info *info);
void tx_rate_ctl_apply(struct rtw_tx_pkt_info *pkt_info);
void bcast_rate_ctl_apply(struct rtw_tx_pkt_info *pkt_info);
#endif
