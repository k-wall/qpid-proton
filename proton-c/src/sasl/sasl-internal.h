/*
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 */

#ifndef PROTON_SASL_INTERNAL_H
#define PROTON_SASL_INTERNAL_H 1

#include "core/buffer.h"
#include "core/engine-internal.h"

#include "proton/types.h"
#include "proton/sasl.h"


// SASL APIs used by transport code
void pn_sasl_free(pn_transport_t *transport);
void pni_sasl_set_user_password(pn_transport_t *transport, const char *user, const char *password);
void pni_sasl_set_remote_hostname(pn_transport_t *transport, const char* fqdn);
void pni_sasl_set_external_security(pn_transport_t *transport, int ssf, const char *authid);

// Internal SASL authenticator interface
void pni_sasl_impl_free(pn_transport_t *transport);
int  pni_sasl_impl_list_mechs(pn_transport_t *transport, char **mechlist);
bool pni_init_server(pn_transport_t *transport);
void pni_process_init(pn_transport_t *transport, const char *mechanism, const pn_bytes_t *recv);
void pni_process_response(pn_transport_t *transport, const pn_bytes_t *recv);

bool pni_init_client(pn_transport_t *transport);
bool pni_process_mechanisms(pn_transport_t *transport, const char *mechs);
void pni_process_challenge(pn_transport_t *transport, const pn_bytes_t *recv);

// Internal SASL security layer interface
bool pni_sasl_impl_can_encrypt(pn_transport_t *transport);
ssize_t pni_sasl_impl_max_encrypt_size(pn_transport_t *transport);
ssize_t pni_sasl_impl_encode(pn_transport_t *transport, pn_bytes_t in, pn_bytes_t *out);
ssize_t pni_sasl_impl_decode(pn_transport_t *transport, pn_bytes_t in, pn_bytes_t *out);

typedef bool (*pni_sasl_function)(pn_transport_t *transport);
typedef void (*pni_sasl_challenge_response)(pn_transport_t *transport, const pn_bytes_t *recv);
typedef void (*pni_sasl_init)(pn_transport_t *transport, const char *mechanism, const pn_bytes_t *recv);
typedef bool (*pni_sasl_set_mechanisms)(pn_transport_t *transport, const char *mechanism);
typedef int (*pni_sasl_get_mechanisms)(pn_transport_t *transport, char **mechlist);

typedef struct
{
    pni_sasl_function free;
    pni_sasl_get_mechanisms get_mechs;
    pni_sasl_function init_server;
    pni_sasl_init process_init;
    pni_sasl_challenge_response process_response;
    pni_sasl_function init_client;
    pni_sasl_set_mechanisms process_mechanisms;
    pni_sasl_challenge_response process_challenge;
    pni_sasl_function process_outcome;
    pni_sasl_function prepare;
} pni_sasl_implementation;

PN_EXTERN void pni_sasl_set_implementation(pni_sasl_implementation);

// Shared SASL API used by the actual SASL authenticators
enum pni_sasl_state {
  SASL_NONE,
  SASL_POSTED_INIT,
  SASL_POSTED_MECHANISMS,
  SASL_POSTED_RESPONSE,
  SASL_POSTED_CHALLENGE,
  SASL_RECVED_OUTCOME_SUCCEED,
  SASL_RECVED_OUTCOME_FAIL,
  SASL_POSTED_OUTCOME,
  SASL_ERROR
};

struct pni_sasl_t {
  void *impl_context;
  char *selected_mechanism;
  char *included_mechanisms;
  const char *username;
  char *password;
  char *config_name;
  char *config_dir;
  const char *remote_fqdn;
  char *external_auth;
  int external_ssf;
  size_t max_encrypt_size;
  pn_buffer_t* decoded_buffer;
  pn_buffer_t* encoded_buffer;
  pn_bytes_t bytes_out;
  pn_sasl_outcome_t outcome;
  enum pni_sasl_state desired_state;
  enum pni_sasl_state last_state;
  bool allow_insecure_mechs;
  bool client;
};

// Shared Utility used by sasl implementations
void pni_split_mechs(char *mechlist, const char* included_mechs, char *mechs[], int *count);
bool pni_included_mech(const char *included_mech_list, pn_bytes_t s);
PN_EXTERN void pni_sasl_set_desired_state(pn_transport_t *transport, enum pni_sasl_state desired_state);

#endif /* sasl-internal.h */
