/*
Octopus-ReEL - Realtime Encephalography Laboratory Network
   Copyright (C) 2007-2025 Barkin Ilhan

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.

 Contact info:
 E-Mail:  barkin@unrlabs.org
 Website: http://icon.unrlabs.org/staff/barkin/
 Repo:    https://github.com/4e0n/
*/

/* PARADIGM 'Adapter-Probe' matched vs. unmatched
 *          'Ipsilateral vs. Contralateral' Clicktrains

    counter0: Counter for single trial
    -- squareburst of 400ms stim duration

    -- multiple adapter support added. 
    .. \_para\_0021a2:

# para\_0021a2 Kernel Module Documentation

## Overview

`para_0021a2` is a simplified, deterministic auditory stimulation kernel module from the Octopus-ReEL system. It implements a fixed-timing Adapter–Probe clicktrain paradigm with continuous adapters and no timing randomization. This module is optimized for experiments requiring constant temporal precision across trials.

## Summary of Differences

Compared to earlier variants (`para_0021a`, `para_0021a1`):

* **No temporal jitter**: All durations are fixed.
* **Adapter duration**: Exactly 800 ms (not 850 ms or randomized).
* **IAI and AP offset**: Constant across trials.

## Paradigm Details

* **Type**: Adapter-Probe clicktrain (continuous adapter)
* **Modes supported**:

  * Matched vs. Unmatched
  * Ipsilateral vs. Contralateral
* **Output method**: DAC-based click stimulation
* **Channel assignment**:

  * `dac_0`: Left
  * `dac_1`: Right

## Timing Parameters

* **Click period**: 10 ms
* **Click high duration**: 500 µs
* **Adapter total duration**: 800 ms (fixed)
* **Probe duration**: 50 ms
* **SOA (Stimulus Onset Asynchrony)**: 4 s
* **Adapter → Probe Offset**: 1 s

## Trigger Identifiers

* `PARA_0021A2_L_L600`: L adapter → L probe (600 µs)
* `PARA_0021A2_L_L200`: L adapter → L probe (200 µs)
* `PARA_0021A2_L_C`:     L adapter → Center probe
* `PARA_0021A2_L_R200`: L adapter → R probe (200 µs)
* `PARA_0021A2_L_R600`: L adapter → R probe (600 µs)
* `PARA_0021A2_R_L600`: R adapter → L probe (600 µs)
* `PARA_0021A2_R_L200`: R adapter → L probe (200 µs)
* `PARA_0021A2_R_C`:     R adapter → Center probe
* `PARA_0021A2_R_R200`: R adapter → R probe (200 µs)
* `PARA_0021A2_R_R600`: R adapter → R probe (600 µs)
* `PARA_0021A2_C_C`:     Center adapter → Center probe

## Functional Interface

* `para_0021a2_init`: Initializes all timing constants
* `para_0021a2_start` / `stop` / `pause` / `resume`: Control functions for experiment lifecycle
* `para_0021a2`: Real-time control loop handling pattern parsing, timing logic, DAC triggering

## Stimulation Logic

* Regions (adapter/probe) determined by current `counter0` value
* Lateralization and temporal windows derived from deltas (200/600 µs)
* DACs toggled with sub-millisecond accuracy based on timing phase and lead/lag conditions

## Use Case

This version is designed for experiments where absolute timing consistency is critical. It is ideal for comparative testing across multiple sessions or subjects with minimum variability.

## License

GNU General Public License v3 or later. Copyright (C) 2007–2025 Barkin Ilhan.

    */

#define PARA_0021A2_CONT_ADAPT

