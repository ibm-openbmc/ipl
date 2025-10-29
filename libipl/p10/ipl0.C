extern "C" {
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

#include <libpdbg.h>
#include <libpdbg_sbe.h>
}

#include "libipl.H"
#include "libipl_internal.H"
#include "common.H"
#include "ipl_sbe.H"
#include <attributes_info.H>

#include <ekb/chips/p10/procedures/hwp/perv/p10_start_cbs.H>
#include <ekb/chips/p10/procedures/hwp/perv/p10_setup_ref_clock.H>
#include <ekb/chips/p10/procedures/hwp/perv/p10_clock_test.H>
#include <ekb/chips/p10/procedures/hwp/perv/p10_setup_sbe_config.H>
#include <ekb/chips/p10/procedures/hwp/perv/p10_select_boot_master.H>

#include <targeting/target_service.H>
#include <targeting/predicates/predicateattrval.H>
#include <targeting/predicates/predicateisfunctional.H>
#include <targeting/predicates/predicatepostfixexpr.H>
#include <targeting/target.H>
#include <targeting/xmltohb/attributeenums.H>
#include <targeting/xmltohb/attributetraits.H>

#include <libguard/guard_interface.hpp>
#include <libguard/guard_entity.hpp>
#include <libguard/include/guard_record.hpp>
#include <libguard/guard_exception.hpp>
#include <filesystem>
#include <fstream>
#include <array>
#include <chrono>
#include <thread>

#define TGT_TYPE_PROC 0x05
#define FRU_TYPE_CORE 0x07
#define FRU_TYPE_MC 0x44
#define FRU_TYPE_FC 0x53

#define OSC_CTL_OFFSET 0x06
#define OSC_RESET_CMD 0x04
#define OSC_RESET_CMD_LEN 0x01

#define OSC_STAT_OFFSET 0x07
#define OSC_STAT_REG_LEN 0x01
#define OSC_STAT_ERR_MASK 0x80

#define CLOCK_RESTART_DELAY_IN_MS 100

#define GUARD_CONTINUE_TGT_TRAVERSAL 0
#define GUARD_TGT_FOUND 1
#define GUARD_TGT_NOT_FOUND 2
#define GUARD_PRIMARY_PROC_NOT_APPLIED 3

constexpr auto BOOTTIME_GUARD_INDICATOR = "/tmp/phal/boottime_guard_indicator";

struct guard_target {
	uint8_t path[21];
	bool set_hwas_state;
	uint8_t guardType;

	guard_target()
	{
		memset(&path, 0, sizeof(path));
		set_hwas_state = false;
		guardType = 0; // GARD_NULL
	}
};

static void ipl_pre0(void)
{
	ipl_pre();
}

static int ipl_poweron(void)
{
	return -1;
}

static int ipl_startipl(void)
{
	return -1;
}

static int ipl_DisableAttns(void)
{
	return -1;
}

static bool small_core_enabled(void)
{
	struct stat statbuf;
	int ret;

	/* If /tmp/small_core file exists, boot in small core mode */
	ret = stat("/tmp/small_core", &statbuf);
	if (ret == -1)
		return false;

	if (S_ISREG(statbuf.st_mode)) {
		ipl_log(IPL_INFO, "Booting in small core mode\n");
		return true;
	}

	return false;
}

static bool set_or_clear_state(struct pdbg_target *target, bool do_set)
{
	uint8_t buf[5];
	uint8_t flag_present = 0x40;
	uint8_t flag_functional = 0x20;

	if (!pdbg_target_get_attribute_packed(target, "ATTR_HWAS_STATE", "41",
					      1, buf)) {
		ipl_log(IPL_ERROR, "Attribute [ATTR_HWAS_STATE] read failed\n");
		return false;
	}

	if (do_set)
		buf[4] |= (flag_present | flag_functional);
	else
		buf[4] &= (uint8_t)(~flag_functional);

	if (!pdbg_target_set_attribute_packed(target, "ATTR_HWAS_STATE", "41",
					      1, buf)) {
		ipl_log(IPL_ERROR,
			"Attribute [ATTR_HWAS_STATE] write failed\n");
		return false;
	}
	return true;
}

static bool set_or_clear_state(TARGETING::TargetPtr target, bool do_set)
{
    using namespace TARGETING;

    AttributeTraits<ATTR_HWAS_STATE>::Type hwas;
    if(!target->tryGetAttr<ATTR_HWAS_STATE>(hwas))
    {
        std::cerr << "phal-refactor set_or_clear_state Attribute read failed\n";
		return false;
    }
    
    hwas.functional = 0;
   
    if(do_set)
    {
        hwas.present = 1;
        hwas.functional = 1;
    }

    if(!target->trySetAttr<ATTR_HWAS_STATE>(hwas))
    {
        std::cerr << "phal-refactor set_or_clear_state Attribute write failed\n";
		return false;
    }

    return true;
}

/*
 * Helper function set the clock functional state based on
 * ATTR_SYS_CLOCK_DECONFIG_STATE values
 * ATTR_SYS_CLOCK_DECONFIG_STATE contains the functional state
 * of the system clocks/oscillators. This is the way that Hostboot
 * communicates to the BMC which clocks have failed.
 *
 * @return true on success, false on failure
 */
