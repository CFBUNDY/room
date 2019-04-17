#include <iostream>
#include <math.h>
#include <vector>
#include <SDL2/SDL.h>
using namespace std;

#define W 640
#define H 480
SDL_Renderer * ren = NULL;

typedef struct _xy {
    float x;
    float y;
    _xy(float x=0, float y=0)
        : x(x), y(y) {}
    _xy operator+(const _xy& a) {
        return _xy(x+a.x, y+a.y);
    }
    _xy operator-(const _xy& a) {
        return _xy(x-a.x, y-a.y);
    }
} xy;

float pointDistance (xy p1, xy p2) {
    return sqrt(pow((p1.x-p2.x),2.0)+pow((p1.y-p2.y),2.0));
}

float dotProduct (xy a, xy b) {
    return a.x*b.x+a.y*b.y;
}

// >0 : clockwise
// =0 : co-linear
// <0 : counter-clockwise
float pointOrientation (xy a, xy b, xy c) {
    return (b.x - a.x)*(c.y - b.y) - (c.x - b.x)*(b.y - a.y);
}

typedef struct _line {
    xy p1;
    xy p2;
    _line(xy p1, xy p2)
        : p1(p1), p2(p2) {}
    void draw() {
        SDL_RenderDrawLine(ren, (W/2)+p1.x, (H/2)-p1.y, (W/2)+p2.x, (H/2)-p2.y);
    }
    float length() {
        return pointDistance(p1, p2);
    }
} line;

typedef struct _camera {
    xy p;
    float dir;
} camera;

xy pointFrom (camera view, xy point) {
    xy proj = point - view.p;
    float tx = proj.x*cos(view.dir) - proj.y*sin(view.dir);
    proj.y = proj.x*sin(view.dir) + proj.y*cos(view.dir);
    proj.x = tx;
    return proj;
} //returns where point appears from view

xy closestPointOnSegment (line l, xy p) {
    float length = l.length();
    float scalar = dotProduct((p-l.p1), (l.p2-l.p1))/length;
    if (scalar < 0) {
        return l.p1;
    } else if (scalar > length) {
        return l.p2;
    } else {
        xy closest = l.p2 - l.p1;
        closest.x *= scalar/length;
        closest.y *= scalar/length;
        closest = closest + l.p1;
    }
}

class Cell {
    vector<xy> points;
    public:
    void addpoint(float x, float y){
        xy n(x, y);
        points.push_back(n);
    }
    int cellSize () {return points.size();}
    line getSegment (int i) {
        line l(points[i%points.size()], points[(i+1)%points.size()]);
        return l;
    }
    void drawfrom(camera c){
        for (int i = 0; i < cellSize(); i++) {
            line l = getSegment(i);
            l.p1 = pointFrom(c, l.p1);
            l.p2 = pointFrom(c, l.p2);
            l.draw();
        }
    }
};

class Player {
    camera pos;
    float movespeed = 0.1, turnspeed = 0.005;
    Cell currentCell;
    public:
    Player(float x, float y, float dir, Cell current) {
        pos.p.x = x;
        pos.p.y = y;
        pos.dir = dir;
        currentCell = current;
    }
    camera getpos() {
        return pos;
    }
    void movement(float frwd, float side, float turn) {
        //change position
        pos.dir += turn*turnspeed;
        pos.p.x += (frwd*sin(pos.dir)+side*cos(pos.dir))*movespeed;
        pos.p.y += (frwd*cos(pos.dir)-side*sin(pos.dir))*movespeed;
        //collision
        for (int i = 0; i < currentCell.cellSize(); i++) {
            line l = currentCell.getSegment(i);
            xy cl = closestPointOnSegment(l, pos.p);
            if (pointOrientation(l.p1, l.p2, pos.p) > 0) {
                SDL_SetRenderDrawColor(ren, 255, 0, 0, SDL_ALPHA_OPAQUE);
            }
            cl = pointFrom(pos, cl);
            SDL_RenderDrawLine(ren, (W/2), (H/2), (W/2)+cl.x, (H/2)-cl.y);
            SDL_SetRenderDrawColor(ren, 255, 255, 255, SDL_ALPHA_OPAQUE);
        }
    }
};

#undef main
//if this becomes a problem, rename 'main' to 'WinMain' for windows version

int main () {
    Cell singlecell;
    singlecell.addpoint(-60, -60);
    singlecell.addpoint(-80, 20);
    singlecell.addpoint(80, 40);
    singlecell.addpoint(20, -80);
    Player you(0, 0, 0, singlecell);
    bool keys[6] = {0,0,0,0,0,0};

    //init
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window * win = SDL_CreateWindow("Hello?", 160, 120, W, H, SDL_WINDOW_SHOWN);
    ren = SDL_CreateRenderer(win, -1, 0);
    bool quit = false;
    while (!quit) {
        SDL_SetRenderDrawColor(ren, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(ren);

        SDL_SetRenderDrawColor(ren, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderDrawLine(ren, (W/2)+4, (H/2)-4, (W/2)-4, (H/2)+4);
        SDL_RenderDrawLine(ren, (W/2)-4, (H/2)-4, (W/2)+4, (H/2)+4);
        singlecell.drawfrom(you.getpos());

        //SDL_RenderPresent(ren);

        //keyboard
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_KEYUP:
                case SDL_KEYDOWN:
                    switch (e.key.keysym.sym) {
                        case SDLK_w: keys[0] = (e.type == SDL_KEYDOWN); break;
                        case SDLK_s: keys[1] = (e.type == SDL_KEYDOWN); break;
                        case SDLK_a: keys[2] = (e.type == SDL_KEYDOWN); break;
                        case SDLK_d: keys[3] = (e.type == SDL_KEYDOWN); break;
                        case SDLK_q: keys[4] = (e.type == SDL_KEYDOWN); break;
                        case SDLK_e: keys[5] = (e.type == SDL_KEYDOWN); break;
                    }
                    quit = (e.key.keysym.sym == SDLK_RETURN);
                    break;
                default: break;
            }
        }
        you.movement(keys[0]-keys[1], keys[3]-keys[2], keys[5]-keys[4]);
        SDL_RenderPresent(ren);
        SDL_Delay(1);
    }
    return 0;
}
