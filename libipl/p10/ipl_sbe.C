extern "C"
{
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
}

#include <iostream>
#include <iomanip>

#include "ipl_sbe.H"
#include "libipl_internal.H"
#include "common.H"
#include "plat_utils.H"

#include <targeting/predicates/predicateattrval.H>
#include <targeting/predicates/predicateisfunctional.H>
#include <targeting/predicates/predicatepostfixexpr.H>
#include <targeting/xmltohb/attributeenums.H>
#include <targeting/xmltohb/attributetraits.H>

#include <ekb/hwpf/fapi2/include/return_code_defs.H>
#include <ekb/chips/p10/procedures/hwp/istep/p10_do_fw_hb_istep.H>
#include <ekb/chips/p10/procedures/hwp/sbe/p10_get_sbe_msg_register.H>

namespace ipl::sbe
{
using namespace TARGETING;
int ipl_sbe_set_state(TARGETING::ConstTargetPtr target, enum sbe_state state)
{
    return hwaccess::HwAccessIntf::putCfamRegister(target, SBE_STATE_REG, 
                                                   static_cast<uint32_t>(state));
}

int ipl_sbe_get_state(TARGETING::ConstTargetPtr target, enum sbe_state *state)
{
    union sbe_msg_register msg;
    uint32_t value;

    int rc = hwaccess::HwAccessIntf::getCfamRegister(target, SBE_STATE_REG, value);

    if (rc)
        return rc;

    if (value == SBE_STATE_CHECK_CFAM)
    {
        rc = hwaccess::HwAccessIntf::getCfamRegister(target, SBE_MSG_REG, msg.reg);

        if (rc)
			return rc;

		*state = msg.sbe_booted ? SBE_STATE_BOOTED : SBE_STATE_CHECK_CFAM;
    }
    else
    {
		*state = static_cast<sbe_state>(value);
	}

    return rc;
}

int sbe_mpipl_continue(TARGETING::ConstTargetPtr /*target*/)
{
    //TODO p12-refactor
	/*struct chipop *chipop;
	int rc;

	chipop = pib_to_chipop(target);
	if (!chipop)
		return -1;

	if (!chipop->mpipl_continue) {
		PR_ERROR("mpipl_continue() not implemented for the target\n");
		return -1;
	}

	rc = chipop->mpipl_continue(chipop);
	if (rc) {
		PR_ERROR("sbe mpipl_continue() returned rc=%d\n", rc);
		return -1;
	}*/
	return 0;
}

ipl_error_type ipl_sbe_mpipl_continue(TARGETING::ConstTargetPtr /*target*/)
{
    /*TODO p12-refactor
	enum sbe_state state;
	int rc = 0;

	ipl_log(IPL_INFO, "ipl_sbe_mpipl_continue: Enter(%s)", pdbg_target_path(proc));

    rc = ipl_sbe_get_state(target, &state);

    if (rc != 0)
    {
		ipl_log(IPL_ERROR, "Failed to read SBE state information (%s)", pdbg_target_path(pib));
		return IPL_ERR_FSI_REG;
	}

	// SBE_STATE_CHECK_CFAM case is already handled by ipl_sbe_get_state function
	if (state != SBE_STATE_BOOTED)
    {
		ipl_log(IPL_ERROR,
			"SBE (%s) is not ready for chip-op: state(0x%08x)",
			pdbg_target_path(pib), state);
		return IPL_ERR_SBE_CHIPOP;
	}

	rc = sbe_mpipl_continue(target);

	if (rc != 0)
    {
		ipl_log(IPL_ERROR, "SBE (%s) mpipl continue chip-op failed", pdbg_target_path(pib));
		return IPL_ERR_SBE_CHIPOP;
	}*/

	return IPL_ERR_OK;
}

bool ipl_sbe_booted(TARGETING::TargetPtr target, uint32_t wait_time_seconds)
{
	sbeMsgReg_t sbeReg;
	fapi2::ReturnCode fapi_rc;
	uint32_t loopcount;

	loopcount = wait_time_seconds > 0 ? wait_time_seconds : 25;

	while (loopcount > 0)
    {
        std::cout << "p12-refactor executing p10_get_sbe_msg_register\n";
	    fapi_rc = p10_get_sbe_msg_register(target, sbeReg);

        if (fapi_rc == fapi2::FAPI2_RC_SUCCESS)
        {
			if (sbeReg.sbeBooted)
            {
                std::cout << "p12-refactor SBE Booted, wait time: " << loopcount << "\n";
				ipl_log(IPL_INFO,
					"SBE booted. sbeReg[0x%08x] Wait time: "
					"[%d]\n",
					uint32_t(sbeReg.reg), loopcount);
				return true;
			}
            else
            {
                std::cout << "p12-refactor SBE boot is in progress. sbeReg[0x"
                     << std::hex << std::setw(8) << std::setfill('0') << uint32_t(sbeReg.reg)
                     << "]" << std::dec << std::endl;
			}
		}
        else
        {
            std::cerr << "p12-refactor p10_get_sbe_msg_register failed fapi_rc = 0x"
                << std::hex << static_cast<uint32_t>(fapi_rc)
                << std::dec << std::endl;
			
            /*TODO ipl_log(IPL_ERROR,
				"p10_get_sbe_msg_register failed for proc %d, "
				"rc=%d\n",
				pdbg_target_index(proc), fapi_rc);*/
		}

		loopcount--;
		sleep(1);
	}

	// Get SBE debug data.
	uint32_t val = 0xFFFFFFFF;

    int rc = hwaccess::HwAccessIntf::getCfamRegister(target, 0x1007, val);

    if (rc)
    {
		//TODO ipl_log(IPL_ERROR, "CFAM(0x1007) on %s failed", pdbg_target_path(fsi));
	}

    std::cout << "p12-refactor SBE Debug Data: 0x2809[0x"
          << std::hex << std::setw(8) << std::setfill('0') << uint32_t(sbeReg.reg)
          << "]  0x1007[0x"
          << std::hex << std::setw(8) << std::setfill('0') << val
          << "]" << std::dec << std::endl;
    
    return false;
}

int ipl_set_sbe_state_all(enum sbe_state state, bool skipMaster)
{
    using namespace TARGETING;
    
    int ret = 0;
    auto& ts = TargetService::instance();

    PredicateAttrVal<ATTR_TYPE> pred(TYPE_PROC);

    auto top = ts.getTopLevelTarget();

    for (auto&& proc :
            ts.getAssociated(top, AssociationType::childByPhysical,
                             RecursionLevel::all, &pred))
    {
        if(skipMaster && ipl_is_master_proc(proc))
            continue;

		if (ipl_is_present(proc))
        {
			if (ipl_sbe_set_state(proc, state))
            {
				ret = 1;
			}
		}
	}
	return ret;
}

} // namespace ipl::sbe
