/**
 * SPDX-License-Identifier: HashCode-EULA-1.1-or-later
 *
 * Description / Summary:   Financial Smart Contract (the "Software")
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
#include <eosio/transaction.hpp>

#define NOTIFY            [[eosio::on_notify("*::transfer")]] void
#define EOS_SYMBOL        symbol("EOS", 4)
#define EOSIO_TOKEN       name("eosio.token")
#define GAME_ACCOUNT      name("fairbet.game")
#define AGENT_ACCOUNT     name("fairbetagent")
#define VAULT_ACCOUNT     name("fairbetvault")
#define TRUSTEE_ACCOUNT   name("johnnesmelov")
#define CODE_PERMISSION   name("code")
#define PARTNER_FEE_MEMO  "Partner license fee"
#define GAME_BET_MEMO_SZ  64

using namespace std;
using namespace eosio;

namespace x10bit {
  CONTRACT bank : public contract {
    public:
      using contract::contract;

      bank(const name& receiver, const name& code, const datastream<const char *>& ds) :
        contract(receiver, code, ds)
      {}

      struct report {
        asset  expenses;
        float  reinvestment_rate;
      };

      struct extended_transfer {
        name   from;
        name   to;
        asset  quantity;
        string memo;
        name   target;
      };

      ACTION withdraw(const asset& profit, const report& current, const report& next);

      NOTIFY transfer(
        const name&   from,
        const name&   to,
        const asset&  quantity,
        const string& memo
      );

      /*TABLE state {
        uuid                 version;
        asset                locked;
        asset                expenses;
        float                reinvestment;
        time_point           sync;

        EOSLIB_SERIALIZE(state, (version)(locked)(expenses)(reinvestment)(sync));
      };*/

    private:
      const vector<name> whitelisted_senders = {
        "jetpack.x"_n,
        "fairbet.game"_n,
        "fairbet.bank"_n,
        "fairbetagent"_n,
        "fairbetvault"_n,
        "fairbetbonus"_n,
        "fairbetproof"_n,
        "greymassnoop"_n,
        "tippedtipped"_n,
        "eosio.token"_n,
        "eosio.stake"_n,
        "eosio.names"_n,
        "eosio.ram"_n,
        "eosio.rex"_n,
        "eosio"_n,
      };
  };
}
