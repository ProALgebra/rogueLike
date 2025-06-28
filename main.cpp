#include "BearLibTerminal.h"
#include<vector>
#include<string>
#include<iostream>
#include <memory>
#include <random>
#include <fstream>
#include <sstream>

struct GameMapPoint {
    int objId = -1;
};
struct Effect {
    int id = -1;
    int value = 0;
    int timing = 0;
};
class GameObject {
public:
    int heals;
    int type = -1;
    std::vector<std::string> effects;
    int x;
    int y;

    void get_collision(){};
    std::vector<Effect> send_collision(){};

    virtual int move() {
        return -1;
    };

    virtual int getType() {
        return type;
    }

    virtual int skin() {
        return '.';
    };
};

class Wall : public GameObject {
private:
    int type = 1;
public:
    void collision(){};
    int skin() override {
        return 9608;
    }
};

class Player : public GameObject {

public:
    int heals;
    int type = 0;
    int damage = 1;
    std::vector<std::string> effects;

    int x;
    int y;

    Player(int x, int y):x(x), y(y) {
        heals = 100;
    };
    int getType() override {
        return 0;
    }

    void get_collision(std::vector<Effect> effects) {
        for (int i = 0; i < effects.size(); i++) {
            if (effects[i].id == 1) {
                heals -= effects[i].value;
            }
            if (effects[i].id == 2) {
                heals += effects[i].value;
                damage += 1;
            }
        }

    };

    std::vector<Effect> send_collision(bool isFirst) {

        std::vector <Effect> effects;
        if (isFirst){
            Effect e;
            e.id = 1;
            e.value = damage;
            effects.push_back(e);
        }
        return effects;
    };

    void collision() {

    };

    int skin() override {
        return '@';
    }

    int move(int state) {
        std::cout << state << std::endl;
        std::cout <<"Heals:"<< heals << std::endl;
        std::cout <<"Damage:"<< damage << std::endl;
        if (state == 22) {
            return 2;
        }
        if (state == 26) {
            return 1;
        }
        if (state == 7) {
            return 3;
        }
        if (state == 4) {
            return 4;
        }
        return 0;
    }
};

class Enemy : public GameObject {

public:

    int heals = 2;
    int type = 0;
    bool is_alive = true;
    bool remove = false;

    void set_remove() {
        remove = true;
    }

    bool is_remove() const {
        return remove;
    }
    int getType() override {
        return 0;
    }

    bool is_dead() {
        return !is_alive;
    }
    void get_collision(std::vector<Effect> effects) {
        for (int i = 0; i < effects.size(); i++) {
            if (effects[i].id == 1) {
                heals -= effects[i].value;
            }

        }
        if (heals < 1) {
            is_alive = false;
        }
        std::cout << "Enemy Heals" << heals << std::endl;
    };

    std::vector<Effect> send_collision(bool isFirst) const {
        std::vector <Effect> effects;
        if (heals < 1 && !isFirst) {

            Effect ef;
            ef.id = 2;
            ef.value = 2;
            effects.push_back(ef);
        }
        if (isFirst){
            Effect ef;
            ef.id = 1;
            ef.value = 1;
            effects.push_back(ef);
        }
        return effects;
    };
    std::vector<Effect> send_collision(bool isFirst,
    const std::vector<std::vector<GameMapPoint>>& map,
    int currX,
    int currY) const {

        int count = 0;
        for (int dx = -10; dx <= 10; dx++) {
            for (int dy = -10; dy <= 10; dy++) {
                if (dx == 0 && dy == 0) continue;
                int nx = currX + dx;
                int ny = currY + dy;

                if (nx >= 0 && nx < map.size() &&
                    ny >= 0 && ny < map[0].size()) {
                    std::cout << map[nx][ny].objId;
                    if (map[nx][ny].objId > 2) {
                        count++;
                    }
                    }
            }
        }
        std::vector <Effect> effects;
        if (heals < 1 && !isFirst) {

            Effect ef;
            ef.id = 2;
            ef.value = 2;
            effects.push_back(ef);
        }
        if (isFirst){
            Effect ef;
            ef.id = 1;
            ef.value = count+1;
            effects.push_back(ef);
        }
        return effects;

    }

