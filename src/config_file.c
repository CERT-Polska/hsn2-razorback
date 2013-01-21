#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/sysctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>

#include "config_file.h"
#include <libconfig.h>

// Libconfig handle
static config_t config;

typedef enum
{
    UNINIT, READY, CONFIGURED, UNCONFIGURED
} CONFIGSTATE;

struct RZBConfigFile
{
    RZBConfKey_t *type;
    config_t config;
    struct RZBConfigFile *next;
};

struct RZBConfigFile;
typedef struct RZBConfigFile RZBConfigFile;

RZBConfigFile *configList = NULL;

CONFIGSTATE configState = UNCONFIGURED;

bool testFile (const char *);
bool parseBlock (config_t *, RZBConfKey_t *);
static char *getConfigFile (const char *, const char *);
bool parseRoutingType (const char *, conf_int_t *);

char *getConfigFile (const char *, const char *);
char * getConfigFile (const char *configDir, const char *configFile) {
	char *result;
	if (configDir == NULL) {
		configDir = ETC_DIR;
	}
	result =
		calloc (strlen (configDir) + strlen (configFile) + 2, sizeof (char));
	if (result == NULL)
		return NULL;

	strncpy (result, configDir, strlen (configDir) + 1);
	strncat (result, "/", 1);
	strncat (result, configFile, strlen (configFile));

	return result;
}

SO_PUBLIC bool
readMyConfig (const char *configDir, const char *configFile,
			  RZBConfKey_t * config_fmt) {
	if (configFile == NULL) {
		rzb_log (LOG_EMERG, "%s: configFile was null", __func__);
		return false;
	}
	if (configState == UNINIT) {
		memset (&config, 0, sizeof (config));
		config_init (&config);
		configState = READY;
	}

	char *configfile = getConfigFile (configDir, configFile);
	RZBConfigFile * file;
	if ((file =calloc(1, sizeof (RZBConfigFile))) == NULL) {
		rzb_log(LOG_ERR, "%s: Failed to allocate config storage", __func__);
		free(configfile);
		return false;
	}

	file->type = config_fmt;
	if (!testFile (configfile)) {
		free (configfile);
		return false;
	}
	if (config_read_file (&file->config, configfile) != CONFIG_TRUE)
		rzb_log (LOG_ERR, "%s: failed to read file: %s", __func__, config_error_text (&config));

	if (configList == NULL) {
		configList = file;
	}
	else {
		file->next = configList;
		configList = file;
	}
	free (configfile);
	return parseBlock (&file->config, config_fmt);
}

static bool
parseArray(config_setting_t *config, RZBConfKey_t *block)
{
    int size;
    struct ConfArray *arrayConf = block->callback;
    void *data;

    conf_int_t *ints;
    char **strings;
    const char *tmp;
    uuid_t *uuids;
    bool *bools;

    size = config_setting_length(config);

    rzb_log(LOG_INFO, "%s: Array found: %d", block->key, size);
    if (size == 0)
    {
        *arrayConf->count = 0;
        return true;
    }

    if (arrayConf->type == RZB_CONF_KEY_TYPE_INT)
    {
        if ((data = calloc(size, sizeof(conf_int_t)))== NULL)
            return false;
        ints = data;
        for (conf_int_t i = 0; i < size; i++)
            ints[i] = config_setting_get_int_elem(config, i);

    }
    else if (arrayConf->type == RZB_CONF_KEY_TYPE_STRING)
    {
        if ((data = calloc(size, sizeof(char *)))== NULL)
            return false;
        strings = data;
        for (conf_int_t i = 0; i < size; i++)
            strings[i] = (char *)config_setting_get_string_elem(config, i);
    }
    else if (arrayConf->type == RZB_CONF_KEY_TYPE_PARSED_STRING)
    {
        if ((data = calloc(size, sizeof(conf_int_t)))== NULL)
            return false;
        ints = data;
        for (conf_int_t i = 0; i < size; i++)
        {
            tmp = config_setting_get_string_elem(config, i);
            if (!(arrayConf->parseString (tmp, ints++)))
                return false;

        }
    }
    else if (arrayConf->type == RZB_CONF_KEY_TYPE_UUID)
    {
        if ((data = calloc(size, sizeof(uuid_t)))== NULL)
            return false;
        uuids = data;
        for (conf_int_t i = 0; i < size; i++)
        {
            tmp = config_setting_get_string_elem(config, i);
            if (uuid_parse (tmp, (uuids[i])) == -1)
            {
                rzb_log (LOG_ERR, "%s: Failed to parse UUID: %s", __func__, tmp);
                return false;
            }
        }

    }
    else if (arrayConf->type == RZB_CONF_KEY_TYPE_BOOL)
    {
        if ((data = calloc(size, sizeof(bool)))== NULL)
            return false;
        bools = data;
        for (conf_int_t i = 0; i < size; i++)
        {
            tmp = config_setting_get_string_elem(config, i);
            if (strncasecmp(tmp,"true",4) == 0)
                bools[i] = true;
            else if (strncasecmp(tmp,"false",5) == 0)
                bools[i] = false;
            else
            {
                rzb_log (LOG_ERR, "%s: Failed to parse bool: %s", __func__, tmp);
                return false;
            }
        }

    }
    else
    {
        rzb_log (LOG_ERR, "%s: Unsupported array config attribute type.", __func__);
        return false;
    }

    *arrayConf->count = size;
    *arrayConf->data = data;
    return true;
}

