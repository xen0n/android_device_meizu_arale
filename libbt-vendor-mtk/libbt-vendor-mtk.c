/*
 * Bluetooth Vendor Library for MTK's libbluetoothdrv blob
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "libbt_vendor_mtk"

#include <stdio.h>
#include <dlfcn.h>
#include <utils/Log.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <bt_vendor_lib.h>
#include <bt_hci_lib.h>
#include <bt_hci_bdroid.h>
#include <utils.h>

/**
 * TODO: check/fix this value. does this make sense for MTK? It is taken from TI
 * Low power mode: default transport idle timer
 */
#define WL_DEFAULT_LPM_IDLE_TIMEOUT 200

static void *mtklib_handle = NULL;
typedef int (*ENABLE)(int flag, void *func_cb);
typedef int (*DISABLE)(int bt_fd);

unsigned int hci_tty_fd = -1;

ENABLE mtk_bt_enable = NULL;
DISABLE mtk_bt_disable = NULL;

bt_vendor_callbacks_t *bt_vendor_cbacks = NULL;
void hw_config_cback(HC_BT_HDR *p_evt_buf);

/*******************************************************************************
 *
 * Function         hw_config_cback
 *
 * Description      Callback function for controller configuration
 *
 * Returns          None
 *
 * *******************************************************************************/
void hw_config_cback(HC_BT_HDR *p_evt_buf)
{
  ALOGI("hw_config_cback");
}

int mtk_init(const bt_vendor_callbacks_t* p_cb, unsigned char *local_bdaddr) {
  const char *dlerrors;

  ALOGI("libbt-vendor init for MTK blob");

  if (p_cb == NULL)
    {
      ALOGE("init failed with no user callbacks!");
      return BT_HC_STATUS_FAIL;
    }

  bt_vendor_cbacks = (bt_vendor_callbacks_t *) p_cb;

  dlerror();

  mtklib_handle = dlopen("libbluetoothdrv.so", RTLD_LAZY);
  if (! mtklib_handle) {
    ALOGE("Failed to open libbluetoothdrv library");
    return (int)mtklib_handle;
  }

  mtk_bt_enable = dlsym(mtklib_handle, "mtk_bt_enable");
  mtk_bt_disable = dlsym(mtklib_handle, "mtk_bt_disable");

  if ((dlerrors = dlerror()) != NULL){
    ALOGE("Errors while opening symbols from mtk blob");
    dlclose(mtklib_handle);
  }
  return 0;
}

void mtk_cleanup(void) {
  ALOGI("vendor cleanup");
  bt_vendor_cbacks = NULL;

  if (mtklib_handle) {
    dlclose(mtklib_handle);
  }
}

int mtk_open(void **param) {
  int (*fd_array)[] = (int (*)[]) param;
  int fd, idx;
  fd = mtk_bt_enable(0, NULL);
  if (fd < 0) {
    ALOGE("Can't open mtk fd");
    return -1;
  }
  for (idx = 0; idx < CH_MAX; idx++)
    (*fd_array)[idx] = fd;

  hci_tty_fd = fd; /* for userial_close op */
  return 1;  /* CMD/EVT/ACL on same fd */
}

int mtk_close() {
  if (hci_tty_fd == (unsigned int) -1)
    return -1;
  return mtk_bt_disable(hci_tty_fd);
}

int mtk_op(bt_vendor_opcode_t opcode, void **param) {
  int ret = 0;

  switch(opcode)
    {
    case BT_VND_OP_POWER_CTRL:
      break;
    case BT_VND_OP_SCO_CFG:
      break;
    case BT_VND_OP_GET_LPM_IDLE_TIMEOUT:
      *((uint32_t *) param) = WL_DEFAULT_LPM_IDLE_TIMEOUT;
      break;
    case BT_VND_OP_LPM_SET_MODE:
      break;
    case BT_VND_OP_LPM_WAKE_SET_STATE:
      break;
    case BT_VND_OP_USERIAL_OPEN:
      ret = mtk_open(param);
      break;
    case BT_VND_OP_USERIAL_CLOSE:
      ret = mtk_close();
      break;
      /* Since new stack expects fwcfg_cb we are returning SUCCESS here
       * in actual, firmware download is already happened when /dev/hci_tty
       * opened.
       */
    case BT_VND_OP_FW_CFG:
      bt_vendor_cbacks->fwcfg_cb(BT_VND_OP_RESULT_SUCCESS);
      break;
    case BT_VND_OP_EPILOG:
      bt_vendor_cbacks->epilog_cb(BT_VND_OP_RESULT_SUCCESS);
      break;
    default:
      ALOGW("Unknown opcode: %d", opcode);
      break;
    }

  return ret;
}
const bt_vendor_interface_t BLUETOOTH_VENDOR_LIB_INTERFACE  = {
  .init = mtk_init,
  .op = mtk_op,
  .cleanup = mtk_cleanup,
};

int main()
{
  return 0;
}
