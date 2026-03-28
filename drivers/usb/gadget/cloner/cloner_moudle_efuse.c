#include <cloner/cloner.h>
#include "cloner_moudle.h"
#include "cloner_log.h"
#include <efuse.h>

static int32_t clmd_efuse_read(struct cloner *cloner, int sub_type, void *ops_data)
{
	int ret = 0;
	u32 id = cloner->cmd->read.offset;
	void *addr = (void *)cloner->read_req->buf;
	u32 length = cloner->read_req->length;

	ret = efuse_read_id(addr, length, id);
	if (ret < 0)
		printf("efuse read error\n");

	return ret;
}

static int32_t clmd_efuse_write(struct cloner *cloner, int sub_type, void *ops_data)
{
	static int enabled = 0;
	u32 partition, length;
	void *addr;
	int ret = 0;

	if(!enabled) {
		ret = efuse_init(efuse_args->efuse_en_gpio, efuse_args->efuse_en_active);
		if(ret < 0) {
			printf("efuse init error\n");
			return ret;
		}
		enabled = 1;
	}

	partition = cloner->cmd->write.partition;
	length = cloner->cmd->write.length;
	addr = (void *)cloner->write_req->buf;

	ret = efuse_write(addr, length, partition);
	if (ret)
		printf("efuse write error\n");

	return ret;
}

int cloner_efuse_init(void)
{
	struct cloner_moudle *clmd = malloc(sizeof(struct cloner_moudle));
	int ret;

	if (!clmd)
		return -ENOMEM;
	clmd->medium = MAGIC_EFUSE;
	clmd->ops = EFUSE;
	clmd->write = clmd_efuse_write;
	clmd->init = NULL;
	clmd->info = NULL;
	clmd->read = clmd_efuse_read;
	clmd->check = NULL;
	clmd->data = NULL;
//	printf("cloner efuse register\n");
	return register_cloner_moudle(clmd);
}
CLONER_MOUDLE_INIT(cloner_efuse_init);

