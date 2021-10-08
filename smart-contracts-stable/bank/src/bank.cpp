// SPDX-License-Identifier: HashCode-EULA-1.1-or-later
//
// Description / Summary:   Financial Smart Contract (the "Software")
//                          Part of the 16Bit Platform ecosystem
//
// Authors & Contributors:  Designed and assembled by GeekHack
//                          In collaboration with 16Bit team
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// Copyright (c) 2020 GeekHack ÐΞV
// Copyright (c) 2021 HashCode Ltd.

#include <bank/bank.hpp>
#include <eosio.token/eosio.token.hpp>

namespace x10bit {
  /*ACTION bank::init(
    const asset& locked,
    const asset& expenses,
    const float& reinvestment,
  ) {
    require_auth(get_self());
    
    check(!state_.exists(), "already initialized");
    utils::check_asset(locked);
    utils::check_asset(expenses);
    check(
      reinvestment >= 0 && reinvestment < 1,
      "reinvestment rate must be equal to or greater than zero and less than one"
    );

    state_.set({0x00, locked, expenses, reinvestment, time_point(microseconds(0x00))}, get_self());
  }*/

  ACTION bank::withdraw(
    const asset&  profit,
    const report& current,
    const report& next
  ) {
    require_auth(get_self());

    check(profit.is_valid(), "invalid profit");
    check(profit.amount > 0, "profit amount must be positive");
    check(profit.symbol == EOS_SYMBOL, "invalid token symbol");

    check(current.expenses.is_valid(), "invalid current expenses");
    check(current.expenses.amount >= 0, "current expenses amount must be non-negative");
    check(current.expenses.symbol == EOS_SYMBOL, "invalid token symbol");

    check(
      current.reinvestment_rate >= 0 && current.reinvestment_rate <= 1,
      "current reinvestment rate must be between 0 and 1 (incl.)"
    );

    check(next.expenses.is_valid(), "invalid next expenses");
    check(next.expenses.amount >= 0, "next expenses amount must be non-negative");
    check(next.expenses.symbol == EOS_SYMBOL, "invalid token symbol");

    check(
      next.reinvestment_rate >= 0 && next.reinvestment_rate <= 1,
      "next reinvestment rate must be between 0 and 1 (incl.)"
    );

    const auto balance = token::get_balance(EOSIO_TOKEN, get_self(), EOS_SYMBOL.code());
    check(profit + current.expenses <= balance, "overdraft is not allowed");

    check(current.expenses.amount > 0 || current.reinvestment_rate < 1, "redundant action");

    if (current.reinvestment_rate < 1) {
      const auto dividends = profit - asset{
        static_cast<int64_t>(profit.amount * current.reinvestment_rate), EOS_SYMBOL
      };
      action(
        permission_level{get_self(), CODE_PERMISSION},
        EOSIO_TOKEN,
        "transfer"_n,
        make_tuple(
          get_self(),
          VAULT_ACCOUNT,
          dividends,
          string("Platform dividends (16bit.game)"),
          ""_n
        )
      ).send();
    }

    if (current.expenses.amount > 0) {
      action(
        permission_level{get_self(), CODE_PERMISSION},
        EOSIO_TOKEN,
        "transfer"_n,
        make_tuple(
          get_self(),
          TRUSTEE_ACCOUNT,
          current.expenses,
          string("Platform expenses (16bit.game)")
        )
      ).send();
    }
  };

  NOTIFY bank::transfer(
    const name&   from,
    const name&   to,
    const asset&  quantity,
    const string& memo
  ) {
    check(quantity.is_valid(), "invalid transfer");
    check(quantity.symbol == EOS_SYMBOL, "invalid token symbol");
    check(quantity.amount > 0, "transfer amount must be positive");

    //const auto current_action_size  = read_action_data((void *)(NULL), 0);
    const auto current_action_size = action_data_size();
    const auto transfer_action_size = action(
      permission_level{from, "active"_n},
      EOSIO_TOKEN,
      "transfer"_n,
      make_tuple(
        from,
        to,
        quantity,
        memo
      )
    ).data.size();

    if (to == get_self()) {
      check(
        get_sender() == EOSIO_TOKEN,
        "deposit of " + get_sender().to_string() + " is not allowed"
      );

      /*if (current_action_size > transfer_action_size) {
        const auto extended_transfer_data = unpack_action_data<extended_transfer>();
        check(false, extended_transfer_data.target.to_string());
        if (extended_transfer_data.target) {
          //utils::require_account(extended_transfer_data.target);
          require_recipient(extended_transfer_data.target);
        }
      } else if (from == get_self()) {
        if (current_action_size > transfer_action_size) {
          const auto extended_transfer_data = unpack_action_data<extended_transfer>();
          if (extended_transfer_data.target) {
            utils::require_account(extended_transfer_data.target);
            require_recipient(extended_transfer_data.target);
          }
        }
      } else { return; }*/

      if (memo.size() == GAME_BET_MEMO_SZ) {
        require_recipient(GAME_ACCOUNT);
      } else if (memo == PARTNER_FEE_MEMO) {
        require_recipient(AGENT_ACCOUNT);
      } else {
        if (
          find(
            whitelisted_senders.begin(),
            whitelisted_senders.end(),
            from
          ) == whitelisted_senders.end()
        ) check(memo.empty(), "memo must be empty (spam protection)");
      }
    } else if (from == get_self()) {
      require_recipient(GAME_ACCOUNT);
    } else { return; }
  }
}
