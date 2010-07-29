#include <mach/am_regs.h>

#include <linux/amports/canvas.h>

#include "vframe.h"
#include "deinterlace.h"

#ifdef DEBUG
unsigned di_pre_underflow = 0, di_pre_overflow = 0;
unsigned long debug_array[4*1024];
#endif

#define PATTERN32_NUM		2
#define PATTERN22_NUM		32
#if (PATTERN22_NUM < 32)
#define PATTERN22_MARK		((1LL<<PATTERN22_NUM)-1)
#elif (PATTERN22_NUM < 64)
#define PATTERN22_MARK		((0x100000000LL<<(PATTERN22_NUM-32))-1)
#else
#define PATTERN22_MARK		0xffffffffffffffffLL
#endif

int prev_struct = 0;
int prog_field_count = 0;
int buf_recycle_done = 1;
int di_pre_post_done = 1;

int field_counter, pre_field_counter, di_checked_field;

int pattern_len;

int di_p32_counter;
unsigned int last_big_data, last_big_num;

unsigned long blend_mode, pattern_22, di_info[4][77];
unsigned long long di_p32_info, di_p22_info, di_p32_info_2, di_p22_info_2;

vframe_t *cur_buf;
vframe_t di_buf_pool[DI_BUF_NUM];

DI_MIF_t di_inp_top_mif;
DI_MIF_t di_inp_bot_mif;
DI_MIF_t di_mem_mif;
DI_MIF_t di_buf0_mif;
DI_MIF_t di_buf1_mif;
DI_MIF_t di_chan2_mif;
DI_SIM_MIF_t di_nrwr_mif;
DI_SIM_MIF_t di_mtnwr_mif;
DI_SIM_MIF_t di_mtncrd_mif;
DI_SIM_MIF_t di_mtnprd_mif;

void disable_deinterlace(void)
{
    WRITE_MPEG_REG(DI_PRE_CTRL, 0x3 << 30);
    WRITE_MPEG_REG(DI_POST_CTRL, 0x3 << 30);
    WRITE_MPEG_REG(DI_PRE_SIZE, (32-1) | ((64-1) << 16));
    WRITE_MPEG_REG(DI_POST_SIZE, (32-1) | ((128-1) << 16));
	WRITE_MPEG_REG(DI_INP_GEN_REG, READ_MPEG_REG(DI_INP_GEN_REG) & 0xfffffffe);
    WRITE_MPEG_REG(DI_MEM_GEN_REG, READ_MPEG_REG(DI_MEM_GEN_REG) & 0xfffffffe);
    WRITE_MPEG_REG(DI_CHAN2_GEN_REG, READ_MPEG_REG(DI_CHAN2_GEN_REG) & 0xfffffffe);
	WRITE_MPEG_REG(DI_IF1_GEN_REG, READ_MPEG_REG(DI_IF1_GEN_REG) & 0xfffffffe);
}

void disable_pre_deinterlace(void)
{
    unsigned status = READ_MPEG_REG(DI_PRE_CTRL) & 0x2;

	if ( prev_struct > 0 )
	{
		unsigned temp = READ_MPEG_REG(DI_PRE_SIZE);
		unsigned total = (temp & 0xffff) * ((temp>>16) & 0xffff);
		unsigned count = 0;
		
		while ( (READ_MPEG_REG(DI_INTR_CTRL) & 0xf) != (status | 0x9) )
		{
			if ( count++ >= total )
				break;
		}
		
		WRITE_MPEG_REG(DI_INTR_CTRL, READ_MPEG_REG(DI_INTR_CTRL));
	}

	WRITE_MPEG_REG(DI_INP_GEN_REG, READ_MPEG_REG(DI_INP_GEN_REG) & 0xfffffffe);
    WRITE_MPEG_REG(DI_MEM_GEN_REG, READ_MPEG_REG(DI_MEM_GEN_REG) & 0xfffffffe);
    WRITE_MPEG_REG(DI_CHAN2_GEN_REG, READ_MPEG_REG(DI_CHAN2_GEN_REG) & 0xfffffffe);

#ifdef DEBUG
	di_pre_underflow = 0;
	di_pre_overflow = 0;
#endif

   	prev_struct = 0;
   	prog_field_count = 0;
   	buf_recycle_done = 1;
   	di_pre_post_done = 1;

    WRITE_MPEG_REG(DI_PRE_CTRL, 0x3 << 30);
    WRITE_MPEG_REG(DI_PRE_SIZE, (32-1) | ((64-1) << 16));
}

void disable_post_deinterlace(void)
{
	WRITE_MPEG_REG(DI_POST_CTRL, 0x3 << 30);
    WRITE_MPEG_REG(DI_POST_SIZE, (32-1) | ((128-1) << 16));
	WRITE_MPEG_REG(DI_IF1_GEN_REG, READ_MPEG_REG(DI_IF1_GEN_REG) & 0xfffffffe);
}

void set_vd1_fmt_more (
		int hfmt_en,
        int hz_yc_ratio,        //2bit 
        int hz_ini_phase,       //4bit
        int vfmt_en,
        int vt_yc_ratio,        //2bit
        int vt_ini_phase,       //4bit
        int y_length,
        int c_length,
        int hz_rpt              //1bit
	)
{
    int vt_phase_step = (16 >> vt_yc_ratio);  

    WRITE_MPEG_REG(VIU_VD1_FMT_CTRL, (hz_rpt << 28) |  		// hz rpt pixel        
                              (hz_ini_phase << 24) |     	// hz ini phase
                              (0 << 23) |        			// repeat p0 enable
                              (hz_yc_ratio << 21) |     	// hz yc ratio
                              (hfmt_en << 20) |        		// hz enable
                              (1 << 17) |        			// nrpt_phase0 enable
                              (0 << 16) |        			// repeat l0 enable
                              (0 << 12) |        			// skip line num
                              (vt_ini_phase << 8) |     	// vt ini phase
                              (vt_phase_step << 1) |     	// vt phase step (3.4)
                              (vfmt_en << 0)             	// vt enable
                      	);
    
    WRITE_MPEG_REG(VIU_VD1_FMT_W, (y_length << 16) |    	// hz format width
                             (c_length << 0)      			// vt format width
                        );
}

void initial_di_prepost ( int hsize_pre, int vsize_pre, int hsize_post, int vsize_post, int hold_line ) 
{
   	WRITE_MPEG_REG(DI_PRE_SIZE, (hsize_pre -1 ) | ((vsize_pre -1) << 16));
  	WRITE_MPEG_REG(DI_POST_SIZE, (hsize_post -1) | ((vsize_post -1 ) << 16));
   	WRITE_MPEG_REG(DI_BLEND_CTRL, 
                      (0x2 << 20) |      				// top mode. EI only 
                       25);              				// KDEINT
   	WRITE_MPEG_REG(DI_EI_CTRL0, (255 << 16) |     		// ei_filter.
                      (5 << 8) |        				// ei_threshold.
                      (1 << 2) |         				// ei bypass cf2.
                      (1 << 1));        				// ei bypass far1
   	WRITE_MPEG_REG(DI_EI_CTRL1, (180 << 24) |      		// ei diff
                      (10 << 16) |       				// ei ang45
                      (15 << 8 ) |        				// ei peak.
                       45);             				// ei cross.
   	WRITE_MPEG_REG(DI_EI_CTRL2, (10 << 23) |       		// close2
                      (10 << 16) |       				// close1
                      (10 << 8 ) |       				// far2
                       10);             				// far1
    WRITE_MPEG_REG(DI_PRE_CTRL, 0 |       				// NR enable
                    (0 << 1 ) |        					// MTN_EN
                    (0 << 2 ) |        					// check 3:2 pulldown
                    (0 << 3 ) |        					// check 2:2 pulldown
                    (0 << 4 ) |        					// 2:2 check mid pixel come from next field after MTN.
                    (0 << 5 ) |        					// hist check enable
                    (0 << 6 ) |        					// hist check not use chan2.
                    (0 << 7 ) |        					// hist check use data before noise reduction.
                    (0 << 8 ) |        					// chan 2 enable for 2:2 pull down check.
                    (0 << 9) |        					// line buffer 2 enable
                    (0 << 10) |        					// pre drop first.
                    (0 << 11) |        					// pre repeat.
                    (1 << 12) |        					// pre viu link
                    (hold_line << 16) |      			// pre hold line number
                    (0 << 29) |        					// pre field number.
                    (0x3 << 30)      					// pre soft rst, pre frame rst.
       		); 
                  
     WRITE_MPEG_REG(DI_POST_CTRL, (0 << 0 ) |        // line buffer 0 enable
                      (0 << 1)  |        				// line buffer 1 enable
                      (0 << 2) |        				// ei  enable
                      (0 << 3) |        				// mtn line buffer enable
                      (0 << 4) |        				// mtnp read mif enable
                      (0 << 5) |        				// di blend enble.
                      (0 << 6) |        				// di mux output enable
                      (0 << 7) |        				// di write to SDRAM enable.
                      (1 << 8) |        				// di to VPP enable.
                      (0 << 9) |        				// mif0 to VPP enable.
                      (0 << 10) |        				// post drop first.
                      (0 << 11) |        				// post repeat.
                      (1 << 12) |        				// post viu link
                      (1 << 13) |        				// prepost_link
                      (hold_line<< 16)|       			// post hold line number
                      (0 << 29) |        				// post field number.
                      (0x3 << 30)       				// post soft rst  post frame rst.
        	);
                      
    WRITE_MPEG_REG(DI_MC_22LVL0, (READ_MPEG_REG(DI_MC_22LVL0) & 0xffff0000 ) | 256);     		// field 22 level 
    WRITE_MPEG_REG(DI_MC_32LVL0, (READ_MPEG_REG(DI_MC_32LVL0) & 0xffffff00 ) | 16);       		// field 32 level

	// set hold line for all ddr req interface.
    WRITE_MPEG_REG(DI_INP_GEN_REG, (hold_line << 19)); 
    WRITE_MPEG_REG(DI_MEM_GEN_REG, (hold_line << 19)); 
    WRITE_MPEG_REG(VD1_IF0_GEN_REG, (hold_line << 19));  
    WRITE_MPEG_REG(DI_IF1_GEN_REG, (hold_line << 19));  
    WRITE_MPEG_REG(DI_CHAN2_GEN_REG, (hold_line << 19));   
}

void initial_di_pre ( int hsize_pre, int vsize_pre, int hold_line )
{
   	WRITE_MPEG_REG(DI_PRE_SIZE, (hsize_pre -1 ) | ((vsize_pre -1) << 16) );
   	WRITE_MPEG_REG(DI_PRE_CTRL, 0 |        		// NR enable
                    (0 << 1 ) |        			// MTN_EN
                    (0 << 2 ) |        			// check 3:2 pulldown
                    (0 << 3 ) |        			// check 2:2 pulldown
                    (0 << 4 ) |        			// 2:2 check mid pixel come from next field after MTN.
                    (0 << 5 ) |        			// hist check enable
                    (0 << 6 ) |        			// hist check not use chan2.
                    (0 << 7 ) |        			// hist check use data before noise reduction.
                    (0 << 8 ) |        			// chan 2 enable for 2:2 pull down check.
                    (0 << 9 ) |        			// line buffer 2 enable
                    (0 << 10) |        			// pre drop first.
                    (0 << 11) |        			// pre repeat.
                    (0 << 12) |        			// pre viu link
                    (hold_line << 16) |      	// pre hold line number
                    (0 << 29) |        			// pre field number.
                    (0x3 << 30)      			// pre soft rst, pre frame rst.
           	); 

    WRITE_MPEG_REG(DI_MC_22LVL0, (READ_MPEG_REG(DI_MC_22LVL0) & 0xffff0000 ) | 256);                //   field 22 level 
    WRITE_MPEG_REG(DI_MC_32LVL0, (READ_MPEG_REG(DI_MC_32LVL0) & 0xffffff00 ) | 16);       				// field 32 level
}

void initial_di_post ( int hsize_post, int vsize_post, int hold_line ) 
{
   	WRITE_MPEG_REG(DI_POST_SIZE, (hsize_post -1) | ((vsize_post -1 ) << 16));
   	WRITE_MPEG_REG(DI_BLEND_CTRL, 
                      (0x2 << 20) |      				// top mode. EI only 
                       25);              				// KDEINT
   	WRITE_MPEG_REG(DI_EI_CTRL0, (255 << 16) |     		// ei_filter.
                      (5 << 8) |        				// ei_threshold.
                      (1 << 2) |         				// ei bypass cf2.
                      (1 << 1));        				// ei bypass far1
   	WRITE_MPEG_REG(DI_EI_CTRL1, (180 << 24) |      		// ei diff
                      (10 << 16) |       				// ei ang45
                      (15 << 8 ) |        				// ei peak.
                       45);             				// ei cross.
   	WRITE_MPEG_REG(DI_EI_CTRL2, (10 << 23) |       		// close2
                      (10 << 16) |       				// close1
                      (10 << 8 ) |       				// far2
                       10);             				// far1
   	WRITE_MPEG_REG(DI_POST_CTRL, (0 << 0 ) |        		// line buffer 0 enable
                      (0 << 1)  |        				// line buffer 1 enable
                      (0 << 2)  |        				// ei  enable
                      (0 << 3)  |        				// mtn line buffer enable
                      (0 << 4)  |        				// mtnp read mif enable
                      (0 << 5)  |        				// di blend enble.
                      (0 << 6)  |        				// di mux output enable
                      (0 << 7)  |        				// di write to SDRAM enable.
                      (1 << 8)  |        				// di to VPP enable.
                      (0 << 9)  |        				// mif0 to VPP enable.
                      (0 << 10) |        				// post drop first.
                      (0 << 11) |        				// post repeat.
                      (1 << 12) |        				// post viu link
                      (hold_line << 16) |      			// post hold line number
                      (0 << 29) |        				// post field number.
                      (0x3 << 30)       				// post soft rst  post frame rst.
        );
}