static bool update_clock_func_state(void)
{
	using namespace TARGETING;

	ipl_log(
	    IPL_INFO,
	    "Updating ref clock target HWAS state based on Hostboot value \n");

    AttributeTraits<ATTR_SYS_CLOCK_DECONFIG_STATE>::Type clk_state =
                        SYS_CLOCK_DECONFIG_STATE_NO_DECONFIG;

    auto& ts = TargetService::instance();
    auto top = ts.getTopLevelTarget();

    if(!top->tryGetAttr<ATTR_SYS_CLOCK_DECONFIG_STATE>(clk_state))
    {
        std::cout << "phal-refactor trygetattr failed ATTR_SYS_CLOCK_DECONFIG_STATE\n";
        ipl_log(
		    IPL_ERROR,
		    "Attribute [ATTR_SYS_CLOCK_DECONFIG_STATE] read failed \n");
		//ipl_plat_procedure_error_handler(IPL_ERR_ATTR_READ_FAIL);
		//return false;
    }

	if (clk_state == SYS_CLOCK_DECONFIG_STATE_NO_DECONFIG)
    {
		// No HWAS state update required
		ipl_log(IPL_INFO, "update_clock_func_state : No updates \n");
		return true;
	}

    PredicateAttrVal<ATTR_TYPE> pred(TYPE_OSCREFCLK);

    for (auto&& clock_target :
            ts.getAssociated(top, AssociationType::childByPhysical,
                             RecursionLevel::all, &pred))
    {
        if (clk_state == SYS_CLOCK_DECONFIG_STATE_ALL_DECONFIG)
        {
			// Update HWAS state to non-functional
			/*TODO ipl_log(IPL_INFO,
				"Clock(%s) setting to non functional \n",
				pdbg_target_path(clock_target));*/
			if (!set_or_clear_state(clock_target, false))
            {
                std::cout << "phal-refactor setorclearstate-1 failed\n";
				//return false;
			}
			continue;
		}

        // Get Clock position
        AttributeTraits<ATTR_POSITION>::Type clk_pos;

        if(!clock_target->tryGetAttr<ATTR_POSITION>(clk_pos))
        {
            std::cout << "phal-refactor trygetattr failed ATTR_POSITION\n";
            /*TODO ipl_log(IPL_ERROR, "Attribute ATTR_POSITION read failed"
                    " for clock '%s' \n", pdbg_target_path(clock_target));*/

            //ipl_plat_procedure_error_handler(IPL_ERR_ATTR_READ_FAIL);

            //return false;
        }

        // Assumption: Clock A linked to ATTR_POSITION value 0 and
		// Clock B linked to ATTR_POSITION value 1
        const bool deconfigClkA = ((clk_pos == 0) &&
		             (clk_state == SYS_CLOCK_DECONFIG_STATE_A_DECONFIG)) ;

        const bool deconfigClkB = ((clk_pos == 1) &&
                     (clk_state == SYS_CLOCK_DECONFIG_STATE_B_DECONFIG));

        if (deconfigClkA || deconfigClkB)
		{
			/*TODO ipl_log(IPL_INFO,
				"deconfig state(%d) Clock(%s) setting to non "
				"functional \n",
				clk_state, pdbg_target_path(clock_target));*/

			if (!set_or_clear_state(clock_target, false))
            {
                std::cout << "phal-refactor setorclearstate-2 failed\n";
				//TODO return false;
			}
		}
    }
	return true;
}

[[maybe_unused]] static int update_hwas_state_callback(struct pdbg_target *target, void *priv)
{
	guard_target *target_info = static_cast<guard_target *>(priv);
	uint8_t path[21] = {0};
	uint8_t type;
	char tgtPhysDevPath[64];
	std::string guard_action(target_info->set_hwas_state ? "Clearing"
							     : "Applying");
	std::string guardTypeStr(
	    openpower::guard::guardReasonToStr(target_info->guardType));

	if (!pdbg_target_get_attribute(target, "ATTR_PHYS_BIN_PATH", 1, 21,
				       path))
		// Returning 0 for continue traversal, as the requested target
		// is not found
		return GUARD_CONTINUE_TGT_TRAVERSAL;

	if (memcmp(target_info->path, path, sizeof(path)) != 0)
		return GUARD_CONTINUE_TGT_TRAVERSAL;

	if (!pdbg_target_get_attribute(target, "ATTR_PHYS_DEV_PATH", 1, 64,
				       tgtPhysDevPath)) {
		ipl_log(IPL_ERROR, "Failed to read ATTR_PHYS_DEV_PATH for %s\n",
			pdbg_target_path(target));
		return GUARD_TGT_NOT_FOUND;
	}

	if (!pdbg_target_get_attribute(target, "ATTR_TYPE", 1, 1, &type)) {
		ipl_log(IPL_ERROR, "Failed to read ATTR_TYPE for %s\n",
			pdbg_target_path(target));
		// ATTR_TYPE attribute not found for the target, hence this is
		// an error case, so need to stop
		return GUARD_TGT_NOT_FOUND;
	}

	if (ipl_type() == IPL_TYPE_MPIPL &&
	    (type == FRU_TYPE_CORE || type == FRU_TYPE_FC)) {

		ipl_log(IPL_INFO, "%s guard record for %s. Type: %s\n",
			guard_action.c_str(), tgtPhysDevPath,
			guardTypeStr.c_str());

		if (!set_or_clear_state(target, target_info->set_hwas_state)) {
			ipl_log(
			    IPL_ERROR,
			    "Failed to update functional state of target 0x%x, "
			    "index=0x%x\n",
			    type, pdbg_target_index(target));
			// Unable to update the functional state of HWAS
			// attribute of the target, so we need to stop
			return GUARD_TGT_NOT_FOUND;
		}

		if ((type == FRU_TYPE_FC) && !small_core_enabled()) {
			struct pdbg_target *core;

			ipl_log(IPL_INFO,
				"%s guard record for associated Core since %s "
				"guared\n",
				guard_action.c_str(), tgtPhysDevPath);

			pdbg_for_each_target("core", target, core)
			{
				if (!set_or_clear_state(
					core, target_info->set_hwas_state)) {
					ipl_log(
					    IPL_ERROR,
					    "Failed to update functional state"
					    " of core with index 0x%x\n",
					    pdbg_target_index(core));
					// Unable to update the functional state
					// of HWAS attribute of the target, so
					// we need to stop
					return GUARD_TGT_NOT_FOUND;
				}
			}
		}

	} else if (ipl_type() == IPL_TYPE_NORMAL) {

		if (!openpower::guard::isEphemeralType(
			target_info->guardType)) {
			if (type == TGT_TYPE_PROC) {
				if (ipl_is_master_proc(target)) {
					ipl_log(
					    IPL_INFO,
					    "Primary processor [%s] is guarded "
					    "so, skipping to apply. Type: %s\n",
					    tgtPhysDevPath,
					    guardTypeStr.c_str());
					return GUARD_PRIMARY_PROC_NOT_APPLIED;
				}
			}
		}

		ipl_log(IPL_INFO, "%s guard record for %s. Type: %s\n",
			guard_action.c_str(), tgtPhysDevPath,
			guardTypeStr.c_str());

		if (!set_or_clear_state(target, target_info->set_hwas_state)) {
			ipl_log(IPL_ERROR,
				"Failed to update functional state of fru type "
				"0x%x\n",
				type);
			// Unable to update the functional state of HWAS
			// attribute of the target, so we need to stop
			return GUARD_TGT_NOT_FOUND;
		}

		if ((type == FRU_TYPE_FC) && !small_core_enabled()) {
			struct pdbg_target *core;

			ipl_log(IPL_INFO,
				"%s guard record for associated Core since %s "
				"guared\n",
				guard_action.c_str(), tgtPhysDevPath);

			pdbg_for_each_target("core", target, core)
			{
				if (!set_or_clear_state(
					core, target_info->set_hwas_state)) {
					ipl_log(
					    IPL_ERROR,
					    "Failed to update functional state"
					    " of core with index 0x%x\n",
					    pdbg_target_index(core));
					// Unable to update the functional state
					// of HWAS attribute of the target, so
					// we need to stop
					return GUARD_TGT_NOT_FOUND;
				}
			}
		}

	} else {
		ipl_log(IPL_INFO, "Skip, %s guard record for %s. Type: %s\n",
			guard_action.c_str(), tgtPhysDevPath);
	}

	// Requested target found
	return GUARD_TGT_FOUND;
}