#define PARA_0021A2_L_L600	1	// PARA0021 L->L600 (Adapter->Probe)
#define PARA_0021A2_L_L200	2	// PARA0021 L->L200 (Adapter->Probe)
#define PARA_0021A2_L_C		3	// PARA0021 L->C    (Adapter->Probe)
#define PARA_0021A2_L_R200	4	// PARA0021 L->R200 (Adapter->Probe)
#define PARA_0021A2_L_R600	5	// PARA0021 L->R600 (Adapter->Probe)
#define PARA_0021A2_R_L600	6	// PARA0021 R->L600 (Adapter->Probe)
#define PARA_0021A2_R_L200	7	// PARA0021 R->L200 (Adapter->Probe)
#define PARA_0021A2_R_C		8	// PARA0021 R->C    (Adapter->Probe)
#define PARA_0021A2_R_R200	9	// PARA0021 R->R200 (Adapter->Probe)
#define PARA_0021A2_R_R600	10	// PARA0021 R->R600 (Adapter->Probe)
#define PARA_0021A2_C_C		11	// PARA0021 C->C    (Adapter->Probe)

static int para_0021a2_experiment_loop=1;

static int para_0021a2_trigger,

	   para_0021a2_soa,

	   para_0021a2_iai,         /* Inter-adapter-interval */

	   /* Stim atomic properties */
	   para_0021a2_click_period,para_0021a2_no_periods_adapter,para_0021a2_no_periods_probe,
	   para_0021a2_hi_period,
	   para_0021a2_adapter_burst_start,
	   para_0021a2_adapter_total_duration,
	   para_0021a2_adapter_period0,para_0021a2_adapter_period1,para_0021a2_probe_period,
	   para_0021a2_stim_local_offset,

	   //para_0021a2_lr_delta,
	   para_0021a2_lr_delta200,para_0021a2_lr_delta600,

	   para_0021a2_adapter_region_center,
	   para_0021a2_adapter_region_lag,para_0021a2_adapter_region_lead,
	   para_0021a2_probe_region_center,
	   para_0021a2_probe_region_lag,para_0021a2_probe_region_lead,
	   para_0021a2_stim_instant,

	   para_0021a2_adapter_type,para_0021a2_probe_type,
	   para_0021a2_ap_offset,

	   para_0021a2_stim_instant_center,
	   para_0021a2_stim_instant_minus,
	   para_0021a2_stim_instant_minus200,para_0021a2_stim_instant_minus600,
	   para_0021a2_stim_instant_plus,
	   para_0021a2_stim_instant_plus200,para_0021a2_stim_instant_plus600;