void enable_di_mode_check( int win0_start_x, int win0_end_x, int win0_start_y, int win0_end_y,
                        int win1_start_x, int win1_end_x, int win1_start_y, int win1_end_y,
                        int win2_start_x, int win2_end_x, int win2_start_y, int win2_end_y,
                        int win3_start_x, int win3_end_x, int win3_start_y, int win3_end_y,
                        int win4_start_x, int win4_end_x, int win4_start_y, int win4_end_y,
                        int win0_32lvl,   int win1_32lvl, int win2_32lvl, int win3_32lvl, int win4_32lvl,
                        int win0_22lvl,   int win1_22lvl, int win2_22lvl, int win3_22lvl, int win4_22lvl,
                        int field_32lvl,  int field_22lvl)
{
    WRITE_MPEG_REG(DI_MC_REG0_X, (win0_start_x <<16) |     		// start_x
                       win0_end_x );       						// end_x
    WRITE_MPEG_REG(DI_MC_REG0_Y, (win0_start_y <<16) |     		// start_y
                       win0_end_y );        					// end_x
    WRITE_MPEG_REG(DI_MC_REG1_X, (win1_start_x <<16) |     		// start_x
                       win1_end_x );       						// end_x
    WRITE_MPEG_REG(DI_MC_REG1_Y, (win1_start_y <<16) |     		// start_y
                       win1_end_y );        					// end_x
    WRITE_MPEG_REG(DI_MC_REG2_X, (win2_start_x <<16) |     		// start_x
                       win2_end_x );       						// end_x
    WRITE_MPEG_REG(DI_MC_REG2_Y, (win2_start_y <<16) |     		// start_y
                       win2_end_y );        					// end_x
    WRITE_MPEG_REG(DI_MC_REG3_X, (win3_start_x <<16) |     		// start_x
                       win3_end_x );       						// end_x
    WRITE_MPEG_REG(DI_MC_REG3_Y, (win3_start_y <<16) |     		// start_y
                       win3_end_y );        					// end_x
    WRITE_MPEG_REG(DI_MC_REG4_X, (win4_start_x <<16) |     		// start_x
                       win4_end_x );       						// end_x
    WRITE_MPEG_REG(DI_MC_REG4_Y, (win4_start_y <<16) |     		// start_y
                       win4_end_y );        					// end_x

    WRITE_MPEG_REG(DI_MC_32LVL1, win3_32lvl |          			//region 3
                     (win4_32lvl << 8));   						//region 4
    WRITE_MPEG_REG(DI_MC_32LVL0, field_32lvl        |   		//field 32 level
                     (win0_32lvl << 8)  |   					//region 0
                     (win1_32lvl << 16) |   					//region 1
                     (win2_32lvl << 24));  						//region 2.
    WRITE_MPEG_REG(DI_MC_22LVL0,  field_22lvl  |           		// field 22 level
                     (win0_22lvl << 16));   					// region 0.

    WRITE_MPEG_REG(DI_MC_22LVL1,  win1_22lvl  |           		// region 1
                     (win2_22lvl << 16));   					// region 2.

    WRITE_MPEG_REG(DI_MC_22LVL2, win3_22lvl  |           		// region 3
                     (win4_22lvl << 16));   					// region 4.
    WRITE_MPEG_REG(DI_MC_CTRL, 0x1f);            				// enable region level
}

// the input data is 4:2:2,  saved in field mode video.
void enable_di_prepost_simple ( int cav_inp_num,     int cav_mem_num, int cav_chan2_num, 
                         int cav_disp_num,    int cav_nrwr_num, int cav_diwr_num, 
                         int cav_mtn_inp_num, int cav_mtn_cur_num,
                         int start_x, int end_x, int start_y, int end_y,
                         int nr_en, int mtn_en, int pd32_check_en, int pd22_check_en, int hist_check_en, 
                         int ei_en, int blend_en, int blend_mtn_en, int di_vpp_en, int di_ddr_en,
                         int post_field_num, int pre_field_num, int prepost_link,
                         int hold_line)
{
 	int hist_check_only;
 	int ei_only;

  	hist_check_only = hist_check_en && !nr_en && !mtn_en && !pd22_check_en && !pd32_check_en ; 
  	ei_only = ei_en && !blend_en && (di_vpp_en || di_ddr_en );

 	if ( nr_en | mtn_en | pd22_check_en || pd32_check_en ) 
 	{
    	set_di_inp_combined_simple(
                start_x,
                end_x,
                start_y,
                end_y,
                0,           							// 1 = RGB (3 bytes per pixel), 0 = 4:2:2 (2 bytes per pixel)
                prepost_link && di_vpp_en, 
                cav_inp_num, 
                hold_line );
    	set_di_inp_fmt_more ( 1,                		// hfmt_en
                        1,                				// hz_yc_ratio
                        0,                				// hz_ini_phase
                        0,                				// vfmt_en
                        0,                				// vt_yc_ratio
                        0,                				// vt_ini_phase
                        end_x - start_x + 1, 			// y_length
                        ((end_x - start_x + 1) >> 1), 	// c length 
                        0 );                 			// hz repeat.

   		set_di_mem_combined_simple(
                start_x,
                end_x,
                start_y,
                end_y,
                0,           							// 1 = RGB (3 bytes per pixel), 0 = 4:2:2 (2 bytes per pixel)
                prepost_link && di_vpp_en,
                cav_mem_num,
                hold_line );
   		set_di_mem_fmt_more ( 1,                		// hfmt_en
                        1,                				// hz_yc_ratio
                        0,                				// hz_ini_phase
                        0,                				// vfmt_en
                        0,               	 			// vt_yc_ratio
                        0,                				// vt_ini_phase
                        end_x - start_x + 1, 			// y_length
                        ((end_x - start_x + 1) >> 1), 	// c length 
                        0 );                 			// hz repeat.
   	}

   	if ( pd22_check_en || hist_check_only ) 
   	{ 
        set_di_chan2_combined_simple(
                start_x,
                end_x,
                start_y,
                end_y,
                0,           							// 1 = RGB (3 bytes per pixel), 0 = 4:2:2 (2 bytes per pixel)
                prepost_link && di_vpp_en,
                cav_chan2_num,
                hold_line );
   	}

   	set_di_if0_combined_simple(
                start_x,
                end_x,
                start_y,
                end_y,
                0,           							// 1 = RGB (3 bytes per pixel), 0 = 4:2:2 (2 bytes per pixel)
                di_vpp_en,
                cav_disp_num,
                hold_line );
   	set_vd1_fmt_more ( 1,                				// hfmt_en
                        1,                				// hz_yc_ratio
                        0,                				// hz_ini_phase
                        0,                				// vfmt_en
                        0,                				// vt_yc_ratio
                        0,                				// vt_ini_phase
                        end_x - start_x + 1, 			// y_length
                        ((end_x - start_x + 1) >> 1), 	//c length 
                        0 );                 			// hz repeat.

  	// set nr wr mif interface.
   	if ( nr_en ) 
   	{
     	WRITE_MPEG_REG(DI_NRWR_X, (start_x <<16) | (end_x));   				// start_x 0 end_x 719.
     	WRITE_MPEG_REG(DI_NRWR_Y, (start_y <<16) | (end_y));             	// start_y 0 end_y 239.
     	WRITE_MPEG_REG(DI_NRWR_CTRL, cav_nrwr_num |     					// canvas index.
                       ((prepost_link && di_vpp_en) << 8));        			// urgent.
   	}

   	// motion wr mif.
    if (mtn_en ) 
    {
       	WRITE_MPEG_REG(DI_MTNWR_X, (start_x <<16) | (end_x));   			// start_x 0 end_x 719.
       	WRITE_MPEG_REG(DI_MTNWR_Y, (start_y <<16) | (end_y));            	// start_y 0 end_y 239.
       	WRITE_MPEG_REG(DI_MTNWR_CTRL, cav_mtn_inp_num |  					// canvas index.
                      ((prepost_link && di_vpp_en) << 8));       			// urgent.
    }

   	// motion for current display field.
    if ( blend_mtn_en ) 
    {
      	WRITE_MPEG_REG(DI_MTNCRD_X, (start_x <<16) | (end_x));   			// start_x 0 end_x 719.
      	WRITE_MPEG_REG(DI_MTNCRD_Y, (start_y <<16) | (end_y));           	// start_y 0 end_y 239.
      	WRITE_MPEG_REG(DI_MTNRD_CTRL, (0 <<8 )     |          				//mtnp canvas index.
                        (di_vpp_en << 16)     |          					// urgent
                        cav_mtn_cur_num );      							// current field mtn canvas index. 
    }

    if ( di_ddr_en ) 
    {
       	WRITE_MPEG_REG(DI_DIWR_X, (start_x <<16) | (end_x));   				// start_x 0 end_x 719.
       	WRITE_MPEG_REG(DI_DIWR_Y, (start_y <<16) | (end_y *2 + 1 ));    	// start_y 0 end_y 479.
       	WRITE_MPEG_REG(DI_DIWR_CTRL, cav_diwr_num |               			// canvas index.
                       (di_vpp_en << 8));            						// urgent.
    } 

    WRITE_MPEG_REG(DI_PRE_CTRL, nr_en |        								// NR enable
                    (mtn_en << 1 ) |        								// MTN_EN
                    (pd32_check_en << 2 ) |        							// check 3:2 pulldown
                    (pd22_check_en << 3 ) |        							// check 2:2 pulldown
                    (1 << 4 ) |        										// 2:2 check mid pixel come from next field after MTN.
                    (hist_check_en << 5 ) |        							// hist check enable
                    (0 << 6 ) |        										// hist check not use chan2.
                    ((!nr_en) << 7 ) |        								// hist check use data before noise reduction.
                    (pd22_check_en << 8 ) |        							// chan 2 enable for 2:2 pull down check.
                    (pd22_check_en << 9) |        							// line buffer 2 enable
                    (0 << 10) |        										// pre drop first.
                    (0 << 11) |        										// pre repeat.
                    (di_vpp_en << 12) |        								// pre viu link
                    (hold_line << 16) |      								// pre hold line number
                    (pre_field_num << 29) |        							// pre field number.
                    (0x1 << 30 )      										// pre soft rst, pre frame rst.
                   ); 
                  
     WRITE_MPEG_REG(DI_POST_CTRL, ((ei_en | blend_en) << 0 ) |        		// line buffer 0 enable
                      (0 << 1)  |        									// line buffer 1 enable
                      (ei_en << 2) |        								// ei  enable
                      (blend_mtn_en << 3) |        							// mtn line buffer enable
                      ((blend_mtn_en && !prepost_link) << 4) |        		// mtnp read mif enable
                      (blend_en << 5) |        								// di blend enble.
                      (1 << 6) |        									// di mux output enable
                      (di_ddr_en << 7) |        							// di write to SDRAM enable.
                      (di_vpp_en << 8) |        							// di to VPP enable.
                      (0 << 9) |        									// mif0 to VPP enable.
                      (0 << 10) |        									// post drop first.
                      (0 << 11) |        									// post repeat.
                      (1 << 12) |        									// post viu link
                      (prepost_link << 13) |  								// prepost_link
                      (hold_line << 16)|       								// post hold line number
                      (post_field_num << 29) |        						// post field number.
                      (0x1 << 30 )       									// post soft rst  post frame rst.
        );

   	if ( ei_only == 0) 
   	{
           WRITE_MPEG_REG(DI_BLEND_CTRL, (READ_MPEG_REG(DI_BLEND_CTRL) & (~((1 << 25) | (3 << 20 )))) | 		// clean some bit we need to set. 
                              (blend_mtn_en << 26 ) |   														// blend mtn enable.
                              (0 << 25 ) |   																	// blend with the mtn of the pre display field and next display field.
                              (1 << 24 ) |   																	// blend with pre display field.
                              (0x3 << 20)    																	// motion adaptive blend.
               ); 
   	} 
}