/**
 * @brief Allow guard actions in the below cases to support resource recovery.
 *
 * - If the current boot is MPIPL.
 *
 * - If the BOOTTIME_GUARD_INDICATOR file (that will be created by the BMC
 *   in the PowerOn or TI or Checkstop or Watchdog timeout path) is exist
 *   in the current boot.
 */
[[maybe_unused]] static bool guard_action_allowed()
{
	if (ipl_type() == IPL_TYPE_MPIPL) {
		return true;
	}

	namespace fs = std::filesystem;
	fs::path boottime_guard_indicator(BOOTTIME_GUARD_INDICATOR);

	if (fs::exists(boottime_guard_indicator)) {
		// Remove indicator since that will be created by the BMC
		// based on the different boot path.
		fs::remove(boottime_guard_indicator);
		return true;
	} else {
		return false;
	}
}

//@Brief Function will get the guard records and will update the functional
// state of the guarded resources in HWAS state attribute in device tree based
// on the guard actions in the different boots.
[[maybe_unused]] static void process_guard_records()
{
	if (!guard_action_allowed()) {
		ipl_log(IPL_INFO, "No guard actions in the current boot");
		return;
	}

	try {
		openpower::guard::libguard_init(false);
		auto records = openpower::guard::getAll();

		if (records.size()) {
			ipl_log(IPL_INFO, "Number of Records = %d\n",
				records.size());

			if (!ipl_guard()) {
				// Don't return, we should handle reconfig type
				// guard records even if guard setting is
				// disabled.
				ipl_log(IPL_INFO,
					"Disabled to apply the guard records");
			}

			for (const auto &elem : records) {

				if (!ipl_guard() &&
				    !openpower::guard::isEphemeralType(
					elem.errType)) {
					// Disabled to apply the guard records
					// so should not allow the records to
					// apply except ephemeral type guard
					// records.
					continue;
				} else if (elem.recordId == GUARD_RESOLVED) {
					// No need to apply the resolved guard
					// records.
					continue;
				}

				guard_target targetinfo;
				targetinfo.guardType = elem.errType;
				int index = 0, i, err;

				targetinfo.path[index] =
				    elem.targetId.type_size;
				index += 1;

				for (i = 0;
				     i < (0x0F & elem.targetId.type_size);
				     i++) {

					targetinfo.path[index] =
					    elem.targetId.pathElements[i]
						.targetType;
					targetinfo.path[index + 1] =
					    elem.targetId.pathElements[i]
						.instance;

					index += sizeof(
					    elem.targetId.pathElements[0]);
				}

				// Clear ephemeral type guard records in the
				// normal ipl.
				if ((ipl_type() == IPL_TYPE_NORMAL) &&
				    openpower::guard::isEphemeralType(
					elem.errType)) {
					openpower::guard::clear(elem.recordId);
					targetinfo.set_hwas_state = true;
				} else {
					targetinfo.set_hwas_state = false;
				}

				err = pdbg_target_traverse(
				    NULL, update_hwas_state_callback,
				    &targetinfo);
				if ((err == GUARD_CONTINUE_TGT_TRAVERSAL) ||
				    (err == GUARD_TGT_NOT_FOUND))
					ipl_log(
					    IPL_ERROR,
					    "Failed to set HWAS state for guard"
					    " record[ID: %d]\n",
					    elem.recordId);
			}
		}
	} catch (const openpower::guard::exception::GuardException &ex) {
		// For any exeption related to guard, add the PEL and continue
		// to boot
		ipl_log(IPL_ERROR,
			"Caught the exception %s and continuing to boot "
			"without processing guard records",
			ex.what());
		ipl_error_callback(IPL_ERR_GUARD_PARTITION_ACCESS);
	}
}

/*
 * @Brief Function will check if the FCO state is set for hardware units
 * consumed by sbe or not. If FCO override bit is set for such hardware
 * units, need to mark present and functional state as set in devicetree
 * during the boot.
 *
 */
static void apply_fco_override(void)
{
    std::cout << "phal-refactor Executing apply_fco_override\n" ;
    using namespace TARGETING;
    auto& ts = TargetService::instance();
    auto top = ts.getTopLevelTarget();

    auto typeProc = std::make_shared<PredicateAttrVal<ATTR_TYPE>>(TYPE_PROC);
    auto masterProc = std::make_shared<PredicateAttrVal<ATTR_PROC_MASTER_TYPE>>(0);
    PredicatePostfixExpr masterFuncProcPred;
    masterFuncProcPred.push(typeProc).push(masterProc).And();
   
    auto isMc = std::make_shared<PredicateAttrVal<ATTR_TYPE>>(TYPE_MC);
    auto isCore = std::make_shared<PredicateAttrVal<ATTR_TYPE>>(TYPE_CORE);
    auto isPauc = std::make_shared<PredicateAttrVal<ATTR_TYPE>>(TYPE_PAUC);
    auto isPau = std::make_shared<PredicateAttrVal<ATTR_TYPE>>(TYPE_PAU);
    auto isIohs = std::make_shared<PredicateAttrVal<ATTR_TYPE>>(TYPE_IOHS);
    auto isPec = std::make_shared<PredicateAttrVal<ATTR_TYPE>>(TYPE_PEC);
    auto isFc = std::make_shared<PredicateAttrVal<ATTR_TYPE>>(TYPE_FC);
    auto isPerv = std::make_shared<PredicateAttrVal<ATTR_TYPE>>(TYPE_PERV);

    PredicatePostfixExpr expr;
    expr.push(isMc).push(isCore).Or()
        .push(isPauc).Or()
        .push(isPau).Or()
        .push(isIohs).Or()
        .push(isPec).Or()
        .push(isFc).Or()
        .push(isPerv).Or();

    auto proc_targets = ts.getAssociated(top, AssociationType::childByPhysical,
                             RecursionLevel::all, &masterFuncProcPred);
 
    if(proc_targets.empty() || (proc_targets.size() != 1))
    {
        std::cerr << "phal-refactor apply_fco_override: invalid master proc count\n";
        return;
    }

    for (auto&& pchild :
            ts.getAssociated(proc_targets.front(), AssociationType::childByPhysical,
                             RecursionLevel::all, &expr))
    {
        auto hwas = pchild->getAttr<ATTR_HWAS_STATE>();

        if(hwas.functionalOverride)
        {
            std::cout << "phal-refactor applying functional override\n" ;
            hwas.present = 1;
            hwas.functional = 1;
            if(!pchild->trySetAttr<ATTR_HWAS_STATE>(hwas))
            {
               std::cerr << "phal-refactor apply_fco_override Attribute write failed\n";
            }
        }
    }
    std::cout << "phal-refactor Done apply_fco_override\n" ;
}

