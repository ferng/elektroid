/*
 *   common.c
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

#include "backend.h"

struct common_simple_read_dir_data
{
  guint32 next;
  guint32 max;
  gboolean index;
};

gchar *common_slot_get_upload_path (struct backend *backend,
				    const struct fs_operations *ops,
				    const gchar * dst_path,
				    const gchar * src_path);

gint common_slot_get_id_name_from_path (const char *path, guint * id,
					gchar ** name);

gchar *common_get_id_as_slot (struct item *item, struct backend *backend);

void common_print_item (struct item_iterator *iter, struct backend *backend,
			const struct fs_operations *fs_ops);

void common_midi_program_change (struct backend *backend, const gchar * dir,
				 struct item *item);

guint common_simple_next_dentry (struct item_iterator *iter);

guint common_data_upload (struct backend *backend, GByteArray * msg,
			  struct job_control *control);

guint common_data_download (struct backend *backend, GByteArray * tx_msg,
			    GByteArray ** rx_msg,
			    struct job_control *control);

guint common_data_download_part (struct backend *backend, GByteArray * tx_msg,
				 GByteArray ** rx_msg,
				 struct job_control *control);

gchar *common_get_download_path_with_params (struct backend *backend,
					     const struct fs_operations *ops,
					     const gchar * dst_dir,
					     guint id, guint digits,
					     const gchar * name);

gchar *common_get_download_path (struct backend *backend,
				 const struct fs_operations *ops,
				 const gchar * dst_dir,
				 const gchar * src_path, GByteArray * sysex);