// handle all case of prepost link. 
void enable_di_prepost_full (  
   	DI_MIF_t        *di_inp_mif,
   	DI_MIF_t        *di_mem_mif,
   	DI_MIF_t        *di_buf0_mif,
   	DI_MIF_t        *di_buf1_mif,
   	DI_MIF_t        *di_chan2_mif,
   	DI_SIM_MIF_t    *di_nrwr_mif,
   	DI_SIM_MIF_t    *di_diwr_mif,
   	DI_SIM_MIF_t    *di_mtnwr_mif,
   	DI_SIM_MIF_t    *di_mtncrd_mif,
   	DI_SIM_MIF_t    *di_mtnprd_mif,
   	int nr_en, int mtn_en, int pd32_check_en, int pd22_check_en, int hist_check_en, 
   	int ei_en, int blend_en, int blend_mtn_en, int blend_mode, int di_vpp_en, int di_ddr_en,
   	int post_field_num, int pre_field_num, int prepost_link, int hold_line )
{
  	int hist_check_only;
  	int ei_only;
  	int buf1_en;

  	hist_check_only = hist_check_en && !nr_en && !mtn_en && !pd22_check_en && !pd32_check_en; 
  	ei_only = ei_en && !blend_en && (di_vpp_en || di_ddr_en );
  	buf1_en =  ( !prepost_link && !ei_only && (di_ddr_en || di_vpp_en )); 

  	if ( nr_en | mtn_en | pd22_check_en || pd32_check_en ) 
  	{
       	set_di_inp_mif( di_inp_mif, di_vpp_en && prepost_link , hold_line);
       	set_di_mem_mif( di_mem_mif, di_vpp_en && prepost_link, hold_line);
  	}

  	if ( pd22_check_en || hist_check_only ) 
  	{ 
       	set_di_chan2_mif(  di_chan2_mif, di_vpp_en && prepost_link, hold_line);
  	}

  	if ( ei_en || di_vpp_en || di_ddr_en ) 
  	{ 
     	set_di_if0_mif( di_buf0_mif, di_vpp_en, hold_line);
  	}

  	if ( !prepost_link && !ei_only && (di_ddr_en || di_vpp_en ) ) 
  	{
     	set_di_if1_mif( di_buf1_mif, di_vpp_en, hold_line);
  	} 

  	// set nr wr mif interface.
   	if ( nr_en ) 
   	{
     	WRITE_MPEG_REG(DI_NRWR_X, (di_nrwr_mif->start_x <<16) | (di_nrwr_mif->end_x));   	// start_x 0 end_x 719.
     	WRITE_MPEG_REG(DI_NRWR_Y, (di_nrwr_mif->start_y <<16) | (di_nrwr_mif->end_y));   	// start_y 0 end_y 239.
     	WRITE_MPEG_REG(DI_NRWR_CTRL, di_nrwr_mif->canvas_num |     							// canvas index.
                       ((prepost_link && di_vpp_en) << 8));        							// urgent.
   	}

   	// motion wr mif.
    if ( mtn_en ) 
    {
       	WRITE_MPEG_REG(DI_MTNWR_X, (di_mtnwr_mif->start_x <<16) | (di_mtnwr_mif->end_x));   	// start_x 0 end_x 719.
       	WRITE_MPEG_REG(DI_MTNWR_Y, (di_mtnwr_mif->start_y <<16) | (di_mtnwr_mif->end_y));   	// start_y 0 end_y 239.
       	WRITE_MPEG_REG(DI_MTNWR_CTRL, di_mtnwr_mif->canvas_num |  								// canvas index.
                      ( (prepost_link && di_vpp_en) << 8));       								// urgent.
    }

   	// motion for current display field.
    if ( blend_mtn_en ) 
    {
      	WRITE_MPEG_REG(DI_MTNCRD_X,   (di_mtncrd_mif->start_x <<16) | (di_mtncrd_mif->end_x));   		// start_x 0 end_x 719.
      	WRITE_MPEG_REG(DI_MTNCRD_Y,   (di_mtncrd_mif->start_y <<16) | (di_mtncrd_mif->end_y));       	// start_y 0 end_y 239.
      	if (!prepost_link) 
      	{
          	WRITE_MPEG_REG(DI_MTNRD_CTRL, (di_mtnprd_mif->canvas_num <<8 ) |          					//mtnp canvas index.
                            (0 << 16) |          														// urgent
                            di_mtncrd_mif->canvas_num );                    							// current field mtn canvas index. 
      	} 
      	else 
      	{
          	WRITE_MPEG_REG(DI_MTNRD_CTRL, (di_mtnprd_mif->canvas_num <<8 ) |          					//mtnp canvas index.
                            ((prepost_link && di_vpp_en) << 16)  |          							// urgent
                            di_mtncrd_mif->canvas_num );                   							 	// current field mtn canvas index. 
      	}
    }

    if ( blend_mtn_en && !prepost_link ) 
    {
        WRITE_MPEG_REG(DI_MTNPRD_X, (di_mtnprd_mif->start_x <<16) | (di_mtnprd_mif->end_x));   			// start_x 0 end_x 719.
        WRITE_MPEG_REG(DI_MTNPRD_Y, (di_mtnprd_mif->start_y <<16) | (di_mtnprd_mif->end_y));   			// start_y 0 end_y 239.
    }

    if ( di_ddr_en ) 
    {
       	WRITE_MPEG_REG(DI_DIWR_X, (di_diwr_mif->start_x <<16) | (di_diwr_mif->end_x));   				// start_x 0 end_x 719.
       	WRITE_MPEG_REG(DI_DIWR_Y, (di_diwr_mif->start_y <<16) | (di_diwr_mif->end_y *2 + 1 ));         // start_y 0 end_y 479.
       	WRITE_MPEG_REG(DI_DIWR_CTRL, di_diwr_mif->canvas_num |               							// canvas index.
                        (di_vpp_en << 8));            													// urgent.
    } 

    WRITE_MPEG_REG(DI_PRE_CTRL, nr_en |        					// NR enable
                    (mtn_en << 1 ) |        					// MTN_EN
                    (pd32_check_en << 2 ) |        				// check 3:2 pulldown
                    (pd22_check_en << 3 ) |        				// check 2:2 pulldown
                    (1 << 4 ) |        							// 2:2 check mid pixel come from next field after MTN.
                    (hist_check_en << 5 ) |        				// hist check enable
                    (0 << 6 ) |        							// hist check not use chan2.
                    ((!nr_en) << 7 ) |        					// hist check use data before noise reduction.
                    (pd22_check_en << 8 ) |        				// chan 2 enable for 2:2 pull down check.
                    (pd22_check_en << 9) |        				// line buffer 2 enable
                    (0 << 10) |        							// pre drop first.
                    (0 << 11) |        							// pre repeat.
                    (di_vpp_en << 12) |        					// pre viu link
                    (hold_line << 16) |      					// pre hold line number
                    (pre_field_num << 29) |        				// pre field number.
                    (0x1 << 30 )      							// pre soft rst, pre frame rst.
          	); 
                  
    WRITE_MPEG_REG(DI_POST_CTRL, ((ei_en || di_vpp_en || di_ddr_en) << 0 ) |       			// line buffer 0 enable
                      (buf1_en << 1)  |        												// line buffer 1 enable
                      (ei_en << 2) |        												// ei  enable
                      (blend_mtn_en << 3) |        											// mtn line buffer enable
                      ((blend_mtn_en && !prepost_link) << 4) |        						// mtnp read mif enable
                      (blend_en << 5) |        												// di blend enble.
                      (1 << 6) |        													// di mux output enable
                      (di_ddr_en << 7) |        											// di write to SDRAM enable.
                      (di_vpp_en << 8) |        											// di to VPP enable.
                      (0 << 9) |        													// mif0 to VPP enable.
                      (0 << 10) |        													// post drop first.
                      (0 << 11) |        													// post repeat.
                      (1 << 12) |        													// post viu link
                      (prepost_link << 13) |
                      (hold_line << 16)|       												// post hold line number
                      (post_field_num << 29) |        										// post field number.
                      (0x1 << 30 )       													// post soft rst  post frame rst.
        	);


   	if ( ei_only == 0) 
   	{
     	WRITE_MPEG_REG(DI_BLEND_CTRL, (READ_MPEG_REG(DI_BLEND_CTRL) & (~((1 << 25) | (3 << 20 )))) | 	// clean some bit we need to set. 
                              (blend_mtn_en << 26 ) |   													// blend mtn enable.
                              (0 << 25 ) |   																// blend with the mtn of the pre display field and next display field.
                              (1 << 24 ) |   																// blend with pre display field.
                              (blend_mode << 20)    														// motion adaptive blend.
               ); 
   	}
}

int di_mode_check(int cur_field)
{
    int i;

    WRITE_MPEG_REG(DI_INFO_ADDR, 0 );
    for ( i  = 0; i <= 76; i++ ) 
    {   
       	di_info[cur_field][i] = READ_MPEG_REG(DI_INFO_DATA);
    }

   	WRITE_MPEG_REG(DI_PRE_CTRL, READ_MPEG_REG(DI_PRE_CTRL) | ( 0x1 << 30 ) );       		// pre soft rst 
   	WRITE_MPEG_REG(DI_POST_CTRL, READ_MPEG_REG(DI_POST_CTRL) | ( 0x1 << 30 ) );       	// post soft rst 

    return (0);
}

void set_di_inp_fmt_more (int hfmt_en,
                int hz_yc_ratio,        //2bit 
                int hz_ini_phase,       //4bit
                int vfmt_en,
                int vt_yc_ratio,        //2bit
                int vt_ini_phase,       //4bit
                int y_length,
                int c_length,
                int hz_rpt              //1bit
    	)
{
    int vt_phase_step = (16 >> vt_yc_ratio);  

    WRITE_MPEG_REG(DI_INP_FMT_CTRL,      
                              (hz_rpt << 28) 		|    		//hz rpt pixel        
                              (hz_ini_phase << 24) 	|     		//hz ini phase
                              (0 << 23)         	|        	//repeat p0 enable
                              (hz_yc_ratio << 21)  	|     		//hz yc ratio
                              (hfmt_en << 20)   	|        	//hz enable
                              (1 << 17)         	|        	//nrpt_phase0 enable
                              (0 << 16)         	|        	//repeat l0 enable
                              (0 << 12)         	|        	//skip line num
                              (vt_ini_phase << 8)  	|     		//vt ini phase
                              (vt_phase_step << 1) 	|     		//vt phase step (3.4)
                              (vfmt_en << 0)             		//vt enable
              		);
    
    WRITE_MPEG_REG(DI_INP_FMT_W, (y_length << 16) |        		//hz format width
                             (c_length << 0)                  	//vt format width
                 	);
}

void set_di_inp_combined_simple(
	unsigned long   x_start,
	unsigned long   x_end,
	unsigned long   y_start,
	unsigned long   y_end,
	unsigned long   mode,           	// 1 = RGB (3 bytes per pixel), 0 = 4:2:2 (2 bytes per pixel)
	unsigned long   urgent,           	// urgent request
	unsigned long   canvas,
	int             hold_line )
{
    // General register setup
    unsigned long   demux_mode   	= mode; 						// 0 = 4:2:2 demux, 1 = RGB demuxing from a single FIFO
    unsigned long   bytes_per_pixel	= (mode  == 1) ? 2 : 1;     	// RGB = 3 bytes per
    unsigned long   burst_size_cr  	= 0;    						// unused
    unsigned long   burst_size_cb  	= 0;    						// unused
    unsigned long   burst_size_y   	= 1;    						// 64x64 burst size
    unsigned long   st_separate_en 	= 0;

    // ----------------------
    // General register
    // ----------------------

    WRITE_MPEG_REG(DI_INP_GEN_REG, (hold_line << 19) 	|   		//hold lines
                                (urgent << 28)         	|   		// urgent  
                                (urgent << 27)        	|  			// luma urgent 
                                (1 << 18)               |   		// push pixel value
                                (demux_mode << 16)   	|
                                (bytes_per_pixel << 14)	|
                                (burst_size_cr << 12)  	|
                                (burst_size_cb << 10)  	|
                                (burst_size_y << 8)    	|
                                (0 << 6)                |   		// TODO: cntl_chro_rpt_lastl_ctrl
                                (0 << 4)                |   		// TODO: cntl_vt_yc_ratio
                                (0 << 2)                |   		// TODO: cntl_hz_yc_ratio
                                (st_separate_en << 1)  	|
                                (0 << 0)                    		// cntl_enable (don't enable just yet)
      	);

    // ----------------------
    // Canvas
    // ----------------------
    WRITE_MPEG_REG(DI_INP_CANVAS0, (0 << 16) |   					// cntl_canvas0_addr2
                                (0 << 8) |   						// cntl_canvas0_addr1
                                (canvas << 0)               		// cntl_canvas0_addr0
    	);
    
    // ----------------------
    // Picture 0 X/Y start,end
    // ----------------------
    WRITE_MPEG_REG(DI_INP_LUMA_X0, (x_end << 16) |   				// cntl_luma_x_end0
                                (x_start << 0)              		// cntl_luma_x_start0
    	);
    WRITE_MPEG_REG(DI_INP_LUMA_Y0, (y_end << 16) |   				// cntl_luma_y_end0
                                (y_start << 0)              		// cntl_luma_y_start0
    	);
    WRITE_MPEG_REG(DI_INP_CHROMA_X0,     0);                     // unused
    WRITE_MPEG_REG(DI_INP_CHROMA_Y0,     0);                     // unused


    // ----------------------
    // No Repeat or skip
    // ----------------------
    WRITE_MPEG_REG(DI_INP_RPT_LOOP, (0 << 24)	|   				// cntl_chroma1_rpt_loop
                                (0 << 16)      	|   				// cntl_luma1_rpt_loop
                                (0 << 8)    	|   				// cntl_chroma0_rpt_loop
                                (0 << 0)                    		// cntl_luma0_rpt_loop
    );

    WRITE_MPEG_REG(DI_INP_LUMA0_RPT_PAT, 0);                   		// no skip /repeat
    WRITE_MPEG_REG(DI_INP_CHROMA0_RPT_PAT, 0);           			// unused


    WRITE_MPEG_REG(DI_INP_DUMMY_PIXEL, 0xAABBCC00);        			// RGB = AABBCC or YCb = AABB, YCr = AACC

    // Enable VD1: vd_di_inp
    WRITE_MPEG_REG(DI_INP_GEN_REG, (READ_MPEG_REG(DI_INP_GEN_REG) | (1 << 0)));     			// cntl_enable
}