//@Brief Function will set the functional and present state of master proc
// and of available procs in the system. For few children of master proc
// functional and present state will be set in device tree during genesis
// boot.
// It will also update functional state oscrefclk target.
static bool update_genesis_hwas_state(void)
{
    using namespace TARGETING;
    auto& ts = TargetService::instance();
    auto top = ts.getTopLevelTarget();

    PredicatePostfixExpr procExpr;
    procExpr.push(std::make_shared<PredicateAttrVal<ATTR_TYPE>>(TYPE_PROC));

    auto isMc = std::make_shared<PredicateAttrVal<ATTR_TYPE>>(TYPE_MC);
    auto isCore = std::make_shared<PredicateAttrVal<ATTR_TYPE>>(TYPE_CORE);
    auto isPauc = std::make_shared<PredicateAttrVal<ATTR_TYPE>>(TYPE_PAUC);
    auto isPau = std::make_shared<PredicateAttrVal<ATTR_TYPE>>(TYPE_PAU);
    auto isIohs = std::make_shared<PredicateAttrVal<ATTR_TYPE>>(TYPE_IOHS);
    auto isPec = std::make_shared<PredicateAttrVal<ATTR_TYPE>>(TYPE_PEC);
    auto isFc = std::make_shared<PredicateAttrVal<ATTR_TYPE>>(TYPE_FC);
    auto isPerv = std::make_shared<PredicateAttrVal<ATTR_TYPE>>(TYPE_PERV);
    
    PredicatePostfixExpr expr;
    expr.push(isMc).push(isCore).Or()
        .push(isPauc).Or()
        .push(isPau).Or()
        .push(isIohs).Or()
        .push(isPec).Or()
        .push(isFc).Or()
        .push(isPerv).Or();

    for (auto&& proc :
            ts.getAssociated(top, AssociationType::childByPhysical,
                             RecursionLevel::all, &procExpr))
    {
		if (!set_or_clear_state(proc, true))
        {
            std::cout << "phal-refactor failed to set proc hwas state\n";
			/*TODO ipl_log(IPL_ERROR,
				"Failed to set HWAS state of proc %d\n",
				pdbg_target_index(proc));
			ipl_error_callback(IPL_ERR_ATTR_WRITE);*/
			return false;
		}
    }

    //the master proc
    procExpr.push(std::make_shared<PredicateAttrVal<ATTR_PROC_MASTER_TYPE>>(0))
            .And();

    auto master_proc = ts.getAssociated(top, AssociationType::childByPhysical,
                             RecursionLevel::all, &procExpr);
    
    if(master_proc.empty() || (master_proc.size() != 1))
    {
        std::cerr << "phal-refactor update_genesis_hwas_state: invalid master proc count\n";
        return 1;
    }

    for (auto&& pchild :
            ts.getAssociated(master_proc.front(), AssociationType::childByPhysical,
                             RecursionLevel::all, &expr))
    {
        if (!set_or_clear_state(pchild,true))
        {
            /*TODO ipl_log(IPL_ERROR,
                "Failed to set HWAS state of "
                "%s, index %d\n",
                data, pdbg_target_index(child));
            ipl_error_callback(IPL_ERR_ATTR_WRITE);*/
            return false;
        }
    }

    PredicateAttrVal<ATTR_TYPE> clkpred(TYPE_OSCREFCLK);

    for (auto&& clock_target :
            ts.getAssociated(top, AssociationType::childByPhysical,
                             RecursionLevel::all, &clkpred))
    {
        // Get Clock position
        AttributeTraits<ATTR_POSITION>::Type clk_pos;

        if(!clock_target->tryGetAttr<ATTR_POSITION>(clk_pos))
        {

            std::cout << "phal-refactor trygetattr failed to get clock's ATTR_POSITION\n";
            /*TODO ipl_log(IPL_ERROR, "Attribute ATTR_POSITION read failed"
                    " for clock '%s' \n", pdbg_target_path(clock_target));*/

            //ipl_plat_procedure_error_handler(IPL_ERR_ATTR_READ_FAIL);

            return false;
        }

        if (!set_or_clear_state(clock_target, true))
        {
            std::cout << "phal-refactor failed to set clock as functional \n";
            return false;
        }
    }
    return true;
}

static int ipl_updatehwmodel(void)
{
	namespace fs = std::filesystem;
	bool boot_file_absent = false;
	constexpr auto GENESIS_BOOT_FILE = "/var/lib/phal/genesisboot";
	fs::path genesis_boot_file = GENESIS_BOOT_FILE;

    std::cout << "phal-refactor istep0.4 ( updatehwmodel ): started\n";

	if (!fs::exists(genesis_boot_file)) {
		ipl_log(IPL_INFO, "updatehwmodel: Genesis mode boot\n");
		if (!update_genesis_hwas_state()) {
			ipl_log(IPL_ERROR,
				"Failed to set genesis boot state\n");
			return 1;
		}

		// Create new file to skip the genesis setup in next boot.
		if (!fs::exists(genesis_boot_file.parent_path())) {
			if (!fs::create_directories(
				genesis_boot_file.parent_path())) {
				ipl_log(IPL_ERROR,
					"Failed to create genesis boot file\n");
				return 1;
			}
		}
		boot_file_absent = true;
		std::ofstream file(GENESIS_BOOT_FILE);
	}


	if ((ipl_type() == IPL_TYPE_MPIPL) ||
	    (!fs::exists(BOOTTIME_GUARD_INDICATOR)))
            apply_fco_override();

	//TODO process_guard_records();

	if (!boot_file_absent && (ipl_type() != IPL_TYPE_MPIPL))
    {
		// Update SBE state to Not usable in reboot path(not on MPIPL)
		// Boot error callback is only required for failure
		ipl_set_sbe_state_all(ipl::sbe::SBE_STATE_NOT_USABLE);

		// update refclock targets  functional sate based on
		// SYS_CLOCK_DECONFIG_STATE values.
		if (!update_clock_func_state())
        {
            std::cout << "phal-refactor ipl0.4 ( updatehwmodel ): updateclockfuncstate failed\n";
		}
	}

	if (getFunctionalMasterProc() == nullptr)
    {
        std::cout << "phal-refactor istep0.4 ( updatehwmodel ): no functional master\n";
		ipl_error_callback(IPL_ERR_PRI_PROC_NON_FUNC);
		return 1;
	}

    std::cout << "phal-refactor istep0.4 ( updatehwmodel ): done\n";
	return 0;
}

