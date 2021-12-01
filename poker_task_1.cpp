#include <iostream>
#include <vector>
#include <array>
#include <random>
#include <algorithm>
#include <string>

class Card
{
public:
    enum class SUIT { HEART = 1, DIAMOND, CLUB, SPADE };
    enum class RANK { A = 1, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, J, Q, K };

    Card(SUIT suit, RANK rank) : m_suit(suit), m_rank(rank) { }

    SUIT suit() const { return m_suit; }
    RANK rank() const { return m_rank; }

    // operator overloads
    friend bool operator < (const Card& lhs, const Card& rhs);  // lhs.rank < rhs.rank
    friend bool operator > (const Card& lhs, const Card& rhs);  // lhs.rank > rhs.rank
    friend bool operator == (const Card& lhs, const Card& rhs); // lhs.suit == rhs.suit
    friend bool operator != (const Card& lhs, const Card& rhs); // lhs.suit != rhs.suit
    friend std::ostream& operator << (std::ostream& os, const Card& card);

private:
    SUIT m_suit;
    RANK m_rank;
};


using CardList = std::initializer_list<Card>;
using CardArray_5 = const std::array<Card, 5>;
class Deck
{
public:
    Deck(CardList cards) : m_cards(cards) {}

    unsigned totalCards() const { return m_cards.size(); }      // total number of cards in the deck
    const Card& at(unsigned i) const { return m_cards.at(i); }  // access card by given index
    void shuffle() { std::random_shuffle(m_cards.begin(), m_cards.end()); }  // randomize all the cards in a deck

    static bool isFlush(CardList cards);                // if all cards have the same suit
    static bool isStraight(CardArray_5  &cards);  // if all 5 cards have consecutive ranks

private:
    std::vector<Card> m_cards;
};