static bool
parseList(config_setting_t *config, RZBConfKey_t *block)
{
    int size =0, itemSize =0;
    struct ConfList *listConf = block->callback;
    RZBConfKey_t * cur;
    void *data;
    char *itemData;
    config_setting_t *item;
	conf_int_t *intItem;
    const char *tmp;
    bool *boolItem;
    uuid_t *uuidItem;
	conf_int_t i;

    size = config_setting_length(config);

    rzb_log(LOG_INFO, "%s: List found: %d", block->key, size);
    if (size == 0)
    {
        *listConf->count = 0;
        return true;
    }
    cur = listConf->items;
    while (cur->type != RZB_CONF_KEY_TYPE_END)
    {
        if (cur->type == RZB_CONF_KEY_TYPE_INT)
            itemSize += sizeof(conf_int_t);
        else if (cur->type == RZB_CONF_KEY_TYPE_STRING)
            itemSize += sizeof(char *);
        else if (cur->type == RZB_CONF_KEY_TYPE_PARSED_STRING)
            itemSize += sizeof(conf_int_t);
        else if (cur->type == RZB_CONF_KEY_TYPE_UUID)
            itemSize += sizeof(uuid_t);
        else if (cur->type == RZB_CONF_KEY_TYPE_BOOL)
            itemSize += sizeof(bool);
        else
        {
            rzb_log (LOG_ERR, "%s: Unsupported list member attribute type.", __func__);
            return false;
        }

        cur++;
    }
    if ((data = calloc(size, itemSize)) == NULL)
        return false;


    itemData = data;

    for (i = 0; i < size; i++)
    {
        cur = listConf->items;
        item = config_setting_get_elem(config, i);
        while (cur->type != RZB_CONF_KEY_TYPE_END)
        {
            if (cur->type == RZB_CONF_KEY_TYPE_INT)
            {
                intItem= (conf_int_t*)itemData;
                config_setting_lookup_int(item, cur->key, intItem);
                itemData += sizeof(conf_int_t);
            }
            else if (cur->type == RZB_CONF_KEY_TYPE_STRING)
            {
                config_setting_lookup_string(item, cur->key, &tmp);
                // Filthy
                memcpy(itemData, (char *)&tmp, sizeof(char *));
                itemData += sizeof(char *);
            }
            else if (cur->type == RZB_CONF_KEY_TYPE_PARSED_STRING)
            {
                config_setting_lookup_string(item, cur->key, &tmp);
                intItem= (conf_int_t*)itemData;
                if (!((RZBConfCallBack *)cur->callback)->parseString (tmp, intItem))
                    return false;
                itemData +=sizeof(conf_int_t);
            }
            else if (cur->type == RZB_CONF_KEY_TYPE_UUID)
            {
                config_setting_lookup_string(item, cur->key, &tmp);
                uuidItem = (uuid_t *)itemData;
                if (uuid_parse (tmp, *uuidItem) == -1)
                {
                    rzb_log (LOG_ERR, "%s: Failed to parse UUID: %s", __func__, tmp);
                    return false;
                }

                itemData += sizeof(uuid_t);
            }
            else if (cur->type == RZB_CONF_KEY_TYPE_BOOL)
            {
                config_setting_lookup_string(item, cur->key, &tmp);
                boolItem= (bool*)itemData;
                if (strncasecmp(tmp,"true",4) == 0)
                    *boolItem = true;
                else if (strncasecmp(tmp,"false",5) == 0)
                    *boolItem = false;
                else
                {
                    rzb_log (LOG_ERR, "%s: Failed to parse bool: %s", __func__, tmp);
                    return false;
                }
                itemData += sizeof(bool);
            }
            else
            {
                rzb_log (LOG_ERR, "%s: Unsupported list member attribute type.", __func__);
                return false;
            }

            cur++;
        }
    }
    *listConf->data = data;
    *listConf->count = size;

    return true;
}