void set_di_inp_mif ( DI_MIF_t *mif, int urgent,int hold_line)
{
    unsigned long bytes_per_pixel;  
    unsigned long demux_mode;
    unsigned long chro_rpt_lastl_ctrl;
    unsigned long luma0_rpt_loop_start;
    unsigned long luma0_rpt_loop_end;
    unsigned long luma0_rpt_loop_pat;
    unsigned long chroma0_rpt_loop_start;
    unsigned long chroma0_rpt_loop_end;
    unsigned long chroma0_rpt_loop_pat;

    if ( mif->set_separate_en == 1 && mif->src_field_mode == 1 ) 
    {
      	chro_rpt_lastl_ctrl =1;
      	luma0_rpt_loop_start = 1;
      	luma0_rpt_loop_end = 1;
      	chroma0_rpt_loop_start = 1;
      	chroma0_rpt_loop_end = 1;
        luma0_rpt_loop_pat = 0x80;
        chroma0_rpt_loop_pat = 0x80;
    } 
    else if ( mif->set_separate_en == 1 && mif->src_field_mode == 0 ) 
    {
      	chro_rpt_lastl_ctrl =1;
      	luma0_rpt_loop_start = 0;
      	luma0_rpt_loop_end = 0;
      	chroma0_rpt_loop_start = 0;
      	chroma0_rpt_loop_end = 0;
      	luma0_rpt_loop_pat = 0x0;
      	chroma0_rpt_loop_pat = 0x0;
    } 
    else if ( mif->set_separate_en == 0 && mif->src_field_mode == 1 ) 
    {
      	chro_rpt_lastl_ctrl =1;
      	luma0_rpt_loop_start = 1;
      	luma0_rpt_loop_end = 1;
      	chroma0_rpt_loop_start = 0;
      	chroma0_rpt_loop_end = 0;
        luma0_rpt_loop_pat = 0x80;
        chroma0_rpt_loop_pat = 0x00;
    } 
    else 
    {
      	chro_rpt_lastl_ctrl =0;
      	luma0_rpt_loop_start = 0;
      	luma0_rpt_loop_end = 0;
      	chroma0_rpt_loop_start = 0;
      	chroma0_rpt_loop_end = 0;
      	luma0_rpt_loop_pat = 0x00;
      	chroma0_rpt_loop_pat = 0x00;
    }
     
    
    bytes_per_pixel = mif->set_separate_en ? 0 : (mif->video_mode ? 2 : 1);
    demux_mode = mif->video_mode;


    // ----------------------
    // General register
    // ----------------------

    WRITE_MPEG_REG(DI_INP_GEN_REG, (urgent << 28)			| 		// chroma urgent bit 
                                (urgent << 27)          	| 		// luma urgent bit. 
                                (1 << 25)                  	| 		// no dummy data. 
                                (hold_line << 19)       	| 		// hold lines
                                (1 << 18)                 	| 		// push dummy pixel
                                (demux_mode << 16)      	| 		// demux_mode
                                (bytes_per_pixel << 14)    	| 
                                (mif->burst_size_cr << 12)	|
                                (mif->burst_size_cb << 10) 	|
                                (mif->burst_size_y << 8)  	|
                                (chro_rpt_lastl_ctrl << 6) 	|
                                (mif->set_separate_en << 1)	|
                                (1 << 0)                     		// cntl_enable
      );
                            
    // ----------------------
    // Canvas
    // ----------------------
    WRITE_MPEG_REG(DI_INP_CANVAS0, (mif->canvas0_addr2 << 16) 		| 		// cntl_canvas0_addr2
                               (mif->canvas0_addr1 << 8)   			| 		// cntl_canvas0_addr1
                               (mif->canvas0_addr0 << 0)        			// cntl_canvas0_addr0
    );

    // ----------------------
    // Picture 0 X/Y start,end
    // ----------------------
    WRITE_MPEG_REG(DI_INP_LUMA_X0, (mif->luma_x_end0 << 16) | 				// cntl_luma_x_end0
                               (mif->luma_x_start0 << 0)        			// cntl_luma_x_start0
    	);
    WRITE_MPEG_REG(DI_INP_LUMA_Y0, (mif->luma_y_end0 << 16) | 				// cntl_luma_y_end0
                               (mif->luma_y_start0 << 0)        			// cntl_luma_y_start0
    	);
    WRITE_MPEG_REG(DI_INP_CHROMA_X0, (mif->chroma_x_end0 << 16) |
                               (mif->chroma_x_start0 << 0)
    	);                           
    WRITE_MPEG_REG(DI_INP_CHROMA_Y0, (mif->chroma_y_end0 << 16) |
                               (mif->chroma_y_start0 << 0)
    	);                           
                   
    // ----------------------
    // Repeat or skip
    // ----------------------
    WRITE_MPEG_REG(DI_INP_RPT_LOOP, (0 << 28) |
                               (0 << 24) |
                               (0 << 20) |
                               (0 << 16) |
                               (chroma0_rpt_loop_start << 12) |
                               (chroma0_rpt_loop_end << 8)  |
                               (luma0_rpt_loop_start << 4)  |
                               (luma0_rpt_loop_end << 0) 
        ) ;

    WRITE_MPEG_REG(DI_INP_LUMA0_RPT_PAT, luma0_rpt_loop_pat);
    WRITE_MPEG_REG(DI_INP_CHROMA0_RPT_PAT, chroma0_rpt_loop_pat);
    
    // Dummy pixel value
    WRITE_MPEG_REG(DI_INP_DUMMY_PIXEL, 0x00808000); 
    if ( (mif->set_separate_en == 1) )   // 4:2:0 block mode. 
    {
        set_di_inp_fmt_more (  
                        1,                									// hfmt_en
                        1,                									// hz_yc_ratio
                        0,                									// hz_ini_phase
                        1,                									// vfmt_en
                        1,                									// vt_yc_ratio
                        0,                									// vt_ini_phase
                        mif->luma_x_end0 - mif->luma_x_start0 + 1, 			// y_length
                        mif->chroma_x_end0 - mif->chroma_x_start0 + 1 , 	// c length 
                        0 );                 								// hz repeat.
    } 
    else 
    {
        set_di_inp_fmt_more (  
                        1,                											// hfmt_en
                        1,                											// hz_yc_ratio
                        0,                											// hz_ini_phase
                        0,                											// vfmt_en
                        0,                											// vt_yc_ratio
                        0,                											// vt_ini_phase
                        mif->luma_x_end0 - mif->luma_x_start0 + 1, 					// y_length
                        ((mif->luma_x_end0 >>1 ) - (mif->luma_x_start0>>1) + 1),	// c length 
                        0 );                 // hz repeat.
    }
}

void set_di_mem_fmt_more (int hfmt_en,
                int hz_yc_ratio,        //2bit 
                int hz_ini_phase,       //4bit
                int vfmt_en,
                int vt_yc_ratio,        //2bit
                int vt_ini_phase,       //4bit
                int y_length,
                int c_length,
                int hz_rpt              //1bit
     	)
{
    int vt_phase_step = (16 >> vt_yc_ratio);  

    WRITE_MPEG_REG(DI_MEM_FMT_CTRL,      
                              (hz_rpt << 28)       	|     		//hz rpt pixel        
                              (hz_ini_phase << 24) 	|     		//hz ini phase
                              (0 << 23)         	|        	//repeat p0 enable
                              (hz_yc_ratio << 21)  	|     		//hz yc ratio
                              (hfmt_en << 20)   	|        	//hz enable
                              (1 << 17)         	|        	//nrpt_phase0 enable
                              (0 << 16)         	|        	//repeat l0 enable
                              (0 << 12)         	|        	//skip line num
                              (vt_ini_phase << 8)  	|     		//vt ini phase
                              (vt_phase_step << 1) 	|     		//vt phase step (3.4)
                              (vfmt_en << 0)             		//vt enable
             	);
    
    WRITE_MPEG_REG(DI_MEM_FMT_W, (y_length << 16) |        	//hz format width
                             (c_length << 0)                  	//vt format width
            	);
}

void set_di_mem_combined_simple(
	unsigned long   x_start,
	unsigned long   x_end,
	unsigned long   y_start,
	unsigned long   y_end,
	unsigned long   mode,           // 1 = RGB (3 bytes per pixel), 0 = 4:2:2 (2 bytes per pixel)
	unsigned long   urgent,      
	unsigned long   canvas,
	int             hold_line )
{
    // General register setup
    unsigned long   demux_mode   	= mode; 						// 0 = 4:2:2 demux, 1 = RGB demuxing from a single FIFO
    unsigned long   bytes_per_pixel	= (mode  == 1) ? 2 : 1;     	// RGB = 3 bytes per
    unsigned long   burst_size_cr  	= 0;    						// unused
    unsigned long   burst_size_cb  	= 0;    						// unused
    unsigned long   burst_size_y   	= 1;    						// 64x64 burst size
    unsigned long   st_separate_en 	= 0;

    // ----------------------
    // General register
    // ----------------------

    WRITE_MPEG_REG(DI_MEM_GEN_REG, (hold_line << 19)	|   	//hold lines
                                (urgent << 28) 			|   	// urgent  
                                (urgent << 27)      	|  		// luma urgent 
                                (urgent << 18)        	|   	// push pixel value
                                (demux_mode << 16)  	|
                                (bytes_per_pixel << 14)	|
                                (burst_size_cr << 12)  	|
                                (burst_size_cb << 10) 	|
                                (burst_size_y << 8)   	|
                                (0 << 6)                |   	// TODO: cntl_chro_rpt_lastl_ctrl
                                (0 << 4)                |   	// TODO: cntl_vt_yc_ratio
                                (0 << 2)                |   	// TODO: cntl_hz_yc_ratio
                                (st_separate_en << 1) 	|
                                (0 << 0)                    	// cntl_enable (don't enable just yet)
      );

    // ----------------------
    // Canvas
    // ----------------------
    WRITE_MPEG_REG(DI_MEM_CANVAS0, (0 << 16)	 	|   		// cntl_canvas0_addr2
                                (0 << 8)   			|   		// cntl_canvas0_addr1
                                (canvas << 0)               	// cntl_canvas0_addr0
    	);

   	// ----------------------
    // Picture 0 X/Y start,end
    // ----------------------
    WRITE_MPEG_REG(DI_MEM_LUMA_X0, (x_end << 16) |   			// cntl_luma_x_end0
                                (x_start << 0)              	// cntl_luma_x_start0
    	);
    WRITE_MPEG_REG(DI_MEM_LUMA_Y0, (y_end << 16) |   			// cntl_luma_y_end0
                                (y_start << 0)              	// cntl_luma_y_start0
    	);
    WRITE_MPEG_REG(DI_MEM_CHROMA_X0, 0);                  		// unused
    WRITE_MPEG_REG(DI_MEM_CHROMA_Y0, 0);              			// unused


    // ----------------------
    // No Repeat or skip
    // ----------------------
    WRITE_MPEG_REG(DI_MEM_RPT_LOOP, (0 << 24)		|   		// cntl_chroma1_rpt_loop
                                (0 << 16)   		|   		// cntl_luma1_rpt_loop
                                (0 << 8)            |   		// cntl_chroma0_rpt_loop
                                (0 << 0)                    	// cntl_luma0_rpt_loop
    	);

    WRITE_MPEG_REG(DI_MEM_LUMA0_RPT_PAT, 0);              		// no skip /repeat
    WRITE_MPEG_REG(DI_MEM_CHROMA0_RPT_PAT, 0);           		// unused


    WRITE_MPEG_REG(DI_MEM_DUMMY_PIXEL, 0xAABBCC00);      		// RGB = AABBCC or YCb = AABB, YCr = AACC

    // Enable VD1: vd_di_inp
    WRITE_MPEG_REG(DI_MEM_GEN_REG, (READ_MPEG_REG(DI_MEM_GEN_REG) | (1 << 0)));   			// cntl_enable
}

void set_di_mem_mif ( DI_MIF_t *mif, int urgent, int hold_line)
{
    unsigned long bytes_per_pixel;  
    unsigned long demux_mode;
    unsigned long chro_rpt_lastl_ctrl;
    unsigned long luma0_rpt_loop_start;
    unsigned long luma0_rpt_loop_end;
    unsigned long luma0_rpt_loop_pat;
    unsigned long chroma0_rpt_loop_start;
    unsigned long chroma0_rpt_loop_end;
    unsigned long chroma0_rpt_loop_pat;

    if ( mif->set_separate_en == 1 && mif->src_field_mode == 1 ) 
    {
      	chro_rpt_lastl_ctrl =1;
      	luma0_rpt_loop_start = 1;
      	luma0_rpt_loop_end = 1;
      	chroma0_rpt_loop_start = 1;
      	chroma0_rpt_loop_end = 1;
        luma0_rpt_loop_pat = 0x80;
        chroma0_rpt_loop_pat = 0x80;
    } 
    else if ( mif->set_separate_en == 1 && mif->src_field_mode == 0 ) 
    {
      	chro_rpt_lastl_ctrl =1;
      	luma0_rpt_loop_start = 0;
      	luma0_rpt_loop_end = 0;
      	chroma0_rpt_loop_start = 0;
      	chroma0_rpt_loop_end = 0;
      	luma0_rpt_loop_pat = 0x0;
      	chroma0_rpt_loop_pat = 0x0;
    } 
    else if ( mif->set_separate_en == 0 && mif->src_field_mode == 1 ) 
    {
      	chro_rpt_lastl_ctrl =1;
      	luma0_rpt_loop_start = 1;
      	luma0_rpt_loop_end = 1;
      	chroma0_rpt_loop_start = 0;
      	chroma0_rpt_loop_end = 0;
        luma0_rpt_loop_pat = 0x80;
        chroma0_rpt_loop_pat = 0x00;
    } 
    else 
    {
      	chro_rpt_lastl_ctrl =0;
      	luma0_rpt_loop_start = 0;
      	luma0_rpt_loop_end = 0;
      	chroma0_rpt_loop_start = 0;
      	chroma0_rpt_loop_end = 0;
      	luma0_rpt_loop_pat = 0x00;
      	chroma0_rpt_loop_pat = 0x00;
    }
    
    bytes_per_pixel = mif->set_separate_en ? 0 : (mif->video_mode ? 2 : 1);
    demux_mode = mif->video_mode;


    // ----------------------
    // General register
    // ----------------------

    WRITE_MPEG_REG(DI_MEM_GEN_REG,          
                                (urgent << 28)    			| 		// urgent bit. 
                                (urgent << 27)             	| 		// urgent bit.
                                (1 << 25)                  	| 		// no dummy data. 
                                (hold_line << 19)     		| 		// hold lines
                                (1 << 18)          			| 		// push dummy pixel
                                (demux_mode << 16)  		| 		// demux_mode
                                (bytes_per_pixel << 14)    	| 
                                (mif->burst_size_cr << 12) 	|
                                (mif->burst_size_cb << 10) 	|
                                (mif->burst_size_y << 8)  	|
                                (chro_rpt_lastl_ctrl << 6) 	|
                                (mif->set_separate_en << 1)	|
                                (1 << 0)                    	 	// cntl_enable
      );
                            
    // ----------------------
    // Canvas
    // ----------------------
    WRITE_MPEG_REG(DI_MEM_CANVAS0, (mif->canvas0_addr2 << 16)		| 	// cntl_canvas0_addr2
                               (mif->canvas0_addr1 << 8)      		| 	// cntl_canvas0_addr1
                               (mif->canvas0_addr0 << 0)        		// cntl_canvas0_addr0
    	);

    // ----------------------
    // Picture 0 X/Y start,end
    // ----------------------
    WRITE_MPEG_REG(DI_MEM_LUMA_X0, (mif->luma_x_end0 << 16) 		| 	// cntl_luma_x_end0
                               (mif->luma_x_start0 << 0)        		// cntl_luma_x_start0
    	);
    WRITE_MPEG_REG(DI_MEM_LUMA_Y0, (mif->luma_y_end0 << 16)   		| 	// cntl_luma_y_end0
                               (mif->luma_y_start0 << 0)        		// cntl_luma_y_start0
    	);
    WRITE_MPEG_REG(DI_MEM_CHROMA_X0, (mif->chroma_x_end0 << 16) |
                               (mif->chroma_x_start0 << 0)
    	);                           
    WRITE_MPEG_REG(DI_MEM_CHROMA_Y0, (mif->chroma_y_end0 << 16) |
                               (mif->chroma_y_start0 << 0)
    	);                           
                   
    // ----------------------
    // Repeat or skip
    // ----------------------
    WRITE_MPEG_REG(DI_MEM_RPT_LOOP, (0 << 28) |
                               (0   << 24) |
                               (0   << 20) |
                               (0     << 16) |
                               (chroma0_rpt_loop_start << 12) |
                               (chroma0_rpt_loop_end << 8) |
                               (luma0_rpt_loop_start << 4) |
                               (luma0_rpt_loop_end << 0) 
        ) ;

    WRITE_MPEG_REG(DI_MEM_LUMA0_RPT_PAT, luma0_rpt_loop_pat);
    WRITE_MPEG_REG(DI_MEM_CHROMA0_RPT_PAT, chroma0_rpt_loop_pat);
    
    // Dummy pixel value
    WRITE_MPEG_REG(DI_MEM_DUMMY_PIXEL, 0x00808000); 
    if ( (mif->set_separate_en == 1))   // 4:2:0 block mode. 
    {
        set_di_mem_fmt_more (  
                        1,                										// hfmt_en
                        1,                										// hz_yc_ratio
                        0,                										// hz_ini_phase
                        1,                										// vfmt_en
                        1,                										// vt_yc_ratio
                        0,                										// vt_ini_phase
                        mif->luma_x_end0 - mif->luma_x_start0 + 1, 				// y_length
                        mif->chroma_x_end0 - mif->chroma_x_start0 + 1, 			// c length 
                        0 );                 									// hz repeat.
    } else {
        set_di_mem_fmt_more (  
                        1,                											// hfmt_en
                        1,                											// hz_yc_ratio
                        0,                											// hz_ini_phase
                        0,                											// vfmt_en
                        0,                											// vt_yc_ratio
                        0,                											// vt_ini_phase
                        mif->luma_x_end0 - mif->luma_x_start0 + 1, 					// y_length
                        ((mif->luma_x_end0 >>1 ) - (mif->luma_x_start0>>1) + 1),  	// c length 
                        0 );                 										// hz repeat.
    }
}

