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

#ifndef SMARTCARDDBUS_H_
#define SMARTCARDDBUS_H_

/* standard library header */
#include <glib-object.h>

/* SLP library header */

/* local header */

typedef struct _Smartcard_Service Smartcard_Service;
typedef struct _Smartcard_ServiceClass Smartcard_ServiceClass;

#define SMARTCARD_SERVICE_NAME "org.tizen.smartcard_service"
#define SMARTCARD_SERVICE_PATH "/org/tizen/smartcard_service"

GType smartcard_service_get_type(void);

struct _Smartcard_Service
{
	GObject parent;
	int status;
};

struct _Smartcard_ServiceClass
{
	GObjectClass parent;
};

#define SMARTCARD_SERVICE_TYPE				(smartcard_service_get_type ())
#define SMARTCARD_SERVICE(object)			(G_TYPE_CHECK_INSTANCE_CAST ((object), SMARTCARD_SERVICE_TYPE, Smartcard_Service))
#define SMARTCARD_SERVICE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), SMARTCARD_SERVICE_TYPE, Smartcard_Service_Class))
#define IS_SMARTCARD_SERVICE(object)			(G_TYPE_CHECK_INSTANCE_TYPE ((object), SMARTCARD_SERVICE_TYPE))
#define IS_SMARTCARD_SERVICE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), SMARTCARD_SERVICE_TYPE))
#define SMARTCARD_SERVICE_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), SMARTCARD_SERVICE_TYPE, Smartcard_Service_Class))

typedef enum
{
	SMARTCARD_SERVICE_ERROR_INVALID_PRAM
} Smartcard_Service_Error;

GQuark smartcard_service_error_quark(void);
#define SMARTCARD_SERVICE_ERROR smartcard_service_error_quark ()

/**
 *     launch the nfc-manager
 */
gboolean smartcard_service_launch(Smartcard_Service *smartcard_service, guint *result_val, GError **error);

#endif /* SMARTCARDDBUS_H_ */
