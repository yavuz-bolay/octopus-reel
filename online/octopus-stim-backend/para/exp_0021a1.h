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
    .. \_para\_0021a1:

# para\_0021a1 Kernel Module Documentation

## Overview

`para_0021a1` is a real-time auditory stimulation kernel module used in the Octopus-ReEL system. It is a simplified, non-randomized version of the `para_0021a` paradigm. This module delivers fixed-duration, continuous adapter-probe clicktrain sequences for use in neurophysiological auditory experiments.

## Key Differences from para\_0021a

* **No randomization**: Adapter durations are fixed (850ms) in continuous mode.
* **Simplified logic**: Random jitter in the inter-stimulus interval (ISI) has been removed.
* **Use case**: Designed for trials requiring temporally stable stimuli.

## Paradigm Features

* **Type:** Adapter-Probe Clicktrain Paradigm
* **Modes:**

  * Matched vs. Unmatched
  * Ipsilateral vs. Contralateral
* **Adapter type:** Continuous
* **Timing resolution:** Sub-millisecond via AUDIO\_RATE scaling

## Stimulus Timing Parameters

* **Click period:** 10 ms
* **Click high duration (hi\_period):** 500 µs
* **Adapter duration:** 850 ms (fixed, continuous)
* **Probe duration:** 50 ms
* **SOA (Stimulus Onset Asynchrony):** 4 s
* **AP Offset (adapter → probe):** 1 s
* **IAI (Inter-adapter interval):** 200 ms

## Trigger Identifiers

* `PARA_0021A1_L_L600`: L adapter → L probe (600 µs ITD)
* `PARA_0021A1_L_L200`: L adapter → L probe (200 µs ITD)
* `PARA_0021A1_L_C`:     L adapter → Center probe
* `PARA_0021A1_L_R200`: L adapter → R probe (200 µs ITD)
* `PARA_0021A1_L_R600`: L adapter → R probe (600 µs ITD)
* `PARA_0021A1_R_L600`: R adapter → L probe (600 µs ITD)
* `PARA_0021A1_R_L200`: R adapter → L probe (200 µs ITD)
* `PARA_0021A1_R_C`:     R adapter → Center probe
* `PARA_0021A1_R_R200`: R adapter → R probe (200 µs ITD)
* `PARA_0021A1_R_R600`: R adapter → R probe (600 µs ITD)
* `PARA_0021A1_C_C`:     Center adapter → Center probe

## Lifecycle Functions

* `para_0021a1_init`: Sets all timing constants. Duration is fixed.
* `para_0021a1_start`: Starts counters and enables DAC/audio outputs.
* `para_0021a1_stop`: Stops stimulation and turns on lights.
* `para_0021a1_pause`: Pauses stimulus playback.
* `para_0021a1_resume`: Resumes from pause with fresh counters.
* `para_0021a1`: Real-time loop that:

  * Parses character patterns (A–L)
  * Determines lateralization and timing windows
  * Applies DAC settings for adapter and probe
  * Triggers stimuli at precise offsets

## DAC Logic

* DAC outputs alternate between high (`AMP_OPPCHN`) and zero based on timing phase.
* Output channels:

  * `dac_0` for left ear
  * `dac_1` for right ear
* Center stimuli stimulate both channels simultaneously.

## Region Encoding

* **Adapter regions:** lead / center / lag
* **Probe regions:** lead / center / lag
* Stimulus window activation is calculated based on the SOA and delta offsets (200/600 µs).

## Use Case

This module is ideal for experiments requiring precisely-timed, consistent auditory stimulation without jitter, particularly where adaptation effects and interaural latency need controlled testing.

## License

GNU General Public License v3 or later. Copyright (C) 2007–2025 Barkin Ilhan.

    */

#define PARA_0021A1_CONT_ADAPT

