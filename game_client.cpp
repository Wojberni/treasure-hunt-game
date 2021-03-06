#include "game_client.h"
#include <ncurses.h>
#include <unistd.h>
#include <cstdlib>
#include <thread>

int main(){

    Client *client = new Client();
    if(client->init() == EXIT_FAILURE){
        printw("Error initializing client_info!\n");
        delete client;
        return EXIT_FAILURE;
    }
    if(client->connection() == EXIT_FAILURE){
        printw("Error connecting to the server!\n");
        delete client;
        return EXIT_FAILURE;
    }
    client->play_game(client);
    delete client;
    return 0;
}

Client::Client() {

}

int Client::init(){
    init_window();
    if(!has_colors() || check_size_terminal()){
        printw("Invalid size of terminal (min.%dx%d) or no colors support!\nCheck you terminal preferences!\n"
                , MARGIN_SCOREBOARD + SCOREBOARD_WIDTH, BOARD_ROWS);
        return EXIT_FAILURE;
    }
    init_colors();
    is_connected = true;
    return EXIT_SUCCESS;
}

Client::~Client() {
    endwin();
}

void Client::init_window() {
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);
}

void Client::init_colors() {
    start_color();
    use_default_colors();
    init_pair(WALL_COLOR, COLOR_WHITE, COLOR_BLACK);
    init_pair(COIN_COLOR, COLOR_BLACK, COLOR_YELLOW);
    init_pair(CAMPSITE_COLOR, COLOR_WHITE, COLOR_GREEN);
    init_pair(PLAYER_COLOR, COLOR_WHITE, COLOR_BLUE);
    init_pair(TEXT_COLOR, COLOR_BLACK, COLOR_WHITE);
}

bool Client::check_size_terminal() {
    getmaxyx(stdscr, rows, columns);
    if (rows < BOARD_ROWS || columns < BOARD_COLS + SCOREBOARD_WIDTH)
        return EXIT_FAILURE;
    return EXIT_SUCCESS;

}

