/*
 * Copyright (C) 2014, Lytsing Huang <hlqing@gmail.com>.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HTTPD_H
#define HTTPD_H

#include <evhtp.h>

#ifdef __cplusplus
extern "C" {
#endif

void dump_request_cb(evhtp_request_t* req, void* arg);

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: HTTP_H */