#define PARA_0021A1_L_L600	1	// PARA0021 L->L600 (Adapter->Probe)
#define PARA_0021A1_L_L200	2	// PARA0021 L->L200 (Adapter->Probe)
#define PARA_0021A1_L_C		3	// PARA0021 L->C    (Adapter->Probe)
#define PARA_0021A1_L_R200	4	// PARA0021 L->R200 (Adapter->Probe)
#define PARA_0021A1_L_R600	5	// PARA0021 L->R600 (Adapter->Probe)
#define PARA_0021A1_R_L600	6	// PARA0021 R->L600 (Adapter->Probe)
#define PARA_0021A1_R_L200	7	// PARA0021 R->L200 (Adapter->Probe)
#define PARA_0021A1_R_C		8	// PARA0021 R->C    (Adapter->Probe)
#define PARA_0021A1_R_R200	9	// PARA0021 R->R200 (Adapter->Probe)
#define PARA_0021A1_R_R600	10	// PARA0021 R->R600 (Adapter->Probe)
#define PARA_0021A1_C_C		11	// PARA0021 C->C    (Adapter->Probe)

static int para_0021a1_experiment_loop=1;

static int para_0021a1_trigger,

	   para_0021a1_soa,

	   para_0021a1_iai,         /* Inter-adapter-interval */

	   /* Stim atomic properties */
	   para_0021a1_click_period,para_0021a1_no_periods_adapter,para_0021a1_no_periods_probe,
	   para_0021a1_hi_period,
	   para_0021a1_adapter_burst_start,
	   para_0021a1_adapter_total_duration,
	   para_0021a1_adapter_period0,para_0021a1_adapter_period1,para_0021a1_probe_period,
	   para_0021a1_stim_local_offset,

	   //para_0021a1_lr_delta,
	   para_0021a1_lr_delta200,para_0021a1_lr_delta600,

	   para_0021a1_adapter_region_center,
	   para_0021a1_adapter_region_lag,para_0021a1_adapter_region_lead,
	   para_0021a1_probe_region_center,
	   para_0021a1_probe_region_lag,para_0021a1_probe_region_lead,
	   para_0021a1_stim_instant,

	   para_0021a1_adapter_type,para_0021a1_probe_type,
	   para_0021a1_ap_offset,

	   para_0021a1_stim_instant_center,
	   para_0021a1_stim_instant_minus,
	   para_0021a1_stim_instant_minus200,para_0021a1_stim_instant_minus600,
	   para_0021a1_stim_instant_plus,
	   para_0021a1_stim_instant_plus200,para_0021a1_stim_instant_plus600;

