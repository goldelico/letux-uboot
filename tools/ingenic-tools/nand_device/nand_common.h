#ifndef __NAND_COMMON_H
#define __NAND_COMMON_H


struct device_struct {
	unsigned short device_id;
	unsigned int  page_size;
	unsigned char addr_len;
	unsigned char ecc_bit;
	unsigned char bit_counts;
	unsigned char eccstat_count;
	unsigned char *eccerrstatus;
        unsigned char plane_select;
};

struct nand_desc {
	unsigned char id_manufactory;
	unsigned int  device_counts;
	struct device_struct *device;
};

#define DEVICE_STRUCT(id, pagesize, addrlen, bit, bitcounts, eccstatcount, err, plane) {  \
		.device_id = id, 	\
		.page_size = pagesize,  \
		.addr_len = addrlen,    \
		.ecc_bit = bit,	\
		.bit_counts = bitcounts,	\
		.eccstat_count = eccstatcount, \
		.eccerrstatus = err,	\
		.plane_select = plane, 	\
}

#define ARRAY_SIZE(x)		((sizeof(x))/(sizeof(x[0])))

int nand_register(struct nand_desc *);
#endif
