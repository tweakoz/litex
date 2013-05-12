#include <stdio.h>
#include <stdlib.h>

#include <irq.h>
#include <uart.h>
#include <hw/csr.h>
#include <hw/flags.h>

#include "dvisamplerX.h"

#define FRAMEBUFFER_COUNT 4
#define FRAMEBUFFER_MASK (FRAMEBUFFER_COUNT - 1)

static unsigned int dvisamplerX_framebuffers[FRAMEBUFFER_COUNT][640*480] __attribute__((aligned(16)));
static int dvisamplerX_fb_slot_indexes[2];
static int dvisamplerX_next_fb_index;

void dvisamplerX_isr(void)
{
	int fb_index = -1;

	if(dvisamplerX_dma_slot0_status_read() == DVISAMPLER_SLOT_PENDING) {
		fb_index = dvisamplerX_fb_slot_indexes[0];
		dvisamplerX_fb_slot_indexes[0] = dvisamplerX_next_fb_index;
		dvisamplerX_dma_slot0_address_write((unsigned int)dvisamplerX_framebuffers[dvisamplerX_next_fb_index]);
		dvisamplerX_dma_slot0_status_write(DVISAMPLER_SLOT_LOADED);
		dvisamplerX_next_fb_index = (dvisamplerX_next_fb_index + 1) & FRAMEBUFFER_MASK;
	}
	if(dvisamplerX_dma_slot1_status_read() == DVISAMPLER_SLOT_PENDING) {
		fb_index = dvisamplerX_fb_slot_indexes[1];
		dvisamplerX_fb_slot_indexes[1] = dvisamplerX_next_fb_index;
		dvisamplerX_dma_slot1_address_write((unsigned int)dvisamplerX_framebuffers[dvisamplerX_next_fb_index]);
		dvisamplerX_dma_slot1_status_write(DVISAMPLER_SLOT_LOADED);
		dvisamplerX_next_fb_index = (dvisamplerX_next_fb_index + 1) & FRAMEBUFFER_MASK;
	}

	if(fb_index != -1)
		fb_dmaX_base_write((unsigned int)dvisamplerX_framebuffers[fb_index]);
}

void dvisamplerX_init_video(void)
{
	unsigned int mask;

	dvisamplerX_dma_ev_pending_write(dvisamplerX_dma_ev_pending_read());
	dvisamplerX_dma_ev_enable_write(0x3);
	mask = irq_getmask();
	mask |= 1 << DVISAMPLERX_INTERRUPT;
	irq_setmask(mask);

	dvisamplerX_dma_frame_size_write(sizeof(dvisamplerX_framebuffers[0]));
	dvisamplerX_fb_slot_indexes[0] = 0;
	dvisamplerX_dma_slot0_address_write((unsigned int)dvisamplerX_framebuffers[0]);
	dvisamplerX_dma_slot0_status_write(DVISAMPLER_SLOT_LOADED);
	dvisamplerX_fb_slot_indexes[1] = 1;
	dvisamplerX_dma_slot1_address_write((unsigned int)dvisamplerX_framebuffers[1]);
	dvisamplerX_dma_slot1_status_write(DVISAMPLER_SLOT_LOADED);
	dvisamplerX_next_fb_index = 2;

	fb_dmaX_base_write((unsigned int)dvisamplerX_framebuffers[3]);
}

static int dvisamplerX_d0, dvisamplerX_d1, dvisamplerX_d2;

void dvisamplerX_print_status(void)
{
	printf("dvisamplerX: ph:%4d %4d %4d // charsync:%d%d%d [%d %d %d] // chansync:%d // res:%dx%d\n",
		dvisamplerX_d0, dvisamplerX_d1, dvisamplerX_d2,
		dvisamplerX_data0_charsync_char_synced_read(),
		dvisamplerX_data1_charsync_char_synced_read(),
		dvisamplerX_data2_charsync_char_synced_read(),
		dvisamplerX_data0_charsync_ctl_pos_read(),
		dvisamplerX_data1_charsync_ctl_pos_read(),
		dvisamplerX_data2_charsync_ctl_pos_read(),
		dvisamplerX_chansync_channels_synced_read(),
		dvisamplerX_resdetection_hres_read(),
		dvisamplerX_resdetection_vres_read());
}