static void para_0021a1_init(void) {
 current_pattern_offset=0;

 para_0021a1_soa=(4.00001)*AUDIO_RATE; /* 4.0 seconds */
 para_0021a1_ap_offset=(0.800001)*AUDIO_RATE; /* DISCRETE */
#ifdef PARA_0021A1_CONT_ADAPT
 para_0021a1_ap_offset=(1.000001)*AUDIO_RATE; /* CONTINUOUS */
#endif
 para_0021a1_hi_period=(0.00051)*AUDIO_RATE; /* 500 us - 25 steps */
 para_0021a1_click_period=(0.01001)*AUDIO_RATE; /* 10 ms - 700 steps */

 /* --------- */
 para_0021a1_no_periods_adapter=5*12; /* 60*10 ms */
 para_0021a1_no_periods_probe=5; /* 5*10 ms */

 para_0021a1_adapter_burst_start=(0.200001)*AUDIO_RATE; /* 200ms - Start of burst in CONTINUOUS adapter case */
 para_0021a1_adapter_total_duration=para_0021a1_ap_offset; /* 800ms - DISCRETE (e.g. quadruple) adapter case */
#ifdef PARA_0021A1_CONT_ADAPT
 para_0021a1_adapter_total_duration=(0.850001)*AUDIO_RATE; /* 850ms - CONTINUOUS adapter case */
#endif
 para_0021a1_probe_period=para_0021a1_click_period*para_0021a1_no_periods_probe; /* 50ms */
 para_0021a1_adapter_period0=para_0021a1_probe_period; /*  50ms - for DISCRETE (e.g. quadruple) adapter setting */
 para_0021a1_adapter_period1=para_0021a1_probe_period;
#ifdef PARA_0021A1_CONT_ADAPT
 para_0021a1_adapter_period1*=4; /* 200ms - for CONTINUOUS adapter setting --- */
#endif

 para_0021a1_iai=(0.200001)*AUDIO_RATE; /* Inter-adapter Interval: 1s -- never to happen for long single adapter case */;
 /* --------- */

 para_0021a1_lr_delta200=(0.00021)*AUDIO_RATE; /*  L-R delta: 200us - 10 steps */
 para_0021a1_lr_delta600=(0.00061)*AUDIO_RATE; /*  L-R delta: 600us - 30 steps */

 para_0021a1_stim_instant=(0.200001)*AUDIO_RATE; /* 200ms (arbitrary at beginning) */;
 para_0021a1_stim_instant_center=para_0021a1_stim_instant;
 para_0021a1_stim_instant_minus200=para_0021a1_stim_instant-para_0021a1_lr_delta200/2;
 para_0021a1_stim_instant_plus200=para_0021a1_stim_instant+para_0021a1_lr_delta200/2;
 para_0021a1_stim_instant_minus600=para_0021a1_stim_instant-para_0021a1_lr_delta600/2;
 para_0021a1_stim_instant_plus600=para_0021a1_stim_instant+para_0021a1_lr_delta600/2;

 rt_printk("%d %d %d %d %d %d %d %d %d %d\n",
		 	 para_0021a1_hi_period,
		 	 para_0021a1_click_period,
		 	 para_0021a1_lr_delta200,
		 	 para_0021a1_lr_delta600,
		 	 para_0021a1_stim_instant,
		 	 para_0021a1_stim_instant_center,
		 	 para_0021a1_stim_instant_minus200,
		 	 para_0021a1_stim_instant_minus600,
		 	 para_0021a1_stim_instant_plus200,
		 	 para_0021a1_stim_instant_plus600);

 lights_on();
 rt_printk("octopus-stim-backend.o: Stim initialized.\n");
}

static void para_0021a1_start(void) {
 lights_dimm();
 counter0=0; counter1=0; trigger_active=1; audio_active=1;
 rt_printk("octopus-stim-backend.o: Stim started.\n");
}

static void para_0021a1_stop(void) {
 audio_active=0;
 lights_on();
 rt_printk("octopus-stim-backend.o: Stim stopped.\n");
}

static void para_0021a1_pause(void) {
 audio_active=0;
 lights_on();
 rt_printk("octopus-stim-backend.o: Stim paused.\n");
}

static void para_0021a1_resume(void) {
 lights_dimm();
 counter0=0; counter1=0; audio_active=1;
 rt_printk("octopus-stim-backend.o: Stim resumed.\n");
}

