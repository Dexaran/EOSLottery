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
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/singleton.hpp>
#include <eosio/transaction.hpp>
#include "affiliate/datastore.hpp"
#include "utils/utils.hpp"

#define NOTIFY            [[eosio::on_notify("*::transfer")]] void
#define EOS_SYMBOL        symbol("EOS", 4)
#define EOSIO_TOKEN       name("eosio.token")
#define BANK_ACCOUNT      name("fairbet.bank")
#define CUSTODIAN_ACCOUNT name("airgap.bank")
#define CODE_PERMISSION   name("code")

using namespace std;
using namespace eosio;

namespace x10bit {
  CONTRACT affiliate : public datastore::affiliate {
    public:
      using datastore = datastore::affiliate;
      using datastore::datastore;

      affiliate(name receiver, name code, datastream<const char *> ds)
        : datastore(receiver, code, ds) {}

      ACTION init(
        const asset&            locked,
        const permission_level& witness,
        const global_ttl&       ttl
      );
      ACTION config(const global_ttl& ttl);
      ACTION rotate(const permission_level& witness);
      ACTION evacuate(const string& reason);
      ACTION announce(
        const string&        license,
        const asset&         fee,
        const license_rate&  rate,
        const license_terms& terms
      );
      ACTION activate(const uuid& license);
      ACTION allocate(
        const name&            partner,
        const name&            affiliate,
        const uuid&            license,
        const optional<float>& rate,
        const asset&           discount,
              optional<actor>& broker,
        const bool&            approved
      );
      ACTION empower(const entity& object, const uuid& capability, const string& reason);
      ACTION revoke(const entity& object, const uuid& capability, const string& reason);
      ACTION archive(const uuid& license, const string& reason);
      ACTION suspend(const name& partner, const optional<uuid> deal, const string& reason);
      ACTION resume(const name& partner, const optional<uuid> deal, const string& reason);
      ACTION sync(const vector<account>& accounts);
      ACTION apply(
        const name&  account,
        const uuid&  license,
        const float& rate,
        const asset& discount,
              actor& broker
      );
      ACTION refuse(const uuid& application);
      NOTIFY transfer(
        const name&   from,
        const name&   to,
        const asset&  quantity,
        const string& memo
      );
      ACTION claim(const name& partner);
      ACTION prolong(const name& partner);
      ACTION withdraw(const name& partner, const name& payee, const asset& quantity);
      ACTION update(
        const name&            partner,
        const uuid&            license,
        const optional<float>& rate,
        const optional<asset>& balance
      );
      ACTION upgrade(
        const name&             partner,
        const pair<uuid, uuid>& license,
        const optional<float>&  rate
      );
      ACTION insure(const name& partner, const name& standby);
      ACTION failover(const name& partner);
      ACTION delegate(const name& partner, const name& successor);
      ACTION quit(const name& partner);

      #ifndef DEBUG
        ACTION migrate(const uuid& version);
      #else
        ACTION reset(const uint8_t& type, const uint64_t& limit);
      #endif

      static asset get_locked_balance(const name& contract_account) {
        singleton_state _state(contract_account, contract_account.value);
        return _state.get().locked;
      }

      static license get_license(
        const name& contract,
        const uuid& id,
        const string error_msg = "invalid license"
      ) {
        multi_index_licenses _licenses(contract, contract.value);
        return _licenses.get(id, error_msg.c_str());
      }

      static partner get_partner(
        const name& contract,
        const name& account,
        const string error_msg = "partner does not exist"
      ) {
        multi_index_partners _partners(contract, contract.value);
        const auto& partners_account_idx = _partners.get_index<"account"_n>();
        return partners_account_idx.get(account.value, error_msg.c_str());
      }

      using init_action     = action_wrapper<"init"_n, &affiliate::init>;
      using config_action   = action_wrapper<"config"_n, &affiliate::config>;
      using rotate_action   = action_wrapper<"rotate"_n, &affiliate::rotate>;
      using evacuate_action = action_wrapper<"evacuate"_n, &affiliate::evacuate>;
      using announce_action = action_wrapper<"announce"_n, &affiliate::announce>;
      using activate_action = action_wrapper<"activate"_n, &affiliate::activate>;
      using allocate_action = action_wrapper<"allocate"_n, &affiliate::allocate>;
      using empower_action  = action_wrapper<"empower"_n, &affiliate::empower>;
      using revoke_action   = action_wrapper<"revoke"_n, &affiliate::revoke>;
      using archive_action  = action_wrapper<"archive"_n, &affiliate::archive>;
      using suspend_action  = action_wrapper<"suspend"_n, &affiliate::suspend>;
      using resume_action   = action_wrapper<"resume"_n, &affiliate::resume>;
      using sync_action     = action_wrapper<"sync"_n, &affiliate::sync>;
      using apply_action    = action_wrapper<"apply"_n, &affiliate::apply>;
      using refuse_action   = action_wrapper<"refuse"_n, &affiliate::refuse>;
      using claim_action    = action_wrapper<"claim"_n, &affiliate::claim>;
      using prolong_action  = action_wrapper<"prolong"_n, &affiliate::prolong>;
      using withdraw_action = action_wrapper<"withdraw"_n, &affiliate::withdraw>;
      using update_action   = action_wrapper<"update"_n, &affiliate::update>;
      using upgrade_action  = action_wrapper<"upgrade"_n, &affiliate::upgrade>;
      using insure_action   = action_wrapper<"insure"_n, &affiliate::insure>;
      using failover_action = action_wrapper<"failover"_n, &affiliate::failover>;
      using delegate_action = action_wrapper<"delegate"_n, &affiliate::delegate>;
      using quit_action     = action_wrapper<"quit"_n, &affiliate::quit>;
    private:
      const vector<name> whitelisted_senders = {
        "jetpack.x"_n,
        "airgap.bank"_n,
        "fairbet.bank"_n,
        "fairbet.game"_n,
        "fairbetagent"_n,
        "fairbetvault"_n,
        "fairbetproof"_n,
        "fairbetbonus"_n,
        "greymassnoop"_n,
        "tippedtipped"_n,
        "eosio.token"_n,
        "eosio.stake"_n,
        "eosio.names"_n,
        "eosio.ram"_n,
        "eosio.rex"_n,
        "eosio"_n,
      };

      constexpr name get_self_permission() {
        return CODE_PERMISSION;
      }

      inline void check_partner(const name& account) {
        const auto account_name             = utils::to_string(account);
        const auto applications_account_idx = applications.get_index<"account"_n>();
        const auto partners_account_idx     = partners.get_index<"account"_n>();
        const auto partners_standby_idx     = partners.get_index<"standby"_n>();

        check(
          applications_account_idx.find(account.value) == applications_account_idx.end(),
          account_name + " account is already in use"
        );
        check(
          partners_account_idx.find(account.value) == partners_account_idx.end(),
          account_name + " account is already in use"
        );
        check(
          partners_standby_idx.find(account.value) == partners_standby_idx.end(),
          account_name + " account is already in use"
        );
      }

      inline void check_affiliate(const name& affiliate) {
        const auto affiliate_name             = utils::to_string(affiliate);
        const auto applications_affiliate_idx = applications.get_index<"affiliate"_n>();
        const auto partners_affiliate_idx     = partners.get_index<"affiliate"_n>();

        check(
          applications_affiliate_idx.find(affiliate.value) == applications_affiliate_idx.end(),
          "affiliate " + affiliate_name + " is already declared"
        );
        check(
          partners_affiliate_idx.find(affiliate.value) == partners_affiliate_idx.end(),
          "affiliate " + affiliate_name + " is already exists"
        );
      }
  };
}
