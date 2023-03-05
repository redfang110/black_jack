#include <unistd.h>
#include <ncurses.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "poker.h"
//Setup
//Everyone gets 2 cards (you only see 1 of the dealer's card)
//

//Playing decisions
// Hitting
// Standing
// Splitting- if you have 2 matching cards, you can split 
    // this splits your cards into 2 hands (putting a new bet equal to initial bet for the second hand), and you play both hands as normal
    // (ace splits only get dealt one card)
// Doubling down- Bet 2x initial bet, hit once, and you cannot hit again 
// Buying insurance- if the dealer has an ace, you can pay half of your bet and if they have a blackjack, you keep your money
// Surrender- lose half your bet immediately and forfeit the hand

//Outcomes
//If the player wins, they get money equal to bet
//If they lose, they lose their bet
//If they tie, they don't win or lose
//If they get blackjack, they win 1.5
float money = 1000;

void init()
{
    initscr();
    raw();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
}

int place_bet()
{
    WINDOW *bet = newwin(3, 30, 4, 1);
    bool quit = false;
    int input;
    char c[80] = "";
    std::string str;
    while (!quit) {
        wclear(bet);
        box(bet, 0, 0);
        mvwprintw(bet, 1, 1, "Place bet (min 6): ");
        wrefresh(bet);
        curs_set(2);

        echo();
        wgetstr(bet, c);
        noecho();
        str = c;
        input = stoi(str);
        curs_set(0);

        if (input > money) {
            wclear(bet);
            box(bet, 0, 0);
            mvwprintw(bet, 1, 1, "You cannot bet more than you have...");
            wrefresh(bet);
            sleep(3);
        } else if (input <= 5) {
            wclear(bet);
            box(bet, 0, 0);
            mvwprintw(bet, 1, 1, "You cannot bet less than 6...");
            wrefresh(bet);
            sleep(3);
        } else {
            quit = true;
        }
    }
    wclear(bet);
    wrefresh(bet);
    delwin(bet);
    // money -= input;
    return input;
}

Hand split(Card card, Hand hand)
{
    Hand temp(hand.get(hand.size() - 1), card);
    return temp;
}

void bust(int hand)
{
    WINDOW *bust = newwin(3, 30, 4, 1);
    box(bust, 0, 0);
    std::string str = "Your ";
    switch(hand) {
        case 1:
            str += "First ";
            break;
        case 2:
            str += "Second ";
            break;
        case 3:
            str += "Third ";
            break;
        default:
            str += "Fourth ";
            break;
    }
    str += "Hand Busted!";
    mvwprintw(bust, 1, 1, str.c_str());
    wrefresh(bust);
    wgetch(bust);
    wclear(bust);
    wrefresh(bust);
    delwin(bust);
}

void winnings(float var)
{
    WINDOW *winnings = newwin(3, 30, 4, 1);
    box(winnings, 0, 0);
    std::stringstream stream;
    std::string str = "You ";
    if (var > 0) {
        stream << std::fixed << std::setprecision(2) << var;
        str += "Won ";
        str += stream.str();
        str += "!";
    } else if (var < 0) {
        var = var * -1;
        stream << std::fixed << std::setprecision(2) << var;
        str += "Lost ";
        str += stream.str();
        str += "...";
    } else {
        str += "Broke Even";
    }
    mvwprintw(winnings, 1, 1, str.c_str());
    wrefresh(winnings);
    wgetch(winnings);
    wclear(winnings);
    wrefresh(winnings);
    delwin(winnings);
}

