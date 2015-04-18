#include "hevc_log.h"

int total_frame = 0;
void  totalframe_add(void)
{
 	total_frame++;
	LOGI_point(total_frame,"Total Decoded frame %d", total_frame);
	
}

void  clear_totalframe(void)
{
      total_frame = 1;
}

int get_totalframe(void)
{
    return total_frame;
}


