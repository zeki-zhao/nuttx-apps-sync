#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "netutils/cJSON.h"
#include "sd_handler.h"
#include "lvgl_event.h"
#include <arch/board/board_paths.h>
#include <nuttx/mtd/mtd.h>
#include <nuttx/fs/fs.h>
#include <syslog.h>

volatile bool g_upgrade_busy = false;

#define SD_SAVE_FILE  "lvgl_input.txt"

void save_text_handler(const struct lvgl_msg_s *msg)
{
    if (!msg->text)
        return;

    /* generate unique filename with a counter */
    char path[128];
    snprintf(path, sizeof(path), SD_LOG_DIR "/" SD_SAVE_FILE);
    mkdir(SPI_FLASH_CONFIG_DIR, 0777);
    FILE *fp = fopen(path, "w");
    if (fp){
        fwrite(msg->text, 1, strlen(msg->text), fp);
        fclose(fp);
        syslog(LOG_NOTICE,"Saved: %s\n", path);
    }else{
        syslog(LOG_NOTICE,"ERROR: Failed to write %s: %d\n", path, errno);
    }

    free(msg->text);
}


void save_modbus_slave_show_config_hander(const struct lvgl_msg_s *msg)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "start_addr", msg->start_addr);
    cJSON_AddNumberToObject(root, "num_rows", msg->num_rows);
    cJSON_AddNumberToObject(root, "reg_type", msg->reg_type);

    char *out = cJSON_Print(root);

    char path[128];
    snprintf(path, sizeof(path), SPI_FLASH_CONFIG_DIR "/modbus_slave_show_status.json");
    mkdir(SPI_FLASH_CONFIG_DIR, 0777);
    FILE *fp = fopen(path, "w");
    if (fp) {
        fwrite(out, 1, strlen(out), fp);
        fclose(fp);
        syslog(LOG_NOTICE,"Saved: %s\n", path);
    } else {
        syslog(LOG_NOTICE,"ERROR: Failed to write %s: %d\n", path, errno);
    }

    cJSON_Delete(root);
    free(out);
}

char *load_text_from_sd(void)
{
    char path[128];
    snprintf(path, sizeof(path), SD_LOG_DIR "/" SD_SAVE_FILE);

    FILE *fp = fopen(path, "r");
    if (!fp)
        return NULL;

    /* Get file size */
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (fsize <= 0)
    {
        fclose(fp);
        return NULL;
    }

    char *buf = malloc(fsize + 1);
    if (!buf)
    {
        fclose(fp);
        return NULL;
    }

    size_t nread = fread(buf, 1, fsize, fp);
    fclose(fp);
    buf[nread] = '\0';

    return buf;
}


void move_sd_firmware_to_flash(const struct lvgl_msg_s *msg)
{
    int fd;
    ssize_t nread;
    struct mtd_geometry_s geo;
    off_t blk = 0;
    size_t total = 0;
    uint8_t* g_ota_buf = malloc(4096);

  
    char path[100];
    char IamgeDevPath[20];
    switch (msg->slot)
    {
        case 2:
            printf("move update firmware to flash\n");
            snprintf(IamgeDevPath, sizeof(IamgeDevPath), CONFIG_NXBOOT_SECONDARY_SLOT_PATH);
            snprintf(path, sizeof(path), SD_FIRMWARE_DIR "/update/nuttx.img");
            break;

        default:
            printf("error slot\n");
            return;
    }

    fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        printf("can't open %s: %d\n", path, errno);
        return;
    }
    FAR struct inode *mtd_inode;
    int ret = find_mtddriver(IamgeDevPath, &mtd_inode);
    if (ret < 0)
    {
        printf("FAILED to find secondary mtd driver: %d\n", ret);
        close(fd);
        free(g_ota_buf);
        return;
    }

    FAR struct mtd_dev_s *mtd = mtd_inode->u.i_mtd;

    /* Get MTD geometry for block size */
    if (mtd->ioctl(mtd, MTDIOC_GEOMETRY,
                                (unsigned long)(uintptr_t)&geo) < 0)
    {
        printf("FAILED to get MTD geometry\n");
        close_mtddriver(mtd_inode);
        close(fd);
        return;
    }

    /* 擦除 Secondary */
    if (mtd->ioctl(mtd, MTDIOC_BULKERASE, 0) < 0)
    {
        printf("FAILED to erase Secondary\n");
        close_mtddriver(mtd_inode);
        close(fd);
        return;
    }

    /* 重新读取并写入 Secondary */
    lseek(fd, 0, SEEK_SET);
    blk = 0;
    total = 0;

    while ((nread = read(fd, g_ota_buf, 4096)) > 0)
    {
        size_t nsectors = (nread + geo.blocksize - 1) / geo.blocksize;
        size_t full_size = nsectors * geo.blocksize;

        /* NOR Flash 擦除态为 0xFF，填 0xFF 保持擦除态 */
        if (nread < full_size)
            memset(g_ota_buf + nread, 0xff, full_size - nread);

        if (mtd->bwrite(mtd, blk, nsectors,
                                    g_ota_buf) != nsectors)
        {
            printf("FAILED to write at block %ld\n", (long)blk);
            close_mtddriver(mtd_inode);
            close(fd);
            free(g_ota_buf);
            return;
        }
        blk += nsectors;

        total += nread;
        printf("\r  write %lu bytes to flash", (unsigned long)total);
        fflush(stdout);
    }

    printf("\n  total %lu bytes written\n", (unsigned long)total);

    /* 回读验证 */
    lseek(fd, 0, SEEK_SET);
    blk = 0;

    uint8_t *verify_buf = malloc(4096);
    if (!verify_buf)
    {
        printf("FAILED to alloc verify buffer\n");
        close_mtddriver(mtd_inode);
        free(g_ota_buf);
        close(fd);
        return;
    }

    printf("Verifying...\n");
    while ((nread = read(fd, g_ota_buf, 4096)) > 0)
    {
        size_t nsectors = (nread + geo.blocksize - 1) / geo.blocksize;

        if (mtd->bread(mtd, blk, nsectors,
                                   verify_buf) != nsectors)
        {
            printf("FAILED to read back at block %ld\n", (long)blk);
            close_mtddriver(mtd_inode);
            free(verify_buf);
            free(g_ota_buf);
            close(fd);
            return;
        }

        if (memcmp(g_ota_buf, verify_buf, nread) != 0)
        {
            printf("VERIFY FAILED at block %ld\n", (long)blk);
            close_mtddriver(mtd_inode);
            free(verify_buf);
            free(g_ota_buf);
            close(fd);
            return;
        }

        blk += nsectors;
    }

    printf("Verify OK\n");
    g_upgrade_busy = false;
    close_mtddriver(mtd_inode);
    free(verify_buf);
    free(g_ota_buf);
    close(fd);

    // int ret = unlink(path);
    // if (ret < 0)
    //     printf("WARNING: Failed to remove %s: %d\n", path, errno);
    // else
    //     printf("Removed %s\n", path);
}