void round(WINDOW *round, Deck &deck, bool cheat)
{
    // Deck deck;
    WINDOW *extra = newwin(5, 40, 1, 32);
    wclear(round);
    box(round, 0, 0);
    std::string dealer_msg, player_msg, hand_msg, space = " ", invalid;
    bool quit = false;
    int cur_hand = 1, bust_count = 0, total_hands = 1;
    float wins = 0;
    // int dealer_ind, player_ind;
    // dealer_ind = player_ind = 1;

    deck.reset_deck();
    Card first = deck.deal(), second = deck.deal(), third = deck.deal(), fourth = deck.deal();
    Hand dealer(second, fourth), player;
    if (cheat) {
        Card card = deck.deal();
        Card pair = deck.deal();
        while (card.value != pair.value) {
            pair = deck.deal();
        }
        player.hit(card);
        player.hit(pair);
    } else {
        player.hit(first);
        player.hit(third);
    }
    std::vector<Hand> extra_hands;
    std::vector<std::string> extra_msgs;
    player.bet = place_bet();

    dealer_msg = "Dealer's Hand: ";
    dealer_msg += dealer.get(0).get_value() + dealer.get(0).get_suit();
    dealer_msg += " ??";

    player_msg = "Your Hand: ";
    player_msg += player.get(0).get_value() + player.get(0).get_suit() + space + player.get(1).get_value() + player.get(1).get_suit();

    mvwprintw(round, 1, 1, dealer_msg.c_str());
    mvwprintw(round, 2, 1, player_msg.c_str());
    mvwprintw(round, 4, 1, "1- Hit");
    mvwprintw(round, 5, 1, "2- Stand");
    mvwprintw(round, 6, 1, "3- Double");

    while (!quit)
    {
        invalid = "                ";
        if (player.get(0).value == player.get(1).value)
        {
            mvwprintw(round, 7, 1, "4- Split");
        } else if (cur_hand > 1 && extra_hands.at(extra_hands.size() - 1).get(0).value == extra_hands.at(extra_hands.size() - 1).get(1).value) {
            mvwprintw(round, 7, 1, "4- Split");
        } else {
            mvwprintw(round, 7, 1, "        ");
        }
        hand_msg = "Playing Hand #";
        hand_msg += std::to_string(cur_hand);
        hand_msg += ": ";
        mvwprintw(round, 9, 1, hand_msg.c_str());
        // mvwprintw(round, 8, 1, "q- quit");
        wrefresh(round);
        char input;
        input = wgetch(round);
        switch(input)
        {
            case '1':
                if (player.stand == false) {
                    player.hit(deck.deal());
                    player_msg += space + player.get(player.size() - 1).get_value() + player.get(player.size() - 1).get_suit();
                    mvwprintw(round, 2, 1, player_msg.c_str());
                    wrefresh(round);
                    if (player.bust()) 
                    {
                        bust(cur_hand);
                        box(round, 0, 0);
                        mvwprintw(round, 4, 1, "1- Hit");
                        mvwprintw(round, 5, 1, "2- Stand");
                        wrefresh(round);
                        cur_hand++;
                        bust_count++;
                        wins -= player.bet;
                        player.stand = true;
                    }
                } else {
                    int i = 0;
                    while (extra_hands.at(i).stand == true) {
                        i++;
                    }
                    extra_hands.at(i).hit(deck.deal());
                    extra_msgs.at(i) += space + extra_hands.at(i).get(extra_hands.at(i).size() - 1).get_value() + extra_hands.at(i).get(extra_hands.at(i).size() - 1).get_suit();
                    mvwprintw(extra, extra_hands.size(), 1, extra_msgs.at(i).c_str());
                    wrefresh(extra);
                    if (extra_hands.at(i).bust()) {
                        bust(cur_hand);
                        box(round, 0, 0);
                        mvwprintw(round, 4, 1, "1- Hit");
                        mvwprintw(round, 5, 1, "2- Stand");
                        wrefresh(round);
                        cur_hand++;
                        bust_count++;
                        wins -= extra_hands.at(i).bet;
                        extra_hands.at(i).stand = true;
                    }
                }
                if (bust_count == total_hands) {
                    quit = true;
                }
                break;
            case '2':
                if (!player.stand) {
                    player.stand = true;
                } else if (extra_hands.size() > 0) {
                    int i = 0;
                    while (extra_hands.at(i).stand == true) {
                        i++;
                    }
                    extra_hands.at(i).stand = true;
                    if (i == extra_hands.size() - 1) {
                        quit = true;
                    }
                } else {
                    quit = true;
                }
                cur_hand++;
                break;
            case '3':
                if (cur_hand == 1) {
                    // money -= player.bet;
                    player.bet += player.bet;
                    player.hit(deck.deal());
                    player.stand = true;
                    cur_hand++;
                    player_msg += space + player.get(player.size() - 1).get_value() + player.get(player.size() - 1).get_suit();
                    mvwprintw(round, 2, 1, player_msg.c_str());
                    wrefresh(round);
                    if (player.bust()) 
                    {
                        bust(cur_hand - 1);
                        box(round, 0, 0);
                        mvwprintw(round, 4, 1, "1- Hit");
                        mvwprintw(round, 5, 1, "2- Stand");
                        wrefresh(round);
                        cur_hand++;
                        bust_count++;
                        wins -= player.bet;
                        player.stand = true;
                    }
                } else {
                    int i = 0;
                    while (extra_hands.at(i).stand == true) {
                        i++;
                    }
                    // money -= extra_hands.at(i).bet;
                    extra_hands.at(i).bet += extra_hands.at(i).bet;
                    extra_hands.at(i).hit(deck.deal());
                    extra_hands.at(i).stand = true;
                    cur_hand++;
                    extra_msgs.at(i) += space + extra_hands.at(i).get(extra_hands.at(i).size() - 1).get_value() + extra_hands.at(i).get(extra_hands.at(i).size() - 1).get_suit();
                    mvwprintw(extra, extra_hands.size(), 1, extra_msgs.at(i).c_str());
                    wrefresh(extra);
                    if (extra_hands.at(i).bust()) {
                        bust(cur_hand - 1);
                        box(round, 0, 0);
                        mvwprintw(round, 4, 1, "1- Hit");
                        mvwprintw(round, 5, 1, "2- Stand");
                        wrefresh(round);
                        cur_hand++;
                        bust_count++;
                        wins -= extra_hands.at(i).bet;
                        extra_hands.at(i).stand = true;
                    }
                }
                if (bust_count == total_hands) {
                    quit = true;
                }
                break;
            case '4':
                if (player.get(0).value == player.get(1).value)
                {
                    total_hands++;
                    Card temp_card = deck.deal();
                    extra_hands.push_back(split(deck.deal(), player));
                    player.hand.pop_back();
                    player.hand.push_back(temp_card);
                    player.reset_total();
                    extra_hands.at(extra_hands.size() - 1).bet = player.bet;
                    // money -= extra_hands.at(extra_hands.size() - 1).bet;
                    std::string extra_msg = "Your Second Hand: ";
                    
                    extra_msg += extra_hands.at(extra_hands.size() - 1).get(0).get_value() + extra_hands.at(extra_hands.size() - 1).get(0).get_suit();
                    extra_msg += space + extra_hands.at(extra_hands.size() - 1).get(1).get_value() + extra_hands.at(extra_hands.size() - 1).get(1).get_suit();
                    extra_msgs.push_back(extra_msg);
                    box(extra, 0, 0);
                    mvwprintw(extra, extra_hands.size(), 1, extra_msgs.at(extra_msgs.size() - 1).c_str());
                    wrefresh(extra);
                    player_msg = "Your Hand: ";
                    player_msg += player.get(0).get_value() + player.get(0).get_suit() + space + player.get(1).get_value() + player.get(1).get_suit();
                    mvwprintw(round, 2, 1, player_msg.c_str());
                    wrefresh(round);
                    break;
                } else if (extra_hands.size() > 0 && extra_hands.at(extra_hands.size() - 1).get(0).value == extra_hands.at(extra_hands.size() - 1).get(1).value) {
                    total_hands++;
                    Card temp_card = deck.deal();
                    extra_hands.push_back(split(deck.deal(), extra_hands.at(extra_hands.size() - 1)));
                    extra_hands.at(extra_hands.size() - 2).hand.pop_back();
                    extra_hands.at(extra_hands.size() - 2).hand.push_back(temp_card);
                    extra_hands.at(extra_hands.size() - 2).reset_total();
                    extra_hands.at(extra_hands.size() - 1).bet = extra_hands.at(extra_hands.size() - 2).bet;
                    // money -= extra_hands.at(extra_hands.size() - 1).bet;
                    std::string extra_msg = "Your ";
                    switch(extra_hands.size()) {
                        case 2:
                            extra_msg += "Third ";
                            break;
                        default:
                            extra_msg += "Fourth ";
                            break;
                    }
                    extra_msg += "Hand: ";
                    extra_msg += extra_hands.at(extra_hands.size() - 1).get(0).get_value() + extra_hands.at(extra_hands.size() - 1).get(0).get_suit();
                    extra_msg += space + extra_hands.at(extra_hands.size() - 1).get(1).get_value() + extra_hands.at(extra_hands.size() - 1).get(1).get_suit();

                    extra_msgs.at(extra_msgs.size() - 2) = "Your ";
                    switch(extra_hands.size() - 1) {
                        case 1:
                            extra_msgs.at(extra_msgs.size() - 2) += "Second ";
                        case 2:
                            extra_msgs.at(extra_msgs.size() - 2) += "Third ";
                            break;
                        default:
                            extra_msgs.at(extra_msgs.size() - 2) += "Fourth ";
                            break;
                    }
                    extra_msgs.at(extra_msgs.size() - 2) += "Hand: ";
                    mvwprintw(extra, extra_hands.size() - 1, 1, extra_msgs.at(extra_msgs.size() - 2).c_str());

                    extra_msgs.push_back(extra_msg);
                    mvwprintw(extra, extra_hands.size(), 1, extra_msgs.at(extra_msgs.size() - 1).c_str());
                    wrefresh(extra);
                    break;
                }
            default:
                invalid = "Invalid Input...";
                break;
        }
        mvwprintw(round, 10, 1, invalid.c_str());
        wrefresh(round);
        if (cur_hand > total_hands) {
            quit = true;
        }
    }
    if (total_hands > bust_count) {
        dealer.reset_total();
        dealer_msg = "Dealer's Hand: ";
        dealer_msg += dealer.get(0).get_value() + dealer.get(0).get_suit() + space + dealer.get(1).get_value() + dealer.get(1).get_suit();
        mvwprintw(round, 1, 1, dealer_msg.c_str());
        wrefresh(round);
        sleep(1);
        if (dealer.total <= 16) {
            dealer.hit(deck.deal());
            dealer_msg += space + dealer.get(2).get_value() + dealer.get(2).get_suit();
            mvwprintw(round, 1, 1, dealer_msg.c_str());
            wrefresh(round);
            sleep(1);
        }
        if (!dealer.bust())
        {
            player.reset_total();
            if (!player.bust()) {
                if (player.total > dealer.total) {
                    if (player.blackjack()) {
                        wins += player.bet * 1.5;
                    } else {
                        wins += player.bet;
                    }
                } else if (player.total < dealer.total) {
                    wins -= player.bet;
                }
            }
            for (int i = 0; i < extra_hands.size(); i++) {
                if (!extra_hands.at(i).bust()) {
                    if (extra_hands.at(i).total > dealer.total) {
                        if (extra_hands.at(i).blackjack()) {
                            wins += extra_hands.at(i).bet * 1.5;
                        } else {
                            wins += extra_hands.at(i).bet;
                        }
                    } else if (extra_hands.at(i).total < dealer.total) {
                        wins -= extra_hands.at(i).bet;
                    }
                }
            }
        } else {
            if (!player.bust()) {
                if (player.blackjack()) {
                    wins += player.bet * 1.5;
                } else {
                    wins += player.bet;
                }
            }
            for (int i = 0; i < extra_hands.size(); i++) {
                if (!extra_hands.at(i).bust()) {
                    if (extra_hands.at(i).blackjack()) {
                        wins += extra_hands.at(i).bet * 1.5;
                    } else {
                        wins += extra_hands.at(i).bet;
                    }
                }
            }
        }
    }
    money += wins;
    winnings(wins);
    touchwin(round);
    wclear(extra);
    wrefresh(extra);
    delwin(extra);
}