static void para_0021a2_init(void) {
 current_pattern_offset=0;

 para_0021a2_soa=(4.00001)*AUDIO_RATE; /* 4.0 seconds */
 para_0021a2_ap_offset=(0.800001)*AUDIO_RATE; /* DISCRETE */
#ifdef PARA_0021A2_CONT_ADAPT
 para_0021a2_ap_offset=(1.000001)*AUDIO_RATE; /* CONTINUOUS */
#endif
 para_0021a2_hi_period=(0.00051)*AUDIO_RATE; /* 500 us - 25 steps */
 para_0021a2_click_period=(0.01001)*AUDIO_RATE; /* 10 ms - 700 steps */

 /* --------- */
 para_0021a2_no_periods_adapter=5*12; /* 60*10 ms */
 para_0021a2_no_periods_probe=5; /* 5*10 ms */

 para_0021a2_adapter_burst_start=(0.200001)*AUDIO_RATE; /* 200ms - Start of burst in CONTINUOUS adapter case */
 para_0021a2_adapter_total_duration=para_0021a2_ap_offset; /* 800ms - DISCRETE (e.g. quadruple) adapter case */
#ifdef PARA_0021A2_CONT_ADAPT
 para_0021a2_adapter_total_duration=(0.800001)*AUDIO_RATE; /* 850ms - CONTINUOUS adapter case */
#endif
 para_0021a2_probe_period=para_0021a2_click_period*para_0021a2_no_periods_probe; /* 50ms */
 para_0021a2_adapter_period0=para_0021a2_probe_period; /*  50ms - for DISCRETE (e.g. quadruple) adapter setting */
 para_0021a2_adapter_period1=para_0021a2_probe_period;
#ifdef PARA_0021A2_CONT_ADAPT
 para_0021a2_adapter_period1*=4; /* 200ms - for CONTINUOUS adapter setting --- */
#endif

 para_0021a2_iai=(0.200001)*AUDIO_RATE; /* Inter-adapter Interval: 1s -- never to happen for long single adapter case */;
 /* --------- */

 para_0021a2_lr_delta200=(0.00021)*AUDIO_RATE; /*  L-R delta: 200us - 10 steps */
 para_0021a2_lr_delta600=(0.00061)*AUDIO_RATE; /*  L-R delta: 600us - 30 steps */

 para_0021a2_stim_instant=(0.200001)*AUDIO_RATE; /* 200ms (arbitrary at beginning) */;
 para_0021a2_stim_instant_center=para_0021a2_stim_instant;
 para_0021a2_stim_instant_minus200=para_0021a2_stim_instant-para_0021a2_lr_delta200/2;
 para_0021a2_stim_instant_plus200=para_0021a2_stim_instant+para_0021a2_lr_delta200/2;
 para_0021a2_stim_instant_minus600=para_0021a2_stim_instant-para_0021a2_lr_delta600/2;
 para_0021a2_stim_instant_plus600=para_0021a2_stim_instant+para_0021a2_lr_delta600/2;

 rt_printk("%d %d %d %d %d %d %d %d %d %d\n",
		 	 para_0021a2_hi_period,
		 	 para_0021a2_click_period,
		 	 para_0021a2_lr_delta200,
		 	 para_0021a2_lr_delta600,
		 	 para_0021a2_stim_instant,
		 	 para_0021a2_stim_instant_center,
		 	 para_0021a2_stim_instant_minus200,
		 	 para_0021a2_stim_instant_minus600,
		 	 para_0021a2_stim_instant_plus200,
		 	 para_0021a2_stim_instant_plus600);

 lights_on();
 rt_printk("octopus-stim-backend.o: Stim initialized.\n");
}

static void para_0021a2_start(void) {
 lights_dimm();
 counter0=0; counter1=0; trigger_active=1; audio_active=1;
 rt_printk("octopus-stim-backend.o: Stim started.\n");
}

static void para_0021a2_stop(void) {
 audio_active=0;
 lights_on();
 rt_printk("octopus-stim-backend.o: Stim stopped.\n");
}

static void para_0021a2_pause(void) {
 audio_active=0;
 lights_on();
 rt_printk("octopus-stim-backend.o: Stim paused.\n");
}

static void para_0021a2_resume(void) {
 lights_dimm();
 counter0=0; counter1=0; audio_active=1;
 rt_printk("octopus-stim-backend.o: Stim resumed.\n");
}