void dvisamplerX_calibrate_delays(void)
{
	dvisamplerX_data0_cap_dly_ctl_write(DVISAMPLER_DELAY_CAL);
	dvisamplerX_data1_cap_dly_ctl_write(DVISAMPLER_DELAY_CAL);
	dvisamplerX_data2_cap_dly_ctl_write(DVISAMPLER_DELAY_CAL);
	while(dvisamplerX_data0_cap_dly_busy_read()
		|| dvisamplerX_data1_cap_dly_busy_read()
		|| dvisamplerX_data2_cap_dly_busy_read());
	dvisamplerX_data0_cap_dly_ctl_write(DVISAMPLER_DELAY_RST);
	dvisamplerX_data1_cap_dly_ctl_write(DVISAMPLER_DELAY_RST);
	dvisamplerX_data2_cap_dly_ctl_write(DVISAMPLER_DELAY_RST);
	dvisamplerX_data0_cap_phase_reset_write(1);
	dvisamplerX_data1_cap_phase_reset_write(1);
	dvisamplerX_data2_cap_phase_reset_write(1);
	dvisamplerX_d0 = dvisamplerX_d1 = dvisamplerX_d2 = 0;
}

void dvisamplerX_adjust_phase(void)
{
	switch(dvisamplerX_data0_cap_phase_read()) {
		case DVISAMPLER_TOO_LATE:
			dvisamplerX_data0_cap_dly_ctl_write(DVISAMPLER_DELAY_DEC);
			dvisamplerX_d0--;
			dvisamplerX_data0_cap_phase_reset_write(1);
			break;
		case DVISAMPLER_TOO_EARLY:
			dvisamplerX_data0_cap_dly_ctl_write(DVISAMPLER_DELAY_INC);
			dvisamplerX_d0++;
			dvisamplerX_data0_cap_phase_reset_write(1);
			break;
	}
	switch(dvisamplerX_data1_cap_phase_read()) {
		case DVISAMPLER_TOO_LATE:
			dvisamplerX_data1_cap_dly_ctl_write(DVISAMPLER_DELAY_DEC);
			dvisamplerX_d1--;
			dvisamplerX_data1_cap_phase_reset_write(1);
			break;
		case DVISAMPLER_TOO_EARLY:
			dvisamplerX_data1_cap_dly_ctl_write(DVISAMPLER_DELAY_INC);
			dvisamplerX_d1++;
			dvisamplerX_data1_cap_phase_reset_write(1);
			break;
	}
	switch(dvisamplerX_data2_cap_phase_read()) {
		case DVISAMPLER_TOO_LATE:
			dvisamplerX_data2_cap_dly_ctl_write(DVISAMPLER_DELAY_DEC);
			dvisamplerX_d2--;
			dvisamplerX_data2_cap_phase_reset_write(1);
			break;
		case DVISAMPLER_TOO_EARLY:
			dvisamplerX_data2_cap_dly_ctl_write(DVISAMPLER_DELAY_INC);
			dvisamplerX_d2++;
			dvisamplerX_data2_cap_phase_reset_write(1);
			break;
	}
}

int dvisamplerX_init_phase(void)
{
	int o_d0, o_d1, o_d2; 
	int i, j;

	for(i=0;i<100;i++) {
		o_d0 = dvisamplerX_d0;
		o_d1 = dvisamplerX_d1;
		o_d2 = dvisamplerX_d2;
		for(j=0;j<1000;j++)
			dvisamplerX_adjust_phase();
		if((abs(dvisamplerX_d0 - o_d0) < 4) && (abs(dvisamplerX_d1 - o_d1) < 4) && (abs(dvisamplerX_d2 - o_d2) < 4))
			return 1;
	}
	return 0;
}

static int dvisamplerX_locked;

static int elapsed(int period)
{
	static int last_event;
	int t, dt;

	t = timer0_reload_read() - timer0_value_read(); // TODO: atomic read
	dt = t - last_event;
	if(dt < 0)
		dt += timer0_reload_read();
	if((dt > period) || (dt < 0)) {
		last_event = t;
		return 1;
	} else
		return 0;
}

void dvisamplerX_service(void)
{
	int ret;

	if(dvisamplerX_locked) {
		if(dvisamplerX_clocking_locked_read()) {
			if(elapsed(identifier_frequency_read())) {
				dvisamplerX_adjust_phase();
				dvisamplerX_print_status();
			}
		} else {
			printf("dvisamplerX: lost PLL lock\n");
			dvisamplerX_locked = 0;
		}
	} else {
		if(dvisamplerX_clocking_locked_read()) {
			printf("dvisamplerX: PLL locked\n");
			dvisamplerX_calibrate_delays();
			printf("dvisamplerX: delays calibrated\n");
			ret = dvisamplerX_init_phase();
			if(ret)
				printf("dvisamplerX: phase init OK\n");
			else
				printf("dvisamplerX: phase did not settle\n");
			dvisamplerX_print_status();
			dvisamplerX_locked = 1;
		} else {
			if(elapsed(identifier_frequency_read()/4)) {
				dvisamplerX_clocking_pll_reset_write(1);
				while(!elapsed(identifier_frequency_read()/16));
				dvisamplerX_clocking_pll_reset_write(0);
			}
		}
	}
}