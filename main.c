#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>  // bool, true, false

// --- Globales ---
int x, y;           // posición del jugador
int ty, tx;         // posición del goblin
int gold = 0;
int pocion_inicial = 0;
int r_place = 0;    // cuántas salas fueron colocadas
bool t_placed = false;

int game_loop(int, int, char(*)[], int);
int gen_dungeon(int fil, int col, char (*map)[col]);
int draw_dungeon(int fil, int col, char (*map)[col]);

int main()
{
    int c = 0;
    srand(time(NULL));

    initscr();
    keypad(stdscr, 1);
    noecho();
    curs_set(0);

    int fila, col;
    getmaxyx(stdscr, fila, col);

    char map[fila][col];

    do
    {
        game_loop(fila - 1, col - 1, map, c);
        refresh();
    } while ((c = getch()) != 27);

    endwin();
    return 0;
}

int gen_dungeon(int fil, int col, char (*map)[col])
{

    // 1. INICIALIZAR el mapa solo la primera vez
    if (r_place == 0)
    {
        
        for (int yy = 0; yy <= fil; yy++)
        {
            for (int xx = 0; xx <= col; xx++)
            {
                if (((xx == 0 || xx == col) && yy != fil) || (yy == 0 || yy == fil - 1))
                    map[yy][xx] = '%';
                else if (yy == fil)
                    map[yy][xx] = ' '; // línea UI
                else
                    map[yy][xx] = '#';
            }
        }
    }

        if (r_place == 0)
    {
    // 2. GENERAR habitaciones y corredores (solo hasta cant_room)
    // Variables para los corredores — deben persistir entre iteraciones del while

        int ry, rx;         // coordenadas de la habitación
        int tam_ry, tam_rx; // tamaño de la habitación
            
        int r_centre_y=0;
        int r_centre_x=0;
        int cant_room = rand() % 5 + 3;// cant_room se calcula UNA sola vez aquí

        while (r_place < cant_room)
        {
            bool colition;
            int room_loop_count=0;
            do
            {
                colition=0;
            // Elegir posición aleatoria válida para la habitación
                do
                {
                    rx = rand() % (col - 3);
                    ry = rand() % (fil - 3);
                    // Tamaño aleatorio
                    tam_rx = rand() % 10 + 5;
                    tam_ry = rand() % 5 + 2;
                } while (ry < 1 || rx < 1);


                // Ajustar si se sale del mapa
                if (rx + tam_rx >= col) tam_rx = col - rx - 2;
                if (ry + tam_ry >= fil) tam_ry = fil - ry - 2; 

                // cheqeuo de de colision
                for (int yy = ry; yy <= ry + tam_ry; yy++)
                {
                    for (int xx = rx; xx <= rx + tam_rx; xx++)
                    {
                        if (map[yy][xx] == ' ' ||map[yy+2][xx] == ' ' ||
                            map[yy-2][xx] == ' '||map[yy][xx+2] == ' '
                            || map[yy][xx-2] == ' ')
                        {
                            yy = ry + tam_ry;
                            colition=1;
                            break;
                        }
                    }
                }
                room_loop_count++;
                if (room_loop_count >100)
                {
                    ry=11;rx=11;
                    tam_rx=2;tam_ry=2;
                    break;
                }
                
            }
            while (colition);
            
            //fill map DB with room cordenads
            for (int yy = ry; yy <= ry + tam_ry; yy++)
            {
                for (int xx = rx; xx <= rx + tam_rx; xx++)
                {
                    if (map[yy][xx] != '%' &&
                        yy > 0 && yy < fil - 1 &&
                        xx > 0 && xx < col)
                    {
                        map[yy][xx] = ' ';
                    }
                }
            }
            r_place++;
            int r_centre_y_anterior=r_centre_y;
            int r_centre_x_anterior=r_centre_x;
            
            r_centre_y=ry+(tam_ry/2);
            r_centre_x=rx +(tam_rx/2);
            //if(r_place !=0)
            if (r_place > 1)
            {
                int path_y;
                // tramo vertical: desde centro anterior hasta centro nuevo
                for (path_y= r_centre_y_anterior; path_y != r_centre_y; )
                {
                    map[path_y][r_centre_x_anterior] = ' ';
                    if (r_centre_y_anterior < r_centre_y) path_y++;
                    else path_y--;
                }

                // tramo horizontal: desde centro anterior hasta centro nuevo
                for (int xx = r_centre_x_anterior; xx != r_centre_x; )
                {
                    map[path_y][xx] = ' ';
                    if (r_centre_x_anterior < r_centre_x) xx++;
                    else xx--;
                }
            }


        }
    }

    // 3. POSICIONAR al jugador en un espacio libre (solo la primera vez)
    if (pocion_inicial == 0)
    {
        do
        {
            x = rand() % col;
            y = rand() % fil;
        } while (map[y][x] != ' ');
        pocion_inicial = 1;
    }
    return 0;
}

int draw_dungeon(int fil, int col, char (*map)[col])
{
// 4. DIBUJAR el mapa
    for (int yy = 0; yy <= fil; yy++)
    {
        for (int xx = 0; xx <= col; xx++)
        {
            if (map[yy][xx] == ' ')
                mvaddch(yy, xx, ' ');
            else if (((xx == 0 || xx == col) && yy != fil) || (yy == 0 || yy == fil - 1))
                mvaddch(yy, xx, '%');
            else if (yy == fil)
                mvaddch(yy, xx, ' ');
            else
                mvaddch(yy, xx, '#');
        }
    }
    return 0;
}

int game_loop(int fil, int col, char (*map)[col], int player)
{
    gen_dungeon( fil, col, map);
    draw_dungeon(fil, col, map);

    // 5. MOVER al jugador (verificando que el destino sea ' ')
    if      (player == KEY_UP    && y > 0   && map[y-1][x] == ' ') y--;
    else if (player == KEY_DOWN  && y < fil  && map[y+1][x] == ' ') y++;
    else if (player == KEY_RIGHT && x < col  && map[y][x+1] == ' ') x++;
    else if (player == KEY_LEFT  && x > 0   && map[y][x-1] == ' ') x--;

    // 6. POSICIONAR al goblin en un espacio libre (solo la primera vez)
    if (!t_placed)
    {
        do
        {
            tx = rand() % col;
            ty = rand() % fil;
        } while (map[ty][tx] != ' ');
        t_placed = true;
    }

    // Recoger oro al tocar al goblin
    if (y == ty && x == tx)
    {
        t_placed = false;
        gold += rand() % 10 + 1;
    }

    // 7. DIBUJAR goblin y jugador encima del mapa
    mvaddch(ty, tx, 't');
    mvaddch(y,  x,  '@');

    // 8. UI
    mvprintw(fil, 0, "Gold: %d ", gold);

    return 0;
}

/*
gcc main.c -o main -lncurses
./main
*/