    int skin() override {
        return 'E';
    }
    int move()override {
        return 0;
    };
    int move(std::vector<std::vector<GameMapPoint>> map, int playerX, int playerY, int selfId, int& newX, int& newY, int currX, int currY) {
        int dx = 0, dy = 0;
        if (playerX < currX) dx = -1;
        else if (playerX > currX) dx = 1;
        if (playerY < currY) dy = -1;
        else if (playerY > currY) dy = 1;

        int nx = currX + dx;
        int ny = currY + dy;

        if (nx >= 0 && nx < map.size() && ny >= 0 && ny < map[0].size()) {
            if (map[nx][ny].objId == -1 || map[nx][ny].objId == 1) {
                newX = nx;
                newY = ny;
                return 1;
            }
        }
        return 0;
    }
};



struct GameWorld {
    std::vector<std::vector<GameMapPoint>> map;
    Player player = Player(0,0);
    std::vector<Enemy> enemies;
};


class LevelBuilder {
private:
    GameWorld world;
    std::vector<std::vector<GameMapPoint>> map;
    int width = 0;
    int height = 0;
    std::mt19937 rng;
    int playerX = 0, playerY = 0;

public:
    LevelBuilder(int width, int height) : width(width), height(height) {
        map = std::vector<std::vector<GameMapPoint>>(width, std::vector<GameMapPoint>(height));
        std::random_device rd;
        rng = std::mt19937(rd());
    }
    bool loadFromFile(const std::string& filename)
    {
        std::ifstream fin(filename);
        if (!fin) {
            std::cerr << "Файла не существует: " << filename << std::endl;
            return false;
        }

        std::vector<std::string> lines;
        std::string line;
        while (std::getline(fin, line)) {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            lines.push_back(line);
        }
        if (lines.empty()) return false;

        height = static_cast<int>(lines.size());
        width  = static_cast<int>(lines[0].size());
        map.assign(width, std::vector<GameMapPoint>(height, GameMapPoint{}));
        world.enemies.clear();                                                  // сбрасываем врагов

        int nextEnemyId = 2;   // 0 – стены, 1 – игрок, 2+ – враги
        for (int y = 0; y < height; ++y) {
            if (static_cast<int>(lines[y].size()) != width) {
                std::cerr << "Map is not rectangular!\n";
                return false;
            }
            for (int x = 0; x < width; ++x) {
                char c = lines[y][x];
                switch (c) {
                    case '#':
                        map[x][y].objId = 0;
                    break;
                    case '@':
                        map[x][y].objId = 1;
                    playerX = x;
                    playerY = y;
                    world.player = Player(playerX, playerY);
                    break;
                    case 'E': {
                        map[x][y].objId = nextEnemyId++;
                        world.enemies.emplace_back();
                        break;
                    }
                    default:
                        map[x][y].objId = -1;   // пусто
                }
            }
        }
        return true;
    }
    void buildBorders() {
        for (int i = 0; i < height; i++) {
            map[0][i].objId = 0;
            map[width - 1][i].objId = 0;
        }
        for (int i = 0; i < width; i++) {
            map[i][0].objId = 0;
            map[i][height - 1].objId = 0;
        }
    }

    void buildPlayer() {
        playerX = width / 2;
        playerY = height / 2;
        world.player = Player(playerX, playerY);
        map[playerX][playerY].objId = 1;
    }

    void buildEnemies(int count = 10) {
        std::uniform_int_distribution<int> distX(1, width - 2);
        std::uniform_int_distribution<int> distY(1, height - 2);

        int enemyId = 2;
        while (count > 0) {
            int x = distX(rng);
            int y = distY(rng);

            if (map[x][y].objId == -1 && (abs(x - playerX) + abs(y - playerY)) > 2) {
                map[x][y].objId = enemyId++;
                world.enemies.push_back(Enemy());
                count--;
            }
        }
    }

    void buildWalls(int chancePercent = 15) {
        std::uniform_int_distribution<int> chance(0, 100);
        for (int i = 1; i < width - 1; i++) {
            for (int j = 1; j < height - 1; j++) {
                if (map[i][j].objId == -1 && chance(rng) < chancePercent) {
                    map[i][j].objId = 0; // wall
                }
            }
        }
    }

    GameWorld getResult() {
        world.map = map;
        return world;
    }
};


