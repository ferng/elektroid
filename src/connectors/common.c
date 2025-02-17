/*
 *   common.h
 *   Copyright (C) 2022 David García Goñi <dagargo@gmail.com>
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

#include <math.h>
#include "common.h"

gchar *
common_slot_get_upload_path (struct backend *backend,
			     const struct fs_operations *ops,
			     const gchar * dst_path, const gchar * src_path)
{
  //In SLOT mode, dst_path includes the index, ':' and the item name.
  return strdup (dst_path);
}

int
common_slot_get_id_name_from_path (const char *path, guint * id,
				   gchar ** name)
{
  gint err = 0;
  gchar *path_copy, *index_name, *remainder;

  path_copy = strdup (path);
  index_name = basename (path_copy);
  *id = (gint) strtol (index_name, &remainder, 10);
  if (strncmp (remainder, BE_SAMPLE_ID_NAME_SEPARATOR,
	       strlen (BE_SAMPLE_ID_NAME_SEPARATOR)) == 0)
    {
      remainder++;		//Skip ':'
    }
  else
    {
      if (name)
	{
	  error_print ("Path name not provided properly\n");
	  err = -EINVAL;
	  goto end;
	}
    }

  if (name)
    {
      if (*remainder)
	{
	  *name = strdup (remainder);
	}
      else
	{
	  *name = NULL;
	}
    }

end:
  g_free (path_copy);
  return err;
}

gchar *
common_get_id_as_slot (struct item *item, struct backend *backend)
{
  gchar *slot = malloc (LABEL_MAX);
  snprintf (slot, LABEL_MAX, "%d", item->id);
  return slot;
}

void
common_print_item (struct item_iterator *iter, struct backend *backend,
		   const struct fs_operations *fs_ops)
{
  gchar *slot = NULL;
  if (fs_ops->get_slot)
    {
      slot = fs_ops->get_slot (&iter->item, backend);
    }
  else
    {
      slot = common_get_id_as_slot (&iter->item, backend);
    }
  printf ("%c % 4" PRId64 "B %10s%s%s\n", iter->item.type, iter->item.size,
	  slot ? slot : "", slot ? " " : "", iter->item.name);
  g_free (slot);
}

void
common_midi_program_change (struct backend *backend, const gchar * dir,
			    struct item *item)
{
  if (item->id > BE_MAX_MIDI_PROGRAMS)
    {
      return;
    }
  backend_program_change (backend, 0, item->id);
}

guint
common_simple_next_dentry (struct item_iterator *iter)
{
  struct common_simple_read_dir_data *data = iter->data;
  guint digits = floor (log10 (data->max));

  if (data->next >= data->max)
    {
      return -ENOENT;
    }

  snprintf (iter->item.name, LABEL_MAX, "%.*d", digits, data->next);
  iter->item.id = data->next;
  iter->item.type = ELEKTROID_FILE;
  iter->item.size = -1;
  data->next++;

  return 0;
}

guint
common_data_upload (struct backend *backend, GByteArray * msg,
		    struct job_control *control)
{
  gint err = 0;
  gboolean active;
  struct sysex_transfer transfer;

  g_mutex_lock (&backend->mutex);

  control->parts = 1;
  control->part = 0;
  set_job_control_progress (control, 0.0);

  transfer.raw = msg;
  err = backend_tx_sysex (backend, &transfer);
  if (err < 0)
    {
      goto cleanup;
    }

  g_mutex_lock (&control->mutex);
  active = control->active;
  g_mutex_unlock (&control->mutex);
  if (active)
    {
      set_job_control_progress (control, 1.0);
    }
  else
    {
      err = -ECANCELED;
    }

cleanup:
  g_mutex_unlock (&backend->mutex);
  return err;
}


guint
common_data_download_part (struct backend *backend, GByteArray * tx_msg,
			   GByteArray ** rx_msg, struct job_control *control)
{
  gint err = 0;
  gboolean active;

  set_job_control_progress (control, 0.0);

  *rx_msg = backend_tx_and_rx_sysex (backend, tx_msg, -1);
  if (!*rx_msg)
    {
      err = -EIO;
      goto cleanup;
    }

  g_mutex_lock (&control->mutex);
  active = control->active;
  g_mutex_unlock (&control->mutex);
  if (active)
    {
      set_job_control_progress (control, 1.0);
    }
  else
    {
      free_msg (*rx_msg);
      *rx_msg = NULL;
      err = -ECANCELED;
    }

cleanup:
  return err;
}

guint
common_data_download (struct backend *backend, GByteArray * tx_msg,
		      GByteArray ** rx_msg, struct job_control *control)
{
  control->parts = 1;
  control->part = 0;
  return common_data_download_part (backend, tx_msg, rx_msg, control);
}

gchar *
common_get_download_path_with_params (struct backend *backend,
				      const struct fs_operations *ops,
				      const gchar * dst_dir,
				      guint id, guint digits,
				      const gchar * name)
{
  gchar *path;
  path = malloc (PATH_MAX);
  snprintf (path, PATH_MAX, "%s/%s %s %.*d", dst_dir,
	    backend->name, ops->name, digits, id);
  if (name)
    {
      strcat (path, " - ");
      strcat (path, name);
    }
  strcat (path, ".");
  strcat (path, ops->type_ext);
  return path;
}

gchar *
common_get_download_path (struct backend *backend,
			  const struct fs_operations *ops,
			  const gchar * dst_dir, const gchar * src_path,
			  GByteArray * sysex)
{
  guint id;
  common_slot_get_id_name_from_path (src_path, &id, NULL);
  return common_get_download_path_with_params (backend, ops, dst_dir,
					       id, 3, NULL);
}
