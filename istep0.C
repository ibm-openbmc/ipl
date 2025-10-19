extern "C" {
#include <stdio.h>
#include <stdlib.h>

#include <libpdbg.h>
}

#include <libekb.H>
#include <libipl/libipl.H>

#include <targeting/target_service.H>

int main(void)
{
    TARGETING::TargetService::instance().init("/tmp/targeting_test.dtb");

	if (!pdbg_targets_init(NULL))
		exit(1);

	if (libekb_init())
		exit(1);

	if (ipl_init(IPL_AUTOBOOT))
		exit(1);

	return ipl_run_major(0);
}