void rules()
{
    WINDOW *rules = newwin(18, 90, 1, 1);
    box(rules, 0, 0);

    mvwprintw(rules, 1, 1, "Goal: to beat the dealer - closest player to 21 without going over wins");
    mvwprintw(rules, 2, 1, "Card Values: 2-10 = face value, J-K = 10, A = either 1 or 11");
    mvwprintw(rules, 4, 1, "Hit:    Get an extra card (careful, don't bust!)");
    mvwprintw(rules, 5, 1, "Stand:  Keep your cards and proceed to dealer's turn");
    mvwprintw(rules, 6, 1, "Double: Double your bet, hit once, and immediately stand");
    mvwprintw(rules, 7, 1, "Split:  If you have a pair, add a second bet equal to your first bet,");
    mvwprintw(rules, 8, 1, "        split cards into 2 seperate hands, and hit an extra card to each");
    mvwprintw(rules, 9, 1, "        (Ace splits immediately stand)");
    mvwprintw(rules, 11, 1, "Blackjack: Getting exactly 21 with your initial 2 cards");
    mvwprintw(rules, 12, 1, "Busting:   Going over 21");
    mvwprintw(rules, 13, 1, "Push:      If both the player and dealer have equal card values (a tie),");
    mvwprintw(rules, 14, 1, "           money is neither gained nor lost");
    mvwprintw(rules, 16, 1, "Press any key to exit rulebook...");
    wrefresh(rules);

    wgetch(rules);
    wclear(rules);
    wrefresh(rules);
    delwin(rules);
}

