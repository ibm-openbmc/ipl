#pragma once

#include "libipl.H"
#include <targeting/target.H>
#include <targeting/target_service.H>
#include <hwaccess/hw_access_intf.H>

/*The sbe register and state values in this file picked
  from libpdbg/libpdbg_sbe.H & libpdbg/sbe_api.C to decouple
  targeting-sbe method's dependency on pdbg library*/

namespace ipl::sbe
{
#define SBE_MSG_REG	0x2809
#define SBE_STATE_REG	0x2986

enum sbe_state {
	SBE_STATE_NOT_USABLE  = 0x00000000,
	SBE_STATE_BOOTED      = 0x00000001,
	SBE_STATE_CHECK_CFAM  = 0x00000002,
	SBE_STATE_DEBUG_MODE  = 0x00000003,
	SBE_STATE_FAILED      = 0x00000004,

	SBE_STATE_INVALID     = 0x0000000F,
};

enum {
	SBE_MSG_STATE_UNKNOWN = 0x0,
	SBE_MSG_STATE_IPLING  = 0x1,
	SBE_MSG_STATE_ISTEP   = 0x2,
	SBE_MSG_STATE_MPIPL   = 0x3,
	SBE_MSG_STATE_RUNTIME = 0x4,
	SBE_MSG_STATE_DMT     = 0x5,
	SBE_MSG_STATE_DUMP    = 0x6,
	SBE_MSG_STATE_FAILURE = 0x7,
	SBE_MSG_STATE_QUIESCE = 0x8,

	SBE_MSG_STATE_INVALID = 0xF,
};

union sbe_msg_register {
	uint32_t reg;
	struct {
		uint32_t reserved2: 6;
		uint32_t minor_step: 6;
		uint32_t major_step: 8;
		uint32_t curr_state: 4;
		uint32_t prev_state: 4;
		uint32_t reserved1: 2;
		uint32_t async_ffdc: 1;
		uint32_t sbe_booted: 1;
	};
};


int ipl_sbe_set_state(TARGETING::ConstTargetPtr target, enum sbe_state state);
int ipl_sbe_get_state(TARGETING::ConstTargetPtr target, enum sbe_state *state);
int sbe_mpipl_continue(TARGETING::ConstTargetPtr target);
bool ipl_sbe_booted(TARGETING::TargetPtr target, uint32_t wait_time_seconds);
int ipl_set_sbe_state_all(enum sbe_state state, bool skipMaster=false);
ipl_error_type ipl_sbe_mpipl_continue(TARGETING::ConstTargetPtr target);
} // namespace ipl::sbe