void set_di_if1_fmt_more (int hfmt_en,
                int hz_yc_ratio,        //2bit 
                int hz_ini_phase,       //4bit
                int vfmt_en,
                int vt_yc_ratio,        //2bit
                int vt_ini_phase,       //4bit
                int y_length,
                int c_length,
                int hz_rpt              //1bit
                )
{
    int vt_phase_step = (16 >> vt_yc_ratio);  

    WRITE_MPEG_REG(DI_IF1_FMT_CTRL,      
                              (hz_rpt << 28)       	|     	//hz rpt pixel        
                              (hz_ini_phase << 24) 	|     	//hz ini phase
                              (0 << 23)         	|      	//repeat p0 enable
                              (hz_yc_ratio << 21)	|     	//hz yc ratio
                              (hfmt_en << 20)   	|    	//hz enable
                              (1 << 17)         	|     	//nrpt_phase0 enable
                              (0 << 16)         	|     	//repeat l0 enable
                              (0 << 12)         	|      	//skip line num
                              (vt_ini_phase << 8)  	|     	//vt ini phase
                              (vt_phase_step << 1) 	|     	//vt phase step (3.4)
                              (vfmt_en << 0)             	//vt enable
                   	);
    
    WRITE_MPEG_REG(DI_IF1_FMT_W, (y_length << 16) | 		//hz format width
                             (c_length << 0)            	//vt format width
             		);
}

void set_di_if1_combined_simple(
	unsigned long   x_start,
	unsigned long   x_end,
	unsigned long   y_start,
	unsigned long   y_end,
	unsigned long   mode,           // 1 = RGB (3 bytes per pixel), 0 = 4:2:2 (2 bytes per pixel)
	unsigned long   urgent,
	unsigned long   canvas,
	int             hold_line )
{
    // General register setup
    unsigned long   demux_mode   	= mode; 						// 0 = 4:2:2 demux, 1 = RGB demuxing from a single FIFO
    unsigned long   bytes_per_pixel	= (mode  == 1) ? 2 : 1;     	// RGB = 3 bytes per
    unsigned long   burst_size_cr  	= 0;    						// unused
    unsigned long   burst_size_cb 	= 0;    						// unused
    unsigned long   burst_size_y  	= 3;    						// 64x64 burst size
    unsigned long   st_separate_en 	= 0;

    // ----------------------
    // General register
    // ----------------------

    WRITE_MPEG_REG(DI_IF1_GEN_REG,          
                                (urgent << 28)  		|   	// urgent  
                                (urgent << 27)      	|  		// luma urgent 
                                (hold_line << 19)  		|   	//hold lines
                                (1 << 18)       		|   	// push pixel value
                                (demux_mode << 16) 		|
                                (bytes_per_pixel << 14)	|
                                (burst_size_cr << 12) 	|
                                (burst_size_cb << 10)  	|
                                (burst_size_y << 8) 	|
                                (0 << 6)                |   	// TODO: cntl_chro_rpt_lastl_ctrl
                                (0 << 4)                |   	// TODO: cntl_vt_yc_ratio
                                (0 << 2)                |   	// TODO: cntl_hz_yc_ratio
                                (st_separate_en << 1) 	|
                                (0 << 0)                    	// cntl_enable (don't enable just yet)
      	);

    // ----------------------
    // Canvas
    // ----------------------
    WRITE_MPEG_REG(DI_IF1_CANVAS0, (0 << 16)	|   		// cntl_canvas0_addr2
                                (0 << 8) 		|   		// cntl_canvas0_addr1
                                (canvas << 0)       		// cntl_canvas0_addr0
    	);

    // ----------------------
    // Picture 0 X/Y start,end
    // ----------------------
    WRITE_MPEG_REG(DI_IF1_LUMA_X0, (x_end << 16)	|   	// cntl_luma_x_end0
                                (x_start << 0)              // cntl_luma_x_start0
    	);
    WRITE_MPEG_REG(DI_IF1_LUMA_Y0, (y_end << 16) |   		// cntl_luma_y_end0
                                (y_start << 0)              // cntl_luma_y_start0
    	);
    WRITE_MPEG_REG(DI_IF1_CHROMA_X0, 0);              		// unused
    WRITE_MPEG_REG(DI_IF1_CHROMA_Y0, 0);          			// unused

    // ----------------------
    // No Repeat or skip
    // ----------------------
    WRITE_MPEG_REG(DI_IF1_RPT_LOOP, (0 << 24)	|   		// cntl_chroma1_rpt_loop
                                (0 << 16)  		|   		// cntl_luma1_rpt_loop
                                (0 << 8)  		|   		// cntl_chroma0_rpt_loop
                                (0 << 0)  					// cntl_luma0_rpt_loop
    );

    WRITE_MPEG_REG(DI_IF1_LUMA0_RPT_PAT, 0);              // no skip /repeat
    WRITE_MPEG_REG(DI_IF1_CHROMA0_RPT_PAT, 0);      		// unused



    WRITE_MPEG_REG(DI_IF1_DUMMY_PIXEL, 0xAABBCC00);   		// RGB = AABBCC or YCb = AABB, YCr = AACC

    // Enable VD1: vd_di_inp
    WRITE_MPEG_REG(DI_IF1_GEN_REG, (READ_MPEG_REG(DI_IF1_GEN_REG) | (1 << 0)));                  // cntl_enable
}

void set_di_if1_mif ( DI_MIF_t *mif, int urgent, int hold_line)
{
    unsigned long bytes_per_pixel;  
    unsigned long demux_mode;
    unsigned long chro_rpt_lastl_ctrl;
    unsigned long luma0_rpt_loop_start;
    unsigned long luma0_rpt_loop_end;
    unsigned long luma0_rpt_loop_pat;
    unsigned long chroma0_rpt_loop_start;
    unsigned long chroma0_rpt_loop_end;
    unsigned long chroma0_rpt_loop_pat;

    if ( mif->set_separate_en == 1 && mif->src_field_mode == 1 ) 
    {
      	chro_rpt_lastl_ctrl =1;
      	luma0_rpt_loop_start = 1;
      	luma0_rpt_loop_end = 1;
      	chroma0_rpt_loop_start = 1;
      	chroma0_rpt_loop_end = 1;
        luma0_rpt_loop_pat = 0x80;
        chroma0_rpt_loop_pat = 0x80;
    } 
    else if ( mif->set_separate_en == 1 && mif->src_field_mode == 0 ) 
    {
      	chro_rpt_lastl_ctrl =1;
      	luma0_rpt_loop_start = 0;
      	luma0_rpt_loop_end = 0;
      	chroma0_rpt_loop_start = 0;
      	chroma0_rpt_loop_end = 0;
      	luma0_rpt_loop_pat = 0x0;
      	chroma0_rpt_loop_pat = 0x0;
    } 
    else if ( mif->set_separate_en == 0 && mif->src_field_mode == 1 ) 
    {
      	chro_rpt_lastl_ctrl =1;
      	luma0_rpt_loop_start = 1;
      	luma0_rpt_loop_end = 1;
      	chroma0_rpt_loop_start = 0;
      	chroma0_rpt_loop_end = 0;
        luma0_rpt_loop_pat = 0x80;
        chroma0_rpt_loop_pat = 0x00;
    } 
    else 
    {
      	chro_rpt_lastl_ctrl =0;
      	luma0_rpt_loop_start = 0;
      	luma0_rpt_loop_end = 0;
      	chroma0_rpt_loop_start = 0;
      	chroma0_rpt_loop_end = 0;
      	luma0_rpt_loop_pat = 0x00;
      	chroma0_rpt_loop_pat = 0x00;
    }
    
    bytes_per_pixel = mif->set_separate_en ? 0 : (mif->video_mode ? 2 : 1);
    demux_mode = mif->video_mode;


    // ----------------------
    // General register
    // ----------------------

    WRITE_MPEG_REG(DI_IF1_GEN_REG, (1 << 25)       			| 	// no dummy data. 
                                (urgent << 28)      		|   // urgent  
                                (urgent << 27)        		|  	// luma urgent 
                                (hold_line << 19)        	| 	// hold lines
                                (1 << 18)            		| 	// push dummy pixel
                                (demux_mode << 16)   		| 	// demux_mode
                                (bytes_per_pixel << 14)    	| 
                                (mif->burst_size_cr << 12) 	|
                                (mif->burst_size_cb << 10)	|
                                (mif->burst_size_y << 8)   	|
                                (chro_rpt_lastl_ctrl << 6) 	|
                                (mif->set_separate_en << 1)	|
                                (1 << 0)                     	// cntl_enable
      	);
                            
    // ----------------------
    // Canvas
    // ----------------------
    WRITE_MPEG_REG(DI_IF1_CANVAS0, (mif->canvas0_addr2 << 16)	| 	// cntl_canvas0_addr2
                               (mif->canvas0_addr1 << 8)      	| 	// cntl_canvas0_addr1
                               (mif->canvas0_addr0 << 0)        	// cntl_canvas0_addr0
    	);

    // ----------------------
    // Picture 0 X/Y start,end
    // ----------------------
    WRITE_MPEG_REG(DI_IF1_LUMA_X0, (mif->luma_x_end0 << 16) | 		// cntl_luma_x_end0
                               (mif->luma_x_start0 << 0)        	// cntl_luma_x_start0
    	);
    WRITE_MPEG_REG(DI_IF1_LUMA_Y0, (mif->luma_y_end0 << 16) | 		// cntl_luma_y_end0
                               (mif->luma_y_start0 << 0)        	// cntl_luma_y_start0
    	);
    WRITE_MPEG_REG(DI_IF1_CHROMA_X0, (mif->chroma_x_end0 << 16) |
                               (mif->chroma_x_start0 << 0)
    	);                           
    WRITE_MPEG_REG(DI_IF1_CHROMA_Y0, (mif->chroma_y_end0 << 16) |
                               (mif->chroma_y_start0 << 0)
    	);                           
                   
    // ----------------------
    // Repeat or skip
    // ----------------------
    WRITE_MPEG_REG(DI_IF1_RPT_LOOP, (0 << 28)	|
                               (0   << 24) 		|
                               (0   << 20) 		|
                               (0     << 16) 	|
                               (chroma0_rpt_loop_start << 12) |
                               (chroma0_rpt_loop_end << 8) |
                               (luma0_rpt_loop_start << 4) |
                               (luma0_rpt_loop_end << 0) 
        ) ;

    WRITE_MPEG_REG(DI_IF1_LUMA0_RPT_PAT, luma0_rpt_loop_pat);
    WRITE_MPEG_REG(DI_IF1_CHROMA0_RPT_PAT, chroma0_rpt_loop_pat);
    
    // Dummy pixel value
    WRITE_MPEG_REG(DI_IF1_DUMMY_PIXEL, 0x00808000); 
    if ( (mif->set_separate_en == 1))   // 4:2:0 block mode. 
    {
        set_di_if1_fmt_more (  
                        1,                										// hfmt_en
                        1,                										// hz_yc_ratio
                        0,                										// hz_ini_phase
                        1,                										// vfmt_en
                        1,                										// vt_yc_ratio
                        0,                										// vt_ini_phase
                        mif->luma_x_end0 - mif->luma_x_start0 + 1, 				// y_length
                        mif->chroma_x_end0 - mif->chroma_x_start0 + 1 , 		// c length 
                        0 );                 									// hz repeat.
    } else {
        set_di_if1_fmt_more (  
                        1,                											// hfmt_en
                        1,                											// hz_yc_ratio
                        0,                											// hz_ini_phase
                        0,                											// vfmt_en
                        0,                											// vt_yc_ratio
                        0,                											// vt_ini_phase
                        mif->luma_x_end0 - mif->luma_x_start0 + 1, 					// y_length
                        ((mif->luma_x_end0 >>1 ) - (mif->luma_x_start0>>1) + 1),  	// c length 
                        0 );                 // hz repeat.
    }
}

