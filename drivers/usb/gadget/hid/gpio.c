#include <asm/io.h>

#define GPIO_Z_BASE      (0xb0010700)
#define PZINTS           (0x14)
#define PZINTC           (0x18)
#define PZMSKS           (0x24)
#define PZMSKC           (0x28)
#define PZPAT1S          (0x34)
#define PZPAT1C          (0x38)
#define PZPAT0S          (0x44)
#define PZPAT0C          (0x48)
#define PzGID2LD         (0xf0)


#define GPIO_A_BASE      (0xb0010000)
#define PAPINL           (0x00)


#define	gpio_read_reg(addr)		readl(addr)
#define gpio_write_reg(value, addr)	writel(value, addr)
enum function {
	OUTPUT_LOW,
	OUTPUT_HIGH,
	INPUT,
	LOW_INT,
	HIGH_INT,
	FALL_INT,
	RISE_INT,
};

enum gpio_group {
	GPIO_GROUP_A,
	GPIO_GROUP_B,
	GPIO_GROUP_C,
	GPIO_GROUP_D,
};

static void gpio_in_put(const enum gpio_group gpio_group_t, const int offset)
{
	unsigned int value = (0x01 << offset);

	gpio_write_reg(value, GPIO_Z_BASE + PZINTC);
	gpio_write_reg(value, GPIO_Z_BASE + PZMSKS);
	gpio_write_reg(value, GPIO_Z_BASE + PZPAT1S);
	gpio_write_reg((unsigned int )gpio_group_t, GPIO_Z_BASE + PzGID2LD);

	return;
}

int init_gpio(const enum gpio_group gpio_group_t, const int offset, const enum function f)
{
	switch (f) {
	case OUTPUT_LOW:
	case OUTPUT_HIGH:
		break;
	case INPUT:
		gpio_in_put(gpio_group_t, offset);
		break;
	case LOW_INT:
	case HIGH_INT:
	case FALL_INT:
	case RISE_INT:
		break;
	}

}

static unsigned int gpio_status(const unsigned int addr, const int offset)
{
	unsigned int value = gpio_read_reg(addr);

	return (value >> offset) & 0x01;
}

int get_gpio_status(const enum gpio_group gpio_group_t, const int offset)
{
	unsigned int value = 0;

	switch (gpio_group_t) {
	case GPIO_GROUP_A:
	value = gpio_status(GPIO_A_BASE + PAPINL, offset);
	break;

	case GPIO_GROUP_B:
	break;

	case GPIO_GROUP_C:
	break;

	case GPIO_GROUP_D:
	break;
	}

	return value;
}

int is_press_down(const enum gpio_group gpio_group_t, const int offset)
{
	if (get_gpio_status(gpio_group_t, offset) == 0) {
		udelay(1000 * 200);
	}

	return get_gpio_status(gpio_group_t, offset);
}
