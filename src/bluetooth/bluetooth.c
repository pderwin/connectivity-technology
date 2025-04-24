#ifdef CONFIG_BT

#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/settings/settings.h>
#include <zephyr/bluetooth/gatt.h>

#if 0
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>

#include <zephyr/bluetooth/hci.h>

#include "bluetooth.h"
#include "vars.h"
#endif

uint32_t bt_debug;

#define BLUETOOTH_PRIORITY  (2)
#define BLUETOOTH_STACKSIZE (1024)

static const struct bt_data ad[] = {
   BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
   BT_DATA_BYTES(BT_DATA_UUID128_ALL,
		 BT_UUID_128_ENCODE(0x1ff71400, 0xaddc, 0x49da, 0x8bb2, 0xa7026e65426d))
   };

/*-------------------------------------------------------------------------
 *
 * name:        auth_passkey_display
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void auth_passkey_display (struct bt_conn *conn, unsigned int passkey)
{
   char addr[BT_ADDR_LE_STR_LEN];

   bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

   printk("Passkey for %s: %06u\n", addr, passkey);
}

/*-------------------------------------------------------------------------
 *
 * name:        auth_cancel
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void auth_cancel(struct bt_conn *conn)
{
   char addr[BT_ADDR_LE_STR_LEN];

   bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

   printk("Pairing	cancelled: %s\n", addr);
}



/*-------------------------------------------------------------------------
 *
 * name:
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void bt_connected(struct bt_conn *conn, uint8_t err)
{
   (void) conn;

   if (err) {
      printk("Connection failed (err 0x%02x)\n", err);
   } else {
      bt_debug = 1;
      printk("\n\n\n\nConnected\n");
   }
}

/*-------------------------------------------------------------------------
 *
 * name:        bt_disconnected
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void bt_disconnected(struct bt_conn *conn, uint8_t reason)
{
   (void) conn;

   printk("Disconnected (reason 0x%02x)\n", reason);

   bt_debug = 0;

}

/*
 * This macro gets rid of the need to call bt_connection_cb_register()
 */
BT_CONN_CB_DEFINE(conn_callbacks) = {
   .connected    = bt_connected,
   .disconnected = bt_disconnected
};

/*-------------------------------------------------------------------------
 *
 * name:        bt_ready
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void bt_ready(void)
{
   int	err;

   printk("Bluetooth initialized\n");

   if (IS_ENABLED(CONFIG_SETTINGS)) {
      settings_load();
   }

   err	= bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);

   if (err) {
      printk("Advertising	failed to start	(err %d)\n", err);
      return;
   }

   printk("Advertising	successfully started\n");
}

static struct bt_conn_auth_cb auth_cb_display =	{
	.passkey_display = auth_passkey_display,
	.passkey_entry   = NULL,
	.cancel		 = auth_cancel,
};

static const struct bt_uuid_128 test_uuid = BT_UUID_INIT_128(
	0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12,
	0x78, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12);
static const struct bt_uuid_128 test_chrc_uuid = BT_UUID_INIT_128(
	0xf2, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12,
	0x7a, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12);

static uint8_t test_value[] = { 'T', 'e', 's', 't', '\0' };

static ssize_t read_test(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset)
{
	const char *value = attr->user_data;

	printk("RRRRRRRRRRRRRRRRRRRR %s %d (from %p) \n", __func__,__LINE__, __builtin_return_address(0) );

	return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
				 strlen(value));
}

static ssize_t write_test(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags)
{
	uint8_t *value = attr->user_data;

	printk("WWWWWWWWWWWWWWWWWWWWWWWW %s %d (from %p) \n", __func__,__LINE__, __builtin_return_address(0) );

	if (offset + len > sizeof(test_value)) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);

	return len;
}

static struct bt_gatt_attr test_attrs[] = {
	/* Vendor Primary Service Declaration */
	BT_GATT_PRIMARY_SERVICE(&test_uuid),

	BT_GATT_CHARACTERISTIC(&test_chrc_uuid.uuid,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
			       (BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN),
			       read_test, write_test, test_value),
	BT_GATT_CHARACTERISTIC(&test_chrc_uuid.uuid+1,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
			       (BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN),
			       read_test, write_test, test_value),
	BT_GATT_CHARACTERISTIC(&test_chrc_uuid.uuid+2,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
			       (BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN),
			       read_test, write_test, test_value),
};

static struct bt_gatt_service test_svc = BT_GATT_SERVICE(test_attrs);

/*-------------------------------------------------------------------------
 *
 * name:        bluetooth_thread
 *
 * description: thread for bluetooth support
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
void bluetooth_thread(void *p1, void *p2, void *p3)
{
   char
      buf[CONFIG_BT_DEVICE_NAME_MAX];
   int
      rc;

   (void) p1;
   (void) p2;
   (void) p3;

   k_thread_name_set(NULL, "bt_thread");

   rc = bt_enable(NULL);
   if (rc) {
      printk("%s: bt_enable failed (rc: %d)\n", __func__, rc);
      return;
   }

   /*
    * TO-DO: include the device ID from the Lora DEV EUI
    */
   snprintf(buf, sizeof(buf), "%s-%s",	CONFIG_BT_DEVICE_NAME, "phil" );
   bt_set_name(buf);

   bt_ready();

   bt_conn_auth_cb_register(&auth_cb_display);

   rc = bt_gatt_service_register(&test_svc);
   printk("GATT service register rc: %d \n", rc);

   while (1) {
      k_sleep(K_MSEC(1000));
   }
}

K_THREAD_STACK_DEFINE(bluetooth_stack, BLUETOOTH_STACKSIZE);
struct k_thread	bluetooth_k_thread;


/*
 * start up bluetooth thread
 */
void bluetooth_init (void)
{
   k_thread_create(&bluetooth_k_thread, bluetooth_stack,
		   K_THREAD_STACK_SIZEOF(bluetooth_stack),
		   bluetooth_thread,
		   NULL, NULL,	NULL,
		   BLUETOOTH_PRIORITY,
		   0,
		   K_NO_WAIT);
}

#endif
