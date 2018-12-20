#ifndef __FIM5360_H_INCLUDED__
#define __FIM5360_H_INCLUDED__

#include "Arduino.h"

#define CMD_REQUEST_CONNECTION              0x01
#define CMD_GET_FIRMWARE_VERSION2           0x04
#define CMD_GET_DEVICE_INFO                 0x05
#define CMD_VERIIFY_FP                      0x11
#define CMD_IDENTIFY_FP                     0x12
#define CMD_AUTO_IDENTIFY                   0x1A
#define CMD_AUTO_IDENTIFY_RESULT            0x1B
#define CMD_DELETE_FP                       0x22
#define CMD_DELETE_ALL_FP                   0x23
#define CMD_SET_MASTER                      0x24
#define CMD_LEAVE_MASTER_MODE               0x26
#define CMD_SET_MASTER_PASSWORD             0x27
#define CMD_READ_USER_DATA                  0x2B
#define CMD_WRITE_USER_DATA                 0x2C
#define CMD_DELETE_MASTER_PASSWORD          0x2E
#define CMD_ENTER_MASTER_MODE2              0x2F
#define CMD_GET_FP_LIST2                    0x30
#define CMD_GET_MASTER_LIST2                0x31
#define CMD_READ_LOG_DATA                   0x32
#define CMD_REGISTER_FP                     0x33
#define CMD_SET_LOG_OPTION                  0x48
#define CMD_GET_LOG_OPTION                  0x49
#define CMD_GET_SYS_INFO                    0x4D
#define CMD_SET_TIME                        0x65
#define CMD_GET_TIME                        0x66
#define CMD_GET_IMAGE_QUALITY               0x68

#define ERR_NONE                            0x00
#define ERR_CHECKSUM_ERROR                  0x02
#define ERR_INVALID_CMD                     0x05

#define RESULT_SUCCEEDED           0x01
#define RESULT_FAILED              0x02 
#define RESULT_NOT_MASTER_MODE     0x03 
#define RESULT_USED_ID             0x04 
#define RESULT_INVALID_ID          0x05 
#define RESULT_DB_IS_FULL          0x06 
#define RESULT_NOT_IN_TIME         0x07 
#define RESULT_INVALID_PARAM       0x09 
#define RESULT_OPP_INIT_FAILED     0x0C 
#define RESULT_CANCELED            0x0D 
#define RESULT_ANOTHER_FINGER      0x0E 
#define RESULT_IDLE_STATUS         0x10 
#define RESULT_TOO_LARGE_DATA      0x11 
#define RESULT_IDENTIFY_TIMEOUT    0x12 
#define RESULT_DB_ISNOT_EMPTY      0x13 
#define RESULT_WRONG_TEMP_MODE     0x14 
#define RESULT_INVALID_DATASIZE    0x15 
#define RESULT_INVALID_DATA        0x16 
#define RESULT_EXTRACT_FAIL        0x17 
#define RESULT_NOT_SUPPORTED       0x18 
#define RESULT_AUTO_IDENTIFY_MODE  0x19 
#define RESULT_INVALID_SEQUNCE     0x20 

#define FIM5360_SUCC_PORT    43
#define FIM5360_FAIL_PORT    42
#define FIM5360_REG_PORT     37
#define FIM5360_DEL_PORT     36
#define FIM5360_IDENT_PORT   35

typedef struct tagFIM5360HDT {
  uint32_t cmd;
  uint32_t p1;
  uint32_t p2;
  uint32_t ds;
  uint32_t ec;
} FIM5360HDT;

typedef struct tagFIM5360HCS {
  FIM5360HDT hdt;
  uint32_t chsum;
} FIM5360HCS;

typedef struct tagFIM5360SPH {
  uint8_t lead;
  union u {
    byte bufx[24];
    FIM5360HCS hcs;
  } u;
  uint8_t sdr;
} FIM5360SPH;

#endif /* __FIM5360_H_INCLUDED__ */

