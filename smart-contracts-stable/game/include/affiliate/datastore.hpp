/**
 * SPDX-License-Identifier: HashCode-EULA-1.1-or-later
 *
 * Description / Summary:   Affiliate Program Smart Contract (the "Software")
 *                          Part of the 16Bit Platform ecosystem
 *
 * Authors & Contributors:  Designed and assembled by GeekHack
 *                          In collaboration with 16Bit team
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Copyright (c) 2020 GeekHack ÐΞV
 * Copyright (c) 2021 HashCode Ltd.
 */

#pragma once

#include <eosio/eosio.hpp>
#include "utils/eosio.hpp"
#include "utils/variant.hpp"

using namespace std;
using namespace eosio;

namespace x10bit {
  namespace datastore {
    CONTRACT affiliate : public contract {
      public:
        using contract::contract;

        affiliate(name receiver, name code, datastream<const char *> ds)
          : contract(receiver, code, ds),
            state(receiver, receiver.value),
            licenses(receiver, receiver.value),
            applications(receiver, receiver.value),
            partners(receiver, receiver.value)
          {}

        struct global_ttl {
          microseconds                             safeguard;
          microseconds                             application;
        };

        struct license_rate {
          enum class payer : uint8_t { partner, platform };
          using      fixed = float;
          struct     range {
            float    from;
            float    to;
          };
          struct     p2p   : range {};

          uint8_t                                  payer;
          std::variant<fixed, range, p2p>          limit;

          EOSLIB_SERIALIZE(license_rate, (payer)(limit));
        };

        struct license_terms {
          uint8_t                                  platform_commission;
          bool                                     privileged_access;
          bool                                     global_program;
          bool                                     instant_payout;
          vector<uuid>                             allowed_scope;
          microseconds                             referral_ttl;
          bool                                     additionable;
          bool                                     upgradable;
          bool                                     revocable;
          bool                                     franchise;
        };

        using actor_cursor = std::variant<uuid, name>;

        struct actor {
          actor_cursor                             self;
          optional<actor_cursor>                   root;
          uuid                                     license;
          float                                    rate;
          bool                                     suspended;

          EOSLIB_SERIALIZE(actor, (self)(root)(license)(rate)(suspended));
        };

        struct entity {
          using license_uuid = uuid;
          using partner_name = name;

          std::variant<license_uuid, partner_name> id;

          EOSLIB_SERIALIZE(entity, (id));
        };

        struct account {
          name                                     owner;
          uuid                                     license;
          asset                                    saldo;
        
          EOSLIB_SERIALIZE(account, (owner)(license)(saldo));
        };

        TABLE state {
          uuid                                     version;
          asset                                    locked;
          permission_level                         witness;
          time_point                               sync;
          global_ttl                               ttl;

          EOSLIB_SERIALIZE(state, (version)(locked)(witness)(sync)(ttl));
        };

        TABLE license {
          uuid                                     id;
          string                                   name;
          asset                                    fee;
          license_rate                             rate;
          license_terms                            terms;
          bool                                     active;
          bool                                     archived;

          uuid primary_key() const { return id; }

          EOSLIB_SERIALIZE(license, (id)(name)(fee)(rate)(terms)(active)(archived));
        };

        TABLE application {
          uuid                                     id;
          name                                     owner;
          name                                     affiliate;
          uuid                                     license;
          optional<float>                          rate;
          asset                                    fee;
          asset                                    discount;
          asset                                    deposit;
          optional<actor>                          broker;
          time_point                               eol;

          uuid primary_key()   const { return id; }
          uuid secondary_key() const { return owner.value; }
          uuid affiliate_key() const { return affiliate.value; }

          EOSLIB_SERIALIZE(application, (id)(owner)(affiliate)(license)(rate)
                                        (fee)(discount)(deposit)(broker)(eol));
        };

        // eosio::chain::abi_serializer can't distinguish map<uint64_t, optional<float>>
        // ref: https://github.com/EOSIO/eos/issues/9763#issuecomment-743091982
        using opt_float = optional<float>;
        using map_license_rate = map<uuid, opt_float>;

        TABLE partner {
          uuid                                     id;
          name                                     primary;
          name                                     standby;
          name                                     affiliate;
          map_license_rate                         contracts;
          asset                                    balance;
          optional<actor>                          broker;
          bool                                     suspended;
          time_point                               eol;

          uuid primary_key()     const { return id; }
          uuid secondary_key()   const { return primary.value; }
          uuid auxiliary_key()   const { return standby.value; }
          uuid affiliate_key()   const { return affiliate.value; }
          uuid broker_root_key() const {
            return (broker.has_value() && broker->root.has_value())
              ? visit(
                  utils::make_visitor(
                    [&](const uuid &id) { return id; },
                    [&](const name &account) { return account.value; },
                    [&](auto &&) { check(false, "unimplemented cast of broker cursor to uuid"); }
                  ),
                  broker->root.value()
                )
              : 0;
          }

          EOSLIB_SERIALIZE(partner, (id)(primary)(standby)(affiliate)
                                    (contracts)(balance)(broker)(suspended)(eol));
        };
      protected:
        using singleton_state = singleton<"state"_n, state>;
        using multi_index_licenses = multi_index<"licenses"_n, license>;
        using multi_index_applications = multi_index<"applications"_n, application,
          indexed_by<"account"_n, const_mem_fun<application, uuid, &application::secondary_key>>,
          indexed_by<"affiliate"_n, const_mem_fun<application, uuid, &application::affiliate_key>>
        >;
        using multi_index_partners = multi_index<"partners"_n, partner,
          indexed_by<"account"_n, const_mem_fun<partner, uuid, &partner::secondary_key>>,
          indexed_by<"standby"_n, const_mem_fun<partner, uuid, &partner::auxiliary_key>>,
          indexed_by<"affiliate"_n, const_mem_fun<partner, uuid, &partner::affiliate_key>>,
          indexed_by<"root"_n, const_mem_fun<partner, uuid, &partner::broker_root_key>>
        >;

        singleton_state          state;
        multi_index_licenses     licenses;
        multi_index_applications applications;
        multi_index_partners     partners;
    };
  }
}
