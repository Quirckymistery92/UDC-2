#pragma once
#include "stdbool.h"
#include "stdio.h"
#include "esp_err.h"

bool NVSmModule_user_m_R_guest_state(void);

esp_err_t NVSmModule_user_m_W_guest_state(bool enable);
