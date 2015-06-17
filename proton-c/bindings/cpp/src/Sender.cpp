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
#include "proton/Link.hpp"
#include "proton/Sender.hpp"
#include "proton/Error.hpp"
#include "Msg.hpp"
#include "contexts.hpp"

#include "proton/connection.h"
#include "proton/session.h"
#include "proton/link.h"
#include "proton/types.h"
#include "proton/codec.h"
#include "proton/message.h"
#include "proton/delivery.h"
#include <stdlib.h>
#include <string.h>

namespace proton {
namespace reactor {


Sender::Sender(pn_link_t *lnk) : Link(lnk) {}

void Sender::verifyType(pn_link_t *lnk) {
    if (lnk && pn_link_is_receiver(lnk))
        throw Error(MSG("Creating sender with receiver context"));
}

Sender::Sender(const Link& c) : Link(c.getPnLink()) {}


namespace{
// revisit if thread safety required
std::uint64_t tagCounter = 0;
}

Delivery Sender::send(Message &message) {
    char tag[8];
    void *ptr = &tag;
    std::uint64_t id = ++tagCounter;
    *((std::uint64_t *) ptr) = id;
    pn_delivery_t *dlv = pn_delivery(getPnLink(), pn_dtag(tag, 8));
    std::string buf;
    message.encode(buf);
    pn_link_t *link = getPnLink();
    pn_link_send(link, buf.data(), buf.size());
    pn_link_advance(link);
    if (pn_link_snd_settle_mode(link) == PN_SND_SETTLED)
        pn_delivery_settle(dlv);
    return Delivery(dlv);
}

}} // namespace proton::reactor