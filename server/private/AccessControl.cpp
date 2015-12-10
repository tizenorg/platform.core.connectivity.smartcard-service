/*
 * Copyright (c) 2012, 2013 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* standard library header */
#include <glib.h>
#include <gio/gio.h>
#include <map>

/* local header */
#include "Debug.h"
#include "APDUHelper.h"
#include "ServerResource.h"
#include "SignatureHelper.h"
#include "ServerGDBus.h"
#include "PKCS15CDFACL.h"
#include "access-control-gdbus.h"

using namespace std;
using namespace smartcard_service_api;

static SmartcardServiceAccessControl *access_control;
static map<string, ByteArray> mapGranted;
static PKCS15CDFACL cdfAcl;

static void _load_granted_package_info()
{
	mapGranted.clear();

	/* TODO : load information form file */
	mapGranted.insert(make_pair("nfc-manager", ByteArray::EMPTY));
}

static gboolean _compare_hash(char *package, ByteArray &hash)
{
	gboolean result = false;
	vector<ByteArray> hashes;

	/* get certificate hashes by pid */
	if (SignatureHelper::getCertificationHashes(package, hashes) == true) {
		vector<ByteArray>::iterator item;

		for (item = hashes.begin(); item != hashes.end(); item++) {
			if (*item == hash) {
				result = true;
				break;
			}
		}
	} else {
		_ERR("getCertificationHashes failed, [%s]", package);
	}

	return result;
}

static gboolean _check_permission(pid_t pid)
{
	gboolean result = false;
	char package[1024];

	if (SignatureHelper::getPackageName(pid,
		package, sizeof(package)) == 0) {
		map<string, ByteArray>::iterator item;

		item = mapGranted.find(package);
		if (item != mapGranted.end()) {
			/* TODO : check privilege */
			if (false) {
				result = _compare_hash(package, item->second);
			} else {
				result = true;
			}
		}
	} else {
		_ERR("aul_app_get_pkgname_bypid failed");
	}

	return result;
}

static bool _get_se_name(unsigned int se_type, char *buf, size_t len)
{
	bool result = false;

	switch ((se_type >> 4) & 0x0F) {
	case 1 :
		snprintf(buf, len, "SIM%d", (se_type & 0x0F) + 1);
		result = true;
		break;

	case 2 :
		snprintf(buf, len, "eSE");
		result = true;
		break;

	default :
		break;
	}

	return result;
}

static bool _is_authorized_request(GDBusMethodInvocation *invocation,
	const char *rights)
{
	bool result = true;

	return result;
}

static gboolean __process_is_authorized_nfc_access(
	SmartcardServiceAccessControl *object,
	GDBusMethodInvocation *invocation,
	guint se_type,
	const gchar *package,
	GVariant *aid,
	void *user_data)
{
	bool result = false;
	const char *error;
	const char *name;
	char se[10];
	pid_t pid;

	ByteArray temp;
	Terminal *terminal;
	vector<ByteArray> hashes;

	_INFO("[MSG_IS_AUTHORIZED_NFC_ACCESS]");

	ServerResource &resource = ServerResource::getInstance();

	name = g_dbus_method_invocation_get_sender(invocation);

	pid = ServerGDBus::getInstance().getPID(name);

	_INFO("service requested, pid [%d]", pid);

	if (pid < 0) {
		error = "invalid pid";

		_ERR("%s, [%d]", error, pid);

		goto ERR;
	}

	/* check process permission */
	if (_check_permission(pid) == false) {
		error = "permission denied";

		_ERR("%s, [%d]", error, pid);

		goto ERR;
	}

	/* load secure elements */
	resource.loadSecureElements();

	if (_get_se_name(se_type, se, sizeof(se)) == false) {
		error = "unknown SE type";

		_ERR("%s, [%d]", error, se_type);

		goto ERR;
	}

	_INFO("SE : [%s]", se);

	terminal = resource.getTerminal(se);
	if (terminal == NULL) {
		error = "failed getting terminal";

		_ERR("%s, [%d]", error, se_type);

		goto ERR;
	}

	if (terminal->isSecureElementPresence() == false) {
		error = "terminal is not available now";

		_ERR("%s, [%d]", error, se_type);

		goto ERR;
	}

	/* get certificate hashes */
	if (SignatureHelper::getCertificationHashes(package, hashes) == false) {
		error = "failed getting certificates";

		_ERR("%s, [%s]", error, package);

		goto ERR;
	}

	/* convert AID */
	GDBusHelper::convertVariantToByteArray(aid, temp);

	result = resource.isAuthorizedNFCAccess(terminal, temp, hashes);

	/* response to client */
	smartcard_service_access_control_complete_is_authorized_nfc_access(
		object, invocation, result);

	return true;

ERR :
	g_dbus_method_invocation_return_dbus_error(
		invocation,
		"org.tizen.SmartcardService.AccessControl.Error",
		error);

	return false;
}

