/*
 *   connector.c
 *   Copyright (C) 2019 David García Goñi <dagargo@gmail.com>
 *
 *   This file is part of Elektroid.
 *
 *   Elektroid is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Elektroid is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Elektroid. If not, see <http://www.gnu.org/licenses/>.
 */

#include "backend.h"
#include "connector.h"
#include "connectors/elektron.h"
#include "connectors/microbrute.h"
#include "connectors/cz.h"
#include "connectors/sds.h"

gint
default_handshake (struct backend *backend)
{
  backend->device_desc.filesystems = 0;
  backend->fs_ops = NULL;
  backend->destroy_data = backend_destroy_data;
  if (!strlen (backend->device_name))
    {
      snprintf (backend->device_name, LABEL_MAX, "MIDI");
    }
  return 0;
}

struct connector
{
  gint (*handshake) (struct backend * backend);
  const gchar *name;
};

static const struct connector CONNECTOR_ELEKTRON = {
  .handshake = elektron_handshake,
  .name = "elektron"
};

static const struct connector CONNECTOR_MICROBRUTE = {
  .handshake = microbrute_handshake,
  .name = "microbrute"
};

static const struct connector CONNECTOR_CZ = {
  .handshake = cz_handshake,
  .name = "cz"
};

static const struct connector CONNECTOR_SDS = {
  .handshake = sds_handshake,
  .name = "sds"
};

static const struct connector CONNECTOR_DEFAULT = {
  .handshake = default_handshake,
  .name = "default"
};

static const struct connector *CONNECTORS[] = {
  &CONNECTOR_ELEKTRON, &CONNECTOR_MICROBRUTE, &CONNECTOR_CZ, &CONNECTOR_SDS,
  &CONNECTOR_DEFAULT, NULL
};

gint
connector_init (struct backend *backend, const gchar * midi_id,
		const gchar * conn_name)
{
  const struct connector **connector;
  int err = backend_init (backend, midi_id);
  if (err)
    {
      return err;
    }

  connector = CONNECTORS;
  while (*connector)
    {
      if (conn_name)
	{
	  if (!strcmp (conn_name, (*connector)->name))
	    {
	      debug_print (1, "Testing %s connector...\n",
			   (*connector)->name);
	      (*connector)->handshake (backend);
	      debug_print (1, "Using %s connector...\n", (*connector)->name);
	      return 0;
	    }
	}
      else
	{
	  debug_print (1, "Testing %s connector...\n", (*connector)->name);
	  if (!(*connector)->handshake (backend))
	    {
	      debug_print (1, "Using %s connector...\n", (*connector)->name);
	      return 0;
	    }
	}
      connector++;
    }

  error_print ("Device not recognized\n");

  backend_destroy (backend);

  return -1;
}
