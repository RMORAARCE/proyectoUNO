#include <iostream>
#include <ctime>
#include <algorithm>
#include <vector>
#include <array>
#include <map>
#include <thread>
#include <random>
#include <string>
namespace UNO
{
    const std::array<std::string, 4> COMMON_COLORS =
        {"AZUL", "VERDE", "ROJO", "AMARILLO"};
    const std::array<std::string, 1> SPECIAL_COLORS =
        {"NEGRO"};

    const std::array<std::string, 10> NUMBERS =
        {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
    const std::array<std::string, 3> ACTION_TYPES =
        {"REVERSE", "SKIP", "2 PLUS"};
    const std::array<std::string, 2> WILD_TYPES =
        {" Color changed ", "4 PLUS"};

    std::map<std::string, std::string> COLOR_MAP = {
        {"AZUL", "\033[34m"},
        {"VERDE", "\033[32m"},
        {"ROJO", "\033[31m"},
        {"AMARILLO", "\033[93m"},
        {"NEGRO", "\033[30m"},
    };
    const std::string RESET_COLOR = "\033[0m";

    class Card
    {
    public:
        std::string color;
        std::string type;

        Card() : color{"AZUL"}, type{"0"}{};
        Card(std::string color_, std::string type_) : color{color_}, type{type_} {};

        operator std::string() const
        {
            return COLOR_MAP[color] + type + RESET_COLOR;
        }

        friend std::ostream& operator<<(std::ostream& os, const Card& card)
        {
            os << COLOR_MAP[card.color] << card.type << RESET_COLOR;
            return os;
        }
    };

    bool can_play_card(const Card& play_card, const Card& top_card)
    {
        if (
            top_card.color == play_card.color ||
            top_card.type == play_card.type ||
            play_card.color == "NEGRO" ||
            play_card.type == "4 PLUS"
        )
        {
            return true;
        }
        return false;
    };

    std::string cards_to_str(const std::vector<Card>& cards)
    {
        int len_cards = cards.size();
        std::string str_cards = "[";

        for (int i = 0; i < len_cards; ++i)
        {
            str_cards += std::string(cards[i]);

            if (i != len_cards - 1)
                str_cards += ", ";
        }
        str_cards += "]";

        return str_cards;
    }

    std::vector<Card> create_cards(
        int cards_num = 2,
        int cards_action = 1,
        int cards_wild = 2
    )
    {
        std::vector<Card> cards;

        for (auto color : COMMON_COLORS)
        {
            for (auto num_type : NUMBERS)
            {
                for (int i = 0; i < cards_num; ++i)
                    cards.push_back(Card(color, num_type));
            }

            for (auto action_type : ACTION_TYPES)
            {
                for (int i = 0; i < cards_action; ++i)
                    cards.push_back(Card(color, action_type));
            }
        }

        for (auto wild_type : WILD_TYPES)
        {
            for (int i = 0; i < cards_wild; ++i)
                cards.push_back(Card("NEGRO", wild_type));
        }

        return cards;
    }

    class Deck
    {
    public:
        std::vector<Card> cards;

        Deck() : cards{create_cards()} {};

        friend std::ostream& operator<<(std::ostream& os, const Deck& deck)
        {
            os << cards_to_str(deck.cards);
            return os;
        }

        void shuffle()
        {
            std::random_device rd;
            std::mt19937 rng(rd());
            std::shuffle(cards.begin(), cards.end(), rng);
        }

        std::vector<Card> deal_cards(int amount)
        {
            if (cards.size() < amount)
            {
                for (const Card& card : create_cards())
                    cards.push_back(card);
            }

            std::vector<Card> dealt_cards;
            for (int i = 0; i < amount; ++i)
            {
                dealt_cards.push_back(cards.back());
                cards.pop_back();
            }

            return dealt_cards;
        }
    };

    class Player
    {
    public:
        std::string name;
        std::vector<Card> cards;

        Player(std::string name_, std::vector<Card> cards_) :
            name{name_}, cards{cards_} {};

        friend std::ostream& operator<<(std::ostream& os, const Player& player)
        {
            os << player.name;
            return os;
        }

        int get_play_card_index(const Card& top_card) const
        {
            for (int i = 0; i < cards.size(); ++i)
            {
                if (can_play_card(cards[i], top_card))
                    return i;
            }
            return -1;
        }

        Card pop_card(int index)
        {
            Card popped_card = cards[index];
            cards.erase(cards.begin() + index);
            return popped_card;
        }

        bool is_win() const
        {
            return (cards.size() == 0);
        }
    };

    template <typename Type>
    class Cycle
    {
    private:
        int index = -1;
        bool reversed = false;

    public:
        std::vector<Type> items;
        Cycle() {};
        Cycle(std::vector<Type> items_) : items{items_} {};

        Type* next()
        {
            index += (reversed) ? -1 : 1;
            if (index == items.size())
                index = 0;
            else if (index == -1)
                index = items.size() - 1;

            return &items[index];
        }

        void reverse()
        {
            reversed = !reversed;
        }
    };

    class Game
    {
    public:
        Deck cards_deck;
        std::vector<Player> players;
        Cycle<Player> player_cycle;
        Card top_card;

        Game()
        {
            int no_of_players;
            std::cout << "INGRESE NUMERO DE JUGADORES: "; // numero de jugador
            std::cin >> no_of_players;
            if (std::cin.fail())
                std::cerr << "Input Failure";

            std::string player_name;
            std::cout << "INGRESE SU NOMBRE: ";    //  ingresa nombre
            std::cin >> player_name;
            if (std::cin.fail())
                std::cerr << "Input Failure";

            cards_deck.shuffle();
            players.push_back(Player(player_name + " #1", cards_deck.deal_cards(7)));

            for (int i = 0; i < no_of_players - 1; ++i)
            {
                players.push_back(
                    Player(
                        "Player #" + std::to_string(i + 2),
                        cards_deck.deal_cards(7))
                );
            }
            top_card = cards_deck.deal_cards(1)[0];
            player_cycle.items = players;
        };

        Game(Deck cards, Cycle<Player> player_cycle_) :
            cards_deck{cards}, player_cycle{player_cycle_} {};

        bool human_player(const Player& player) const
        {
            return (player.name.back() == '1');
        }

        void play_game()          // metodo para jugar
        {
            print_intro();
            while (true)
            {
                std::this_thread::sleep_for(std::chrono::seconds(3)); // Pause for 3 seconds
                Player* player = next_player();
                print_turn(*player);
                handle_player(player);

                if (player->is_win())
                {
                    std::cout << *player << " AH GANADO EL JUEGO!!\n";    //
                    break;
                }
            }
        };

        void handle_player(Player* player)
        {
            if (human_player(*player))
                handle_human_player(player);
            else
                handle_ai_player(player);
        }

        void print_intro() const       //  bienvenida al juego
        {
            std::cout << "BIENVENIDO AL JUEGO UNO!\n";
            std::cout << "TERMINA TUS CARTAS LO MAS RAPIDO POSIBLE!\n\n";
        }

        void print_turn(Player& player) const    // turno de juego 
        {
            if (human_player(player))
            {
                std::cout << "TIENES " << player.cards.size()
                    << " cards left - " << cards_to_str(player.cards)
                    << '\n';
            }
            else
            {
                std::cout << player << " has " << player.cards.size()
                    << " cards left...\n";
            }
            std::cout << "Top card - " << top_card << "\n\n";
        }

        void handle_human_player(Player* player)
        {
            bool any_play_card = false;
            for (const Card& card : player->cards)
            {
                if (can_play_card(card, top_card))
                {
                    any_play_card = true;
                    break;
                }
            }

            char choice = 'd';  // Para elegir las cartas
            if (any_play_card)
            {
                while (true)
                {
                    std::cout << "Play or Draw? (p/d): ";
                    std::cin >> choice;
                    if (std::cin.fail())
                        std::cerr << "Input Failure";  // error de uso

                    choice = tolower(choice);
                    if (choice == 'p' || choice == 'd')
                        break;
                    else
                        std::cout << "Invalid Input\n";    // no puede usar
                }
            }

            if (choice == 'p')
            {
                int index;
                while (true)
                {
                    std::cout << "Enter card index " << '(' << "1 - " <<      // numero de carta al vector
                        player->cards.size() << ')' << ": ";
                    std::cin >> index;
                    if (std::cin.fail())
                        std::cerr << "Input Failure";

                    if (index < 0 || index > player->cards.size())
                        std::cout << "Invalid index!\n";
                    else if (!can_play_card(player->cards[index - 1], top_card))
                        std::cout << "Can not play " << player->cards[index - 1] << "!\n";
                    else
                        break;
                }

                Card card = player->pop_card(index - 1);
                top_card = card;
                std::cout << *player << " plays " << card << '\n';
                handle_card_effect(&card, player);
            }
            else if (choice == 'd')
            {
                Card card = cards_deck.deal_cards(1)[0];
                player->cards.push_back(card);
                std::cout << "TIENES UN " << card << "...\n";
                std::this_thread::sleep_for(std::chrono::seconds(2)); // Pause for 2 seconds

                if (can_play_card(card, top_card))
                {
                    std::cout << "QUIERES JUGAR ESTE FANTASTICO JUEGO " << card << "? (y/n): ";
                    char choice;
                    std::cin >> choice;
                    if (std::cin.fail())
                        std::cerr << "Input Failure";
                    choice = tolower(choice);

                    if (choice == 'y')
                    {
                        top_card = card;
                        player->cards.pop_back();
                        std::cout << *player << " plays " << card << '\n';
                        handle_card_effect(&card, player);
                    }
                }
                std::cout << '\n';
            }
        }

        void handle_ai_player(Player* player)    //  MAQUINA JUGADOR
        {
            int card_index = player->get_play_card_index(top_card);
            if (card_index != -1)
            {
                Card play_card = player->pop_card(card_index);
                std::cout << *player << " plays " << play_card << '\n';
                handle_card_effect(&play_card, player);
            }
            else
            {
                Card card = cards_deck.deal_cards(1)[0];
                std::cout << "REPARTIR 1 card to " << *player << "...\n";
                std::this_thread::sleep_for(std::chrono::seconds(2)); // Pause for 2 seconds
                if (can_play_card(card, top_card))
                {
                    top_card = card;
                    std::cout << *player << " plays " << card << '\n';
                    handle_card_effect(&card, player);
                }
                else
                    player->cards.push_back(card);
                std::cout << '\n';
            }
        }

        void handle_card_effect(Card* card, Player* card_player)
        {
            bool card_is_number = (std::find(
                NUMBERS.begin(), NUMBERS.end(), card->type) != NUMBERS.end());
            top_card = *card;

            if (card_is_number) {}
            else if (card->type == "REVERSE")
            {
                player_cycle.reverse();
                std::cout << "Cycle reversed...\n";
            }
            else if (card->type == "SKIP")
            {
                Player* skip_player = next_player();
                std::cout << *skip_player << " skipped...\n";
            }
            else if (card->type == "2 PLUS")
            {
                Player* player = next_player();
                std::vector<Card> deal_cards = cards_deck.deal_cards(2);
                for (int i = 0; i < 2; ++i)
                    player->cards.push_back(deal_cards[i]);

                std::cout << "REPARTIR 2 cards to " << *player << ", skipped...\n";
            }
            else if (card->type == "COLOR CHANGE")
            {
                if (human_player(*card_player))
                    top_card.color = get_color_input();
                else
                    top_card.color = COMMON_COLORS[rand() % 4];
                std::cout << "Color changed to " << COLOR_MAP[top_card.color]
                    << top_card.color << RESET_COLOR << '\n';
            }
            else if (card->type == "4 PLUS")
            {
                if (human_player(*card_player))
                    top_card.color = get_color_input();
                else
                    top_card.color = COMMON_COLORS[rand() % 4];
                std::cout << "Color changed to " << COLOR_MAP[top_card.color]
                    << top_card.color << RESET_COLOR << '\n';

                Player* player = next_player();
                std::vector<Card> deal_cards = cards_deck.deal_cards(4);
                for (int i = 0; i < 4; ++i)
                    player->cards.push_back(deal_cards[i]);

                std::cout << "REPARTIR 4 cards to " << *player << ", skipped...\n";
            }
            std::cout << '\n';
        }

        Player* next_player()
        {
            while (true)
            {
                Player* player = player_cycle.next();
                if (!player->is_win())
                    return player;
            }
        };

        std::string get_color_input()
        {
            std::string color;
            do
            {
                std::cout << "INGRESAR COLOR: ";
                std::cin >> color;
                if (std::cin.fail())
                    std::cerr << "Input Failure";
                for (int i = 0; i < color.length(); ++i)
                    color[i] = toupper(color[i]);
            } while (
                std::find(COMMON_COLORS.begin(),
                    COMMON_COLORS.end(), color) == COMMON_COLORS.end()
                );

            return color;
        };
    };
}

int main()
{
    std::srand(static_cast<unsigned>(std::time(nullptr))); // Initialize random seed

    UNO::Game my_game;
    my_game.play_game();
    return 0;
}