static void _process_is_authorized_nfc_access(vector<void *> &params)
{
	SmartcardServiceAccessControl *object;
	GDBusMethodInvocation *invocation;
	guint se_type;
	gchar *package;
	GVariant *aid;
	void *user_data;

	if (params.size() != 6) {
		_ERR("invalid parameter");

		return;
	}

	object = (SmartcardServiceAccessControl *)params[0];
	invocation = (GDBusMethodInvocation *)params[1];
	se_type = (gulong)params[2];
	package = (gchar *)params[3];
	aid = (GVariant *)params[4];
	user_data = params[5];

	__process_is_authorized_nfc_access(object, invocation, se_type,
		package, aid, user_data);

	g_variant_unref(aid);

	g_free(package);

	g_object_unref(invocation);
	g_object_unref(object);

	/* FIXME : disable killing process code */
//	ServerResource::getInstance().finish();
}

static gboolean _handle_is_authorized_nfc_access(
	SmartcardServiceAccessControl *object,
	GDBusMethodInvocation *invocation,
	guint se_type,
	const gchar *package,
	GVariant *aid,
	void *user_data)
{
	vector<void *> params;

	/* apply user space smack */
	if (_is_authorized_request(invocation, "r") == true) {
		g_object_ref(object);
		params.push_back((void *)object);

		g_object_ref(invocation);
		params.push_back((void *)invocation);

		params.push_back((void *)se_type);
		params.push_back((void *)g_strdup(package));

		g_variant_ref(aid);
		params.push_back((void *)aid);

		params.push_back((void *)user_data);

		GDBusDispatcher::getInstance().push(
			_process_is_authorized_nfc_access,
			params);
	} else {
		_ERR("access denied");

		g_dbus_method_invocation_return_dbus_error(
			invocation,
			"org.tizen.SmartcardService.AccessControl.Error",
			"access denied");

	/* FIXME : disable killing process code */
//		ServerResource::getInstance().finish();
	}

	return true;
}

static bool __load_cdf_acl(Terminal *terminal)
{
	bool result = false;
	ServerResource &resource = ServerResource::getInstance();
	ServerChannel *channel;

	if (terminal == NULL) {
		return result;
	}

//	if (terminal->open() == SCARD_ERROR_OK) {
		channel = resource.createInternalChannel(terminal, 1);
		if (channel != NULL) {
			int ret;

			ret = cdfAcl.updateACL(channel);
			if (ret == 0) {
				result = true;
			} else {
				_ERR("acl is null");
				result = false;
			}

			delete channel;
		} else {
			_ERR("alloc failed");
		}
//
//		terminal->close();
//	} else {
//		_ERR("terminal open failed");
//	}

	return result;
}

