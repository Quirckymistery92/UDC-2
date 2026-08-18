#pragma once

#include <lvgl.h>
#include "ui_image_sia.h"
/*
    * SPDX-FileCopyrightText: 2025
    * SPDX-License-Identifier: Apache-2.0
    *
    * ui_image_sia_internal.h — Internal API for Static Icon Assigned (SIA) helper
    *
    * Este header define la función interna images_ctl_internal_get_src() que
    * resuelve un ui_icon_id_t contra la tabla sia_icons[] y retorna el
    * lv_img_dsc_t* correspondiente, o NULL si el ID es inválido.
    *
    * Esta función es usada internamente por ui_image_set_src_sia() para obtener
    * el recurso de imagen correcto sin exponer la tabla ni los detalles de SIA
    * a código externo. De esta forma, ui_image_set_src_sia() puede delegar
    * directamente al comando LVGL_CMD_IMAGE_SET_SRC con el resultado de esta
    * función, sin añadir lógica extra ni comandos adicionales al queue.
    *
    * Nota: esta función NO es parte de la API pública y no debe ser usada
    * fuera de ui_image_sia.c.
*/
const lv_img_dsc_t *images_ctl_internal_get_src(ui_icon_id_t icon_id);
