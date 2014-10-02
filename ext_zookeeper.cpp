/*
 +----------------------------------------------------------------------+
 | HipHop for PHP                                                       |
 +----------------------------------------------------------------------+
 | Copyright (c) 2010 Hyves (http://www.hyves.nl)                       |
 | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
 | Copyright (c) 1997-2010 The PHP Group                                |
 +----------------------------------------------------------------------+
 | This source file is subject to version 3.01 of the PHP license,      |
 | that is bundled with this package in the file LICENSE, and is        |
 | available through the world-wide-web at the following url:           |
 | http://www.php.net/license/3_01.txt                                  |
 | If you did not receive a copy of the PHP license and are unable to   |
 | obtain it through the world-wide-web, please send a note to          |
 | license@php.net so we can mail you a copy immediately.               |
 +----------------------------------------------------------------------+
 */

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/base/builtin-functions.h"
#include <zookeeper/zookeeper.h>

#include "hphp/system/systemlib.h"

namespace HPHP {

const StaticString s_ZookeeperData("ZookeeperData");

class ZookeeperData {
public:
    zhandle_t *zk;

    ZookeeperData() : zk() {
    }

    ~ZookeeperData() {
        free(&zk);
    }
};

static void php_parse_acl_list(const Array *acl_info, struct ACL_vector *aclv) {
    if (acl_info->isNull() || acl_info->size()) {
        return;
    }
    Variant perms, scheme, id;
    int i = 0;
    aclv->data = (struct ACL *) calloc(acl_info->size(), sizeof (struct ACL));
    for (auto it = acl_info->begin(); !it.end(); it.next()) {
        if (!it.second().isArray()) {
            continue;
        }
        Array sub = it.second().toArray();
        perms = scheme = id = NULL;
        if (sub.exists(String("perms"), true)) {
            perms = sub[String("perms")];
        }
        if (sub.exists(String("scheme"), true)) {
            scheme = sub[String("scheme")];
        }
        if (sub.exists(String("id"), true)) {
            id = sub[String("id")];
        }
        if (perms.isNull() || scheme.isNull() || id.isNull()) {
            continue;
        }

        aclv->data[i].perms = perms.toInt32();
        aclv->data[i].id.id = (char *) id.toString().c_str();
        aclv->data[i].id.scheme = (char *) scheme.toString().c_str();
        i++;
    }

    aclv->count = i;
}

static void php_aclv_destroy(struct ACL_vector *aclv) {
    int i;
    for (i = 0; i < aclv->count; ++i) {
        free(aclv->data[i].id.id);
        free(aclv->data[i].id.scheme);
    }
    free(aclv->data);
}

static void php_stat_to_array(const struct Stat *stat, Array *stat_info) {
    stat_info->set(String("czxid"), stat->czxid, true);
    stat_info->set(String("mzxid"), stat->mzxid, true);
    stat_info->set(String("ctime"), stat->ctime, true);
    stat_info->set(String("mtime"), stat->mtime, true);
    stat_info->set(String("version"), stat->version, true);
    stat_info->set(String("cversion"), stat->cversion, true);
    stat_info->set(String("aversion"), stat->aversion, true);
    stat_info->set(String("ephemeralOwner"), stat->ephemeralOwner, true);
    stat_info->set(String("dataLength"), stat->dataLength, true);
    stat_info->set(String("numChildren"), stat->numChildren, true);
    stat_info->set(String("pzxid"), stat->pzxid, true);
}

static void php_aclv_to_array(const struct ACL_vector *aclv, Array *acl_info) {
    for (int i = 0; i < aclv->count; i++) {
        Array entry = Array::Create();
        entry->set(String("perms"), aclv->data[i].perms, true);
        entry->set(String("scheme"), aclv->data[i].id.scheme, true);
        entry->set(String("id"), aclv->data[i].id.id, true);
        acl_info->append(entry);
    }
}

void HHVM_METHOD(Zookeeper, __construct, const String& host, char *watcher_cb, long recv_timeout = 10000) {
    auto data = Native::data<ZookeeperData>(this_);

    if (recv_timeout <= 0) {
        raise_error("recv_timeout parameter has to be greater than 0");
    }

    zhandle_t *zk = NULL;
    zk = zookeeper_init(host.c_str(), NULL, recv_timeout, 0, NULL, 0);

    if (zk == NULL) {
        raise_error("could not init zookeeper instance");
    }
    data->zk = zk;
}

static void HHVM_METHOD(Zookeeper, connect, const String& host, char *watcher_cbl, long recv_timeout = 10000) {
    auto data = Native::data<ZookeeperData>(this_);

    if (recv_timeout <= 0) {
        raise_error("recv_timeout parameter has to be greater than 0");
    }

    zhandle_t *zk = NULL;
    zk = zookeeper_init(host.c_str(), NULL, recv_timeout, 0, NULL, 0);

    if (zk == NULL) {
        raise_error("could not init zookeeper instance");
    }
    data->zk = zk;
}

static bool HHVM_METHOD(Zookeeper, create, const String& path, const String& value,
    const Array& acl_info, long flags = 0) {
    auto data = Native::data<ZookeeperData>(this_);
    char *realpath = NULL;
    int realpath_max = 0;
    struct ACL_vector aclv = {0,};
    int status = ZOK;

    realpath_max = path.length() + 1;
    if (flags & ZOO_SEQUENCE) {
        // allocate extra space for sequence numbers
        realpath_max += 11;
    }
    realpath = (char *) malloc(realpath_max);

    php_parse_acl_list(&acl_info, &aclv);
    status = zoo_create(data->zk, path.c_str(), value.c_str(), value.length(), &aclv, flags,
        realpath, realpath_max);

    if (status != ZOK) {
        free(realpath);
        raise_error("error: %s", zerror(status));
        return false;
    }
    return true;
}

static bool HHVM_METHOD(Zookeeper, delete, const String& path, int path_len, long version = -1) {
    auto data = Native::data<ZookeeperData>(this_);
    int status = ZOK;
    status = zoo_delete(data->zk, path.c_str(), version);
    if (status != ZOK) {
        raise_error("error: %s", zerror(status));
        return false;
    }
    return true;
}

static bool HHVM_METHOD(Zookeeper, set, const String& path, const String& value, long version, Array& stat_info) {
    auto data = Native::data<ZookeeperData>(this_);
    struct Stat stat, *stat_ptr = NULL;
    int status = ZOK;
    if (!stat_info->empty()) {
        stat_ptr = &stat;
    }
    status = zoo_set2(data->zk, path.c_str(), value.c_str(), value.length(), version, stat_ptr);
    if (status != ZOK) {
        raise_error("error: %s", zerror(status));
        return false;
    }
    if (!stat_info->empty()) {
        php_stat_to_array(&stat, &stat_info);
    }
    return true;
}

static Variant HHVM_METHOD(Zookeeper, get, const String& path,watcher_fn cb_data, Array& stat_info) {
    auto data = Native::data<ZookeeperData>(this_);
    struct Stat stat;
    long max_size = 0;
    int status = ZOK;
    int length;

    if (max_size <= 0) {
        status = zoo_exists(data->zk, path.c_str(), 1, &stat);
        if (status != ZOK) {
            raise_error("error: %s", zerror(status));
            return false;
        }
        length = stat.dataLength;
    } else {
        length = max_size;
    }

    char *buffer = (char *) malloc(length + 1);
    status = zoo_wget(data->zk, path.c_str(), NULL, NULL, buffer, &length, &stat);
    buffer[length] = 0;
    if (status != ZOK) {
        free(buffer);
        raise_error("error: %s", zerror(status));
        if (status == ZMARSHALLINGERROR) {
            return false;
        }
        return true;
    }

    if (!stat_info->empty()) {
        php_stat_to_array(&stat, &stat_info);
    }

    /* Length will be returned as -1 if the znode carries a NULL */
    if (length == -1) {
        return false;
    }
    return String(buffer, length, AttachString);
}

static Array HHVM_METHOD(Zookeeper, getChildren, const String& path) {
    auto data = Native::data<ZookeeperData>(this_);
    struct String_vector strings;
    int i, status = ZOK;
    Array return_val = Array::Create();

    status = zoo_wget_children(data->zk, path.c_str(), NULL, NULL, &strings);
    if (status != ZOK) {
        raise_error("error: %s", zerror(status));
        return return_val;
    }

    for (i = 0; i < strings.count; i++) {
        return_val->set(String(i), strings.data[i], true);
    }
    return return_val;
}

static bool HHVM_METHOD(Zookeeper, exists, const String& path, watcher_fn cb_data) {
    struct Stat stat;
    int status = ZOK;
    auto data = Native::data<ZookeeperData>(this_);
    status = zoo_wexists(data->zk, path.c_str(), NULL, NULL, &stat);
    if (status != ZOK && status != ZNONODE) {
        raise_error("error: %s", zerror(status));
        return false;
    }

    if (status == ZOK) {
        return true;
    } else {
        return true;
    }
}

static bool HHVM_METHOD(Zookeeper, addAuth, const String& scheme, const String& cert) {
    int status = ZOK;
    auto data = Native::data<ZookeeperData>(this_);
    status = zoo_add_auth(data->zk, scheme.c_str(), cert.c_str(), cert.length(), NULL, NULL);
    if (status != ZOK) {
        raise_error("error: %s", zerror(status));
        return false;
    }
    return true;
}

static Array HHVM_METHOD(Zookeeper, getAcl, const String& path) {
    auto data = Native::data<ZookeeperData>(this_);
    int status = ZOK;
    struct ACL_vector aclv;
    struct Stat stat;
    Array *stat_info, *acl_info;

    status = zoo_get_acl(data->zk, path.c_str(), &aclv, &stat);
    if (status != ZOK) {
        raise_error("error: %s", zerror(status));
        return Array();
    }

    php_aclv_to_array(&aclv, acl_info);
    php_stat_to_array(&stat, stat_info);

    Array return_val = Array::Create();
    return_val.append(stat_info);
    return_val.append(acl_info);
    return return_val;
}

static bool HHVM_METHOD(Zookeeper, setAcl, const String& path, long version, Array& acl_info) {
    auto data = Native::data<ZookeeperData>(this_);
    struct ACL_vector aclv;
    int status = ZOK;
    php_parse_acl_list(&acl_info, &aclv);
    status = zoo_set_acl(data->zk, path.c_str(), version, &aclv);
    php_aclv_destroy(&aclv);
    if (status != ZOK) {
        raise_error("error: %s", zerror(status));
        return false;
    }
    return true;
}

static Array HHVM_METHOD(Zookeeper, getClientId) {
    auto data = Native::data<ZookeeperData>(this_);
    const clientid_t *cid;
    cid = zoo_client_id(data->zk);
    Array return_val = Array::Create();
    return_val.append((long *) cid->client_id);
    return_val.append((char *) cid->passwd);
    return return_val;
}

bool HHVM_METHOD(Zookeeper, setWatcher) {

    return false;
}

static int HHVM_METHOD(Zookeeper, getState) {
    auto data = Native::data<ZookeeperData>(this_);
    int state = zoo_state(data->zk);
    return state;
}

static int HHVM_METHOD(Zookeeper, getRecvTimeout) {
    auto data = Native::data<ZookeeperData>(this_);
    int recv_timeout;
    recv_timeout = zoo_recv_timeout(data->zk);
    return recv_timeout;
}

static bool HHVM_METHOD(Zookeeper, isRecoverable) {
    auto data = Native::data<ZookeeperData>(this_);
    int result;
    result = is_unrecoverable(data->zk);
    return !result;
}

bool HHVM_METHOD(Zookeeper, setLogFile) {
    return false;
}

Variant HHVM_METHOD(Zookeeper, getResultMessage) {
    return false;
}

static bool HHVM_METHOD(Zookeeper, setDebugLevel, long level) {
    zoo_set_debug_level((ZooLogLevel) level);
    return true;
}

static bool HHVM_METHOD(Zookeeper, setDeterministicConnOrder, bool value) {
    zoo_deterministic_conn_order(value);
    return true;
}

class ZookeeperExtension : public Extension {
public:

