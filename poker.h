# include <stdlib.h>
# include <assert.h>
# include <vector>
# include <algorithm>
# include <random>

unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
std::default_random_engine rng(seed);
// rd = std::random_device {};
// rng = std::default_random_engine { rd() };

enum Suits{
    spade,
    heart,
    diamond,
    club
};

Suits getSuit(int i)
{
    switch(i)
    {
        case 0:
            return spade;
            break;
        case 1:
            return heart;
            break;
        case 2:
            return diamond;
            break;
        default:
            return club;
            break;
    }
}

// 1 = ace, 11 = jack, 12 = queen, 13 = king
class Card {
public:
    Card(Suits suit, int value)
    {
        this->suit = suit;
        this->value = value;
    }
    Suits suit;
    int value;

    std::string get_suit()
    {
        switch(suit)
        {
            case spade:
                return "S";
                break;
            case heart:
                return "H";
                break;
            case diamond:
                return "D";
                break;
            default:
                return "C";
                break;
        }
    }
    std::string get_value()
    {
        switch(value)
        {
            case 11:
                return "J";
                break;
            case 12:
                return "Q";
                break;
            case 13:
                return "K";
                break;
            case 14:
                return "A";
                break;
            default:
                return std::to_string(value);
                break;
        }
    }
};

class Hand {
private:
    int ace_count;
    int normalize(int i)
    {
        if (i > 10) {
            if (i == 14) {
                return 11;
            } else {
                return 10;
            }
        } else {
            return i;
        }
    }
public:
    int total;
    std::vector<Card> hand;
    bool stand;
    int bet;
    Hand(Card first_card, Card second_card)
    {
        hand.push_back(first_card);
        hand.push_back(second_card);
        stand = false;
        reset_total();
    }

    Hand()
    {
        stand = false;
    }

    void hit(Card card)
    {
        hand.push_back(card);
        reset_total();
    }

    int size()
    {
        return hand.size();
    }

    Card get(int ind)
    {
        return hand.at(ind);
    }

    bool bust()
    {
        if (total > 21) {
            return true;
        }
        return false;
    }

    bool blackjack()
    {
        reset_total();
        if (total == 21 && hand.size() == 2) {
            return true;
        }
        return false;
    }

    void reset_total()
    {
        ace_count = 0;
        total = 0;
        for (int i = 0; i < hand.size(); i++) {
            if (normalize(hand.at(i).value) == 11) {
                ace_count++;
            }
            total += normalize(hand.at(i).value);
        }
        while (total > 21 && ace_count > 0) {
            total -= 10;
            ace_count--;
        }
    }
};

class Deck {
private:
    void shuffle()
    {
        std::shuffle(std::begin(cards), std::end(cards), rng);
    }
public:
    std::vector<Card> cards;
    std::vector<Card> fresh_cards;
    Deck() 
    {
        for (int i = 0; i < 4; i++) {
            for (int j = 2; j < 15; j++) {
                Card *temp = new Card(getSuit(i), j);
                fresh_cards.push_back(*temp);
            }
        }
        reset_deck();
    }

    void reset_deck()
    {
        cards = fresh_cards;
        shuffle();
    }

    Card deal()
    {
        Card temp = cards[cards.size() - 1];
        cards.pop_back();
        if (cards.size() < 1) {
            reset_deck();
        }
        return temp;
    }
};