void game()
{
    std::setprecision(2);
    WINDOW *game = newwin(12, 30, 1, 1);
    bool quit = false;
    
    while (!quit) {
        box(game, 0, 0);
        Deck deck;
        std::string invalid = "                ";
        // char invalid[] = "                ";
        std::string msg = "Balance: ";
        std::stringstream stream;
        stream << std::fixed << std::setprecision(2) << money;
        msg += stream.str();
        // msg += std::to_string(money);

        mvwprintw(game, 1, 1, "Black Jack");
        mvwprintw(game, 2, 1, "                  ");
        mvwprintw(game, 2, 1, msg.c_str());
        mvwprintw(game, 4, 1, "1- Play");
        mvwprintw(game, 5, 1, "2- Rulebook");
        mvwprintw(game, 6, 1, "3- Quit");
        wrefresh(game);

        switch(wgetch(game))
        {
            case '1':
                round(game, deck, false);
                wclear(game);
                break;
            case '2':
                rules();
                touchwin(game);
                break;
            case '3':
                quit = true;
                break;
            case 'C':
                round(game, deck, true);
                wclear(game);
                break;
            default:
                invalid = "Invalid Input...";
                break;
        }
        mvwprintw(game, 9, 1, invalid.c_str());
        wrefresh(game);
    }
    wclear(game);
    wrefresh(game);
    delwin(game);
}

int main(int argc, char* argv[])
{
    init();
    game();
    endwin();
    return 0;
}