int main()
{
    using std::cout;
    using std::endl;

    cout << "Welcome to Poker App" << endl << endl;

    Card card1 = { Card::SUIT::HEART, Card::RANK::A };
    Card card2 = { Card::SUIT::HEART, Card::RANK::EIGHT };
    Card card3 = { Card::SUIT::DIAMOND, Card::RANK::FIVE };
    Card card4 = { Card::SUIT::HEART, Card::RANK::A };

    cout << "Testing basic card data structure...." << endl;
    if (card1 < card2) cout << "\tCard1 < Card2" << endl;
    if (card2 > card1) cout << "\tCard2 < Card1" << endl;
    if (!(card1 == card2)) cout << "\tCard1 != Card2" << endl;
    if (card1 != card3) cout << "\tCard1 != Card3" << endl;
    if (card1 == card4) cout << "\tCard1 == Card4" << endl;

    cout << endl;

    Deck standartDeck = {
        { Card::SUIT::HEART, Card::RANK::A },
        { Card::SUIT::HEART, Card::RANK::TWO },
        { Card::SUIT::HEART, Card::RANK::THREE },
        { Card::SUIT::HEART, Card::RANK::FOUR },
        { Card::SUIT::HEART, Card::RANK::FIVE },
        { Card::SUIT::HEART, Card::RANK::SIX },
        { Card::SUIT::HEART, Card::RANK::SEVEN },
        { Card::SUIT::HEART, Card::RANK::EIGHT },
        { Card::SUIT::HEART, Card::RANK::NINE },
        { Card::SUIT::HEART, Card::RANK::TEN },
        { Card::SUIT::HEART, Card::RANK::J },
        { Card::SUIT::HEART, Card::RANK::Q },
        { Card::SUIT::HEART, Card::RANK::K },

        { Card::SUIT::DIAMOND, Card::RANK::A },
        { Card::SUIT::DIAMOND, Card::RANK::TWO },
        { Card::SUIT::DIAMOND, Card::RANK::THREE },
        { Card::SUIT::DIAMOND, Card::RANK::FOUR },
        { Card::SUIT::DIAMOND, Card::RANK::FIVE },
        { Card::SUIT::DIAMOND, Card::RANK::SIX },
        { Card::SUIT::DIAMOND, Card::RANK::SEVEN },
        { Card::SUIT::DIAMOND, Card::RANK::EIGHT },
        { Card::SUIT::DIAMOND, Card::RANK::NINE },
        { Card::SUIT::DIAMOND, Card::RANK::TEN },
        { Card::SUIT::DIAMOND, Card::RANK::J },
        { Card::SUIT::DIAMOND, Card::RANK::Q },
        { Card::SUIT::DIAMOND, Card::RANK::K },

        { Card::SUIT::CLUB, Card::RANK::A },
        { Card::SUIT::CLUB, Card::RANK::TWO },
        { Card::SUIT::CLUB, Card::RANK::THREE },
        { Card::SUIT::CLUB, Card::RANK::FOUR },
        { Card::SUIT::CLUB, Card::RANK::FIVE },
        { Card::SUIT::CLUB, Card::RANK::SIX },
        { Card::SUIT::CLUB, Card::RANK::SEVEN },
        { Card::SUIT::CLUB, Card::RANK::EIGHT },
        { Card::SUIT::CLUB, Card::RANK::NINE },
        { Card::SUIT::CLUB, Card::RANK::TEN },
        { Card::SUIT::CLUB, Card::RANK::J },
        { Card::SUIT::CLUB, Card::RANK::Q },
        { Card::SUIT::CLUB, Card::RANK::K },

        { Card::SUIT::SPADE, Card::RANK::A },
        { Card::SUIT::SPADE, Card::RANK::TWO },
        { Card::SUIT::SPADE, Card::RANK::THREE },
        { Card::SUIT::SPADE, Card::RANK::FOUR },
        { Card::SUIT::SPADE, Card::RANK::FIVE },
        { Card::SUIT::SPADE, Card::RANK::SIX },
        { Card::SUIT::SPADE, Card::RANK::SEVEN },
        { Card::SUIT::SPADE, Card::RANK::EIGHT },
        { Card::SUIT::SPADE, Card::RANK::NINE },
        { Card::SUIT::SPADE, Card::RANK::TEN },
        { Card::SUIT::SPADE, Card::RANK::J },
        { Card::SUIT::SPADE, Card::RANK::Q },
        { Card::SUIT::SPADE, Card::RANK::K },
    };

    /////////// SHUFFLE ///////////
    cout << "Testing shuffle...." << endl;
    for (int i = 0; i < 10; ++i)
    {
        cout << "\tshuffle call: " << i << " -> ";
        cout << standartDeck.at(0) << endl;
        standartDeck.shuffle();
    }

    /////////// FLUSH ///////////
    cout << std::boolalpha;
    cout << "Testing is flush...." << endl;

    cout << "\tis flush? (true) -> ";
    cout << Deck::isFlush({
                              { Card::SUIT::HEART, Card::RANK::A },
                              { Card::SUIT::HEART, Card::RANK::TWO },
                              { Card::SUIT::HEART, Card::RANK::THREE },
                              { Card::SUIT::HEART, Card::RANK::FOUR },
                              { Card::SUIT::HEART, Card::RANK::FIVE },
                          }) << endl;

    cout << "\tis flush? (false) -> ";
    cout << Deck::isFlush({
                              { Card::SUIT::HEART, Card::RANK::A },
                              { Card::SUIT::CLUB, Card::RANK::TWO },
                              { Card::SUIT::SPADE, Card::RANK::THREE },
                              { Card::SUIT::DIAMOND, Card::RANK::FOUR },
                              { Card::SUIT::DIAMOND, Card::RANK::FIVE },
                          }) << endl;


    /////////// STRAIGHT ///////////
    cout << "Testing is straight...." << endl;

    cout << "\t[A, 5, 2, 3, 4] is straight? (true) -> ";
    cout << Deck::isStraight({
                                 Card(Card::SUIT::HEART, Card::RANK::A ),
                                 Card(Card::SUIT::HEART, Card::RANK::FIVE ),
                                 Card(Card::SUIT::HEART, Card::RANK::TWO),
                                 Card(Card::SUIT::HEART, Card::RANK::THREE ),
                                 Card(Card::SUIT::HEART, Card::RANK::FOUR ),
                             }) << endl;

    cout << "\t[A, 5, 3, 4, 6] is straight? (false) -> ";
    cout << Deck::isStraight({
                                 Card( Card::SUIT::HEART, Card::RANK::A ),
                                 Card( Card::SUIT::HEART, Card::RANK::FIVE ),
                                 Card( Card::SUIT::HEART, Card::RANK::THREE ),
                                 Card( Card::SUIT::HEART, Card::RANK::FOUR ),
                                 Card( Card::SUIT::HEART, Card::RANK::SIX ),
                             }) << endl;

    cout << "\t[6, 7, 10, 9, 8] is straight? (true) -> ";
    cout << Deck::isStraight({
                                 Card( Card::SUIT::HEART, Card::RANK::SIX ),
                                 Card( Card::SUIT::HEART, Card::RANK::SEVEN ),
                                 Card( Card::SUIT::HEART, Card::RANK::TEN ),
                                 Card( Card::SUIT::HEART, Card::RANK::NINE ),
                                 Card( Card::SUIT::HEART, Card::RANK::EIGHT ),
                             }) << endl;

    cout << "\t[10, K, A, J, Q] is straight? (true) -> ";
    cout << Deck::isStraight({
                                 Card( Card::SUIT::HEART, Card::RANK::TEN ),
                                 Card( Card::SUIT::HEART, Card::RANK::K ),
                                 Card( Card::SUIT::HEART, Card::RANK::A ),
                                 Card( Card::SUIT::HEART, Card::RANK::J ),
                                 Card( Card::SUIT::HEART, Card::RANK::Q ),
                             }) << endl;

    cout << endl << "Bye!!!" << endl;
    return 0;
}


/////////////////////////// Card class //////////////////////////////
bool operator < (const Card& lhs, const Card& rhs)
{
    return lhs.m_rank < rhs.m_rank;
}

bool operator > (const Card& lhs, const Card& rhs)
{
    return rhs < lhs;
}

bool operator == (const Card& lhs, const Card& rhs)
{
    return lhs.m_suit == rhs.m_suit;
}

bool operator != (const Card& lhs, const Card& rhs)
{
    return !(lhs == rhs);
}

std::ostream& operator << (std::ostream& os, const Card& card)
{
    return os << "suit: " << unsigned(card.m_suit) << ", rank: " << unsigned(card.m_rank);
}



/////////////////////////// Deck class //////////////////////////////
bool Deck::isFlush(CardList cards)
{
    return cards.size() && std::equal(cards.begin() + 1, cards.end(), cards.begin());
}

bool Deck::isStraight(const std::array<Card, 5> &cards)
{                                                               // FYI - cards = [10, K, A, J, Q]
    auto maxRank = unsigned(Card::RANK::K);                     // FYI - maxRank = 13
    auto strbits = std::string(maxRank, '0');                   // FYI - strbits = "0000000000000"
    for(auto card: cards) {
        strbits[unsigned(card.rank()) - 1] = '1';               // FYI - strbits = "1000000001111"
    }
    strbits += strbits.substr(0, 5);                            // FYI - strbits = "100000000111110000"
    auto found = strbits.find("11111");                         // FYI - search for "11111"

    return found != std::string::npos;                          // FYI - return result
}