static gboolean __process_is_authorized_extra_access(
	SmartcardServiceAccessControl *object,
	GDBusMethodInvocation *invocation,
	guint se_type,
	const gchar *package,
	void *user_data)
{
	bool result = false;
	const char *error;
	const char *name;
	char se[10];
	pid_t pid;

	ByteArray temp;
	Terminal *terminal;
	vector<ByteArray> hashes;

	_INFO("[MSG_IS_AUTHORIZED_EXTRA_ACCESS]");

	ServerResource &resource = ServerResource::getInstance();

	name = g_dbus_method_invocation_get_sender(invocation);

	pid = ServerGDBus::getInstance().getPID(name);

	_INFO("service requested, pid [%d]", pid);

	if (pid < 0) {
		error = "invalid pid";

		_ERR("%s, [%d]", error, pid);

		goto ERR;
	}

	/* check process permission */
	if (_check_permission(pid) == false) {
		error = "permission denied";

		_ERR("%s, [%d]", error, pid);

		goto ERR;
	}

	/* load secure elements */
	resource.loadSecureElements();

	if (_get_se_name(se_type, se, sizeof(se)) == false) {
		error = "unknown SE type";

		_ERR("%s, [%d]", error, se_type);

		goto ERR;
	}

	_INFO("SE : [%s]", se);

	terminal = resource.getTerminal(se);
	if (terminal == NULL) {
		error = "failed getting terminal";

		_ERR("%s, [%d]", error, se_type);

		goto ERR;
	}

	if (terminal->isSecureElementPresence() == false) {
		error = "terminal is not available now";

		_ERR("%s, [%d]", error, se_type);

		goto ERR;
	}

	/* get certificate hashes */
	if (SignatureHelper::getCertificationHashes(package, hashes) == false) {
		error = "failed getting certificates";

		_ERR("%s, [%s]", error, package);

		goto ERR;
	}

	if ((se_type & 0xF0) == 0x10/* SIM */) {
		/* load CDF */
		if (cdfAcl.hasConditions() == false) {
			_ERR("cdf rule doesn't be load");
			__load_cdf_acl(terminal);
		}

		/* check access */
		result = cdfAcl.isAuthorizedAccess(
			AccessControlList::ALL_SE_APPS, hashes);
	} else if ((se_type & 0xF0) == 0x20/* eSE */) {
		if (terminal->open() == true) {
			result = resource.isAuthorizedAccess(terminal,
				AccessControlList::ALL_SE_APPS, hashes);

			terminal->close();
		}
	}

	/* response to client */
	smartcard_service_access_control_complete_is_authorized_extra_access(
		object, invocation, result);

	return true;

ERR :
	g_dbus_method_invocation_return_dbus_error(
		invocation,
		"org.tizen.SmartcardService.AccessControl.Error",
		error);

	return false;
}

static void _process_is_authorized_extra_access(vector<void *> &params)
{
	SmartcardServiceAccessControl *object;
	GDBusMethodInvocation *invocation;
	guint se_type;
	gchar *package;
	void *user_data;

	if (params.size() != 5) {
		_ERR("invalid parameter");

		return;
	}

	object = (SmartcardServiceAccessControl *)params[0];
	invocation = (GDBusMethodInvocation *)params[1];
	se_type = (gulong)params[2];
	package = (gchar *)params[3];
	user_data = params[4];

	__process_is_authorized_extra_access(object, invocation, se_type,
		package, user_data);

	g_free(package);

	g_object_unref(invocation);
	g_object_unref(object);

	/* FIXME : disable killing process code */
//	ServerResource::getInstance().finish();
}

static gboolean _handle_is_authorized_extra_access(
	SmartcardServiceAccessControl *object,
	GDBusMethodInvocation *invocation,
	guint se_type,
	const gchar *package,
	void *user_data)
{
	vector<void *> params;

	/* apply user space smack */
	if (_is_authorized_request(invocation, "r") == true) {
		g_object_ref(object);
		params.push_back((void *)object);

		g_object_ref(invocation);
		params.push_back((void *)invocation);

		params.push_back((void *)se_type);
		params.push_back((void *)g_strdup(package));

		params.push_back((void *)user_data);

		GDBusDispatcher::getInstance().push(
			_process_is_authorized_extra_access,
			params);
	} else {
		_ERR("access denied");

		g_dbus_method_invocation_return_dbus_error(
			invocation,
			"org.tizen.SmartcardService.AccessControl.Error",
			"access denied");

		/* FIXME : disable killing process code */
//		ServerResource::getInstance().finish();
	}

	return true;
}

static bool _init_access_control(void *connection)
{
	GError *error = NULL;

	access_control = smartcard_service_access_control_skeleton_new();

	g_signal_connect(access_control,
		"handle-is-authorized-nfc-access",
		G_CALLBACK(_handle_is_authorized_nfc_access),
		NULL);

	g_signal_connect(access_control,
		"handle-is-authorized-extra-access",
		G_CALLBACK(_handle_is_authorized_extra_access),
		NULL);

	if (g_dbus_interface_skeleton_export(
		G_DBUS_INTERFACE_SKELETON(access_control),
		(GDBusConnection *)connection,
		"/org/tizen/SmartcardService/AccessControl",
		&error) == false)
	{
		_ERR("Can not skeleton_export %s", error->message);

		g_error_free(error);
		g_object_unref(access_control);
		access_control = NULL;

		return false;
	}

	return true;
}

static void _deinit_access_control()
{
	if (access_control != NULL) {
		g_object_unref(access_control);
		access_control = NULL;
	}
}

extern "C" bool smartcard_service_init_access_control(void *connection)
{
	_load_granted_package_info();

	return _init_access_control(connection);
}

extern "C" void smartcard_service_deinit_access_control()
{
	_deinit_access_control();
}