static int ipl_alignment_check(void)
{
	return -1;
}

/**
 * @brief Check whether clock reset has to be skipped or not
 *
 * Will look whether file '/tmp/skip_clock_reset' exist or not. If exist
 * this function will return true, indicating clock reset has to be skipped
 *
 *
 * @return true if clock reset has to be skipped, false if clock reset has to be
 * 		performed
 */
static bool skip_clock_reset()
{
	int ret;
	struct stat statbuf;

	// If /tmp/skip_clock_reset file exists, skip clock soft reset
	ret = stat("/tmp/skip_clock_reset", &statbuf);
	if (ret == -1)
		return false;

	if (S_ISREG(statbuf.st_mode)) {
		ipl_log(IPL_INFO, "Skipping clock reset\n");
		return true;
	}

	return false;
}

/**
 * @brief Initialise the clock chip and then check the status of all clocks
 *
 * For all clocks available, do a soft reset, and then check the status register
 * to check whether it encounter a calibration failure or not.
 *
 * @param[out]  clock_select  clock selection type
 *
 * @return 0 on success, 1 on failure
 */
[[maybe_unused]] static int initialize_and_check_clock_chip(uint8_t &clock_select)
{
	struct pdbg_target *clock_target;
	enum pdbg_target_status status;
	uint8_t data;
	int rc = 0;
	int i2c_rc = 0;
	std::vector<std::pair<std::string, std::string>> ffdcs;
	uint8_t clk_pos = 0;
	bool skip_reset = false;
	uint8_t clock_count = 0;

	// Clock reset will revert all i2c write done, which may have written
	// to test firmware for different error scenario, in lab environment.
	// So test team can use this method to skip clock reset while testing.
	skip_reset = skip_clock_reset();

	pdbg_for_each_class_target("oscrefclk", clock_target)
	{
		if (!ipl_is_functional(clock_target))
			continue;

		// clock count has to be incremented even if it failed to
		// initiallize
		clock_count++;
		ffdcs.clear();

		if (!pdbg_target_get_attribute(clock_target, "ATTR_POSITION", 2,
					       1, &clk_pos)) {

			ipl_log(IPL_ERROR,
				"Attribute ATTR_POSITION read failed"
				" for clock '%s' \n",
				pdbg_target_path(clock_target));
			ipl_plat_procedure_error_handler(
			    IPL_ERR_ATTR_READ_FAIL);
			rc++;
			continue;
		}

		// Update clock select value based on clock position
		// Note : Unsupported index value defaults to user initliased
		// value
		if (clk_pos == 0) {
			clock_select = ENUM_ATTR_CP_REFCLOCK_SELECT_OSC0;
		} else if (clk_pos == 1) {
			clock_select = ENUM_ATTR_CP_REFCLOCK_SELECT_OSC1;
		}

		status = pdbg_target_probe(clock_target);
		if (status != PDBG_TARGET_ENABLED) {
			ipl_log(
			    IPL_ERROR,
			    "clock '%s' is not operational, pdbg status = %d\n",
			    pdbg_target_path(clock_target), status);

			ffdcs.push_back(std::make_pair("PDBG_STATUS",
						       std::to_string(status)));
			ffdcs.push_back(std::make_pair("FAIL_TYPE",
						       "CHIP_NOT_OPERATIONAL"));
			ipl_plat_clock_error_handler(ffdcs, clk_pos);
			rc++;
			continue;
		}
		ipl_log(
		    IPL_DEBUG,
		    "Istep: soft reset clock, and verify clock status register"
		    " for clock-%d\n",
		    clk_pos);

		if (!skip_reset) {
			// Resetting the clock chip, so that it will recalibrate
			// input oscillator signal and identifies bad
			// oscillators.
			data = OSC_RESET_CMD;
			i2c_rc = i2c_write(clock_target, 0, OSC_CTL_OFFSET,
					   OSC_RESET_CMD_LEN, &data);
			if (i2c_rc) {
				ipl_log(IPL_ERROR,
					"soft reset command is failed for "
					"clock '%s' with rc = %d\n",
					pdbg_target_path(clock_target), i2c_rc);

				ffdcs.push_back(std::make_pair(
				    "I2C_RC", std::to_string(i2c_rc)));
				ffdcs.push_back(
				    std::make_pair("FAIL_TYPE", "SOFT_RESET"));
				ipl_plat_clock_error_handler(ffdcs, clk_pos);
				rc++;
				continue;
			} else {
				ipl_log(IPL_DEBUG,
					"soft reset command is successfull for "
					"clock '%s'\n",
					pdbg_target_path(clock_target));
			}

			// wait for clock to restart
			std::this_thread::sleep_for(std::chrono::milliseconds(
			    CLOCK_RESTART_DELAY_IN_MS));
		}

		// Read clock status register to check whether it reports
		// calibration error. Bit-0 will be set if there is a
		// calibration error.

		i2c_rc = i2c_read(clock_target, 0, OSC_STAT_OFFSET,
				  OSC_STAT_REG_LEN, &data);
		if (i2c_rc) {
			ipl_log(IPL_ERROR,
				"status register read is failed for clock "
				"'%s', with rc = %d\n",
				pdbg_target_path(clock_target), i2c_rc);

			ffdcs.push_back(
			    std::make_pair("I2C_RC", std::to_string(i2c_rc)));
			ffdcs.push_back(
			    std::make_pair("FAIL_TYPE", "STATUS_READ"));
			ipl_plat_clock_error_handler(ffdcs, clk_pos);
			rc++;
			continue;
		} else {
			ipl_log(
			    IPL_DEBUG,
			    "status register value for clock '%s' is 0x%2X\n",
			    pdbg_target_path(clock_target), data);

			if (data & OSC_STAT_ERR_MASK) {
				ipl_log(IPL_ERROR,
					"Calibration is failed for clock '%s', "
					"status=0x%2X",
					pdbg_target_path(clock_target), data);

				ffdcs.push_back(std::make_pair(
				    "CLOCK_STATUS", std::to_string(data)));
				ffdcs.push_back(
				    std::make_pair("FAIL_TYPE", "CALIB_ERR"));
				ipl_plat_clock_error_handler(ffdcs, clk_pos);
				rc++;
				continue;
			}
		}
	}

	// Check clock count value is valid.
	if ((rc == 0) && (clock_count != 1 && clock_count != 2)) {
		ipl_log(IPL_ERROR,
			"Invalid number (%d) of clock target found\n",
			clock_count);

		ipl_plat_procedure_error_handler(IPL_ERR_INVALID_NUM_CLOCK);
		rc++;
	}

	// Override clock slection value incase spare clock support is available
	if ((rc == 0) && (clock_count == NUM_CLOCK_FOR_REDUNDANT_MODE)) {
		clock_select = fapi2::ENUM_ATTR_CP_REFCLOCK_SELECT_BOTH_OSC0;
	}
	return rc;
}