void set_di_chan2_combined_simple(
	unsigned long   x_start,
	unsigned long   x_end,
	unsigned long   y_start,
	unsigned long   y_end,
	unsigned long   mode,           	// 1 = RGB (3 bytes per pixel), 0 = 4:2:2 (2 bytes per pixel)
	unsigned long   urgent,           	// 1 = RGB (3 bytes per pixel), 0 = 4:2:2 (2 bytes per pixel)
	unsigned long   canvas,
	int             hold_line )
{
    // General register setup
    unsigned long   demux_mode   	= mode; 						// 0 = 4:2:2 demux, 1 = RGB demuxing from a single FIFO
    unsigned long   bytes_per_pixel	= (mode  == 1) ? 2 : 1;     	// RGB = 3 bytes per
    unsigned long   burst_size_cr  	= 0;    						// unused
    unsigned long   burst_size_cb 	= 0;    						// unused
    unsigned long   burst_size_y  	= 1;    						// 64x64 burst size
    unsigned long   st_separate_en 	= 0;

    // ----------------------
    // General register
    // ----------------------

    WRITE_MPEG_REG(DI_CHAN2_GEN_REG, (hold_line << 19)		|   //hold lines
                                (urgent << 28) 				|   // urgent  
                                (urgent << 27)          	|  	// luma urgent 
                                (1 << 18)               	|   // push pixel value
                                (demux_mode << 16)      	|
                                (bytes_per_pixel << 14) 	|
                                (burst_size_cr << 12)   	|
                                (burst_size_cb << 10)   	|
                                (burst_size_y << 8)     	|
                                (0 << 6)                	|   // TODO: cntl_chro_rpt_lastl_ctrl
                                (0 << 4)                	|   // TODO: cntl_vt_yc_ratio
                                (0 << 2)                	|   // TODO: cntl_hz_yc_ratio
                                (st_separate_en << 1)   	|
                                (0 << 0)                    	// cntl_enable (don't enable just yet)
      	);

    // ----------------------
    // Canvas
    // ----------------------
    WRITE_MPEG_REG(DI_CHAN2_CANVAS, (0 << 16) 		|   		// cntl_canvas0_addr2
                                (0 << 8)  			|  	  		// cntl_canvas0_addr1
                                (canvas << 0)               	// cntl_canvas0_addr0
    	);
    // ----------------------
    // Picture 0 X/Y start,end
    // ----------------------
    WRITE_MPEG_REG(DI_CHAN2_LUMA_X, (x_end << 16)	|   		// cntl_luma_x_end0
                                (x_start << 0)              	// cntl_luma_x_start0
    	);
    WRITE_MPEG_REG(DI_CHAN2_LUMA_Y, (y_end << 16) |   			// cntl_luma_y_end0
                                (y_start << 0)              	// cntl_luma_y_start0
    	);

    // ----------------------
    // No Repeat or skip
    // ----------------------
    WRITE_MPEG_REG(DI_CHAN2_RPT_LOOP, (0 << 24)		|   		// cntl_chroma1_rpt_loop
                                (0 << 16)       	|   		// cntl_luma1_rpt_loop
                                (0 << 8)       		|   		// cntl_chroma0_rpt_loop
                                (0 << 0)                    	// cntl_luma0_rpt_loop
    );

    WRITE_MPEG_REG(DI_CHAN2_LUMA_RPT_PAT, 0);            		// no skip /repeat

    WRITE_MPEG_REG(DI_CHAN2_DUMMY_PIXEL, 0xAABBCC00);   		// RGB = AABBCC or YCb = AABB, YCr = AACC

    // Enable VD1: vd_di_inp
    WRITE_MPEG_REG(DI_CHAN2_GEN_REG, (READ_MPEG_REG(DI_CHAN2_GEN_REG) |  (1 << 0)));  	// cntl_enable
}

void set_di_chan2_mif ( DI_MIF_t *mif, int urgent, int hold_line)
{
    unsigned long bytes_per_pixel;  
    unsigned long demux_mode;
    unsigned long luma0_rpt_loop_start;
    unsigned long luma0_rpt_loop_end;
    unsigned long luma0_rpt_loop_pat;
    
    bytes_per_pixel = mif->set_separate_en ? 0 : ((mif->video_mode == 1) ? 2 : 1);
    demux_mode =  mif->video_mode & 1; 

    if (mif->src_field_mode == 1 ) 
    {
      	luma0_rpt_loop_start = 1;
      	luma0_rpt_loop_end = 1;
      	luma0_rpt_loop_pat = 0x80;
    } 
    else 
    { 
      	luma0_rpt_loop_start = 0;
      	luma0_rpt_loop_end = 0;
      	luma0_rpt_loop_pat = 0;
    }
    // ----------------------
    // General register
    // ----------------------

    WRITE_MPEG_REG(DI_CHAN2_GEN_REG, (1 << 25)        				| 	// no dummy data. 
                                (urgent << 28)               		|   // urgent  
                                (urgent << 27)               		|  	// luma urgent 
                                (hold_line << 19)               	| 	// hold lines
                                (1 << 18)                  			| 	// push dummy pixel
                                (demux_mode << 16)           		| 	// demux_mode
                                (bytes_per_pixel << 14)    			| 
                                (0 << 12)      						|
                                (0 << 10)      						|
                                (mif->burst_size_y << 8)        	|
                                ( (hold_line == 0 ? 1 : 0 ) << 7 ) 	|  	//manual start.
                                (0 << 6) 							|
                                (0 << 1)      						|
                                (1 << 0)                     			// cntl_enable
      );

                            
    // ----------------------
    // Canvas
    // ----------------------
    WRITE_MPEG_REG(DI_CHAN2_CANVAS, (0 << 16) 		| 		// cntl_canvas0_addr2
                                (0 << 8)      		| 		// cntl_canvas0_addr1
                                (mif->canvas0_addr0 << 0)   // cntl_canvas0_addr0
    );

    // ----------------------
    // Picture 0 X/Y start,end
    // ----------------------
    WRITE_MPEG_REG(DI_CHAN2_LUMA_X, (mif->luma_x_end0 << 16) 	| 		// cntl_luma_x_end0
                                (mif->luma_x_start0 << 0)        		// cntl_luma_x_start0
    	);
    WRITE_MPEG_REG(DI_CHAN2_LUMA_Y, (mif->luma_y_end0 << 16)  | 		// cntl_luma_y_end0
                                (mif->luma_y_start0 << 0)        		// cntl_luma_y_start0
    	);
                   
    // ----------------------
    // Repeat or skip
    // ----------------------
    WRITE_MPEG_REG(DI_CHAN2_RPT_LOOP, (0 << 28) |
                                (0   << 24) |
                                (0   << 20) |
                                (0   << 16) |
                                (0   << 12) |
                                (0   << 8)  |
                                (luma0_rpt_loop_start << 4)  |
                                (luma0_rpt_loop_end << 0)
    ); 

    WRITE_MPEG_REG(DI_CHAN2_LUMA_RPT_PAT, luma0_rpt_loop_pat);
    
    // Dummy pixel value
    WRITE_MPEG_REG(DI_CHAN2_DUMMY_PIXEL, 0x00808000); 
}

void set_di_if0_combined_simple(
	unsigned long  	x_start,
	unsigned long  	x_end,
	unsigned long  	y_start,
	unsigned long  	y_end,
	unsigned long  	mode,           	// 1 = RGB/YCBCR (3 bytes per pixel), 0 = 4:2:2 (2 bytes per pixel)
	unsigned long  	urgent,           	// 1 = RGB/YCBCR (3 bytes per pixel), 0 = 4:2:2 (2 bytes per pixel)
	unsigned long  	canvas_addr, 
	int 		   	hold_line )      
{
    // General register setup
    unsigned long   demux_mode  	= mode; 						// 0 = 4:2:2 demux, 1 = RGB demuxing from a single FIFO
    unsigned long   bytes_per_pixel	= (mode  == 1) ? 2 : 1;     	// RGB = 3 bytes per 
    unsigned long   burst_size_cr  	= 0;    						// unused
    unsigned long   burst_size_cb  	= 0;    						// unused
    unsigned long   burst_size_y   	= 3;    						// 64x64 burst size
    unsigned long   st_separate_en 	= 0;

    // ----------------------
    // General register
    // ----------------------

    WRITE_MPEG_REG(VD1_IF0_GEN_REG, (hold_line << 19)		|   //hold lines
                                (1 << 18)               	|   // push pixel value
                                (urgent << 28)          	|
                                (urgent << 27)          	|
                                (demux_mode << 16)      	|
                                (bytes_per_pixel << 14) 	| 
                                (burst_size_cr << 12)   	|
                                (burst_size_cb << 10)   	|
                                (burst_size_y << 8)     	|
                                (0 << 6)                	|   // TODO: cntl_chro_rpt_lastl_ctrl
                                (0 << 4)                	|   // TODO: cntl_vt_yc_ratio
                                (0 << 2)                	|   // TODO: cntl_hz_yc_ratio
                                (st_separate_en << 1)   	|
                                (0 << 0)                    	// cntl_enable (don't enable just yet)
      );
                            
    // ----------------------
    // Canvas
    // ----------------------
    WRITE_MPEG_REG(VD1_IF0_CANVAS0, (0 << 16)		|   	// cntl_canvas0_addr2
                                (0 << 8)     		|   	// cntl_canvas0_addr1
                                (canvas_addr << 0)          // cntl_canvas0_addr0
    	);

    WRITE_MPEG_REG(VD1_IF0_CANVAS1, (0 << 16) 		|   	// cntl_canvas1_addr2
                                (0 << 8)     		|   	// cntl_canvas1_addr1
                                (0 << 0)                    // cntl_canvas1_addr0
    	);

    // ----------------------
    // Picture 0 X/Y start,end
    // ----------------------
    WRITE_MPEG_REG(VD1_IF0_LUMA_X0, (x_end << 16) |   		// cntl_luma_x_end0
                                (x_start << 0)              // cntl_luma_x_start0
    	);
    WRITE_MPEG_REG(VD1_IF0_LUMA_Y0, (y_end << 16) |   		// cntl_luma_y_end0
                                (y_start << 0)              // cntl_luma_y_start0
    	);
    WRITE_MPEG_REG(VD1_IF0_CHROMA_X0, 0);                      			// unused
    WRITE_MPEG_REG(VD1_IF0_CHROMA_Y0, 0);                            // unused

    // ----------------------
    // Picture 1 unused
    // ----------------------
    WRITE_MPEG_REG(VD1_IF0_LUMA_X1, 0);                            		// unused
    WRITE_MPEG_REG(VD1_IF0_LUMA_Y1, 0);                            		// unused
    WRITE_MPEG_REG(VD1_IF0_CHROMA_X1, 0);                          		// unused
    WRITE_MPEG_REG(VD1_IF0_CHROMA_Y1, 0);                          		// unused

    // ----------------------
    // No Repeat or skip
    // ----------------------
    WRITE_MPEG_REG(VD1_IF0_RPT_LOOP, (0 << 24)		|   	// cntl_chroma1_rpt_loop
                                (0 << 16)        	|   	// cntl_luma1_rpt_loop
                                (0 << 8)    		|   	// cntl_chroma0_rpt_loop
                                (0 << 0)                    // cntl_luma0_rpt_loop
    	); 

    WRITE_MPEG_REG(VD1_IF0_LUMA0_RPT_PAT, 0);         		// no skip /repeat
    WRITE_MPEG_REG(VD1_IF0_CHROMA0_RPT_PAT, 0);          // unused
    WRITE_MPEG_REG(VD1_IF0_LUMA1_RPT_PAT, 0);             // unused
    WRITE_MPEG_REG(VD1_IF0_CHROMA1_RPT_PAT, 0);          // unused

    WRITE_MPEG_REG(VD1_IF0_LUMA_PSEL, 0);                 // unused only one picture 
    WRITE_MPEG_REG(VD1_IF0_CHROMA_PSEL, 0);             	// unused only one picture 


    WRITE_MPEG_REG(VD1_IF0_DUMMY_PIXEL, 0x00808000);     

    // Enable VD1: vd_rmem_if0
    WRITE_MPEG_REG(VD1_IF0_GEN_REG, READ_MPEG_REG(VD1_IF0_GEN_REG) | (1 << 0));    		// cntl_enable
}

void set_di_if0_mif ( DI_MIF_t *mif, int urgent, int hold_line)
{
    unsigned long bytes_per_pixel;  
    unsigned long demux_mode;
    unsigned long chro_rpt_lastl_ctrl;
    unsigned long luma0_rpt_loop_start;
    unsigned long luma0_rpt_loop_end;
    unsigned long luma0_rpt_loop_pat;
    unsigned long chroma0_rpt_loop_start;
    unsigned long chroma0_rpt_loop_end;
    unsigned long chroma0_rpt_loop_pat;

    if ( mif->set_separate_en == 1 && mif->src_field_mode == 1 ) 
    {
      	chro_rpt_lastl_ctrl =1;
      	luma0_rpt_loop_start = 1;
      	luma0_rpt_loop_end = 1;
      	chroma0_rpt_loop_start = 1;
      	chroma0_rpt_loop_end = 1;
        luma0_rpt_loop_pat = 0x80;
        chroma0_rpt_loop_pat = 0x80;
    } 
    else if ( mif->set_separate_en == 1 && mif->src_field_mode == 0 ) 
    {
      	chro_rpt_lastl_ctrl =1;
      	luma0_rpt_loop_start = 0;
      	luma0_rpt_loop_end = 0;
      	chroma0_rpt_loop_start = 0;
      	chroma0_rpt_loop_end = 0;
      	luma0_rpt_loop_pat = 0x0;
      	chroma0_rpt_loop_pat = 0x0;
    } 
    else if ( mif->set_separate_en == 0 && mif->src_field_mode == 1 ) 
    {
      	chro_rpt_lastl_ctrl =1;
      	luma0_rpt_loop_start = 1;
      	luma0_rpt_loop_end = 1;
      	chroma0_rpt_loop_start = 0;
      	chroma0_rpt_loop_end = 0;
        luma0_rpt_loop_pat = 0x80;
        chroma0_rpt_loop_pat = 0x00;
    } 
    else 
    {
      	chro_rpt_lastl_ctrl =0;
      	luma0_rpt_loop_start = 0;
      	luma0_rpt_loop_end = 0;
      	chroma0_rpt_loop_start = 0;
      	chroma0_rpt_loop_end = 0;
      	luma0_rpt_loop_pat = 0x00;
      	chroma0_rpt_loop_pat = 0x00;
    }
    
    bytes_per_pixel = mif->set_separate_en ? 0 : (mif->video_mode ? 2 : 1);
    demux_mode = mif->video_mode;


    // ----------------------
    // General register
    // ----------------------

    WRITE_MPEG_REG(VD1_IF0_GEN_REG, (1 << 25)				| 		// no dummy data. 
                                (urgent << 28)           	|   	// urgent  
                                (urgent << 27)          	|  		// luma urgent 
                                (hold_line << 19)       	| 		// hold lines
                                (1 << 18)               	| 		// push dummy pixel
                                (demux_mode << 16)     		| 		// demux_mode
                                (bytes_per_pixel << 14)    	| 
                                (mif->burst_size_cr << 12) 	|
                                (mif->burst_size_cb << 10)	|
                                (mif->burst_size_y << 8)  	|
                                (chro_rpt_lastl_ctrl << 6) 	|
                                (mif->set_separate_en << 1)	|
                                (1 << 0)                     		// cntl_enable
      	);
                            
    // ----------------------
    // Canvas
    // ----------------------
    WRITE_MPEG_REG(VD1_IF0_CANVAS0, (mif->canvas0_addr2 << 16) 		| 	// cntl_canvas0_addr2
                               (mif->canvas0_addr1 << 8)      		| 	// cntl_canvas0_addr1
                               (mif->canvas0_addr0 << 0)        		// cntl_canvas0_addr0
    	);

    // ----------------------
    // Picture 0 X/Y start,end
    // ----------------------
    WRITE_MPEG_REG(VD1_IF0_LUMA_X0, (mif->luma_x_end0 << 16) | 		// cntl_luma_x_end0
                               (mif->luma_x_start0 << 0)        	// cntl_luma_x_start0
    	);
    WRITE_MPEG_REG(VD1_IF0_LUMA_Y0, (mif->luma_y_end0 << 16) | 		// cntl_luma_y_end0
                               (mif->luma_y_start0 << 0)        	// cntl_luma_y_start0
    	);
    WRITE_MPEG_REG(VD1_IF0_CHROMA_X0, (mif->chroma_x_end0 << 16) |
                               (mif->chroma_x_start0 << 0)
    	);                           
    WRITE_MPEG_REG(VD1_IF0_CHROMA_Y0, (mif->chroma_y_end0 << 16) |
                               (mif->chroma_y_start0 << 0)
    	);                           
                   
    // ----------------------
    // Repeat or skip
    // ----------------------
    WRITE_MPEG_REG(VD1_IF0_RPT_LOOP, (0 << 28) 		|
                               (0   << 24) 			|
                               (0   << 20) 			|
                               (0   << 16) 			|
                               (chroma0_rpt_loop_start << 12) |
                               (chroma0_rpt_loop_end << 8) |
                               (luma0_rpt_loop_start << 4) |
                               (luma0_rpt_loop_end << 0) 
        ) ;

    WRITE_MPEG_REG(VD1_IF0_LUMA0_RPT_PAT, luma0_rpt_loop_pat);
    WRITE_MPEG_REG(VD1_IF0_CHROMA0_RPT_PAT, chroma0_rpt_loop_pat);
    
    // Dummy pixel value
    WRITE_MPEG_REG(VD1_IF0_DUMMY_PIXEL, 0x00808000); 

   	// ----------------------
    // Picture 1 unused
    // ----------------------
    WRITE_MPEG_REG(VD1_IF0_LUMA_X1, 0);                      		// unused
    WRITE_MPEG_REG(VD1_IF0_LUMA_Y1, 0);                           // unused
    WRITE_MPEG_REG(VD1_IF0_CHROMA_X1, 0);                        // unused
    WRITE_MPEG_REG(VD1_IF0_CHROMA_Y1, 0);                        // unused
    WRITE_MPEG_REG(VD1_IF0_LUMA_PSEL, 0);                        	// unused only one picture 
    WRITE_MPEG_REG(VD1_IF0_CHROMA_PSEL, 0);                      // unused only one picture 

    if ( (mif->set_separate_en == 1))   // 4:2:0 block mode. 
    {
        set_vd1_fmt_more (  
                        1,                									// hfmt_en
                        1,                									// hz_yc_ratio
                        0,                									// hz_ini_phase
                        1,                									// vfmt_en
                        1,                									// vt_yc_ratio
                        0,                									// vt_ini_phase
                        mif->luma_x_end0 - mif->luma_x_start0 + 1, 			// y_length
                        mif->chroma_x_end0 - mif->chroma_x_start0 + 1 , 	// c length 
                        0 );                 								// hz repeat.
    } 
    else 
    {
        set_vd1_fmt_more (  
                        1,                											// hfmt_en
                        1,                											// hz_yc_ratio
                        0,                											// hz_ini_phase
                        0,                											// vfmt_en
                        0,                											// vt_yc_ratio
                        0,                											// vt_ini_phase
                        mif->luma_x_end0 - mif->luma_x_start0 + 1, 					// y_length
                        ((mif->luma_x_end0 >>1 ) - (mif->luma_x_start0>>1) + 1) , 	//c length 
                        0 );                 										// hz repeat.
    }
}