int Client::connection() {
    network_socket = socket(AF_INET,SOCK_STREAM, 0);

    SA_IN server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(SERVER_PORT);

    int connection_status = connect(network_socket,(struct sockaddr*)&server_address, sizeof(server_address));

    if (connection_status == SOCKET_ERROR) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void Client::print_board() {
    int first_x = client_info.player.left.x - CLIENT_COLS / 2 + 1;
    int first_y = client_info.player.left.y - CLIENT_ROWS / 2;
    for (int i = 0; i < CLIENT_ROWS; i++) {
        if(first_y + i < 0 || first_y + i >= BOARD_ROWS){
            continue;
        }
        for (int j = 0; j < CLIENT_COLS; j++) {
            if(first_x + j < 0 || first_x + j >= BOARD_COLS){
                continue;
            }
            move(first_y + i, first_x + j);
            switch(client_info.player_board[i][j]){
                case FREE:
                    attron(COLOR_PAIR(TEXT_COLOR));
                    addch(' ');
                    attroff(COLOR_PAIR(TEXT_COLOR));
                    break;
                case WALL:
                    attron(COLOR_PAIR(WALL_COLOR));
                    addch(' ');
                    attroff(COLOR_PAIR(WALL_COLOR));
                    break;
                case BUSH:
                    attron(COLOR_PAIR(TEXT_COLOR));
                    addch('#');
                    attroff(COLOR_PAIR(TEXT_COLOR));
                    break;
                case COIN:
                    attron(COLOR_PAIR(COIN_COLOR));
                    addch('c');
                    attroff(COLOR_PAIR(COIN_COLOR));
                    break;
                case TREASURE:
                    attron(COLOR_PAIR(COIN_COLOR));
                    addch('t');
                    attroff(COLOR_PAIR(COIN_COLOR));
                    break;
                case TREASURE_LARGE:
                    attron(COLOR_PAIR(COIN_COLOR));
                    addch('T');
                    attroff(COLOR_PAIR(COIN_COLOR));
                    break;
                case TREASURE_DROP:
                    attron(COLOR_PAIR(COIN_COLOR));
                    addch('D');
                    attroff(COLOR_PAIR(COIN_COLOR));
                    break;
                case CAMPSITE:
                    attron(COLOR_PAIR(CAMPSITE_COLOR));
                    addch('A');
                    attroff(COLOR_PAIR(CAMPSITE_COLOR));
                    break;
                case BEAST:
                    attron(COLOR_PAIR(TEXT_COLOR));
                    addch('*');
                    attroff(COLOR_PAIR(TEXT_COLOR));
                    break;
                case PLAYER1:
                    attron(COLOR_PAIR(PLAYER_COLOR));
                    addch('1');
                    attroff(COLOR_PAIR(PLAYER_COLOR));
                    break;
                case PLAYER2:
                    attron(COLOR_PAIR(PLAYER_COLOR));
                    addch('2');
                    attroff(COLOR_PAIR(PLAYER_COLOR));
                    break;
                case PLAYER3:
                    attron(COLOR_PAIR(PLAYER_COLOR));
                    addch('3');
                    attroff(COLOR_PAIR(PLAYER_COLOR));
                    break;
                case PLAYER4:
                    attron(COLOR_PAIR(PLAYER_COLOR));
                    addch('4');
                    attroff(COLOR_PAIR(PLAYER_COLOR));
                    break;
            }
        }
        addch('\n');
    }
    refresh();
}

void Client::print_scoreboard(){
    mvprintw(0, MARGIN_SCOREBOARD, "Server PID: %d", client_info.server_pid);
    mvprintw(1, MARGIN_SCOREBOARD, "Campsite X/Y: unknown");
    mvprintw(2, MARGIN_SCOREBOARD, "Round number: %d", client_info.round_nr);
    mvprintw(4, MARGIN_SCOREBOARD, "Player:");
    mvprintw(5, MARGIN_SCOREBOARD+2, "Number");
    mvprintw(6, MARGIN_SCOREBOARD+2, "Type");
    mvprintw(7, MARGIN_SCOREBOARD+2, "Curr X/Y");
    mvprintw(8, MARGIN_SCOREBOARD+2, "Deaths");
    mvprintw(10, MARGIN_SCOREBOARD+2, "Coins carried");
    mvprintw(11, MARGIN_SCOREBOARD+2, "Coins brought");

    mvprintw(5, MARGIN_SCOREBOARD+16, "%d", client_info.player_id + 1);
    mvprintw(6, MARGIN_SCOREBOARD+16, "HUMAN");
    mvprintw(7, MARGIN_SCOREBOARD+16, "%d/%d  ", client_info.player.left.x, client_info.player.left.y);
    mvprintw(8, MARGIN_SCOREBOARD+16, "%d", client_info.deaths);
    mvprintw(10, MARGIN_SCOREBOARD+16, "%d  ", client_info.coins_carried);
    mvprintw(11, MARGIN_SCOREBOARD+16, "%d", client_info.coins_brought);

    mvprintw(13, MARGIN_SCOREBOARD, "Legend:");

    attron(COLOR_PAIR(PLAYER_COLOR));
    mvprintw(14, MARGIN_SCOREBOARD, "%d%d%d%d", 1, 2, 3, 4);
    attroff(COLOR_PAIR(PLAYER_COLOR));
    mvprintw(14, MARGIN_SCOREBOARD+6, "- players");

    attron(COLOR_PAIR(WALL_COLOR));
    mvprintw(15, MARGIN_SCOREBOARD, " ");
    attroff(COLOR_PAIR(WALL_COLOR));
    mvprintw(15, MARGIN_SCOREBOARD+6, "- wall");

    attron(COLOR_PAIR(TEXT_COLOR));
    mvprintw(16, MARGIN_SCOREBOARD, "#");
    attroff(COLOR_PAIR(TEXT_COLOR));
    mvprintw(16, MARGIN_SCOREBOARD+6, "- bushes (slow down)");

    attron(COLOR_PAIR(TEXT_COLOR));
    mvprintw(17, MARGIN_SCOREBOARD, "*");
    attroff(COLOR_PAIR(TEXT_COLOR));
    mvprintw(17, MARGIN_SCOREBOARD+6, "- beast");

    attron(COLOR_PAIR(COIN_COLOR));
    mvprintw(18, MARGIN_SCOREBOARD, "c");
    attroff(COLOR_PAIR(COIN_COLOR));
    mvprintw(18, MARGIN_SCOREBOARD+6, "- one coin");

    attron(COLOR_PAIR(COIN_COLOR));
    mvprintw(19, MARGIN_SCOREBOARD, "t");
    attroff(COLOR_PAIR(COIN_COLOR));
    mvprintw(19, MARGIN_SCOREBOARD+6, "- treasure (10 coins)");

    attron(COLOR_PAIR(COIN_COLOR));
    mvprintw(20, MARGIN_SCOREBOARD, "T");
    attroff(COLOR_PAIR(COIN_COLOR));
    mvprintw(20, MARGIN_SCOREBOARD+6, "- large treasure (50 coins)");

    attron(COLOR_PAIR(COIN_COLOR));
    mvprintw(21, MARGIN_SCOREBOARD, "D");
    attroff(COLOR_PAIR(COIN_COLOR));
    mvprintw(21, MARGIN_SCOREBOARD+6, "- dropped treasure");

    attron(COLOR_PAIR(CAMPSITE_COLOR));
    mvprintw(22, MARGIN_SCOREBOARD, "A");
    attroff(COLOR_PAIR(CAMPSITE_COLOR));
    mvprintw(22, MARGIN_SCOREBOARD+6, "- campsite");

    refresh();
}

void Client::play_game(Client *client) {
    std::thread receiver(&Client::receiver, client);
    receiver.detach();
    while(is_connected){
        int input = getch();
        flushinp();
        long check = send(network_socket, &input, sizeof(input), 0);
        if(check == -1){
            printw("Error sending to server!\n");
        }
        if(input == 'q' || input == 'Q'){
            is_connected = false;
            break;
        }
    }
    close(network_socket);
}

void Client::receiver(){
    while(is_connected){
        int res = recv(network_socket, &client_info, sizeof(struct client_struct), 0);
        if(res == SOCKET_ERROR){
            printw("Receiving socket error!\n");
            continue;
        }
        clear_board();
        print_board();
        print_scoreboard();
    }
}

void Client::clear_board() {
    for(int i = 0; i < BOARD_ROWS; i++){
        for(int j = 0; j < BOARD_COLS; j++){
            mvprintw(i, j, " ");
        }
    }
    refresh();
}