static void para_0021a1(void) {
 //int dummy_counter=0;

 if (counter0==0) {
  current_pattern_data=patt_buf[current_pattern_offset]; /* fetch new.. */

  switch (current_pattern_data) {
   case '@': /* Interblock pause */
             para_0021a1_pause(); /* legitimate pause */
	     break;
   case 'A': /* Click Train L -> L600 */
	     para_0021a1_trigger=PARA_0021A1_L_L600;
	     para_0021a1_adapter_type=1; para_0021a1_probe_type=1;
	     para_0021a1_stim_instant_minus=para_0021a1_stim_instant_minus600;
	     para_0021a1_stim_instant_plus=para_0021a1_stim_instant_plus600;
             break;
   case 'B': /* Click Train L -> L200 */
	     para_0021a1_trigger=PARA_0021A1_L_L200;
	     para_0021a1_adapter_type=1; para_0021a1_probe_type=1;
	     para_0021a1_stim_instant_minus=para_0021a1_stim_instant_minus200;
	     para_0021a1_stim_instant_plus=para_0021a1_stim_instant_plus200;
             break;
   case 'C': /* Click Train L -> C */
	     para_0021a1_trigger=PARA_0021A1_L_C;
	     para_0021a1_adapter_type=1; para_0021a1_probe_type=0;
             break;
   case 'D': /* Click Train L -> R200 */
	     para_0021a1_trigger=PARA_0021A1_L_R200;
	     para_0021a1_adapter_type=1; para_0021a1_probe_type=2;
	     para_0021a1_stim_instant_minus=para_0021a1_stim_instant_minus200;
	     para_0021a1_stim_instant_plus=para_0021a1_stim_instant_plus200;
             break;
   case 'E': /* Click Train L -> R600 */
	     para_0021a1_trigger=PARA_0021A1_L_R600;
	     para_0021a1_adapter_type=1; para_0021a1_probe_type=2;
	     para_0021a1_stim_instant_minus=para_0021a1_stim_instant_minus600;
	     para_0021a1_stim_instant_plus=para_0021a1_stim_instant_plus600;
             break;
   case 'F': /* Click Train R -> L600 */
	     para_0021a1_trigger=PARA_0021A1_R_L600;
	     para_0021a1_adapter_type=2; para_0021a1_probe_type=1;
	     para_0021a1_stim_instant_minus=para_0021a1_stim_instant_minus600;
	     para_0021a1_stim_instant_plus=para_0021a1_stim_instant_plus600;
             break;
   case 'G': /* Click Train R -> L200 */
	     para_0021a1_trigger=PARA_0021A1_R_L200;
	     para_0021a1_adapter_type=2; para_0021a1_probe_type=1;
	     para_0021a1_stim_instant_minus=para_0021a1_stim_instant_minus200;
	     para_0021a1_stim_instant_plus=para_0021a1_stim_instant_plus200;
             break;
   case 'H': /* Click Train R -> C */
	     para_0021a1_trigger=PARA_0021A1_R_C;
	     para_0021a1_adapter_type=2; para_0021a1_probe_type=0;
             break;
   case 'I': /* Click Train R -> R200 */
	     para_0021a1_trigger=PARA_0021A1_R_R200;
	     para_0021a1_adapter_type=2; para_0021a1_probe_type=2;
	     para_0021a1_stim_instant_minus=para_0021a1_stim_instant_minus200;
	     para_0021a1_stim_instant_plus=para_0021a1_stim_instant_plus200;
             break;
   case 'J': /* Click Train R -> R600 */
	     para_0021a1_trigger=PARA_0021A1_R_R600;
	     para_0021a1_adapter_type=2; para_0021a1_probe_type=2;
	     para_0021a1_stim_instant_minus=para_0021a1_stim_instant_minus600;
	     para_0021a1_stim_instant_plus=para_0021a1_stim_instant_plus600;
             break;
   case 'K': /* Click Train C -> C -1 */
	     para_0021a1_trigger=PARA_0021A1_C_C;
	     para_0021a1_adapter_type=0; para_0021a1_probe_type=0;
             break;
   case 'L': /* Click Train C -> C -2 */
	     para_0021a1_trigger=PARA_0021A1_C_C;
	     para_0021a1_adapter_type=0; para_0021a1_probe_type=0;
   default:  break;
  }

  current_pattern_offset++;
  if (current_pattern_offset==pattern_size) { /* roll-back or stop? */
   if (para_0021a1_experiment_loop==0) para_0021a1_stop();
   //else para_0021a1_pause();
   current_pattern_offset=0;
  }
 }

 /* ------------------------------------------------------------------- */
 /* Trigger */
 if ( (counter0==para_0021a1_stim_instant+para_0021a1_ap_offset) && trigger_active &&
      (current_pattern_offset>0) )
  trigger_set(para_0021a1_trigger);
 /* ------------------------------------------------------------------- */

 /* Adapter region center */
 if (counter0 >= para_0021a1_stim_instant_center && \
     counter0 <  para_0021a1_stim_instant_center+para_0021a1_adapter_total_duration) {
  if (counter0 < para_0021a1_stim_instant_center+para_0021a1_adapter_burst_start) {
   if ((counter0-para_0021a1_stim_instant_center)%para_0021a1_iai \
 		                         < para_0021a1_adapter_period0) {
    para_0021a1_adapter_region_center=1;
   } else {
    para_0021a1_adapter_region_center=0;
   }
  } else {
   if ((counter0-para_0021a1_stim_instant_center)%para_0021a1_iai \
 		                         < para_0021a1_adapter_period1) {
    para_0021a1_adapter_region_center=1;
   } else {
    para_0021a1_adapter_region_center=0;
   }
  }
 } else {
  para_0021a1_adapter_region_center=0;
 }

 /* Adapter region (left) lead */
 if (counter0 >= para_0021a1_stim_instant_minus600 && \
     counter0 <  para_0021a1_stim_instant_minus600+para_0021a1_adapter_total_duration) {
  if (counter0 < para_0021a1_stim_instant_minus600+para_0021a1_adapter_burst_start) {
   if ((counter0-para_0021a1_stim_instant_minus600)%para_0021a1_iai \
		                         < para_0021a1_adapter_period0) {
    para_0021a1_adapter_region_lead=1;
   } else {
    para_0021a1_adapter_region_lead=0;
   }
  } else {
   if ((counter0-para_0021a1_stim_instant_minus600)%para_0021a1_iai \
		                         < para_0021a1_adapter_period1) {
    para_0021a1_adapter_region_lead=1;
   } else {
    para_0021a1_adapter_region_lead=0;
   }
  }
 } else {
  para_0021a1_adapter_region_lead=0;
 }

 /* Adapter region (left) lag */
 if (counter0 >= para_0021a1_stim_instant_plus600 && \
     counter0 <  para_0021a1_stim_instant_plus600+para_0021a1_adapter_total_duration) {
  if (counter0 < para_0021a1_stim_instant_plus600+para_0021a1_adapter_burst_start) {
   if ((counter0-para_0021a1_stim_instant_plus600)%para_0021a1_iai \
		                         < para_0021a1_adapter_period0) {
    para_0021a1_adapter_region_lag=1;
   } else {
    para_0021a1_adapter_region_lag=0;
   }
  } else {
   if ((counter0-para_0021a1_stim_instant_plus600)%para_0021a1_iai \
		                         < para_0021a1_adapter_period1) {
    para_0021a1_adapter_region_lag=1;
   } else {
    para_0021a1_adapter_region_lag=0;
   }
  }
 } else {
  para_0021a1_adapter_region_lag=0;
 }

 /* Probe region center */
 if (counter0 >= para_0021a1_stim_instant_center+para_0021a1_ap_offset && \
     counter0 <  para_0021a1_stim_instant_center+para_0021a1_ap_offset \
                                                 +para_0021a1_probe_period) {
  para_0021a1_probe_region_center=1;
 } else {
  para_0021a1_probe_region_center=0;
 }

 /* Probe region (left) lead */
 if (counter0 >= para_0021a1_stim_instant_minus+para_0021a1_ap_offset && \
     counter0 <  para_0021a1_stim_instant_minus+para_0021a1_ap_offset \
                                                 +para_0021a1_probe_period) {
  para_0021a1_probe_region_lead=1;
 } else {
  para_0021a1_probe_region_lead=0;
 }

 /* Probe region (left) lag */
 if (counter0 >= para_0021a1_stim_instant_plus+para_0021a1_ap_offset && \
     counter0 <  para_0021a1_stim_instant_plus+para_0021a1_ap_offset \
                                                 +para_0021a1_probe_period) {
  para_0021a1_probe_region_lag=1;
 } else {
  para_0021a1_probe_region_lag=0;
 }

 /* ------ */

 dac_0=dac_1=0;

 switch (para_0021a1_adapter_type) {
  case 0: /* Center */
   if (para_0021a1_adapter_region_center) {
    para_0021a1_stim_local_offset=counter0-para_0021a1_stim_instant%para_0021a1_probe_period;
    if (para_0021a1_stim_local_offset%para_0021a1_click_period < \
                                                    para_0021a1_hi_period)
     dac_0=dac_1=AMP_OPPCHN;
    else dac_0=dac_1=0;
   }
   break;
  case 1: /* Left Lead */
   if (para_0021a1_adapter_region_lead) {
    para_0021a1_stim_local_offset=counter0-para_0021a1_stim_instant_minus600;
    if (para_0021a1_stim_local_offset%para_0021a1_click_period < \
                                                    para_0021a1_hi_period)
     dac_0=AMP_OPPCHN;
    else dac_0=0;
   }
   if (para_0021a1_adapter_region_lag) {
    para_0021a1_stim_local_offset=counter0-para_0021a1_stim_instant_plus600;
    if (para_0021a1_stim_local_offset%para_0021a1_click_period < \
                                                    para_0021a1_hi_period)
     dac_1=AMP_OPPCHN;
    else dac_1=0;
   }
   break;
  case 2: /* Right Lead*/
   if (para_0021a1_adapter_region_lead) {
    para_0021a1_stim_local_offset=counter0-para_0021a1_stim_instant_minus600;
    if (para_0021a1_stim_local_offset%para_0021a1_click_period < \
                                                    para_0021a1_hi_period)
     dac_1=AMP_OPPCHN;
    else dac_1=0;
   }
   if (para_0021a1_adapter_region_lag) {
    para_0021a1_stim_local_offset=counter0-para_0021a1_stim_instant_plus600;
    if (para_0021a1_stim_local_offset%para_0021a1_click_period < \
                                                    para_0021a1_hi_period)
     dac_0=AMP_OPPCHN;
    else dac_0=0;
   }
   break;
 }

 switch (para_0021a1_probe_type) {
  case 0: /* Center */
   if (para_0021a1_probe_region_center) {
    para_0021a1_stim_local_offset=counter0-para_0021a1_stim_instant \
                                               -para_0021a1_ap_offset;
    if (para_0021a1_stim_local_offset%para_0021a1_click_period < \
                                                    para_0021a1_hi_period)
     dac_0=dac_1=AMP_OPPCHN;
    else dac_0=dac_1=0;
   }
   break;
  case 1: /* Left Lead (L600,L200) */
   if (para_0021a1_probe_region_lead) {
    para_0021a1_stim_local_offset=counter0-para_0021a1_stim_instant_minus \
                                               -para_0021a1_ap_offset;
    if (para_0021a1_stim_local_offset%para_0021a1_click_period < \
                                                    para_0021a1_hi_period)
     dac_0=AMP_OPPCHN;
    else dac_0=0;
   }
   if (para_0021a1_probe_region_lag) {
    para_0021a1_stim_local_offset=counter0-para_0021a1_stim_instant_plus \
                                               -para_0021a1_ap_offset;
    if (para_0021a1_stim_local_offset%para_0021a1_click_period < \
                                                    para_0021a1_hi_period)
     dac_1=AMP_OPPCHN;
    else dac_1=0;
   }
   break;
  case 2: /* Right Lead (R200,R600) */
   if (para_0021a1_probe_region_lead) {
    para_0021a1_stim_local_offset=counter0-para_0021a1_stim_instant_minus \
                                               -para_0021a1_ap_offset;
    if (para_0021a1_stim_local_offset%para_0021a1_click_period < \
                                                    para_0021a1_hi_period)
     dac_1=AMP_OPPCHN;
    else dac_1=0;
   }
   if (para_0021a1_probe_region_lag) {
    para_0021a1_stim_local_offset=counter0-para_0021a1_stim_instant_plus \
                                               -para_0021a1_ap_offset;
    if (para_0021a1_stim_local_offset%para_0021a1_click_period < \
                                                    para_0021a1_hi_period)
     dac_0=AMP_OPPCHN;
    else dac_0=0;
   }
  default: break;
 }

 /* ------------------------------------------------------------------- */

 counter0++;
 counter0%=para_0021a1_soa;
}