//enable deinterlace pre module separated for pre post separate tests.
void enable_di_pre (  
   DI_MIF_t        *di_inp_mif,
   DI_MIF_t        *di_mem_mif,
   DI_MIF_t        *di_chan2_mif,
   DI_SIM_MIF_t    *di_nrwr_mif,
   DI_SIM_MIF_t    *di_mtnwr_mif,
   int nr_en, int mtn_en, int pd32_check_en, int pd22_check_en, int hist_check_en, 
   int pre_field_num, int pre_viu_link, int hold_line)
{
  	int hist_check_only;

  	hist_check_only = hist_check_en && !nr_en && !mtn_en && !pd22_check_en && !pd32_check_en ; 

  	if ( nr_en | mtn_en | pd22_check_en || pd32_check_en ) 
  	{
       	set_di_inp_mif( di_inp_mif, 0, hold_line );   		// set urgent 0
       	set_di_mem_mif(di_mem_mif, 0, hold_line );   		// set urgent 0
  	}

  	if ( pd22_check_en || hist_check_only ) 
  	{ 
       	set_di_chan2_mif(di_chan2_mif, 0, hold_line);   	// set urgent 0.
  	}

  	// set nr wr mif interface.
   	if ( nr_en ) 
   	{
     	WRITE_MPEG_REG(DI_NRWR_X, (di_nrwr_mif->start_x <<16) | (di_nrwr_mif->end_x));   	// start_x 0 end_x 719.
     	WRITE_MPEG_REG(DI_NRWR_Y, (di_nrwr_mif->start_y <<16) | (di_nrwr_mif->end_y));   	// start_y 0 end_y 239.
     	WRITE_MPEG_REG(DI_NRWR_CTRL, di_nrwr_mif->canvas_num );     						// canvas index.
   	}

   	// motion wr mif.
    if (mtn_en ) 
    {
       	WRITE_MPEG_REG(DI_MTNWR_X, (di_mtnwr_mif->start_x <<16) | (di_mtnwr_mif->end_x));   	// start_x 0 end_x 719.
       	WRITE_MPEG_REG(DI_MTNWR_Y, (di_mtnwr_mif->start_y <<16) | (di_mtnwr_mif->end_y));   	// start_y 0 end_y 239.
       	WRITE_MPEG_REG(DI_MTNWR_CTRL, di_mtnwr_mif->canvas_num |  								// canvas index.
                      (0 << 8));       															// urgent.
    }

  	// reset pre 
  	WRITE_MPEG_REG(DI_PRE_CTRL, READ_MPEG_REG(DI_PRE_CTRL) | 
                   1 << 31 );                  						// frame reset for the pre modules.

  	WRITE_MPEG_REG(DI_PRE_CTRL, nr_en |        						// NR enable
                    (mtn_en << 1 ) |        						// MTN_EN
                    (pd32_check_en << 2 ) |        					// check 3:2 pulldown
                    (pd22_check_en << 3 ) |        					// check 2:2 pulldown
                    (1 << 4 ) |        								// 2:2 check mid pixel come from next field after MTN.
                    (hist_check_en << 5 ) |        					// hist check enable
                    (hist_check_only << 6 ) |        				// hist check  use chan2.
                    ((!nr_en) << 7 ) |        						// hist check use data before noise reduction.
                    ((pd22_check_en || hist_check_only) << 8 ) |	// chan 2 enable for 2:2 pull down check.
                    (pd22_check_en << 9) |        					// line buffer 2 enable
                    (0 << 10) |        								// pre drop first.
                    (0 << 11) |        								// pre repeat.
                    (0 << 12) |        								// pre viu link
                    (hold_line << 16) |      						// pre hold line number
                    (pre_field_num << 29) |        					// pre field number.
                    (0x1 << 30 )      								// pre soft rst, pre frame rst.
                   ); 
}

// enable di post module for separate test.
void enable_di_post (  
   DI_MIF_t        *di_buf0_mif,
   DI_MIF_t        *di_buf1_mif,
   DI_SIM_MIF_t    *di_diwr_mif,
   DI_SIM_MIF_t    *di_mtncrd_mif,
   DI_SIM_MIF_t    *di_mtnprd_mif,
   int ei_en, int blend_en, int blend_mtn_en, int blend_mode, int di_vpp_en, int di_ddr_en,
   int post_field_num, int hold_line ) 
{
  	int ei_only;
  	int buf1_en;

  	ei_only = ei_en && !blend_en && (di_vpp_en || di_ddr_en );
  	buf1_en =  ( !ei_only && (di_ddr_en || di_vpp_en ) ); 
  
  	if ( ei_en || di_vpp_en || di_ddr_en ) 
  	{ 
     	set_di_if0_mif( di_buf0_mif, di_vpp_en, hold_line );
  	}

  	if ( !ei_only && (di_ddr_en || di_vpp_en ) ) 
  	{
     	set_di_if1_mif( di_buf1_mif, di_vpp_en, hold_line );
  	} 

   	// motion for current display field.
    if ( blend_mtn_en ) 
    {
      	WRITE_MPEG_REG(DI_MTNCRD_X, (di_mtncrd_mif->start_x <<16) | (di_mtncrd_mif->end_x));   				// start_x 0 end_x 719.
      	WRITE_MPEG_REG(DI_MTNCRD_Y, (di_mtncrd_mif->start_y <<16) | (di_mtncrd_mif->end_y));             	// start_y 0 end_y 239.
      	WRITE_MPEG_REG(DI_MTNRD_CTRL, (di_mtnprd_mif->canvas_num <<8 ) |          							//mtnp canvas index.
                        (1 << 16) |          																// urgent
                         di_mtncrd_mif->canvas_num );                    									// current field mtn canvas index. 
    }

    if ( blend_mtn_en ) 
    {
        WRITE_MPEG_REG(DI_MTNPRD_X, (di_mtnprd_mif->start_x <<16) | (di_mtnprd_mif->end_x));   			// start_x 0 end_x 719.
        WRITE_MPEG_REG(DI_MTNPRD_Y, (di_mtnprd_mif->start_y <<16) | (di_mtnprd_mif->end_y));   			// start_y 0 end_y 239.
    }

    if ( di_ddr_en ) 
    {
       WRITE_MPEG_REG(DI_DIWR_X, (di_diwr_mif->start_x <<16) | (di_diwr_mif->end_x));   				// start_x 0 end_x 719.
       WRITE_MPEG_REG(DI_DIWR_Y, (di_diwr_mif->start_y <<16) | (di_diwr_mif->end_y *2 + 1 ));         	// start_y 0 end_y 479.
       WRITE_MPEG_REG(DI_DIWR_CTRL, di_diwr_mif->canvas_num |               							// canvas index.
                        (di_vpp_en << 8));            													// urgent.
    } 

   	if ( ei_only == 0) 
   	{
      	WRITE_MPEG_REG(DI_BLEND_CTRL,  (READ_MPEG_REG(DI_BLEND_CTRL) & (~((1 << 25) | (3 << 20 )))) | // clean some bit we need to set. 
                              (blend_mtn_en << 26 ) |   													// blend mtn enable.
                              (0 << 25 ) |   																// blend with the mtn of the pre display field and next display field.
                              (1 << 24 ) |   																// blend with pre display field.
                              (blend_mode << 20)    														// motion adaptive blend.
               ); 
   	}

   	WRITE_MPEG_REG(DI_POST_CTRL, ((ei_en | blend_en) << 0 ) | 		// line buffer 0 enable
                      (0 << 1)  |        							// line buffer 1 enable
                      (ei_en << 2) |        						// ei  enable
                      (blend_mtn_en << 3) |        					// mtn line buffer enable
                      (blend_mtn_en  << 4) |        				// mtnp read mif enable
                      (blend_en << 5) |        						// di blend enble.
                      (1 << 6) |        							// di mux output enable
                      (di_ddr_en << 7) |        					// di write to SDRAM enable.
                      (di_vpp_en << 8) |        					// di to VPP enable.
                      (0 << 9) |        							// mif0 to VPP enable.
                      (0 << 10) |        							// post drop first.
                      (0 << 11) |        							// post repeat.
                      (1 << 12) |        							// post viu link
                      (hold_line << 16) |       					// post hold line number
                      (post_field_num << 29) |        				// post field number.
                      (0x1 << 30 )       							// post soft rst  post frame rst.
        );
}

int di_pre_mode_check(int cur_field)
{
    int i;

    WRITE_MPEG_REG(DI_INFO_ADDR, 0 );
    for ( i  = 0; i <= 68; i++) 
    {   
       	di_info[cur_field][i] = READ_MPEG_REG(DI_INFO_DATA);
    }

    return (0);
}

int di_post_mode_check(int cur_field)
{
    int i;

    WRITE_MPEG_REG(DI_INFO_ADDR, 69 );
    for ( i  = 69; i <= 76; i++) 
    {   
       	di_info[cur_field][i] = READ_MPEG_REG(DI_INFO_DATA);
    }

    return (0);
}

void enable_region_blend( 
		int reg0_en, int reg0_start_x, int reg0_end_x, int reg0_start_y, int reg0_end_y, int reg0_mode, 
        int reg1_en, int reg1_start_x, int reg1_end_x, int reg1_start_y, int reg1_end_y, int reg1_mode, 
		int reg2_en, int reg2_start_x, int reg2_end_x, int reg2_start_y, int reg2_end_y, int reg2_mode, 
		int reg3_en, int reg3_start_x, int reg3_end_x, int reg3_start_y, int reg3_end_y, int reg3_mode )
{
   WRITE_MPEG_REG(DI_BLEND_REG0_X,  (reg0_start_x << 16 ) |
                        reg0_end_x );
   WRITE_MPEG_REG(DI_BLEND_REG0_Y,  (reg0_start_y << 16 ) |
                        reg0_end_y );
   WRITE_MPEG_REG(DI_BLEND_REG1_X,  (reg1_start_x << 16 ) |
                        reg1_end_x );
   WRITE_MPEG_REG(DI_BLEND_REG1_Y,  (reg1_start_y << 16 ) |
                        reg1_end_y );
   WRITE_MPEG_REG(DI_BLEND_REG2_X,  (reg2_start_x << 16 ) |
                        reg2_end_x );
   WRITE_MPEG_REG(DI_BLEND_REG2_Y,  (reg2_start_y << 16 ) |
                        reg2_end_y );
   WRITE_MPEG_REG(DI_BLEND_REG3_X,  (reg3_start_x << 16 ) |
                        reg3_end_x );
   WRITE_MPEG_REG(DI_BLEND_REG3_Y,  (reg3_start_y << 16 ) |
                        reg3_end_y );
   WRITE_MPEG_REG(DI_BLEND_CTRL, ( READ_MPEG_REG(DI_BLEND_CTRL) & (~(0xfff <<8)) ) |
                      (reg0_mode << 8 )   |
                      (reg1_mode << 10 )  |
                      (reg2_mode << 12 )  |
                      (reg3_mode << 14 )  |
                      (reg0_en << 16 )    	|
                      (reg1_en << 17 )    	|
                      (reg2_en << 18 )    	|
                      (reg3_en << 19 ) ); 
}