class GameCore {
private:
    GameWorld world;
    Wall standartWall;
    int get_skin(int objId) {
        if (objId == 0) {
            return standartWall.skin();
        }
        if (objId == 1) {
            return world.player.skin();
        }
        if (objId >= 2) {
            return world.enemies[objId - 2].skin();
        }
        return '.';
    }
    void draw() {
        terminal_clear();

        int term_width = terminal_state(TK_WIDTH);
        int term_height = terminal_state(TK_HEIGHT);
        int stats_panel_width = 20;

        int cam_x = world.player.x - (term_width - stats_panel_width) / 2;
        int cam_y = world.player.y - term_height / 2;

        for (int i = 0; i < term_width - stats_panel_width; i++) {
            for (int j = 0; j < term_height; j++) {
                int map_x = i + cam_x;
                int map_y = j + cam_y;

                if (map_x >= 0 && map_x < world.map.size() && map_y >= 0 && map_y < world.map[0].size()) {
                    int skin = get_skin(world.map[map_x][map_y].objId);
                    terminal_put(i, j, skin);
                }
            }
        }

        int stats_x = term_width - stats_panel_width + 1;
        terminal_print(stats_x, 1, "[color=orange]STATS");
        terminal_print(stats_x, 3, ("HP: " + std::to_string(world.player.heals)).c_str());
        terminal_print(stats_x, 4, ("DMG: " + std::to_string(world.player.damage)).c_str());

        terminal_refresh();
    }


public:

    GameCore(int wight, int hight, LevelBuilder& builder) {

        builder.buildBorders();
        builder.buildPlayer();
        builder.buildEnemies(50);   // кол-во врагов
        builder.buildWalls(15);     // шанс стены
        builder.loadFromFile("mapExsample.txt");
        /*
         *builder.loadFromFile("mapExsample.txt")
         */
        world = builder.getResult();

        standartWall = Wall();
    }

    void move_player(int button) {
        int state = world.player.move(button);
        int x = world.player.x, y = world.player.y;
        int x_new = world.player.x, y_new = world.player.y;

        if (state == 2){
            y_new = y_new + 1;
        }

        if (state == 1){
            y_new = y_new - 1;
        }

        if (state == 3){
            x_new = x_new + 1;
        }

        if (state == 4){
            x_new = x_new - 1;

        }
        if (world.map[x_new][y_new].objId == -1){
            world.map[x][y].objId = -1;
            world.map[x_new][y_new].objId = 1;
            world.player.x = x_new;
            world.player.y = y_new;
        }else {
            if (world.map[x_new][y_new].objId == 0) {
                return;
            }
            auto collisions = world.player.send_collision(true);
            world.enemies[world.map[x_new][y_new].objId - 2].get_collision(collisions);
            collisions = world.enemies[world.map[x_new][y_new].objId - 2].send_collision(false);
            world.player.get_collision(collisions);
        }

    }

    void ai_work_section() {
        for (int i = 0; i < world.enemies.size(); i++) {
            if (world.enemies[i].is_dead() && !world.enemies[i].is_remove()) {
                for (int j = 0; j < world.map.size(); j++) {
                    for (int k = 0; k < world.map[0].size(); k++) {
                        if (world.map[j][k].objId == i + 2) {
                            world.map[j][k].objId = -1;
                            world.enemies[i].set_remove();
                            break;
                        }
                    }
                }
                continue;
            }

            int ex = -1, ey = -1;
            for (int x = 0; x < world.map.size(); x++) {
                for (int y = 0; y < world.map[0].size(); y++) {
                    if (world.map[x][y].objId == i + 2) {
                        ex = x;
                        ey = y;
                        break;
                    }
                }
                if (ex != -1) break;
            }

            int newX, newY;
            if (world.enemies[i].move(world.map, world.player.x, world.player.y, i + 2, newX, newY, ex, ey)) {
                if (world.map[newX][newY].objId == -1) {
                    world.map[newX][newY].objId = i + 2;
                    world.map[ex][ey].objId = -1;
                } else if (world.map[newX][newY].objId == 1) {
                    // атака на игрока
                    auto col = world.enemies[i].send_collision(true,world.map, newX,newY);
                    world.player.get_collision(col);
                    auto ret = world.player.send_collision(false);
                    world.enemies[i].get_collision(ret);
                }
            }
        }
    }

    void game_loop() {
        while (true) {
            draw();
            int button = terminal_read();
            if (button == 41) {
                terminal_close();
                return;
            }
            move_player(button);
            ai_work_section();
        }

    }

};

int main() {
    terminal_open();
    terminal_print(1, 1, "Приветствуем тебя путник!");
    terminal_refresh();
    terminal_read();
    terminal_clear();
    terminal_print(1, 1, "Это генератор подземелий,\n здесь ты можешь прогуляться и постараться уничтожить всех врагов!!");
    terminal_refresh();
    terminal_read();

    LevelBuilder bilder = LevelBuilder(100, 100);
    GameCore game_core(100,100,bilder);
    game_core.game_loop();
    return 0;
}