static int ipl_set_ref_clock(void)
{
	int rc = 0;
try{
    using namespace TARGETING;
	fapi2::ReturnCode fapirc;

	if (ipl_type() == IPL_TYPE_MPIPL)
		return -1;

    std::cout << "phal-refactor istep0.6 ( set_ref_clock ): started\n";

    TargetPtr proc = getFunctionalMasterProc();

	if (proc == nullptr)
    {
        std::cout << "phal-refactor istep0.6 ( set_ref_clock ): proc is nullptr\n";
		//ipl_error_callback(IPL_ERR_PRI_PROC_NON_FUNC);
		return 1;
	}

/*TODO phal-refactor
	if (initialize_and_check_clock_chip(clock_select)) {
		ipl_log(IPL_ERROR, "Clock initialization failed\n");
		return 1;
	}
*/
	// Update clock select mode value.
    // Default value of attribute will be for non-redundant mode
    AttributeTraits<ATTR_CP_REFCLOCK_SELECT>::Type
                            clock_select = CP_REFCLOCK_SELECT_OSC0;

    if(!proc->trySetAttr<ATTR_CP_REFCLOCK_SELECT>(clock_select))
    {
        std::cout << "phal-refactor istep0.6 ( set_ref_clock ): trysetattr failed ATTR_CP_REFCLOCK_SELECT\n";
		/*TODO ipl_log(IPL_ERROR,
			"Attribute CP_REFCLOCK_SELECT update failed"
			" for proc %d \n",
			pdbg_target_index(proc));*/
		//ipl_plat_procedure_error_handler(IPL_ERR_ATTR_WRITE);
		rc++;
		return 1;
    }

	/*TODO ipl_log(IPL_INFO,
		"Running p10_setup_ref_clock HWP on primary processor %d\n",
		pdbg_target_index(proc));*/
    std::cout << "phal-refactor Executing HWP( p10_setup_ref_clock )\n";
	fapirc = p10_setup_ref_clock(proc);
    std::cout << "phal-refactor Done HWP( p10_setup_ref_clock ) \n";

    if (fapirc != fapi2::FAPI2_RC_SUCCESS)
    {

        std::cout << "phal-refactor HWP( p10_setup_ref_clock ) failed fapirc =0x" << static_cast<uint32_t>(fapirc) << std::endl;
		/*TODO ipl_log(IPL_ERROR,
			"Istep set_ref_clock failed on chip %s, rc=%d \n",
			pdbg_target_path(proc), fapirc);*/
		rc++;
	}
    
    std::cout << "phal-refactor istep0.6 ( set_ref_clock ): done\n";
    //TODO ipl_process_fapi_error(fapirc, proc);
}
catch(const std::exception& ex)
{
    std::cout << "exception during istep0.6 (set_ref_clock) exception: " << ex.what() << std::endl;
}
    return rc;
}

static int ipl_proc_clock_test(void)
{
	int rc = 0;
try{
    fapi2::ReturnCode fapirc;

	if (ipl_type() == IPL_TYPE_MPIPL)
		return -1;

    std::cout << "phal-refactor istep0.7 ( proc_clock_test ): started\n";

	TARGETING::TargetPtr proc = getFunctionalMasterProc();

    if (proc == nullptr)
    {
        std::cout << "phal-refactor istep0.7 ( proc_clock_test ): proc is nullptr\n";
		//ipl_error_callback(IPL_ERR_PRI_PROC_NON_FUNC);
		return 1;
	}

	/*TODO ipl_log(IPL_INFO,
		"Running p10_clock_test HWP on primary processor %d\n",
		pdbg_target_index(proc));*/
    std::cout << "phal-refactor Executing HWP( p10_clock_test )\n";
	fapirc = p10_clock_test(proc);
    std::cout << "phal-refactor Done HWP( p10_clock_test ) \n";
	if (fapirc != fapi2::FAPI2_RC_SUCCESS)
    {
        std::cout << "phal-refactor HWP( p10_clock_test ) failed fapirc =0x" << static_cast<uint32_t>(fapirc) << std::endl;
		/*TODO ipl_log(IPL_ERROR, "HWP clock_test failed on proc %d, rc=%d\n",
			pdbg_target_index(proc), fapirc);*/
		rc++;
	}

	//TODO ipl_process_fapi_error(fapirc, proc);

    std::cout << "phal-refactor istep0.7 ( proc_clock_test ): done\n";
}
catch(const std::exception& ex)
{
    std::cout << "exception during istep0.7 ( proc_clock_test ) exception: " << ex.what() << std::endl;
}
	return rc;
}

static int ipl_proc_prep_ipl(void)
{
	return -1;
}

static int ipl_edmarepair(void)
{
	return -1;
}

static int ipl_asset_protection(void)
{
	return -1;
}

static int ipl_proc_select_boot_prom(void)
{
	int rc = 1;
try
{
    std::cout << "phal-refactor istep0.11 ( proc_select_boot_prom ): started\n";
	// Check the availabilty of primary processor.
	if (getFunctionalMasterProc() == nullptr)
    {
		ipl_error_callback(IPL_ERR_PRI_PROC_NON_FUNC);
		return 1;
	}

    using namespace TARGETING;

    auto& ts = TargetService::instance();
    auto top = ts.getTopLevelTarget();

    auto typeProc = std::make_shared<PredicateAttrVal<ATTR_TYPE>>(TYPE_PROC);
    auto masterProc = std::make_shared<PredicateAttrVal<ATTR_PROC_MASTER_TYPE>>(0);
    auto isFunctional = std::make_shared<PredicateIsFunctional>();
    
    PredicatePostfixExpr pred;
    pred.push(typeProc).push(masterProc).And()
        .push(isFunctional).And();
        
    auto proc_target = ts.getAssociated(top, AssociationType::childByPhysical,
                             RecursionLevel::all, &pred);
 
    if(proc_target.empty() || (proc_target.size() != 1))
    {
        std::cerr << "phal-refactor istep0.11 (proc_select_boot_prom): invalid master proc count\n";
        return 1;
    }
    
    fapi2::ReturnCode fapirc;

    /*TODO ipl_log(IPL_INFO,
        "Running p10_select_boot_master HWP on processor %d\n",
        pdbg_target_index(proc));*/
    std::cout << "phal-refactor Executing HWP( p10_select_boot_master )\n";

    fapirc = p10_select_boot_master(proc_target.front());

    std::cout << "phal-refactor Done HWP( p10_select_boot_master ) \n";

    if (fapirc == fapi2::FAPI2_RC_SUCCESS)
        rc = 0;

    //TODO ipl_process_fapi_error(fapirc, proc);
}
catch(const std::exception& ex)
{
    std::cout << "exception during proc_select_boot_prom exception: " << ex.what() << std::endl;
}
    rc = 0;
    std::cout << "phal-refactor istep0.11 ( proc_select_boot_prom ): done\n";
	return rc;
}

