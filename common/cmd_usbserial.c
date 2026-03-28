#include <common.h>
#include <command.h>

#define USB_TEST_BUF_SIZE 8192
char usb_test_buf[USB_TEST_BUF_SIZE];
char usb_test_char[] = "Test message!\r\n";
static int total = 0;

static int do_gser(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int read_len, write_len, i;

	if (argc <= 1)
		return cmd_usage(cmdtp);

	if (strcmp(argv[1], "connect") == 0) {
		usb_gser_register();
	} else if (strcmp(argv[1], "read") == 0) {
		while (!ctrlc()) {
			usb_gadget_handle_interrupts();
			memset(usb_test_buf, 0, sizeof(usb_test_buf));
			read_len = acm_read(usb_test_buf, sizeof(usb_test_buf));
			if (read_len > 0) {
				total += read_len;
				printf("read %d bytes, total read data %d bytes\n", read_len, total);
				//for (i = 0; i < read_len; i++)
				//	printf("%c", usb_test_buf[i]);
			}
		}
	} else if (strcmp(argv[1], "write") == 0) {
		while (!ctrlc()) {
			usb_gadget_handle_interrupts();
			write_len = acm_write(usb_test_char, strlen(usb_test_char));
			if (write_len > 0) {
				printf("write data: %d bytes\n", write_len);
			}
		}
	} else {
		return cmd_usage(cmdtp);
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	gser,	3,	1,	do_gser,
	"USB serial commands",
	"gser connect - Register and Connect USB serial device\n"
	"gser read - Read data through a serial device\n"
	"gser write - Write data through a serial device\n"
);

