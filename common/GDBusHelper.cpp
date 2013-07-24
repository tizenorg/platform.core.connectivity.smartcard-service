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

#ifdef USE_GDBUS
#include "GDBusHelper.h"

namespace smartcard_service_api
{
	void GDBusHelper::convertVariantToByteArray(GVariant *var,
		ByteArray &array)
	{
		GVariantIter *iter;
		guint8 element;
		guint8 *buf = NULL;
		guint size = 0;
		guint i;

		g_variant_get(var, "a(y)", &iter);

		size = g_variant_iter_n_children(iter);
		buf  = g_new0(guint8, size);

		for (i = 0; g_variant_iter_loop(iter, "(y)", &element); i++)
		{
			buf[i] = element;
		}

		g_variant_iter_free(iter);

		array.assign((uint8_t *)buf, (uint32_t)i);

		g_free(buf);
	}

	GVariant *GDBusHelper::convertByteArrayToVariant(const ByteArray &array)
	{
		GVariantBuilder builder;
		uint32_t i;

		g_variant_builder_init(&builder, G_VARIANT_TYPE("a(y)"));

		for (i = 0; i < array.size(); i++)
			g_variant_builder_add(&builder, "(y)", array[i]);

		return g_variant_builder_end(&builder);
	}
} /* namespace smartcard_service_api */
#endif
