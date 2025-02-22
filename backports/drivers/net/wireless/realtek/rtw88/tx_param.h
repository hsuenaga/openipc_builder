// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
#ifndef __TX_PARAM_H_
#define __TX_PARAM_H_
extern bool bcast_rate_override;
extern bool bcast_rate_valid;

extern uint bcast_rate_id;
extern uint bcast_bw;
extern uint bcast_rate;
extern bool bcast_stbc;
extern bool bcast_ldpc;

static inline bool bcast_override_enable(void) {
	return (bcast_rate_override && bcast_rate_valid);
}

static inline void bcast_override(struct rtw_tx_pkt_info *pkt_info)
{
	if (!bcast_override_enable())
		return;

	// XXX: we must looking for radiotap headers
	pkt_info->bw = bcast_bw;
	pkt_info->rate = bcast_rate;
	pkt_info->rate_id = bcast_rate_id;
	pkt_info->stbc = bcast_stbc;
	pkt_info->ldpc = bcast_ldpc;
	pkt_info->dis_rate_fallback = true;
	pkt_info->use_rate = true;
}
#endif