static void para_0021a2(void) {
 //int dummy_counter=0;

 if (counter0==0) {
  current_pattern_data=patt_buf[current_pattern_offset]; /* fetch new.. */

  switch (current_pattern_data) {
   case '@': /* Interblock pause */
             para_0021a2_pause(); /* legitimate pause */
	     break;
   case 'A': /* Click Train L -> L600 */
	     para_0021a2_trigger=PARA_0021A2_L_L600;
	     para_0021a2_adapter_type=1; para_0021a2_probe_type=1;
	     para_0021a2_stim_instant_minus=para_0021a2_stim_instant_minus600;
	     para_0021a2_stim_instant_plus=para_0021a2_stim_instant_plus600;
             break;
   case 'B': /* Click Train L -> L200 */
	     para_0021a2_trigger=PARA_0021A2_L_L200;
	     para_0021a2_adapter_type=1; para_0021a2_probe_type=1;
	     para_0021a2_stim_instant_minus=para_0021a2_stim_instant_minus200;
	     para_0021a2_stim_instant_plus=para_0021a2_stim_instant_plus200;
             break;
   case 'C': /* Click Train L -> C */
	     para_0021a2_trigger=PARA_0021A2_L_C;
	     para_0021a2_adapter_type=1; para_0021a2_probe_type=0;
             break;
   case 'D': /* Click Train L -> R200 */
	     para_0021a2_trigger=PARA_0021A2_L_R200;
	     para_0021a2_adapter_type=1; para_0021a2_probe_type=2;
	     para_0021a2_stim_instant_minus=para_0021a2_stim_instant_minus200;
	     para_0021a2_stim_instant_plus=para_0021a2_stim_instant_plus200;
             break;
   case 'E': /* Click Train L -> R600 */
	     para_0021a2_trigger=PARA_0021A2_L_R600;
	     para_0021a2_adapter_type=1; para_0021a2_probe_type=2;
	     para_0021a2_stim_instant_minus=para_0021a2_stim_instant_minus600;
	     para_0021a2_stim_instant_plus=para_0021a2_stim_instant_plus600;
             break;
   case 'F': /* Click Train R -> L600 */
	     para_0021a2_trigger=PARA_0021A2_R_L600;
	     para_0021a2_adapter_type=2; para_0021a2_probe_type=1;
	     para_0021a2_stim_instant_minus=para_0021a2_stim_instant_minus600;
	     para_0021a2_stim_instant_plus=para_0021a2_stim_instant_plus600;
             break;
   case 'G': /* Click Train R -> L200 */
	     para_0021a2_trigger=PARA_0021A2_R_L200;
	     para_0021a2_adapter_type=2; para_0021a2_probe_type=1;
	     para_0021a2_stim_instant_minus=para_0021a2_stim_instant_minus200;
	     para_0021a2_stim_instant_plus=para_0021a2_stim_instant_plus200;
             break;
   case 'H': /* Click Train R -> C */
	     para_0021a2_trigger=PARA_0021A2_R_C;
	     para_0021a2_adapter_type=2; para_0021a2_probe_type=0;
             break;
   case 'I': /* Click Train R -> R200 */
	     para_0021a2_trigger=PARA_0021A2_R_R200;
	     para_0021a2_adapter_type=2; para_0021a2_probe_type=2;
	     para_0021a2_stim_instant_minus=para_0021a2_stim_instant_minus200;
	     para_0021a2_stim_instant_plus=para_0021a2_stim_instant_plus200;
             break;
   case 'J': /* Click Train R -> R600 */
	     para_0021a2_trigger=PARA_0021A2_R_R600;
	     para_0021a2_adapter_type=2; para_0021a2_probe_type=2;
	     para_0021a2_stim_instant_minus=para_0021a2_stim_instant_minus600;
	     para_0021a2_stim_instant_plus=para_0021a2_stim_instant_plus600;
             break;
   case 'K': /* Click Train C -> C -1 */
	     para_0021a2_trigger=PARA_0021A2_C_C;
	     para_0021a2_adapter_type=0; para_0021a2_probe_type=0;
             break;
   case 'L': /* Click Train C -> C -2 */
	     para_0021a2_trigger=PARA_0021A2_C_C;
	     para_0021a2_adapter_type=0; para_0021a2_probe_type=0;
   default:  break;
  }

  current_pattern_offset++;
  if (current_pattern_offset==pattern_size) { /* roll-back or stop? */
   if (para_0021a2_experiment_loop==0) para_0021a2_stop();
   //else para_0021a2_pause();
   current_pattern_offset=0;
  }
 }

 /* ------------------------------------------------------------------- */
 /* Trigger */
 if ( (counter0==para_0021a2_stim_instant+para_0021a2_ap_offset) && trigger_active &&
      (current_pattern_offset>0) )
  trigger_set(para_0021a2_trigger);
 /* ------------------------------------------------------------------- */

 /* Adapter region center */
 if (counter0 >= para_0021a2_stim_instant_center && \
     counter0 <  para_0021a2_stim_instant_center+para_0021a2_adapter_total_duration) {
  if (counter0 < para_0021a2_stim_instant_center+para_0021a2_adapter_burst_start) {
   if ((counter0-para_0021a2_stim_instant_center)%para_0021a2_iai \
 		                         < para_0021a2_adapter_period0) {
    para_0021a2_adapter_region_center=1;
   } else {
    para_0021a2_adapter_region_center=0;
   }
  } else {
   if ((counter0-para_0021a2_stim_instant_center)%para_0021a2_iai \
 		                         < para_0021a2_adapter_period1) {
    para_0021a2_adapter_region_center=1;
   } else {
    para_0021a2_adapter_region_center=0;
   }
  }
 } else {
  para_0021a2_adapter_region_center=0;
 }

 /* Adapter region (left) lead */
 if (counter0 >= para_0021a2_stim_instant_minus600 && \
     counter0 <  para_0021a2_stim_instant_minus600+para_0021a2_adapter_total_duration) {
  if (counter0 < para_0021a2_stim_instant_minus600+para_0021a2_adapter_burst_start) {
   if ((counter0-para_0021a2_stim_instant_minus600)%para_0021a2_iai \
		                         < para_0021a2_adapter_period0) {
    para_0021a2_adapter_region_lead=1;
   } else {
    para_0021a2_adapter_region_lead=0;
   }
  } else {
   if ((counter0-para_0021a2_stim_instant_minus600)%para_0021a2_iai \
		                         < para_0021a2_adapter_period1) {
    para_0021a2_adapter_region_lead=1;
   } else {
    para_0021a2_adapter_region_lead=0;
   }
  }
 } else {
  para_0021a2_adapter_region_lead=0;
 }

 /* Adapter region (left) lag */
 if (counter0 >= para_0021a2_stim_instant_plus600 && \
     counter0 <  para_0021a2_stim_instant_plus600+para_0021a2_adapter_total_duration) {
  if (counter0 < para_0021a2_stim_instant_plus600+para_0021a2_adapter_burst_start) {
   if ((counter0-para_0021a2_stim_instant_plus600)%para_0021a2_iai \
		                         < para_0021a2_adapter_period0) {
    para_0021a2_adapter_region_lag=1;
   } else {
    para_0021a2_adapter_region_lag=0;
   }
  } else {
   if ((counter0-para_0021a2_stim_instant_plus600)%para_0021a2_iai \
		                         < para_0021a2_adapter_period1) {
    para_0021a2_adapter_region_lag=1;
   } else {
    para_0021a2_adapter_region_lag=0;
   }
  }
 } else {
  para_0021a2_adapter_region_lag=0;
 }

 /* Probe region center */
 if (counter0 >= para_0021a2_stim_instant_center+para_0021a2_ap_offset && \
     counter0 <  para_0021a2_stim_instant_center+para_0021a2_ap_offset \
                                                 +para_0021a2_probe_period) {
  para_0021a2_probe_region_center=1;
 } else {
  para_0021a2_probe_region_center=0;
 }

 /* Probe region (left) lead */
 if (counter0 >= para_0021a2_stim_instant_minus+para_0021a2_ap_offset && \
     counter0 <  para_0021a2_stim_instant_minus+para_0021a2_ap_offset \
                                                 +para_0021a2_probe_period) {
  para_0021a2_probe_region_lead=1;
 } else {
  para_0021a2_probe_region_lead=0;
 }

 /* Probe region (left) lag */
 if (counter0 >= para_0021a2_stim_instant_plus+para_0021a2_ap_offset && \
     counter0 <  para_0021a2_stim_instant_plus+para_0021a2_ap_offset \
                                                 +para_0021a2_probe_period) {
  para_0021a2_probe_region_lag=1;
 } else {
  para_0021a2_probe_region_lag=0;
 }

 /* ------ */

 dac_0=dac_1=0;

 switch (para_0021a2_adapter_type) {
  case 0: /* Center */
   if (para_0021a2_adapter_region_center) {
    para_0021a2_stim_local_offset=counter0-para_0021a2_stim_instant%para_0021a2_probe_period;
    if (para_0021a2_stim_local_offset%para_0021a2_click_period < \
                                                    para_0021a2_hi_period)
     dac_0=dac_1=AMP_OPPCHN;
    else dac_0=dac_1=0;
   }
   break;
  case 1: /* Left Lead */
   if (para_0021a2_adapter_region_lead) {
    para_0021a2_stim_local_offset=counter0-para_0021a2_stim_instant_minus600;
    if (para_0021a2_stim_local_offset%para_0021a2_click_period < \
                                                    para_0021a2_hi_period)
     dac_0=AMP_OPPCHN;
    else dac_0=0;
   }
   if (para_0021a2_adapter_region_lag) {
    para_0021a2_stim_local_offset=counter0-para_0021a2_stim_instant_plus600;
    if (para_0021a2_stim_local_offset%para_0021a2_click_period < \
                                                    para_0021a2_hi_period)
     dac_1=AMP_OPPCHN;
    else dac_1=0;
   }
   break;
  case 2: /* Right Lead*/
   if (para_0021a2_adapter_region_lead) {
    para_0021a2_stim_local_offset=counter0-para_0021a2_stim_instant_minus600;
    if (para_0021a2_stim_local_offset%para_0021a2_click_period < \
                                                    para_0021a2_hi_period)
     dac_1=AMP_OPPCHN;
    else dac_1=0;
   }
   if (para_0021a2_adapter_region_lag) {
    para_0021a2_stim_local_offset=counter0-para_0021a2_stim_instant_plus600;
    if (para_0021a2_stim_local_offset%para_0021a2_click_period < \
                                                    para_0021a2_hi_period)
     dac_0=AMP_OPPCHN;
    else dac_0=0;
   }
   break;
 }

 switch (para_0021a2_probe_type) {
  case 0: /* Center */
   if (para_0021a2_probe_region_center) {
    para_0021a2_stim_local_offset=counter0-para_0021a2_stim_instant \
                                               -para_0021a2_ap_offset;
    if (para_0021a2_stim_local_offset%para_0021a2_click_period < \
                                                    para_0021a2_hi_period)
     dac_0=dac_1=AMP_OPPCHN;
    else dac_0=dac_1=0;
   }
   break;
  case 1: /* Left Lead (L600,L200) */
   if (para_0021a2_probe_region_lead) {
    para_0021a2_stim_local_offset=counter0-para_0021a2_stim_instant_minus \
                                               -para_0021a2_ap_offset;
    if (para_0021a2_stim_local_offset%para_0021a2_click_period < \
                                                    para_0021a2_hi_period)
     dac_0=AMP_OPPCHN;
    else dac_0=0;
   }
   if (para_0021a2_probe_region_lag) {
    para_0021a2_stim_local_offset=counter0-para_0021a2_stim_instant_plus \
                                               -para_0021a2_ap_offset;
    if (para_0021a2_stim_local_offset%para_0021a2_click_period < \
                                                    para_0021a2_hi_period)
     dac_1=AMP_OPPCHN;
    else dac_1=0;
   }
   break;
  case 2: /* Right Lead (R200,R600) */
   if (para_0021a2_probe_region_lead) {
    para_0021a2_stim_local_offset=counter0-para_0021a2_stim_instant_minus \
                                               -para_0021a2_ap_offset;
    if (para_0021a2_stim_local_offset%para_0021a2_click_period < \
                                                    para_0021a2_hi_period)
     dac_1=AMP_OPPCHN;
    else dac_1=0;
   }
   if (para_0021a2_probe_region_lag) {
    para_0021a2_stim_local_offset=counter0-para_0021a2_stim_instant_plus \
                                               -para_0021a2_ap_offset;
    if (para_0021a2_stim_local_offset%para_0021a2_click_period < \
                                                    para_0021a2_hi_period)
     dac_0=AMP_OPPCHN;
    else dac_0=0;
   }
  default: break;
 }

 /* ------------------------------------------------------------------- */

 counter0++;
 counter0%=para_0021a2_soa;
}