int check_p32_p22(int cur_field, int pre_field, int pre2_field)
{
	unsigned int cur_data, pre_data, pre2_data;
	unsigned int cur_num, pre_num, pre2_num;
	unsigned int data_diff, num_diff;
	
	di_p22_info = di_p22_info << 1;
	cur_data = di_info[cur_field][2];
	pre_data = di_info[pre_field][2];
	pre2_data = di_info[pre2_field][2];
	cur_num = di_info[cur_field][4] & 0xffffff;
	pre_num = di_info[pre_field][4] & 0xffffff;
	pre2_num = di_info[pre2_field][4] & 0xffffff;

	if ( cur_data*2 <= pre_data && pre2_data*2 <= pre_data && cur_num*2 <= pre_num && pre2_num*2 <= pre_num )
		di_p22_info |= 1;

   	di_p32_info = di_p32_info << 1;
   	di_p32_info_2 = di_p32_info_2 << 1;
   	di_p22_info_2 = di_p22_info_2 << 1;
	cur_data = di_info[cur_field][0];
	cur_num = di_info[cur_field][1] & 0xffffff;
	pre_data = di_info[pre_field][0];
	pre_num = di_info[pre_field][1] & 0xffffff;

	data_diff = cur_data>pre_data ? cur_data-pre_data : pre_data-cur_data;
	num_diff = cur_num>pre_num ? cur_num-pre_num : pre_num-cur_num;

	if ( (di_p22_info & 0x1) && data_diff*10 <= cur_data && num_diff*10 <= cur_num )
		di_p22_info_2 |= 1;

	if ( di_p32_counter > 0 || di_p32_info == 0 )
	{
		if ( cur_data*2 <= pre_data && cur_num*50 <= pre_num )
		{
			di_p32_info |= 1;
			last_big_data = pre_data;
			last_big_num = pre_num;
			di_p32_counter = -1;
		}
		else 
		{
			last_big_data = 0;
			last_big_num = 0;

			if ( (di_p32_counter & 0x1) && data_diff*5 <= cur_data && num_diff*5 <= cur_num )
				di_p32_info_2 |= 1;
		}
	}
	else
	{
		if ( cur_data*2 <= last_big_data && cur_num*50 <= last_big_num )
		{
			di_p32_info |= 1;
			di_p32_counter = -1;
		}
	}

	di_p32_counter++;

	return 0;
}

void pattern_check_prepost(int period)
{
	if ( pre_field_counter != di_checked_field )
	{
    	di_checked_field = pre_field_counter;
    	di_mode_check(pre_field_counter%period);

#ifdef DEBUG
    	debug_array[(pre_field_counter&0x3ff)*4] = di_info[pre_field_counter%period][0];
    	debug_array[(pre_field_counter&0x3ff)*4+1] = di_info[pre_field_counter%period][1] & 0xffffff;
    	debug_array[(pre_field_counter&0x3ff)*4+2] = di_info[pre_field_counter%period][2];
    	debug_array[(pre_field_counter&0x3ff)*4+3] = di_info[pre_field_counter%period][4];
#endif

		if ( pre_field_counter >= 3 )
		{
			check_p32_p22(pre_field_counter%period, (pre_field_counter+period-1)%period, (pre_field_counter+period-2)%period);

			if ( period == 3 )
			{
				pattern_22 = pattern_22 << 1;
				if ( di_info[pre_field_counter%3][4] < di_info[(pre_field_counter+2)%3][4] )
					pattern_22 |= 1;
			}
		}
	}

	di_chan2_mif.canvas0_addr0 = DEINTERLACE_CANVAS_BASE_INDEX + (field_counter+period-1) % period;
	di_mem_mif.canvas0_addr0 = DEINTERLACE_CANVAS_BASE_INDEX + (field_counter+period-2) % period;

	if ( period == 3 )
	{
		di_buf0_mif.canvas0_addr0 = DEINTERLACE_CANVAS_BASE_INDEX + (field_counter+2) % 3;
	}
	else
	{
    	if ( field_counter == 1 )
    		di_buf0_mif.canvas0_addr0 = di_chan2_mif.canvas0_addr0;
    	else
	    	di_buf0_mif.canvas0_addr0 = DEINTERLACE_CANVAS_BASE_INDEX + (field_counter+2) % 4;

	    di_buf1_mif.canvas0_addr0 = DEINTERLACE_CANVAS_BASE_INDEX + (field_counter+1) % 4;
	}

	blend_mode = 3;

	// 2:2 check
	if ( period == 3 )
	{
		if ( ((di_p22_info & PATTERN22_MARK) == (0xaaaaaaaaaaaaaaaaLL & PATTERN22_MARK)) 
			&& ((di_p22_info_2 & PATTERN22_MARK) == (0xaaaaaaaaaaaaaaaaLL & PATTERN22_MARK)) )
		{
		   	blend_mode = 1;
		}
		else if ( ((di_p22_info & PATTERN22_MARK) == (0x5555555555555555LL & PATTERN22_MARK)) 
			&& ((di_p22_info_2 & PATTERN22_MARK) == (0x5555555555555555LL & PATTERN22_MARK)) )
		{
		   	blend_mode = 0;
		}
	}
	else
	{
		if ( ((di_p22_info & PATTERN22_MARK) == (0x5555555555555555LL & PATTERN22_MARK)) 
			&& ((di_p22_info_2 & PATTERN22_MARK) == (0x5555555555555555LL & PATTERN22_MARK)) )
		{
	    	di_buf1_mif.canvas0_addr0 = DEINTERLACE_CANVAS_BASE_INDEX + (field_counter+3) % 4;
	    	blend_mode = 1;
		}
		else if ( ((di_p22_info & PATTERN22_MARK) == (0xaaaaaaaaaaaaaaaaLL & PATTERN22_MARK)) 
			&& ((di_p22_info_2 & PATTERN22_MARK) == (0xaaaaaaaaaaaaaaaaLL & PATTERN22_MARK)) )
		{
	    	di_buf1_mif.canvas0_addr0 = DEINTERLACE_CANVAS_BASE_INDEX + (field_counter+1) % 4;
	    	blend_mode = 0;
		}
	}

	// pull down pattern check
	if ( pattern_len == 0 )
	{
		int i, j, pattern, pattern_2, mask;

		for ( j = 5 ; j < 22 ; j++ )
		{
			mask = (1<<j) - 1;
			pattern = di_p32_info & mask;
			pattern_2 = di_p32_info_2 & mask;

			if ( pattern != 0 && pattern_2 != 0 && pattern != mask )
			{
				for ( i = j ; i < j*3 ; i += j )
					if ( ((di_p32_info>>i) & mask) != pattern || ((di_p32_info_2>>i) & mask) != pattern_2 )
						break;

				if ( i == j*3 )
				{
					if ( period == 3 )
					{
						if ( pattern_22 & (1<<(j-1)) )
					    	blend_mode = 1;
						else
					    	blend_mode = 0;
					}
					else
					{
						if ( di_info[(field_counter+3)%4][4] < di_info[(field_counter+2)%4][4] )
						{
					    	di_buf1_mif.canvas0_addr0 = DEINTERLACE_CANVAS_BASE_INDEX + (field_counter+3) % 4;
					    	blend_mode = 1;
						}
						else
						{
					    	di_buf1_mif.canvas0_addr0 = DEINTERLACE_CANVAS_BASE_INDEX + (field_counter+1) % 4;
					    	blend_mode = 0;
						}
					}

					pattern_len = j;
					break;
				}
			}
		}
	}
	else
	{
		int i, pattern, pattern_2, mask;

		mask = (1<<pattern_len) - 1;
		pattern = di_p32_info & mask;
		pattern_2 = di_p32_info_2 & mask;

		for ( i = pattern_len ; i < pattern_len*3 ; i += pattern_len )
			if ( ((di_p32_info>>i) & mask) != pattern || ((di_p32_info_2>>i) & mask) != pattern_2 )
				break;

		if ( i == pattern_len*3 )
		{
			if ( period == 3 )
			{
				if ( pattern_22 & (1<<(pattern_len-1)) )
			    	blend_mode = 1;
				else
			    	blend_mode = 0;
			}
			else
			{
				if ( di_info[(field_counter+3)%4][4] < di_info[(field_counter+2)%4][4] )
				{
			    	di_buf1_mif.canvas0_addr0 = DEINTERLACE_CANVAS_BASE_INDEX + (field_counter+3) % 4;
			    	blend_mode = 1;
				}
				else
				{
			    	di_buf1_mif.canvas0_addr0 = DEINTERLACE_CANVAS_BASE_INDEX + (field_counter+1) % 4;
			    	blend_mode = 0;
				}
			}
		}
		else
			pattern_len = 0;
	}

	di_nrwr_mif.canvas_num = DEINTERLACE_CANVAS_BASE_INDEX + field_counter % period;

	di_mtnwr_mif.canvas_num = DEINTERLACE_CANVAS_BASE_INDEX + 4 + field_counter % (period-1);
	di_mtncrd_mif.canvas_num = DEINTERLACE_CANVAS_BASE_INDEX + 4 + (field_counter+1) % (period-1);

	if ( period == 4 )
    	di_mtnprd_mif.canvas_num = DEINTERLACE_CANVAS_BASE_INDEX + 4 + (field_counter+2) % 3;
}

void pattern_check_pre(void)
{
    di_mode_check(pre_field_counter%4);

#ifdef DEBUG
   	debug_array[(pre_field_counter&0x3ff)*4] = di_info[pre_field_counter%4][0];
   	debug_array[(pre_field_counter&0x3ff)*4+1] = di_info[pre_field_counter%4][1] & 0xffffff;
   	debug_array[(pre_field_counter&0x3ff)*4+2] = di_info[pre_field_counter%4][2];
   	debug_array[(pre_field_counter&0x3ff)*4+3] = di_info[pre_field_counter%4][4];
#endif

	if ( pre_field_counter >= 3 )
	{
		check_p32_p22(pre_field_counter%4, (pre_field_counter-1)%4, (pre_field_counter-2)%4);

		if ( di_buf_pool[(pre_field_counter-1)%DI_BUF_NUM].blend_mode == 3 )
		{
			if ( ((di_p22_info & PATTERN22_MARK) == (0x5555555555555555LL & PATTERN22_MARK)) 
				&& ((di_p22_info_2 & PATTERN22_MARK) == (0x5555555555555555LL & PATTERN22_MARK)) )
			{
				di_buf_pool[(pre_field_counter-1)%DI_BUF_NUM].blend_mode = 1;
			}
			else if ( ((di_p22_info & PATTERN22_MARK) == (0xaaaaaaaaaaaaaaaaLL & PATTERN22_MARK)) 
				&& ((di_p22_info_2 & PATTERN22_MARK) == (0xaaaaaaaaaaaaaaaaLL & PATTERN22_MARK)) )
			{
		    	di_buf_pool[(pre_field_counter-1)%DI_BUF_NUM].blend_mode = 0;
			}
			else if ( pattern_len == 0 )
				di_buf_pool[(pre_field_counter-2)%DI_BUF_NUM].blend_mode = 3;

			if ( pattern_len == 0 )
			{
				int i, j, pattern, pattern_2, mask;

				for ( j = 5 ; j < 22 ; j++ )
				{
					mask = (1<<j) - 1;
					pattern = di_p32_info & mask;
					pattern_2 = di_p32_info_2 & mask;

					if ( pattern != 0 && pattern_2 != 0 && pattern != mask )
					{
						for ( i = j ; i < j*PATTERN32_NUM ; i += j )
							if ( ((di_p32_info>>i) & mask) != pattern || ((di_p32_info_2>>i) & mask) != pattern_2 )
								break;

						if ( i == j*PATTERN32_NUM )
						{
							if ( (pattern_len == 5) && ((pattern & (pattern-1)) == 0) )
							{
								if ( (di_p32_info & 0x1) || (di_p32_info & 0x2) || (di_p32_info & 0x8) )
									di_buf_pool[(pre_field_counter-1)%DI_BUF_NUM].blend_mode = 0;
								else
									di_buf_pool[(pre_field_counter-1)%DI_BUF_NUM].blend_mode = 1;
							}
							else
							{
								if ( (pattern & (pattern-1)) != 0 )
								{
									if ( di_info[pre_field_counter%4][4] < di_info[(pre_field_counter-1)%4][4] )
								    	di_buf_pool[(pre_field_counter-1)%DI_BUF_NUM].blend_mode = 1;
									else
										di_buf_pool[(pre_field_counter-1)%DI_BUF_NUM].blend_mode = 0;
								}
							}

							pattern_len = j;
							break;
						}
					}
				}
			}
			else
			{
				int i, pattern, pattern_2, mask;

				mask = (1<<pattern_len) - 1;
				pattern = di_p32_info & mask;
				pattern_2 = di_p32_info_2 & mask;

				for ( i = pattern_len ; i < pattern_len*PATTERN32_NUM ; i += pattern_len )
					if ( ((di_p32_info>>i) & mask) != pattern || ((di_p32_info_2>>i) & mask) != pattern_2 )
						break;

				if ( i == pattern_len*PATTERN32_NUM )
				{
					if ( (pattern_len == 5) && ((pattern & (pattern-1)) == 0) )
					{
						if ( (di_p32_info & 0x1) || (di_p32_info & 0x2) || (di_p32_info & 0x8) )
							di_buf_pool[(pre_field_counter-1)%DI_BUF_NUM].blend_mode = 0;
						else
							di_buf_pool[(pre_field_counter-1)%DI_BUF_NUM].blend_mode = 1;
					}
					else
					{
						if ( (pattern & (pattern-1)) != 0 )
						{
							if ( di_info[pre_field_counter%4][4] < di_info[(pre_field_counter-1)%4][4] )
						    	di_buf_pool[(pre_field_counter-1)%DI_BUF_NUM].blend_mode = 1;
							else
								di_buf_pool[(pre_field_counter-1)%DI_BUF_NUM].blend_mode = 0;
						}
					}
				}
				else
				{
					pattern_len = 0;
					di_buf_pool[(pre_field_counter-2)%DI_BUF_NUM].blend_mode = 3;
				}
			}
		}
	}
}