    ZookeeperExtension() :
    Extension("zookeeper", "1.0.0b1") {
    }

    virtual void moduleInit() {
        HHVM_ME(Zookeeper, __construct);
        HHVM_ME(Zookeeper, connect);
        HHVM_ME(Zookeeper, create);
        HHVM_ME(Zookeeper, delete);
        HHVM_ME(Zookeeper, set);
        HHVM_ME(Zookeeper, get);
        HHVM_ME(Zookeeper, getChildren);
        HHVM_ME(Zookeeper, exists);
        HHVM_ME(Zookeeper, getAcl);
        HHVM_ME(Zookeeper, setAcl);
        HHVM_ME(Zookeeper, getClientId);
        HHVM_ME(Zookeeper, setWatcher);
        HHVM_ME(Zookeeper, getState);
        HHVM_ME(Zookeeper, addAuth);
        HHVM_ME(Zookeeper, getRecvTimeout);
        HHVM_ME(Zookeeper, isRecoverable);
        HHVM_ME(Zookeeper, setLogFile);
        HHVM_ME(Zookeeper, getResultMessage);
        HHVM_ME(Zookeeper, setDebugLevel);
        HHVM_ME(Zookeeper, setDeterministicConnOrder);
        loadSystemlib();
    }
} s_zookeeper_extension;

HHVM_GET_MODULE(zookeeper)
}