static int ipl_hb_config_update(void)
{
	return -1;
}

static int ipl_sbe_config_update(void)
{
	using namespace TARGETING;
	int rc = 1;
try
{
    std::cout << "phal-refactor istep0.13 ( sbe_config_update ): started\n";

	// Check the availabilty of primary processor.
	if (getFunctionalMasterProc() == nullptr)
    {
		ipl_error_callback(IPL_ERR_PRI_PROC_NON_FUNC);
		return 1;
	}

    auto& ts = TargetService::instance();
    auto top = ts.getTopLevelTarget();

    AttributeTraits<ATTR_ISTEP_MODE>::Type istep_mode;

    if(!top->tryGetAttr<ATTR_ISTEP_MODE>(istep_mode))
    {
        std::cout << "phal-refactor istep0.13 ( sbe_config_update ): trygetattr failed ATTR_ISTEP_MODE\n";
		return 1;
	}

	fapi2::buffer<uint32_t> boot_flags;

     // Bit 0 indicates istep IPL (0b1) (Used by SBE, HB – ATTR_ISTEP_MODE)
	if (istep_mode)
		boot_flags.setBit(0);
	else
		boot_flags.clearBit(0);

	// Set the Security Disable bit based on the ATTR_DISABLE_SECURITY
	// attribute. Its "0" by default, i.e. security is always enabled by
	// default, unless user overrides the value.
    AttributeTraits<ATTR_DISABLE_SECURITY>::Type disable_security;

    if(!top->tryGetAttr<ATTR_DISABLE_SECURITY>(disable_security))
    {
        std::cout << "phal-refactor istep0.13 ( sbe_config_update ): trygetattr failed ATTR_DISABLE_SECURITY\n";
		return 1;
	}

	// Bit 4 and 5 Enable SBE FFDC collection.
	boot_flags.setBit(4);
	boot_flags.setBit(5);

	// Bit 6 – disable security. 0b1 indicates disable the security
	if (disable_security)
		boot_flags.setBit(6);
	else
		boot_flags.clearBit(6);


	// bit 7 - Allow hostboot attribute overrides. 0b1 indicates enable
    AttributeTraits<ATTR_ALLOW_ATTR_OVERRIDES>::Type attr_override;
    if(!top->tryGetAttr<ATTR_ALLOW_ATTR_OVERRIDES>(attr_override))
    {
        std::cout << "phal-refactor istep0.13 ( sbe_config_update ): trygetattr failed ATTR_ALLOW_ATTR_OVERRIDES\n";
		return 1;
	}

	if (attr_override)
		boot_flags.setBit(7);
	else
		boot_flags.clearBit(7);

	// bit 11 - Disable denial list based SCOM access. 0b1 indicates disable
    AttributeTraits<ATTR_NO_XSCOM_ENFORCEMENT>::Type scom_allowed;
    if(!top->tryGetAttr<ATTR_NO_XSCOM_ENFORCEMENT>(scom_allowed))
    {
        std::cout << "phal-refactor istep0.13 ( sbe_config_update ): trygetattr failed ATTR_NO_XSCOM_ENFORCEMENT\n";
		return 1;
	}

	if (scom_allowed)
		boot_flags.setBit(11);
	else
		boot_flags.clearBit(11);

    if(!top->trySetAttr<ATTR_BOOT_FLAGS>(boot_flags))
    {
        std::cout << "phal-refactor istep0.13 ( sbe_config_update ): trysetattr failed ATTR_BOOT_FLAGS\n";
		return 1;
	}

    auto isFunctional = std::make_shared<PredicateIsFunctional>();
    auto typeProc = std::make_shared<PredicateAttrVal<ATTR_TYPE>>(TYPE_PROC);
    auto masterProc = 
            std::make_shared<PredicateAttrVal<ATTR_PROC_MASTER_TYPE>>(0); 
    // 0 = master proc

    PredicatePostfixExpr pred;
    pred.push(typeProc).push(masterProc).And()
        .push(isFunctional).And();

    for (auto&& proc :
            ts.getAssociated(top, AssociationType::childByPhysical,
                             RecursionLevel::all, &pred))
    {
		fapi2::ReturnCode fapirc;

        std::cout << "phal-refactor Executing HWP( p10_setup_sbe_config )\n";

        fapirc = p10_setup_sbe_config(proc);

        std::cout << "phal-refactor Done  HWP( p10_setup_sbe_config )\n";

		if (fapirc == fapi2::FAPI2_RC_SUCCESS)
			rc = 0;

		//TODO ipl_process_fapi_error(fapirc, proc);
		break;
	}
}
catch(const std::exception& ex)
{
    std::cout << "exception during sbe_config_update exception: " << ex.what() << std::endl;
}
    
    std::cout << "phal-refactor istep0.13 ( sbe_config_update ): done\n";
    rc = 0;
	return rc;
}