bool
parseBlock (config_t * config, RZBConfKey_t * block)
{
    int status = CONFIG_TRUE;
    conf_int_t t;
    config_setting_t *tt;
    const char *type;
    while (block->type != RZB_CONF_KEY_TYPE_END)
    {
        tt = config_lookup (config, block->key);
        if (tt == NULL)
        {
            rzb_log (LOG_WARNING, "%s: Cant find key: %s", __func__, block->key);
            block++;
            continue;
        }

        if (block->type == RZB_CONF_KEY_TYPE_INT)
        {
            status = config_lookup_int (config, block->key, &t);
            *(conf_int_t *) (block->dest) = t;
        }
        else if (block->type == RZB_CONF_KEY_TYPE_STRING)
        {
            status = config_lookup_string (config, block->key, block->dest);
        }
        else if (block->type == RZB_CONF_KEY_TYPE_PARSED_STRING)
        {
            status = config_lookup_string (config, block->key, &type);
            if (status != CONFIG_TRUE)
            {
                rzb_log (LOG_ERR, "%s: failed to lookup string: %s", __func__, config_error_text (config));
                return false;
            }
            if (!((RZBConfCallBack *)block->callback)->parseString (type, &t))
                return false;

            *(int *) (block->dest) = (int) t;
        }
        else if (block->type == RZB_CONF_KEY_TYPE_UUID)
        {
            status = config_lookup_string (config, block->key, &type);
            if (status != CONFIG_TRUE)
            {
                rzb_log (LOG_ERR, "%s: failed to lookup string", __func__, config_error_text (config));
                return false;
            }
            if (uuid_parse (type, block->dest) == -1)
            {
                rzb_log (LOG_ERR, "%s: Failed to parse UUID: %s", __func__, type);
                return false;
            }

        }
        else if (block->type == RZB_CONF_KEY_TYPE_BOOL)
        {
            status = config_lookup_string (config, block->key, &type);
            if (status != CONFIG_TRUE)
            {
                rzb_log (LOG_ERR, "%s: failed to lookup string", __func__, config_error_text (config));
                return false;
            }
            if (strncasecmp(type,"true",4) == 0)
                *(bool *)block->dest = true;
            else if (strncasecmp(type,"false",5) == 0)
                *(bool *)block->dest = false;
            else
            {
                rzb_log (LOG_ERR, "%s: Failed to parse bool: %s", __func__, type);
                return false;
            }

        }
        else if (block->type == RZB_CONF_KEY_TYPE_ARRAY)
        {
            if (config_setting_is_array(tt) == CONFIG_FALSE)
            {
                rzb_log (LOG_ERR, "%s: Failed to parse array: %s", __func__, block->key);
                return false;
            }
            if (!parseArray(tt, block))
                return false;
        }
        else if (block->type == RZB_CONF_KEY_TYPE_LIST)
        {
            if (config_setting_is_list(tt) == CONFIG_FALSE)
            {
                rzb_log (LOG_ERR, "%s: Failed to parse list: %s", __func__, block->key);
                return false;
            }
            if (!parseList(tt, block))
                return false;
        }
        else
        {
            rzb_log (LOG_ERR, "%s: Unknown config attribute type.", __func__);
            return false;
        }
        if (status != CONFIG_TRUE)
        {
            rzb_log (LOG_ERR, "%s: parsing failure: %s", __func__, config_error_text (config));
            return false;
        }
        block++;
    }
    return true;
}

bool
parseRoutingType (const char *string, conf_int_t * val)
{
    if (!strncasecmp (string, "opaque", 6))
    {
        *val = 0;
        return true;
    }
    else if (!strncasecmp (string, "transparent", 11))
    {
        *val = 1;
        return true;
    }
    return false;

}

#ifndef _MSC_VER
bool
testFile (const char *configfile)
{
    struct stat sb;
    int fd = open (configfile, O_RDONLY);

    if (fd == -1)
    {
        rzb_log (LOG_ERR, "%s: Failed to open (%s) in ", __func__, configfile);
        return false;
    }

    if (fstat (fd, &sb) == -1)
    {
        return false;
    }
    close (fd);
    return true;

}
#endif

SO_PUBLIC void
rzbConfCleanUp (void)
{
#ifndef _MSC_VER
    config_destroy (&config);
#endif
    while (configList != NULL)
    {
#ifndef _MSC_VER
        config_destroy (&configList->config);
#endif
        configList = configList->next;
    }
}
