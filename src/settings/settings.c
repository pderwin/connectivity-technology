#include <zephyr/kernel.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/storage/flash_map.h>
#include "settings.h"

#define NVS_PARTITION		storage_partition
#define NVS_PARTITION_DEVICE	FIXED_PARTITION_DEVICE(NVS_PARTITION)
#define NVS_PARTITION_OFFSET	FIXED_PARTITION_OFFSET(NVS_PARTITION)
#define NVS_PARTITION_SIZE      FIXED_PARTITION_SIZE(NVS_PARTITION)

static uint32_t settings_inited;

#if CONFIG_NVS
static struct nvs_fs fs;
#endif

/*-------------------------------------------------------------------------
 *
 * name:        settings_init
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
void settings_init (void)
{
#if CONFIG_NVS
   int
      rc = 0;
   struct flash_pages_info
      info;

   /*
    * If we have already set up NVS, then don't repeat
    */
   if (settings_inited) {
      return;
   }

/* define the nvs file system by settings with:
 *	sector_size equal to the pagesize,
 *	3 sectors
 *	starting at NVS_PARTITION_OFFSET
 */
   fs.flash_device = NVS_PARTITION_DEVICE;

   if (!device_is_ready(fs.flash_device)) {
      printk("%s: Flash device %s is not ready\n", __func__, fs.flash_device->name);
      return;
   }

   fs.offset = NVS_PARTITION_OFFSET;
   rc = flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
   if (rc) {
      printk("Unable to get page info\n");
      return;
   }

   fs.sector_size = info.size;
   fs.sector_count = (NVS_PARTITION_SIZE / info.size);

   rc = nvs_mount(&fs);

   if (rc) {
      printk("Flash Init failed.  rc: %d\n", rc);
      return;
   }
#endif

   settings_inited = 1;
}


/*-------------------------------------------------------------------------
 *
 * name:        settings_read
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
int32_t settings_read( settings_id_e id, void *dst, uint32_t size )
{
   int
      rc;

   (void) rc;

   settings_init();

   /*
    * Try to read the ID.
    */
#if CONFIG_NVS
   rc = nvs_read(&fs, id, dst, size);

   if (rc != size) {
      printk("%s: error reading id: %d size: %d rc: %d \n", __func__, id, size, rc);
   }
#endif

   return 0;
}

/*-------------------------------------------------------------------------
 *
 * name:        settings_write
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
int32_t settings_write( settings_id_e id, const void *src, uint32_t size )
{
   int
      rc;

   (void) rc;

   settings_init();

#if CONFIG_NVS
   rc = nvs_write(&fs, id, src, size );

   if (rc != size) {
      printk("%s: error reading id: %d size: %d rc: %d \n", __func__, id, size, rc);
   }
#endif

 return 0;
}