static int ipl_sbe_start(void)
{
	std::cout << "phal-refactor istep0.14 ( sbe_start ): started\n";
    using namespace TARGETING;
    using namespace ipl;

    int rc = 1, ret = 0;
    fapi2::ReturnCode fapirc; 
try{
    auto& ts = TargetService::instance();

    PredicatePostfixExpr pred;
    pred.push(std::make_shared<PredicateAttrVal<ATTR_TYPE>>(TYPE_PROC))
        .push(std::make_shared<PredicateIsFunctional>())
        .And();

    auto top = ts.getTopLevelTarget();

    for (auto&& proc :
            ts.getAssociated(top, AssociationType::childByPhysical,
                             RecursionLevel::all, &pred))
    {
        if (ipl_mode() == IPL_CRONUS)
        {
			fapirc = p10_start_cbs(proc, true);

			if (fapirc != fapi2::FAPI2_RC_SUCCESS)
				ret++;

			//TODO ipl_process_fapi_error(fapirc, proc);
			rc = ret;
			continue;
		}
    }
    
    auto masterProc = std::make_shared<PredicateAttrVal<ATTR_PROC_MASTER_TYPE>>(0);
    pred.push(masterProc).And();
   
    auto proc_target = ts.getAssociated(top, AssociationType::childByPhysical,
                             RecursionLevel::all, &pred);
 
    if(proc_target.empty() || (proc_target.size() != 1))
    {
        std::cerr << "phal-refactor istep0.14 ( sbe_start ): invalid master proc count\n";
        return 1;
    }

    // Run HWP or MPIPL chip-op only on master processor in
	// non cronus mode
    if (ipl_type() == IPL_TYPE_MPIPL)
    {
        ipl_error_type err = ipl::sbe::ipl_sbe_mpipl_continue(proc_target.front());
        ipl_error_callback(err);
        rc = err;
    }
    else
    {
        //ipl_error_type err_type = IPL_ERR_OK;

        std::cout << "phal-refactor Executing HWP( start_cbs )\n";
        fapirc = p10_start_cbs(proc_target.front(), true);

        std::cout << "phal-refactor Done HWP( start_cbs )\n";
        if (fapirc == fapi2::FAPI2_RC_SUCCESS)
        {
            // Update Primary processor SBE state to
            // check cfam. Boot error callback is
            // only required for failure.
            using namespace ipl::sbe;
            ipl_sbe_set_state(proc_target.front(), ipl::sbe::SBE_STATE_CHECK_CFAM);

            if (!ipl_sbe_booted(proc_target.front(), 25))
            {
                std::cout << "phal-refactor istep0.14 ( sbe_start ): sbe failed to boot\n";
                //err_type = IPL_ERR_SBE_BOOT;
            }
            else
            {
                // Update Primary processor SBE
                // state to booted Boot error
                // callback is only required for
                // failure.
                ipl_sbe_set_state(proc_target.front(), ipl::sbe::SBE_STATE_BOOTED);
                rc = 0;
            }
        }
        else
        {
            //err_type = IPL_ERR_HWP;
        }
        //ipl_error_callback(err_type);
    }

	if (!rc)
    {
		// Update Secondary processors SBE state to check cfam
		// Boot error callback is required for failue
        using namespace ipl::sbe;
		ipl_set_sbe_state_all(ipl::sbe::SBE_STATE_CHECK_CFAM, true);

		if (getFunctionalMasterProc() == nullptr)
        {
			ipl_error_callback(IPL_ERR_PRI_PROC_NON_FUNC);
			return 1;
		}
    }
}
catch(const std::exception& ex)
{
    std::cout << "exception during sbe_start exception: " << ex.what() << std::endl;
}
    
    std::cout << "phal-refactor istep0.14 ( sbe_start ): done\n";
    return rc;
}

static int ipl_startPRD(void)
{
	return -1;
}

static int ipl_proc_attn_listen(void)
{
    std::cout << "phal-refactor istep0.16 ( proc_attn_listen ): started\n";
    using namespace TARGETING;

    auto& ts = TargetService::instance();
    auto top = ts.getTopLevelTarget();
    
    auto typeProc = std::make_shared<PredicateAttrVal<ATTR_TYPE>>(TYPE_PROC);
    auto masterProc = std::make_shared<PredicateAttrVal<ATTR_PROC_MASTER_TYPE>>(0);
    PredicatePostfixExpr masterFuncProcPred;
    masterFuncProcPred.push(typeProc).push(masterProc).And();
   
    auto proc_target = ts.getAssociated(top, AssociationType::childByPhysical,
                             RecursionLevel::all, &masterFuncProcPred);
 
    if(proc_target.empty() || (proc_target.size() != 1))
    {
        std::cerr << "phal-refactor istep0.16 ( proc_attn_listen ): invalid master proc count\n";
        return 1;
    }
   
	/*TODO ipl_log(IPL_INFO, "enable attention listen on processor %d\n",
		pdbg_target_index(proc));*/
    

	uint32_t regval; // for register read/write

    // FSI2_PIB_TRUE_MASK
    int rc = hwaccess::HwAccessIntf::getCfamRegister(proc_target.front(), 0x100d, regval); 
	
    if (rc != 0)
    {
		ipl_log(IPL_ERROR, "read TRUEMASK register failed, rc=%d\n",
			rc);
	}
    else 
    {
		// ANY_ERROR, RECOVERABLE_ERROR
		regval &= ~0x90000000; // mask

		// SYSTEM_CHECKSTOP, SPECIAL_ATTENTION,
		// SELFBOOT_ENGINE_ATTENTION
		regval |= 0x60000002; // un-mask

        // FSI2_PIB_TRUE_MASK
        rc = hwaccess::HwAccessIntf::putCfamRegister(proc_target.front(), 0x100d, regval); 

        if (rc != 0) 
        {
			ipl_log(IPL_ERROR,
				"write TRUEMASK register failed, rc=%d\n", rc);
		}
	}

	ipl_error_callback((rc == 0) ? IPL_ERR_OK : IPL_ERR_FSI_REG);

    std::cout << "phal-refactor istep0.16 ( proc_attn_listen ): done\n";
	return rc;	
}

static struct ipl_step ipl0[] = {
    {IPL_DEF(poweron), 0, 1, true, true},
    {IPL_DEF(startipl), 0, 2, true, true},
    {IPL_DEF(DisableAttns), 0, 3, true, true},
    {IPL_DEF(updatehwmodel), 0, 4, true, true},
    {IPL_DEF(alignment_check), 0, 5, true, true},
    {IPL_DEF(set_ref_clock), 0, 6, true, true},
    {IPL_DEF(proc_clock_test), 0, 7, true, true},
    {IPL_DEF(proc_prep_ipl), 0, 8, true, true},
    {IPL_DEF(edmarepair), 0, 9, true, true},
    {IPL_DEF(asset_protection), 0, 10, true, true},
    {IPL_DEF(proc_select_boot_prom), 0, 11, true, true},
    {IPL_DEF(hb_config_update), 0, 12, true, true},
    {IPL_DEF(sbe_config_update), 0, 13, true, true},
    {IPL_DEF(sbe_start), 0, 14, true, true},
    {IPL_DEF(startPRD), 0, 15, true, true},
    {IPL_DEF(proc_attn_listen), 0, 16, true, true},
    {NULL, NULL, -1, -1, false, false},
};

__attribute__((constructor)) static void ipl_register_ipl0(void)
{
	ipl_register(0, ipl0, ipl_pre0);
}
