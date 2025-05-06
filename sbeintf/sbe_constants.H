/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* Host Firmware for POWER Systems Project                                */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2025                             */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
#pragma once
#include <cstddef>
#include <cstdint>
namespace sbei
{
/**
 * @brief enums for SBE command classes
 * from sbe/sbe_sp_intf.H
 */
enum sbeCommandClass
{
    SBE_CMD_CLASS_UNKNOWN = 0,
    SBE_CMD_CLASS_IPL_CONTROL = 0xA1,
    SBE_CMD_CLASS_SCOM_ACCESS = 0xA2,
    SBE_CMD_CLASS_GENERIC_MESSAGE = 0xA8,
};

/**
 * @brief enums for SCOM Access Messages
 * from sbe/sbe_sp_intf.H
 */
enum sbeScomAccessCommands
{
    SBE_CMD_GETSCOM = 0x01, /* Get SCOM */
};

/**
 * @brief Chipop Command for Executing Istep
 *
 */
enum SbeCommandIstep_t
{
    SBE_CMD_EXECUTE_ISTEP = 0x01, /**< Execute Istep */
};

/**
 * @brief Chipop Command for Generic message chipop
 * from sbe/sbe_sp_intf.H
 */
enum SbeCommandGenericMsg_t
{
    SBE_CMD_GET_FFDC = 0x01, /**< Get FFDC */
};

/**
 * @brief enums for SBE command timeout values
 * from sbe/sbe_sp_intf.H
 */
enum sbeCmdRespTimeout
{
    SBE_CMD_TIMEOUT_SHORT_IN_MSEC = 100,
    SBE_CMD_TIMEOUT_LONG_IN_MSEC = 30000,
};
// 4 header 4 status 4 offset word 4 data
constexpr uint32_t SBEFIFO_MIN_RESP_LEN = 0x10;

//
// scom constants
constexpr uint32_t SBEFIFO_CMD_CLASS_SCOM_LEN = 0x4;

//
// execute istep constants
constexpr uint32_t SBEFIFO_CMD_CLASS_ISTEP_LEN = 0x3;

//
// execute ffdc constants
constexpr uint32_t SBEFIFO_CMD_CLASS_GET_FFDC_LEN = 0x2;

//
// generic constants
constexpr uint32_t MAGIC_MASK = 0xFFFF0000;
constexpr uint32_t MAGIC_HEADER = 0xC0DE0000;
constexpr uint16_t FFDC_MAGIC = 0xFBAD;
constexpr uint16_t FFDC_HEADER_LEN = 0x5;

constexpr uint32_t MAX_SBE_RESP_SIZE = 0x9999;

// generic macros
constexpr size_t WORD_SIZE = sizeof(uint32_t);
constexpr size_t BYTE_SIZE = sizeof(uint8_t);

constexpr uint32_t HIGH32(uint64_t x)
{
    return static_cast<uint32_t>(x >> 32);
}
constexpr uint32_t LOW32(uint64_t x)
{
    return static_cast<uint32_t>(x & 0xFFFFFFFF);
}

/**
 * @brief enums for primary SBE response
 * from sbe/sbe_sp_intf.H
 */
enum sbePrimResponse
{
    SBE_PRI_OPERATION_SUCCESSFUL = 0x00,
    SBE_PRI_INVALID_COMMAND = 0x01,
    SBE_PRI_INVALID_DATA = 0x02,
    SBE_PRI_USER_ERROR = 0x03,
    SBE_PRI_INTERNAL_ERROR = 0x04,
    SBE_PRI_UNSECURE_ACCESS_DENIED = 0x05,
    SBE_PRI_HWPF_ERROR = 0x06,
    SBE_PRI_PAK_ERROR = 0x07,
    SBE_PRI_PK_ERROR = 0x08,
    SBE_PRI_FIFO_ERROR = 0x09,
    SBE_PRI_SCOM_ERROR = 0x0A,
    SBE_PRI_RET_CODE_ERROR = 0x0B,
    SBE_PRI_RET_CODES_ERROR = 0x0C,
    SBE_PRI_GENERIC_EXECUTION_FAILURE = 0xFE,
};

/**
 * @brief enums for secondary SBE response
 * from sbe/sbe_sp_intf.H
 */
enum sbeSecondaryResponse
{
    SBE_SEC_OPERATION_SUCCESSFUL = 0x00,
    SBE_SEC_COMMAND_CLASS_NOT_SUPPORTED = 0x01,
    SBE_SEC_COMMAND_NOT_SUPPORTED = 0x02,
    SBE_SEC_INVALID_ADDRESS_PASSED = 0x03,
    SBE_SEC_INVALID_TARGET_TYPE_PASSED = 0x04,
    SBE_SEC_INVALID_INSTANCE_ID_PASSED = 0x05,
    SBE_SEC_SPECIFIED_TARGET_NOT_PRESENT = 0x06,
    SBE_SEC_SPECIFIED_TARGET_NOT_FUNCTIONAL = 0x07,
    SBE_SEC_COMMAND_NOT_ALLOWED_IN_THIS_STATE = 0x08,
    SBE_SEC_FUNCTIONALITY_NOT_SUPPORTED = 0x09,
    SBE_SEC_GENERIC_FAILURE_IN_EXECUTION = 0x0A,
    SBE_SEC_DENYLIST_REG_ACCESS = 0x0B,
    SBE_SEC_OS_FAILURE = 0x0C,
    SBE_SEC_FIFO_ACCESS_FAILURE = 0x0D,
    SBE_SEC_UNEXPECTED_EOT_INSUFFICIENT_DATA = 0x0E,
    SBE_SEC_UNEXPECTED_EOT_EXCESS_DATA = 0x0F,
    SBE_SEC_HW_OP_TIMEOUT = 0x10,
    SBE_SEC_PCB_PIB_ERR = 0x11,
    SBE_SEC_FIFO_PARITY_ERROR = 0x12,
    SBE_SEC_TIMER_ALREADY_STARTED = 0x13,
    SBE_SEC_BLACKLISTED_MEM_ACCESS = 0x14,
    SBE_SEC_MEM_REGION_NOT_FOUND = 0x15,
    SBE_SEC_MAXIMUM_MEM_REGION_EXCEEDED = 0x16,
    SBE_SEC_MEM_REGION_AMEND_ATTEMPTED = 0x17,
    SBE_SEC_INPUT_BUFFER_OVERFLOW = 0x18,
    SBE_SEC_INVALID_PARAMS = 0x19,
    SBE_SEC_DENYLIST_CHIPOP_ACCESS = 0x20,
    SBE_SEC_DMT_TIMEOUT = 0x21,
    SBE_SEC_SYSTEM_CHECKSTOP = 0x22,
    SBE_SEC_DENYLIST_REG_ACCESS_BLOCKED = 0x23,
    SBE_SEC_S0_START_MPIPL_FAILED = 0x24,
    SBE_SEC_S0_STOP_CLOCK_FAILED = 0x25,
    SBE_SEC_S0_CONTINUE_MPIPL_FAILED = 0x26,
    SBE_SEC_PERIODIC_IO_TOGGLE_FAILED = 0x27,
    SBE_SEC_SPECIAL_WAKEUP_TIMEOUT = 0x28,
    SBE_SEC_SPECIAL_WAKEUP_SCOM_FAILURE = 0x29,
    SBE_SEC_S0_ARCH_REG_DUMP_FAILED = 0x2A,
    SBE_SEC_LPC_ACCESS_FAILED = 0x2B,
    SBE_SEC_HWP_FAILURE = 0x2C,
    SBE_SEC_TI_CORE_SCRATCH_READ_FAILED = 0x2D,
    SBE_SEC_TI_DATA_READ_FAILED = 0x2E,
    SBE_SEC_PUT_SRAM_FAILED = 0x2F,
    SBE_SEC_GET_SRAM_FAILED = 0x30,
    SBE_SEC_THREAD_CONTROL_INSTR_FAILED = 0x31,
    SBE_SEC_RAM_CORE_SETUP_FAILED = 0x32,
    SBE_SEC_RAM_CORE_ACCESS_FAILED = 0x33,
    SBE_SEC_RAM_CORE_CLEANUP_FAILED = 0x34,
    SBE_SEC_SUSPEND_IO_FAILED = 0x35,
    SBE_SEC_ENTER_MPIPL_FAILED = 0X36,
    SBE_SEC_STOP_CLOCK_FAILED = 0x37,
    SBE_SEC_OCMB_SCOM_FAILED = 0x38,
    SBE_SEC_GET_DUMP_FAILED = 0x39,
    SBE_SEC_INVALID_DUMP_TYPE = 0x40,
    SBE_SEC_INVALID_OCMB_INSTANCE = 0x41,
    SBE_SEC_FAILED_TO_CHECKSTOP = 0x42,
    SBE_SEC_GET_DUMP_STREAM_FAILED = 0x43,
    SBE_SEC_MPIPL_DUMP_INVALID_PARAMS = 0x44,
    SBE_SEC_OCMB_TARGET_NOT_PRESENT = 0x45,
    SBE_SEC_OCMB_TARGET_NOT_FUNCTIONAL = 0x46,
    SBE_SEC_INVALID_FASTARRAY_COLLECTION_INFO = 0x47,
    SBE_SEC_PMIC_HEALTH_CHECK_FAILED = 0x48,
    SBE_SEC_INVALID_I2C_CONFIG_VERSION = 0x49,
    SBE_SEC_INVALID_I2C_TARGET_SUPPORT = 0x4A,
    SBE_SEC_LPC_ERROR = 0x4B,
    SBE_SEC_NOT_ALLOWED_VIA_FIFO_1 = 0x4C,
    SBE_SEC_INVALID_PIPE = 0x4E,
    SBE_SEC_HEAP_SPACE_FULL_FAILURE = 0x4F,
    SBE_SEC_GENERIC_BAD_PAK = 0x50,
    SBE_SEC_POP_UNAVAILABE_PAKS = 0x51,
    SBE_SEC_GETRING_SETUP_FAILED = 0x52,
    SBE_SEC_GETRING_VERIFY_CLEANUP = 0x53,
    SBE_SEC_PUTRING_FAILED = 0x54,
    SBE_SEC_CU_INVALID_IMAGE_TYPE = 0x55,
    SBE_SEC_CU_IMAGE_HASH_SIZE_MISMATCH = 0x56,
    SBE_SEC_CU_FILE_IMAGE_HASH_READ_ERROR = 0x57,
    SBE_SEC_CU_PARTITION_MAGIC_WORD_MISMATCH = 0x58,
    SBE_SEC_CU_PARTITION_MAP_INVALID_IMG_TYPE = 0x59,
    SBE_SEC_CU_IMG_NOT_FOUND_IN_PARTITION_TBL = 0x5A,
    SBE_SEC_CU_FAILED_TO_READ_PARTITION_TABLE = 0x5B,
    SBE_SEC_CU_INVALID_IMAGE_SIZE = 0x5C,
    SBE_SEC_CU_IMAGE_NOT_UPDATABLE = 0x5D,
    SBE_SEC_CU_IMAGE_START_POINTER_FAILED = 0x5E,
    SBE_SEC_CU_IMAGE_SIZE_MORE_THAN_MAX_SIZE = 0x5F,
    SBE_SEC_CU_INVALID_PARTITION_ADDR_RXVD = 0x60,
    SBE_SEC_CU_UPDATE_IMAGE_FAILURE = 0x61,
    SBE_SEC_CU_IMAGE_SIZE_NOT_8BYTE_ALIGNED = 0x62,
    SBE_SEC_CU_INVALID_SIDE_NUMBER_PASSED = 0x63,
    SBE_SEC_CU_PARTITION_TABLE_IS_EMPTY = 0x64,
    SBE_SEC_CU_FAILED_TO_READ_COMPRESSED_FILE = 0x65,
    SBE_SEC_CU_PTBL_TOTAL_IMG_SIZE_INVALID = 0x66,
    SBE_SEC_CU_PAK_END_MARKER_NOT_FOUND = 0x67,
    SBE_SEC_CU_FILE_SIZE_NOT_MATCHING = 0x68,
    SBE_SEC_ODY_DTS_READ_FAILED = 0x69,
    SBE_SEC_CU_SECTION_START_ADDR_MISMATCH = 0x6A,
    SBE_SEC_HEAP_BUFFER_ALLOC_FAILED = 0x6B,
    SBE_SEC_CU_IMAGE_HIGH_ORDER_ADDR_MISMATCH = 0x6C,
    SBE_SEC_CU_IMG_ADDR_N_SZ_EXCEEDS_SIDE_SZ = 0x6D,
    SBE_SEC_CU_ERASE_IMAGE_FAILURE = 0x6E,
    SBE_SEC_CU_WRITE_IMAGE_FAILURE = 0x6F,
    SBE_SEC_FILE_CORRUPTED = 0x70,
    SBE_SEC_FILE_NOT_FOUND = 0x71,
    SBE_SEC_CU_BOTH_SIDES_UPDATE_FAILED = 0x72,
    SBE_SEC_CU_SIDE_1_UPDATE_FAILED = 0x73,
    SBE_SEC_CU_SIDE_0_UPDATE_FAILED = 0x74,
    SBE_SEC_CHIPLET_ID_NOT_PRESENT_IN_SBE = 0x75,
    SBE_SEC_CU_INVALID_PAD_SIZE = 0X76,
    SBE_SEC_CU_SYNC_CHIPOP_FAILURE = 0x77,
    SBE_SEC_CU_READ_IMAGE_FAILURE = 0x78,
    SBE_SEC_ATTR_UPD_FORMAT_ERROR = 0x79,
    SBE_SEC_ATTR_UPD_VER_NOT_SUPPORTED = 0x7A,
    SBE_SEC_ATTR_UPD_INVALID_FILE_TYPE = 0x7B,
    SBE_SEC_UNALIGNED_ADDRESS_PASSED = 0x7C,
    SBE_SEC_INVALID_LENGTH_PASSED = 0x7D,
    SBE_SEC_GIT_COMMIT_ID_FAILURE = 0x7E,
    SBE_SEC_GIT_TIME_STAMP_FAILURE = 0x7F,
    SBE_SEC_INVALID_CLOCK_STATE = 0x80,
    SBE_SEC_INFO_TXT_FORMAT_INVALID = 0x81,
    SBE_SEC_INFO_TXT_FORMAT_VERSION_MISMATCH = 0x82,
    SBE_SEC_FILE_SIZE_IS_ZERO = 0x83,
    SBE_SEC_BUFFER_SIZE_SMALL = 0x84,
    SBE_SEC_NON_ALPHA_NUMERIC_CHAR_FOUND = 0x85,
    SBE_SEC_NO_MEM_TRAIN_DATA = 0x86,
    SBE_SEC_FAST_ARRAY_HWP_FAILURE = 0x87,
    SBE_SEC_CU_MEM_DEV_ID_NOT_SUPPORTED = 0x88,
    SBE_SEC_CU_UNKNOWN_DEVICE = 0x89,
    SBE_SEC_CU_INVALID_BOOT_SIDE = 0x8A,
    SBE_SEC_CU_ECC_USE_DISABLED = 0x8B,
    SBE_SEC_CU_WRITE_DATA_IMAGE_FAILURE = 0x8C,
    SBE_SEC_CU_READ_DATA_IMAGE_FAILURE = 0x8D,
    SBE_SEC_ITS_A_BUG = 0x8E,
    SBE_SEC_ATTR_UPD_BLOB_GEN_FAILED = 0x8F,
    SBE_SEC_CHK_UPDATE_BLOB_GEN_TARGET_TYPES = 0x90,
    SBE_SEC_TEST_FFDC_SUCCESSFUL = 0x91,
    SBE_SEC_RAS_INVALID_DEVICE_ID = 0x92,
    SBE_SEC_IMAGE_SIZE_MISMATCH = 0x93,
    SBE_SEC_CU_WRITE_BEGIN_IMAGE_FAILURE = 0x94,
    SBE_SEC_CU_WRITE_END_IMAGE_FAILURE = 0x95,
    SBE_SEC_PUTRING_HEADER_CHECK_FAIL = 0x96,
    SBE_SEC_HWP_TIMEOUT = 0x97,
    SBE_SEC_ABIST_NOT_DONE = 0x98,
    SBE_SEC_OPCG_NOT_DONE = 0x99,
    SBE_SEC_FILE_HASH_VALIDATION_FAILED = 0x9A,
    SBE_SEC_GIT_TAG_FAILURE = 0x9B,
    SBE_SEC_ATTR_UPD_FAILED = 0x9C,
    SBE_SEC_ATTR_UPD_OUT_BUFF_OVERFLOW = 0x9D,
    // 0x9E to 0xAF are reserved for sbeSecondaryResponse added in hw/sbe and
    // ported to HostFW
    SBE_SEC_GENERIC_HWPF_ERROR = 0xB0,
    SBE_SEC_GENERIC_RETURNCODES_ERROR = 0xB1,
    SBE_SEC_GENERIC_PAK_ERROR = 0xB2,
    SBE_SEC_GENERIC_PK_ERROR = 0xB3,
    SBE_SEC_PAK_PUSH_MAX_LIMIT = 0xB4,
    SBE_SEC_PK_SEMA_ERROR = 0xB5,
    SBE_SEC_PIB_BUSY_ERROR = 0xB6,
    SBE_SEC_PIB_ABORT_ERROR = 0xB7,
    SBE_SEC_OS_TIMER_SERVICE_FAILURE = 0xB8,
    SBE_SEC_INVALID_CHIPOP = 0xB9,
    SBE_SEC_ATTR_TANK_ERROR = 0xBA,
    SBE_SEC_STREAM_EMPTY_BUFF = 0xBB,
    SBE_SEC_STREAM_FULL_BUFF = 0xBC,
    SBE_SEC_UNALIGNED_DATA_LEN = 0xBD,
    SBE_SEC_INVALID_STREAM_SIZE = 0xBE,
    SBE_SEC_FIFO_RESET_RECEIVED = 0xC0,
    SBE_SEC_FIFO_OTHER_SIDE_RESET_RECEIVED = 0xC1,
    SBE_SEC_RX_ROUTINE_MASK_FAILURE = 0xC2,
    SBE_SEC_DATA_IRQ_NOT_RECEIVED = 0xC3,
    SBE_SEC_INT_FIFO_TIMEOUT = 0xC4,
    SBE_SEC_INT_WRONG_CHIPOP_RESP_RECEIVED = 0xC5,
    SBE_SEC_INT_INVALID_END_OF_CHIPOP = 0xC6,
    SBE_SEC_INT_INVALID_MARKER_RECEIVED = 0xC7,
    SBE_SEC_INT_INVALID_SIZE_OF_VECTOR = 0xC8,
};

} // namespace sbei
